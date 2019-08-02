#ifndef WAV_FILE
#define WAV_FILE

#include <stdint.h>

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

// return 1 if true else 0
int wav_reader_is_format_f32(WavReader* reader);

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
                           int bits_per_sample);
void wav_writer_close(WavWriter* writer);

// Returns the number of samples written success
//    if return number less than num_samples, check if reach file end
//    if written format is different with openned file format, format convertion will be auto triggerred
int wav_writer_write_f32(WavWriter* writer, int num_samples, const float* samples);
int wav_writer_write_i16(WavWriter* writer, int num_samples, const int16_t* samples);

#ifdef __cplusplus
}
#endif

#endif // WAV_FILE
