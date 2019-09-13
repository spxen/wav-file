#include "wav_file.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const char* wav_file = NULL;
static const char* pcm_file = NULL;
static SampleFormat sample_format = kSampleFormatInvalid;

static void print_help(const char* program) {
    printf("Usage: %s [options]\n", program);
    printf("  -h, --help            show this help message and exit\n");
    printf("  -i WAV_FILE           wav file\n");
    printf("  -o PCM_FILE           pcm file\n");
    printf("  -f SAMPLE_FORMAT      sample format [i16|f32], default the same as wav file\n");
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
            wav_file = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0) {
            pcm_file = argv[++i];
        } else if (strcmp(argv[i], "-f") == 0) {
            const char* format = argv[++i];

            if (format != NULL) {
                if (strcmp(format, "i16") == 0) {
                    sample_format = kSampleFormatI16;
                } else if (strcmp(format, "f32") == 0) {
                    sample_format = kSampleFormatF32;
                } else {
                    printf("invalid sample format %s\n\n", format);
                    return -1;
                }
            }
        } else {
            printf("unknown option %s\n\n", argv[i]);
            return -1;
        }
    }

    if (wav_file == NULL || pcm_file == NULL) {
        printf("both wav file pcm file are required\n");
        return -1;
    }

    if (access(wav_file, R_OK)) {
        printf("read wav file %s failed (%s)\n", wav_file, strerror(errno));
        return -1;
    }

    WavReader* reader = wav_reader_open(wav_file);

    if (reader == NULL) {
        printf("open wav file %s failed\n", wav_file);
        return -1;
    }

    FILE* fp_pcm = fopen(pcm_file, "wb");

    if (fp_pcm == NULL) {
        printf("create pcm file %s failed (%s)\n", pcm_file, strerror(errno));
        wav_reader_close(reader);
        return -1;
    }

    int num_channels = wav_reader_get_num_channels(reader);
    int sample_rate = wav_reader_get_sample_rate(reader);

    if (sample_format == kSampleFormatInvalid) {
        sample_format = wav_reader_get_sample_format(reader);
    }

    int num_samples = sample_rate / 10;
    int block_align = num_channels * sample_format_get_bytes_per_sample(sample_format);
    char* sample_buf = (char*)malloc(num_samples * block_align);

    while (1) {
        int ret = 0;

        if (sample_format == kSampleFormatI16) {
            ret = wav_reader_read_i16(reader, num_samples, (int16_t*)sample_buf);
        } else {
            ret = wav_reader_read_f32(reader, num_samples, (float*)sample_buf);
        }

        if (ret > 0) {
            fwrite(sample_buf, block_align, ret, fp_pcm);
        }

        if (ret < num_samples) {
            break;
        }
    }

    fclose(fp_pcm);
    wav_reader_close(reader);
    return 0;
}
