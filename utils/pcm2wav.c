#include "wav_file.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const char* pcm_file = NULL;
static const char* wav_file = NULL;
static int num_channels = 1;
static int sample_rate = 16000;
static SampleFormat sample_format = kSampleFormatF32;

static void print_help(const char* program) {
    printf("Usage: %s [options]\n", program);
    printf("  -h, --help            show this help message and exit\n");
    printf("  -i PCM_FILE           pcm file\n");
    printf("  -o WAV_FILE           wav file\n");
    printf("  -c NUM_CHANNELS       num channels of pcm file [1-256], default %d\n", num_channels);
    printf("  -s SAMPLE_RATE        sample rate of pcm file, default %d\n", sample_rate);
    printf("  -f SAMPLE_FORMAT      sample format [i16|i32|f32], default %s\n",
           sample_format_get_str(sample_format));
}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        print_help(argv[0]);
        return -1;
    }

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            print_help(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-i") == 0) {
            pcm_file = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0) {
            wav_file = argv[++i];
        } else if (strcmp(argv[i], "-c") == 0) {
            num_channels = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-s") == 0) {
            sample_rate = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-f") == 0) {
            const char* format = argv[++i];

            if (format != NULL) {
                if (strcmp(format, "i16") == 0) {
                    sample_format = kSampleFormatI16;
                } else if (strcmp(format, "i32") == 0) {
                    sample_format = kSampleFormatI32;
                } else if (strcmp(format, "f32") == 0) {
                    sample_format = kSampleFormatF32;
                } else {
                    sample_format = kSampleFormatInvalid;
                    printf("invalid sample format %s\n\n", format);
                    return -1;
                }
            }
        } else {
            printf("unknown option %s\n\n", argv[i]);
            return -1;
        }
    }

    if (pcm_file == NULL || wav_file == NULL) {
        printf("both pcm file and wav file are required\n");
        return -1;
    }

    if (access(pcm_file, R_OK)) {
        printf("read pcm file %s failed (%s)\n", pcm_file, strerror(errno));
        return -1;
    }

    if (num_channels < 1 || num_channels > 255) {
        printf("num channels (%d) is invalid\n", num_channels);
        return -1;
    }

    if (sample_rate < 1 || sample_rate > 48000 * 255) {
        printf("sample rate (%d) is invalid\n", sample_rate);
        return -1;
    }

    FILE* fp_pcm = fopen(pcm_file, "rb");

    if (fp_pcm == NULL) {
        printf("read pcm file %s failed (%s)\n", pcm_file, strerror(errno));
        return -1;
    }

    WavWriter* writer = wav_writer_open(wav_file, num_channels, sample_rate, sample_format);

    if (writer == NULL) {
        printf("open wav file %s failed\n", wav_file);
        fclose(fp_pcm);
        return -1;
    }

    int num_samples = sample_rate / 10;
    int block_align = num_channels * sample_format_get_bytes_per_sample(sample_format);
    char* sample_buf = (char*)malloc(num_samples * block_align);

    while (1) {
        size_t ret = fread(sample_buf, block_align, num_samples, fp_pcm);

        if (ret > 0) {
            if (sample_format == kSampleFormatF32) {
                wav_writer_write_f32(writer, (int)ret, (float*)sample_buf);
            } else if (sample_format == kSampleFormatI16) {
                wav_writer_write_i16(writer, (int)ret, (int16_t*)sample_buf);
            } else {
                wav_writer_write_i32(writer, (int)ret, (int32_t*)sample_buf);
            }
        }

        if (ret < num_samples) {
            break;
        }
    }

    fclose(fp_pcm);
    wav_writer_close(writer);
    return 0;
}
