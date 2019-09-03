cmake_minimum_required(VERSION 3.1)
project(wav-file
    VERSION 1.2.0
    DESCRIPTION "Wav FILE Read/Write Library and Utility Project"
    LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

add_executable(wav-info
    ${PROJECT_SOURCE_DIR}/utils/wav_info.c
    ${PROJECT_SOURCE_DIR}/wav_file.c
    ${PROJECT_SOURCE_DIR}/wav_file.h)

add_executable(pcm2wav
    ${PROJECT_SOURCE_DIR}/utils/pcm2wav.c
    ${PROJECT_SOURCE_DIR}/wav_file.c
    ${PROJECT_SOURCE_DIR}/wav_file.h)

add_executable(wav2pcm
    ${PROJECT_SOURCE_DIR}/utils/wav2pcm.c
    ${PROJECT_SOURCE_DIR}/wav_file.c
    ${PROJECT_SOURCE_DIR}/wav_file.h)

add_executable(wav-amp
    ${PROJECT_SOURCE_DIR}/utils/wav_amp.c
    ${PROJECT_SOURCE_DIR}/wav_file.c
    ${PROJECT_SOURCE_DIR}/wav_file.h)
