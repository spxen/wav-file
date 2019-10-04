#include "wav_file.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_NUM_CHANNELS    256
#define MAX_SAMPLE_RATE     (48000 * MAX_NUM_CHANNELS)
#define MAX_DATA_SIZE       (1 << 30)
#define DEFAULT_PACKET_SIZE (MAX_NUM_CHANNELS * 1024)

/* wav header */

#define FOUR_CC(a, b, c, d) ((((uint32_t)a) << 0) | \
(((uint32_t)b) << 8) | (((uint32_t)c) << 16) | (((uint32_t)d) << 24))

#define ID_RIFF FOUR_CC('R', 'I', 'F', 'F')
#define ID_WAVE FOUR_CC('W', 'A', 'V', 'E')
#define ID_FMT  FOUR_CC('f', 'm', 't', ' ')
#define ID_DATA FOUR_CC('d', 'a', 't', 'a')

#pragma pack(2)
struct FmtSubchunk {
    uint16_t format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};

struct FmtSubchunk1 {
    struct FmtSubchunk fmt;
    uint16_t cb_size;
};

struct FmtSubchunk2 {
    struct FmtSubchunk1 fmt1;
    union {
        uint16_t valid_bits_per_sample;
        uint16_t samples_per_block;
        uint16_t reserved;
    } u;
    uint32_t channel_mask;
    struct GUID {
        uint32_t data1;
        uint16_t data2;
        uint16_t data3;
        uint8_t data4[8];
    } guid;
};
#pragma pack()

enum WavFormat {
    kWavFormatPcm = 0x0001,
    kWavFormatFloat = 0x0003,
    kWavFormatExtensible = 0xFFFE
};

struct WavHeader {
    enum WavFormat format;
    int num_channels;
    int sample_rate;
    int bytes_per_sample;
    int block_align;
    int valid_bits_per_sample;
    long num_samples;
    int data_offset;
};

static int wav_header_check(struct WavHeader* header) {
    if (header->num_channels < 1 || header->num_channels > MAX_NUM_CHANNELS) {
        return -1;
    }

    if (header->sample_rate <= 0 || header->sample_rate > MAX_SAMPLE_RATE) {
        return -1;
    }

    if (header->bytes_per_sample != 2 && header->bytes_per_sample != 4) {
        return -1;
    }

    if (header->valid_bits_per_sample <= 0
            || header->valid_bits_per_sample > header->bytes_per_sample * 8) {
        return -1;
    }

    switch (header->format) {
    case kWavFormatPcm:
        if (header->bytes_per_sample == 2 && header->valid_bits_per_sample != 16) {
            return -1;
        }

        break;

    case kWavFormatFloat:
        if (header->bytes_per_sample != 4 || header->valid_bits_per_sample != 32) {
            return -1;
        }

        break;

    default:
        return -1;
    }

    if (header->sample_rate * header->num_channels * header->bytes_per_sample > MAX_DATA_SIZE) {
        return -1;
    }

    // riff_id + riff_size + wave_id + fmt_id + fmt_size + data_id + data_size
    int max_data_size = MAX_DATA_SIZE - sizeof(struct FmtSubchunk2) - sizeof(uint32_t) * 7;

    if (header->num_samples * header->block_align > max_data_size) {
        return -1;
    }

    if (header->num_samples % header->num_channels != 0) {
        return -1;
    }

    return 0;
}

static int wav_header_read(struct WavHeader* header, FILE* fp) {
    uint32_t id = 0;
    uint32_t size = 0;

    // RIFF ID
    if (fread(&id, sizeof(id), 1, fp) != 1 || id != ID_RIFF) {
        return -1;
    }

    // RIFF SIZE
    if (fread(&size, sizeof(size), 1, fp) != 1) {
        return -1;
    }

    // WAVE ID
    if (fread(&id, sizeof(id), 1, fp) != 1 || id != ID_WAVE) {
        return -1;
    }

    for (;;) {
        uint32_t chunk_id = 0;

        if (fread(&chunk_id, sizeof(chunk_id), 1, fp) != 1) {
            return -1;
        }

        uint32_t chunk_size = 0;

        if (fread(&chunk_size, sizeof(chunk_size), 1, fp) != 1) {
            return -1;
        }

        if (chunk_id == ID_FMT) {
            if (chunk_size != sizeof(struct FmtSubchunk)
                    && chunk_size != sizeof(struct FmtSubchunk1)
                    && chunk_size != sizeof(struct FmtSubchunk2)) {
                return -1;
            }

            struct FmtSubchunk2 fmt2;

            if (fread(&fmt2, chunk_size, 1, fp) != 1) {
                return -1;
            }

            header->num_channels = (int)fmt2.fmt1.fmt.num_channels;
            header->sample_rate = (int)fmt2.fmt1.fmt.sample_rate;
            header->bytes_per_sample = (int)fmt2.fmt1.fmt.bits_per_sample / 8;
            header->block_align = (int)fmt2.fmt1.fmt.block_align;
            header->valid_bits_per_sample = (int)fmt2.fmt1.fmt.bits_per_sample;

            if (fmt2.fmt1.fmt.format == kWavFormatPcm || fmt2.fmt1.fmt.format == kWavFormatFloat) {
                header->format = (int)fmt2.fmt1.fmt.format;
            } else if (fmt2.fmt1.fmt.format == kWavFormatExtensible) {
                header->format = (int)fmt2.guid.data1;
                header->valid_bits_per_sample = (int)fmt2.u.valid_bits_per_sample;
            } else {
                return -1;
            }
        } else if (chunk_id == ID_DATA) {
            long data_size = 0;
            header->data_offset = ftell(fp);

            fseek(fp, 0, SEEK_END);
            long left_size = ftell(fp) - header->data_offset;
            fseek(fp, header->data_offset, SEEK_SET);

            if (chunk_size == 0) {
                data_size = left_size;
            } else {
                data_size = chunk_size < left_size ? chunk_size : left_size;
            }

            header->num_samples = data_size / header->bytes_per_sample;

            return 0;
        } else {
            fseek(fp, chunk_size, SEEK_CUR);
            continue;
        }
    }

    return wav_header_check(header);
}


static int wav_header_write(struct WavHeader* header, FILE* fp) {
    if (wav_header_check(header) != 0) {
        return -1;
    }

    uint32_t id = 0;
    uint32_t size = 0;

    id = ID_RIFF;

    if (fwrite(&id, sizeof(id), 1, fp) != 1) {
        return -1;
    }

    if (fwrite(&size, sizeof(size), 1, fp) != 1) {
        return -1;
    }

    id = ID_WAVE;

    if (fwrite(&id, sizeof(id), 1, fp) != 1) {
        return -1;
    }

    id = ID_FMT;

    if (fwrite(&id, sizeof(id), 1, fp) != 1) {
        return -1;
    }

    struct FmtSubchunk fmt;

    fmt.format = header->format;

    fmt.num_channels = (uint16_t)header->num_channels;

    fmt.sample_rate = (uint32_t)header->sample_rate;

    fmt.block_align = (uint16_t)header->block_align;

    fmt.byte_rate = (uint32_t)(header->sample_rate * header->block_align);

    fmt.bits_per_sample = (uint16_t)(header->bytes_per_sample * 8);

    size = sizeof(struct FmtSubchunk);

    if (fwrite(&size, sizeof(size), 1, fp) != 1) {
        return -1;
    }

    if (fwrite(&fmt, size, 1, fp) != 1) {
        return -1;
    }

    id = ID_DATA;

    if (fwrite(&id, sizeof(id), 1, fp) != 1) {
        return -1;
    }

    size = 0;

    if (fwrite(&size, sizeof(size), 1, fp) != 1) {
        return -1;
    }

    header->data_offset = ftell(fp);
    return 0;
}

// write riff size and data size to wav header
static void wav_header_write_end(struct WavHeader* header, FILE* fp) {
    long file_size = ftell(fp);
    uint32_t riff_size = file_size - 2 * sizeof(uint32_t);   // exclude RIFF header size
    fseek(fp, sizeof(uint32_t), SEEK_SET);
    fwrite(&riff_size, sizeof(uint32_t), 1, fp);
    fseek(fp, header->data_offset - sizeof(uint32_t), SEEK_SET);
    uint32_t data_size = header->num_samples * header->block_align;
    fwrite(&data_size, sizeof(uint32_t), 1, fp);
}

/* inline functions */

static inline void convert_f32_to_i16(const float* src, int count, int16_t* dst) {
    for (int i = 0; i < count; i++) {
        *dst++ = (int16_t)(*src++ * (((uint32_t)1 << 15) - 1));
    }
}

static inline void convert_i16_to_f32(const int16_t* src, int count, float* dst) {
    for (int i = 0; i < count; i++) {
        *dst++ = (float) * src++ / ((uint32_t)1 << 15);
    }
}

static inline void convert_f32_to_i32(const float* src, int count, int32_t* dst) {
    for (int i = 0; i < count; i++) {
        *dst++ = (int32_t)(*src++ * (((uint32_t)1 << 31) - 1));
    }
}

static inline void convert_i32_to_f32(const int32_t* src, int count, float* dst) {
    for (int i = 0; i < count; i++) {
        *dst++ = (float) * src++ / ((uint32_t)1 << 31);
    }
}

static inline void convert_i16_to_i32(const int16_t* src, int count, int32_t* dst) {
    for (int i = 0; i < count; i++) {
        *dst++ = (int32_t) * src++ * ((uint32_t)1 << 24);
    }
}

static inline void convert_i32_to_i16(const int32_t* src, int count, int16_t* dst) {
    for (int i = 0; i < count; i++) {
        *dst++ = (int16_t)(*src++ / ((uint32_t)1 << 24));
    }
}

static inline int compare_format(const enum WavFormat wav_format, int bits_per_sample,
                                 const SampleFormat format) {
    if (wav_format == kWavFormatFloat && format == kSampleFormatF32) {
        return 0;
    } else if (wav_format == kWavFormatPcm) {
        if (bits_per_sample == 16 && format == kSampleFormatI16) {
            return 0;
        } else if (bits_per_sample == 32 && format == kSampleFormatI32) {
            return 0;
        }
    }

    return 1;
}

/* wav reader */

struct WavReader {
    struct WavHeader hdr;
    long num_samples_left;
    FILE* fp;
};

WavReader* wav_reader_open(const char* filename) {
    WavReader* reader = (WavReader*)malloc(sizeof(WavReader));

    if (reader == NULL) {
        return NULL;
    }

    reader->fp = fopen(filename, "rb");

    if (reader->fp == NULL) {
        free(reader);
        return NULL;
    }

    int read_success = wav_header_read(&reader->hdr, reader->fp);

    if (read_success != 0) {
        fclose(reader->fp);
        free(reader);
        return NULL;
    }

    reader->num_samples_left = reader->hdr.num_samples;
    return reader;
}

void wav_reader_close(WavReader* reader) {
    if (reader != NULL) {
        fclose(reader->fp);
        free(reader);
    }
}

static int wav_reader_read(WavReader* reader, SampleFormat format,
                           int num_samples, void* samples_buf);

int wav_reader_read_f32(WavReader* reader, int num_samples, float* samples) {
    return wav_reader_read(reader, kSampleFormatF32, num_samples, samples);
}

int wav_reader_read_i16(WavReader* reader, int num_samples, int16_t* samples) {
    return wav_reader_read(reader, kSampleFormatI16, num_samples, samples);
}

static int wav_reader_read(WavReader* reader, SampleFormat format,
                           int num_samples, void* samples_buf) {
    num_samples = num_samples > reader->num_samples_left ? reader->num_samples_left : num_samples;

    if (compare_format(reader->hdr.format, reader->hdr.bytes_per_sample * 8, format) == 0) {
        size_t read = fread(samples_buf, reader->hdr.block_align, num_samples, reader->fp);
        assert(read <= num_samples);
        reader->num_samples_left -= read;
        return read;
    } else {
        char tmp[DEFAULT_PACKET_SIZE];
        int ret = 0;

        while (ret < num_samples && reader->num_samples_left > 0) {
            int request = DEFAULT_PACKET_SIZE / reader->hdr.block_align;

            if (request > num_samples - ret) {
                request = num_samples - ret;
            }

            size_t read_samples = fread(tmp, reader->hdr.block_align, request, reader->fp);
            assert(read_samples <= request);

            if (read_samples == 0) {
                break;
            }

            reader->num_samples_left -= read_samples;

            if (format == kSampleFormatF32) {    // i16/i32 --> f32
                if (reader->hdr.bytes_per_sample == sizeof(int16_t)) {
                    convert_i16_to_f32((int16_t*)tmp,
                                       read_samples * reader->hdr.num_channels,
                                       (float*)samples_buf + ret * reader->hdr.num_channels);
                } else {
                    convert_i32_to_f32((int32_t*)tmp,
                                       read_samples * reader->hdr.num_channels,
                                       (float*)samples_buf + ret * reader->hdr.num_channels);
                }
            } else if (reader->hdr.format == kWavFormatFloat) {  // f32 --> i16/i32
                if (format == kSampleFormatI16) {
                    convert_f32_to_i16((float*)tmp,
                                       read_samples * reader->hdr.num_channels,
                                       (int16_t*)samples_buf + ret * reader->hdr.num_channels);
                } else {
                    convert_f32_to_i32((float*)tmp,
                                       read_samples * reader->hdr.num_channels,
                                       (int32_t*)samples_buf + ret * reader->hdr.num_channels);
                }
            } else {    // i16 <--> i32
                if (format == kSampleFormatI16) {
                    convert_i32_to_i16((int32_t*)tmp,
                                       read_samples * reader->hdr.num_channels,
                                       (int16_t*)samples_buf + ret * reader->hdr.num_channels);
                } else {
                    convert_i16_to_i32((int16_t*)tmp,
                                       read_samples * reader->hdr.num_channels,
                                       (int32_t*)samples_buf + ret * reader->hdr.num_channels);
                }
            }

            ret += read_samples;

            if (read_samples < request) {
                break;
            }
        }

        return ret;
    }
}

int wav_reader_get_num_channels(WavReader* reader) {
    return reader->hdr.num_channels;
}

int wav_reader_get_sample_rate(WavReader* reader) {
    return reader->hdr.sample_rate;
}

int wav_reader_get_bits_per_sample(WavReader* reader) {
    return reader->hdr.bytes_per_sample * 8;
}

int wav_reader_get_block_align(WavReader* reader) {
    return reader->hdr.block_align;
}

int wav_reader_get_valid_bits_per_sample(WavReader* reader) {
    return reader->hdr.valid_bits_per_sample;
}

SampleFormat wav_reader_get_sample_format(WavReader* reader) {
    if (reader->hdr.format == kWavFormatFloat) {
        return kSampleFormatF32;
    } else {
        int bits_per_sample = wav_reader_get_bits_per_sample(reader);
        return bits_per_sample == 16 ? kSampleFormatI16 : kSampleFormatI32;
    }
}

int wav_reader_get_num_samples(WavReader* reader) {
    return reader->hdr.num_samples;
}

/* wav writer */

struct WavWriter {
    struct WavHeader hdr;
    FILE* fp;
};

WavWriter* wav_writer_open(const char* filename, int num_channels, int sample_rate,
                           SampleFormat format) {
    WavWriter* writer = (WavWriter*)malloc(sizeof(WavWriter));

    if (writer == NULL) {
        return NULL;
    }

    writer->fp = fopen(filename, "wb");

    if (writer->fp == NULL) {
        free(writer);
        return NULL;
    }

    int bits_per_sample = format == kSampleFormatI16 ? 16 : 32;
    writer->hdr.format = format == kSampleFormatF32 ? kWavFormatFloat : kWavFormatPcm;
    writer->hdr.num_channels = num_channels;
    writer->hdr.sample_rate = sample_rate;
    writer->hdr.bytes_per_sample = bits_per_sample / 8;
    writer->hdr.block_align = num_channels * bits_per_sample / 8;
    writer->hdr.valid_bits_per_sample = bits_per_sample;
    writer->hdr.num_samples = 0;
    writer->hdr.data_offset = -1;

    int write_success = wav_header_write(&writer->hdr, writer->fp);

    if (write_success != 0) {
        fclose(writer->fp);
        free(writer);
        return NULL;
    }

    return writer;
}

void wav_writer_close(WavWriter* writer) {
    if (writer != NULL) {
        wav_header_write_end(&writer->hdr, writer->fp);
        fclose(writer->fp);
        free(writer);
    }
}

static long wav_writer_write(WavWriter* writer, SampleFormat format,
                             int num_samples, const void* samples_buf);

int wav_writer_write_f32(WavWriter* writer, int num_samples, const float* samples) {
    return wav_writer_write(writer, kSampleFormatF32, num_samples, samples);
}

int wav_writer_write_i16(WavWriter* writer, int num_samples, const int16_t* samples) {
    return wav_writer_write(writer, kSampleFormatI16, num_samples, samples);
}

int wav_writer_write_i32(WavWriter* writer, int num_samples, const int32_t* samples) {
    return wav_writer_write(writer, kSampleFormatI32, num_samples, samples);
}

static long wav_writer_write(WavWriter* writer, SampleFormat format,
                             int num_samples, const void* samples_buf) {
    if (num_samples + writer->hdr.num_samples > MAX_DATA_SIZE / writer->hdr.block_align) {
        num_samples = MAX_DATA_SIZE / writer->hdr.block_align - writer->hdr.num_samples;
    }

    int out_bits_per_sample = writer->hdr.bytes_per_sample * 8;

    if (compare_format(writer->hdr.format, writer->hdr.bytes_per_sample * 8, format) == 0) {
        size_t write = fwrite(samples_buf, writer->hdr.block_align, num_samples, writer->fp);
        assert(write == num_samples);
        writer->hdr.num_samples += write;
        return write;
    } else {
        char tmp[DEFAULT_PACKET_SIZE];
        int ret = 0;

        while (ret < num_samples) {
            int request = DEFAULT_PACKET_SIZE / writer->hdr.block_align;

            if (request > num_samples - ret) {
                request = num_samples - ret;
            }

            if (writer->hdr.format == kWavFormatFloat) {    // i32/i16 -> f32
                if (format == kSampleFormatI32) {   // i32 -> f32
                    convert_i32_to_f32((int32_t*)samples_buf + ret * writer->hdr.num_channels,
                                       request * writer->hdr.num_channels,
                                       (float*)tmp);
                } else {    // i16 -> f32
                    convert_i16_to_f32((int16_t*)samples_buf + ret * writer->hdr.num_channels,
                                       request * writer->hdr.num_channels,
                                       (float*)tmp);
                }
            } else if (out_bits_per_sample == 32) {    // f32/i16 -> i32
                if (format == kSampleFormatF32) {   // f32 -> i32
                    convert_f32_to_i32((float*)samples_buf + ret * writer->hdr.num_channels,
                                       request * writer->hdr.num_channels,
                                       (int32_t*)tmp);
                } else {    // i16 -> i32
                    convert_i16_to_i32((int16_t*)samples_buf + ret * writer->hdr.num_channels,
                                       request * writer->hdr.num_channels,
                                       (int32_t*)tmp);
                }
            } else if (out_bits_per_sample == 16) {    // f32/i32 -> i16
                if (format == kSampleFormatF32) {   // f32 -> i16
                    convert_f32_to_i16((float*)samples_buf + ret * writer->hdr.num_channels,
                                       request * writer->hdr.num_channels,
                                       (int16_t*)tmp);
                } else {    // i32 -> i16
                    convert_i32_to_i16((int32_t*)samples_buf + ret * writer->hdr.num_channels,
                                       request * writer->hdr.num_channels,
                                       (int16_t*)tmp);
                }
            }

            size_t write_samples = fwrite(tmp, writer->hdr.block_align, request, writer->fp);
            assert(write_samples == request);
            writer->hdr.num_samples += write_samples;
            ret += write_samples;
        }

        return ret;
    }
}
