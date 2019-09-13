cmake_minimum_required(VERSION 3.1)
project(wav-file
    VERSION 2.0.0
    DESCRIPTION "Wav FILE Read/Write Library and Utility Project"
    LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR})
set(SOURCE_FILES ${PROJECT_SOURCE_DIR}/wav_file.c ${PROJECT_SOURCE_DIR}/wav_file.h)

add_executable(wavinfo ${SOURCE_FILES} ${PROJECT_SOURCE_DIR}/utils/wav_info.c)
add_executable(pcm2wav ${SOURCE_FILES} ${PROJECT_SOURCE_DIR}/utils/pcm2wav.c)
add_executable(wav2pcm ${SOURCE_FILES} ${PROJECT_SOURCE_DIR}/utils/wav2pcm.c)
add_executable(wavamp ${SOURCE_FILES} ${PROJECT_SOURCE_DIR}/utils/wav_amp.c)
