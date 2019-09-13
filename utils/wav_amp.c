#include "wav_file.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const char* in_file = NULL;
static const char* out_file = NULL;
static float amp_gain = 1.f;

static void print_help(const char* program) {
    printf("Usage: %s [options]\n", program);
    printf("  -h, --help            show this help message and exit\n");
    printf("  -i WAV_FILE           wav file\n");
    printf("  -o WAV_FILE           pcm file\n");
    printf("  -a amplifier_gain     amplifier gain between 0 and 1e6\n");
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
            in_file = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0) {
            out_file = argv[++i];
        } else if (strcmp(argv[i], "-a") == 0) {
            amp_gain = strtof(argv[++i], NULL);
        } else {
            printf("unknown option %s\n\n", argv[i]);
            return -1;
        }
    }

    if (in_file == NULL || out_file == NULL) {
        printf("both wav file pcm file are required\n");
        return -1;
    }

    if (access(in_file, R_OK)) {
        printf("read wav file %s failed (%s)\n", in_file, strerror(errno));
        return -1;
    }

    if (amp_gain <= 0 || amp_gain > 1e6f) {
        printf("amplifier gain (%.2f) is invalid\n", amp_gain);
        return -1;
    }

    WavReader* reader = wav_reader_open(in_file);

    if (reader == NULL) {
        printf("open in file %s failed\n", in_file);
        return -1;
    }

    int num_channels = wav_reader_get_num_channels(reader);
    int sample_rate = wav_reader_get_sample_rate(reader);
    SampleFormat format = wav_reader_get_sample_format(reader);

    WavWriter* writer = wav_writer_open(out_file, num_channels, sample_rate, format);

    if (writer == NULL) {
        printf("create out file %s failed (%s)\n", out_file, strerror(errno));
        wav_reader_close(reader);
        return -1;
    }

    int num_samples = sample_rate / 10;
    int block_align = num_channels * sizeof(float);
    float* sample_buf = (float*)malloc(num_samples * block_align);

    while (1) {
        int ret = wav_reader_read_f32(reader, num_samples, sample_buf);

        if (ret > 0) {
            for (int i = 0; i < ret * num_channels; i++) {
                sample_buf[i] = sample_buf[i] * amp_gain;

                if (sample_buf[i] > 1.f) {
                    sample_buf[i] = 1.f;
                }

                if (sample_buf[i] < -1.f) {
                    sample_buf[i] = -1.f;
                }
            }

            wav_writer_write_f32(writer, num_samples, sample_buf);
        }

        if (ret < num_samples) {
            break;
        }
    }

    wav_reader_close(reader);
    wav_writer_close(writer);
    return 0;
}
