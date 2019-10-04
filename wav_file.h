#ifndef WAV_FILE
#define WAV_FILE

#include <stdint.h>

typedef enum {
    kSampleFormatInvalid = -1,
    kSampleFormatF32 = 0,
    kSampleFormatI16 = 1,
    kSampleFormatI32 = 2
} SampleFormat;

#ifdef __cplusplus
extern "C" {
#endif

static inline const char* sample_format_get_str(SampleFormat format) {
    if (format == kSampleFormatF32) {
        return "f32";
    } else if (format == kSampleFormatI16) {
        return "i16";
    } else if (format == kSampleFormatI32) {
        return "i32";
    }

    return "invalid";
}

static inline int sample_format_get_bytes_per_sample(SampleFormat format) {
    if (format == kSampleFormatF32) {
        return sizeof(float);
    } else if (format == kSampleFormatI16) {
        return sizeof(int16_t);
    } else if (format == kSampleFormatI32) {
        return sizeof(int32_t);
    }

    return -1;
}

#ifdef __cplusplus
}
#endif

typedef struct WavReader WavReader;
typedef struct WavWriter WavWriter;

#ifdef __cplusplus
extern "C" {
#endif

/** WAV Reader
 *    - support float32/int16/int32 wav format
 *    - support max 256 channels
 *    - support max 48000*256 sample rate
 *    - read multi-channel samples in interleaved mode
 */

WavReader* wav_reader_open(const char* filename);
void wav_reader_close(WavReader* reader);

// Returns the number of samples read interleaved
//    if return number less than num_samples, check if reach file end
//    if read format is different with file format, format convertion will be auto triggerred
int wav_reader_read_f32(WavReader* reader, int num_samples, float* samples);
int wav_reader_read_i16(WavReader* reader, int num_samples, int16_t* samples);

int wav_reader_get_num_channels(WavReader* reader);
int wav_reader_get_sample_rate(WavReader* reader);
int wav_reader_get_bits_per_sample(WavReader* reader);

// block_align equals num_channels*bytes_per_sample
int wav_reader_get_block_align(WavReader* reader);

// valid_bits_per_sample equals bits_per_sample in most case, only if
//   file format is int32, valid_bits in most case is 20/24.
int wav_reader_get_valid_bits_per_sample(WavReader* reader);

// return sample format
SampleFormat wav_reader_get_sample_format(WavReader* reader);

// return number of samples in each channel
int wav_reader_get_num_samples(WavReader* reader);


/** WAV Writer
 *    - support float32/int16 wav format
 *    - support max 256 channels
 *    - support max 48000*256 sample rate
 *    - write multi-channel samples in interleaved mode
 */

// bits_per_sample must be 16 (int16) or 32 (float32)
WavWriter* wav_writer_open(const char* filename, int num_channels, int sample_rate,
                           SampleFormat format);
void wav_writer_close(WavWriter* writer);

// Returns the number of samples written success
//    if return number less than num_samples, check if reach file end
//    if written format is different with openned file format, format convertion will be auto triggerred
int wav_writer_write_f32(WavWriter* writer, int num_samples, const float* samples);
int wav_writer_write_i16(WavWriter* writer, int num_samples, const int16_t* samples);
int wav_writer_write_i32(WavWriter* writer, int num_samples, const int32_t* samples);

#ifdef __cplusplus
}
#endif

#endif // WAV_FILE
