cmake_minimum_required(VERSION 3.21)
project(whisper_transcribe)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find whisper.cpp
set(WHISPER_CPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/whisper.cpp")

if(NOT EXISTS "${WHISPER_CPP_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "whisper.cpp not found. Run setup.ps1 first.")
endif()

# Add whisper.cpp as subdirectory
add_subdirectory(${WHISPER_CPP_DIR} whisper_build)

# Create executable
add_executable(audio_capture_transcribe audio_capture_transcribe.cpp)

# Create device list utility
add_executable(list_audio_devices list_audio_devices.cpp)

# Link libraries
target_link_libraries(audio_capture_transcribe
    PRIVATE
        whisper
        ole32
)

target_link_libraries(list_audio_devices
    PRIVATE
        ole32
)

# Include directories
target_include_directories(audio_capture_transcribe
    PRIVATE
        ${WHISPER_CPP_DIR}/include
        ${WHISPER_CPP_DIR}
)

# Windows specific settings
if(WIN32)
    target_compile_definitions(audio_capture_transcribe PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Set output directory
set_target_properties(audio_capture_transcribe list_audio_devices PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

message(STATUS "Audio Capture Transcribe configured")
message(STATUS "  Whisper.cpp: ${WHISPER_CPP_DIR}")
message(STATUS "  Output: ${CMAKE_BINARY_DIR}/bin")
