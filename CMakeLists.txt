CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(wav-file
    VERSION 1.2.0
    DESCRIPTION "Wav FILE Read/Write Library and Utility Project"
    LANGUAGES C)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

ADD_EXECUTABLE(wav-info
    ${PROJECT_SOURCE_DIR}/utils/wav_info.c
    ${PROJECT_SOURCE_DIR}/wav_file.c
    ${PROJECT_SOURCE_DIR}/wav_file.h)

ADD_EXECUTABLE(pcm2wav
    ${PROJECT_SOURCE_DIR}/utils/pcm2wav.c
    ${PROJECT_SOURCE_DIR}/wav_file.c
    ${PROJECT_SOURCE_DIR}/wav_file.h)

ADD_EXECUTABLE(wav2pcm
    ${PROJECT_SOURCE_DIR}/utils/wav2pcm.c
    ${PROJECT_SOURCE_DIR}/wav_file.c
    ${PROJECT_SOURCE_DIR}/wav_file.h)

ADD_EXECUTABLE(wav-amp
    ${PROJECT_SOURCE_DIR}/utils/wav_amp.c
    ${PROJECT_SOURCE_DIR}/wav_file.c
    ${PROJECT_SOURCE_DIR}/wav_file.h)
