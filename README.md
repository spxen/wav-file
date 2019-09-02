# Wav File Read/Write API

## WavReader

~~~c
typedef struct WavReader WavReader;

/** WAV Reader
 *    - support float32/int16/int32 wav format
 *    - support max 256 channels
 *    - support max 48000*256 sample rate
 *    - read multi-channel samples in interleaved mode
 */

WavReader* wav_reader_open(const char* filename);
void wav_reader_close(WavReader* reader);

// Returns the number of samples read success
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
//   file format is int32, valid_bits_per_sample maybe 20/24.
int wav_reader_get_valid_bits_per_sample(WavReader* reader);

// return 1 if true else 0
int wav_reader_is_format_f32(WavReader* reader);

// return number of samples in each channel
long wav_reader_get_num_samples(WavReader* reader);
~~~


## WavWriter

~~~c
typedef struct WavWriter WavWriter;

/** WAV Writer
 *    - support float32/int16 wav format
 *    - support max 256 channels
 *    - support max 48000*256 sample rate
 *    - write multi-channel samples in interleaved mode
 */

// bytes_per_sample must be 16 (int16) or 32 (float32)
WavWriter* wav_writer_open(const char* filename, int num_channels, int sample_rate,
                           int bits_per_sample);
void wav_writer_close(WavWriter* writer);

// Returns the number of samples written success
//    if return number less than num_samples, check if reach file end
//    if written format is different with openned file format, format convertion will be auto triggerred
int wav_writer_write_f32(WavWriter* writer, int num_samples, const float* samples);
int wav_writer_write_i16(WavWriter* writer, int num_samples, const int16_t* samples);
~~~

## Utilities

- wav_info - display wav file information

~~~
Usage:
./wav-info wav_file

Example:
./wav-info sample.wav

Wav Information of sample.wav
           Audio Format:  int32
        Number Channels:  1
            Sample Rate:  16000
        Bits Per Sample:  32
  Valid Bits Per Sample:  20
         Number Samples:  220960
~~~

- pcm2wav - convert pcm file to wav file

~~~
Usage: ./bin/pcm2wav [options]
  -h, --help            show this help message and exit
  -i PCM_FILE           pcm file
  -o WAV_FILE           wav file
  -c NUM_CHANNELS       num channels of pcm file, default 1
  -s SAMPLE_RATE        sample rate of pcm file, default 16000
  -b BITS_PER_SAMPLE    bits per sample of pcm file, default 16
~~~

- wav2pcm - convert wav file to pcm file

~~~
Usage: ./bin/wav2pcm [options]
  -h, --help            show this help message and exit
  -i WAV_FILE           wav file
  -o PCM_FILE           pcm file
  -b BITS_PER_SAMPLE    bits per sample of pcm file, default the same as wav file
~~~

- wav-apm - wav file amplifier

~~~
Usage: wav-amp [options]
  -h, --help            show this help message and exit
  -i WAV_FILE           wav file
  -o WAV_FILE           pcm file
  -a amplifier_gain     amplifier gain between 0 and 1e6
  -b BITS_PER_SAMPLE    bits per sample of pcm file, default the same as wav file
~~~
