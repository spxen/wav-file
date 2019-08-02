#include "wav_file.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const char* get_str_format(int is_f32, int bits_per_sample) {
    if (is_f32) {
        return "f32";
    } else if (bits_per_sample == 16) {
        return "i16";
    } else {
        return "i32";
    }
}

int main(int argc, const char* argv[]) {
    const char* in_file = NULL;
    const char* out_file = NULL;
    int out_bytes_per_sample = -1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            in_file = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0) {
            out_file = argv[++i];
        } else if (strcmp(argv[i], "--obytes") == 0) {
            out_bytes_per_sample = atoi(argv[++i]);
        } else {
            printf("invalid argument: %s\n", argv[i]);
        }
    }

    if (in_file == NULL) {
        printf("in file is required\n");
        return -1;
    }

    WavReader* reader = wav_reader_open(in_file);

    if (reader == NULL) {
        printf("open wav file %s failed\n", in_file);
        return -1;
    }

    printf("input file info:\n");
    printf("  sample rate is %d\n", wav_reader_get_sample_rate(reader));
    printf("  num channels is %d\n", wav_reader_get_num_channels(reader));
    printf("  bits per sample is %d\n", wav_reader_get_bits_per_sample(reader));
    printf("  valid bytes per sample is %d\n", wav_reader_get_valid_bits_per_sample(reader));
    printf("  format is %s\n", get_str_format(wav_reader_is_format_f32(reader),
            wav_reader_get_bits_per_sample(reader)));
    printf("  num samples is %d\n", wav_reader_get_num_samples(reader));

    if (out_bytes_per_sample < 0) {
        out_bytes_per_sample = wav_reader_get_bits_per_sample(reader);
    }

    if (out_file != NULL) {
        const int sample_rate = wav_reader_get_sample_rate(reader);
        const int num_channels = wav_reader_get_num_channels(reader);
        WavWriter* writer = wav_writer_open(out_file, num_channels, sample_rate,
                                            out_bytes_per_sample);

        if (writer == NULL) {
            printf("open out file %s failed\n", out_file);
            return -1;
        }

        if (out_bytes_per_sample == 2) {
            int16_t samples[1024];
            int num_samples = 0;

            while ((num_samples = wav_reader_read_i16(reader, 1024 / num_channels, samples)) > 0) {
                int write = wav_writer_write_i16(writer, num_samples, samples);
                assert(write == num_samples);
            }
        } else {
            float samples[1024];
            int num_samples = 0;

            while ((num_samples = wav_reader_read_f32(reader, 1024 / num_channels, samples)) > 0) {
                int write = wav_writer_write_f32(writer, num_samples, samples);
                assert(write == num_samples);
            }
        }

        wav_writer_close(writer);
    }

    wav_reader_close(reader);
    return 0;
}
