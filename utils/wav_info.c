#include "wav_file.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void print_help(const char* program) {
    printf("Usage: %s wav_file\n", program);
}

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        print_help(argv[0]);
        return -1;
    }

    if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
        print_help(argv[0]);
        return 0;
    }

    const char* wav_file = argv[1];

    if (access(wav_file, R_OK)) {
        printf("read wav file %s failed (%s)\n", wav_file, strerror(errno));
        return -1;
    }

    WavReader* reader = wav_reader_open(wav_file);

    if (reader == NULL) {
        printf("open wav file %s failed\n", wav_file);
        return -1;
    }

    SampleFormat format = wav_reader_get_sample_format(reader);
    int sample_bits = wav_reader_get_bits_per_sample(reader);

    printf("Wav Information of %s\n", wav_file);
    printf("         Audio Format: %s\n",
           format == kSampleFormatF32 ? "float32" : (format == kSampleFormatI16 ? "int16" : "int32"));
    printf("      Number Channels: %d\n", wav_reader_get_num_channels(reader));
    printf("          Sample Rate: %d\n", wav_reader_get_sample_rate(reader));
    printf("      Bits Per Sample: %d\n", sample_bits);
    printf("Valid Bits Per Sample: %d\n", wav_reader_get_valid_bits_per_sample(reader));
    printf("       Number Samples: %d\n", wav_reader_get_num_samples(reader));

    wav_reader_close(reader);
    return 0;
}
