/*
 * Real-time Audio Transcription using Windows WASAPI and whisper.cpp
 * Captures audio from USB Audio Device (nRF52840 dongle) and transcribes speech
 */

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>

#include "whisper.h"

#pragma comment(lib, "ole32.lib")

// RIFF/WAV header definitions
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

// Wave format constants
#ifndef WAVE_FORMAT_IEEE_FLOAT
#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#endif

// Audio configuration
#define WHISPER_SAMPLE_RATE 16000  // Whisper expects 16kHz
#define AUDIO_CHANNELS 1           // Mono
#define BITS_PER_SAMPLE 16

// Whisper context
struct whisper_context* ctx = nullptr;
std::atomic<bool> running(true);

// Audio buffer for whisper (3 seconds at 16kHz)
#define BUFFER_SIZE (WHISPER_SAMPLE_RATE * 3)
std::vector<float> audio_buffer;
size_t buffer_pos = 0;

// Capture sample rate (will be detected from device)
int capture_sample_rate = 48000;

// Convert int16 PCM to float [-1.0, 1.0]
void pcm16_to_float(const int16_t* pcm, float* out, size_t samples) {
    for (size_t i = 0; i < samples; i++) {
        out[i] = (float)pcm[i] / 32768.0f;
    }
}

// Convert float32 to float (might already be in correct range)
void pcm32f_to_float(const float* pcm, float* out, size_t samples, int channels) {
    // Windows WASAPI uses 32-bit float in range [-1.0, 1.0]
    if (channels == 1) {
        // Mono - just copy
        std::copy(pcm, pcm + samples, out);
    } else if (channels == 2) {
        // Stereo - mix to mono by averaging left and right
        for (size_t i = 0; i < samples; i++) {
            out[i] = (pcm[i * 2] + pcm[i * 2 + 1]) / 2.0f;
        }
    } else {
        // Multi-channel - just use first channel
        for (size_t i = 0; i < samples; i++) {
            out[i] = pcm[i * channels];
        }
    }
}

// Simple linear resampling from source_rate to 16kHz
void resample_audio(const float* input, size_t input_samples, int source_rate,
                   std::vector<float>& output) {
    if (source_rate == WHISPER_SAMPLE_RATE) {
        // No resampling needed
        output.insert(output.end(), input, input + input_samples);
        return;
    }

    // Calculate output size
    float ratio = (float)WHISPER_SAMPLE_RATE / source_rate;
    size_t output_samples = (size_t)(input_samples * ratio);
    
    size_t old_size = output.size();
    output.resize(old_size + output_samples);
    
    // Linear interpolation resampling
    for (size_t i = 0; i < output_samples; i++) {
        float src_pos = i / ratio;
        size_t src_idx = (size_t)src_pos;
        float frac = src_pos - src_idx;
        
        if (src_idx + 1 < input_samples) {
            output[old_size + i] = input[src_idx] * (1.0f - frac) + input[src_idx + 1] * frac;
        } else if (src_idx < input_samples) {
            output[old_size + i] = input[src_idx];
        }
    }
}

// Calculate RMS (volume level) for debugging
float calculate_rms(const float* data, size_t samples) {
    float sum = 0.0f;
    for (size_t i = 0; i < samples; i++) {
        sum += data[i] * data[i];
    }
    return sqrtf(sum / samples);
}

// Transcribe accumulated audio
void transcribe_audio() {
    if (buffer_pos < WHISPER_SAMPLE_RATE) {
        // Need at least 1 second of audio
        return;
    }

    // Calculate RMS for debugging
    float rms = calculate_rms(audio_buffer.data(), buffer_pos);
    printf("\n========================================\n");
    printf("[TRANSCRIBE] Processing %zu samples (%.2f seconds)\n", 
           buffer_pos, (float)buffer_pos / WHISPER_SAMPLE_RATE);
    printf("[TRANSCRIBE] RMS Level: %.4f\n", rms);
    printf("========================================\n");

    whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.print_realtime = true;    // Show real-time output
    params.print_progress = true;    // Show progress  
    params.print_timestamps = true;
    params.print_special = false;
    params.translate = false;
    params.language = "en";
    params.n_threads = 4;
    params.no_context = false;
    params.single_segment = false;

    printf("[TRANSCRIBE] Starting whisper inference...\n\n");

    // Process audio
    int result = whisper_full(ctx, params, audio_buffer.data(), buffer_pos);
    
    printf("\n");
    
    if (result == 0) {
        const int n_segments = whisper_full_n_segments(ctx);
        printf("[TRANSCRIBE] Whisper found %d segment(s)\n", n_segments);
        
        if (n_segments > 0) {
            printf("\n=== TRANSCRIPTION RESULT ===\n");
            for (int i = 0; i < n_segments; i++) {
                const char* text = whisper_full_get_segment_text(ctx, i);
                const int64_t t0 = whisper_full_get_segment_t0(ctx, i);
                const int64_t t1 = whisper_full_get_segment_t1(ctx, i);
                
                // Print with timestamp
                printf("[%02d:%02d.%03d --> %02d:%02d.%03d]  %s\n",
                       (int)(t0 / 100 / 60),
                       (int)(t0 / 100 % 60),
                       (int)(t0 % 100 * 10),
                       (int)(t1 / 100 / 60),
                       (int)(t1 / 100 % 60),
                       (int)(t1 % 100 * 10),
                       text);
            }
            printf("============================\n\n");
        } else {
            printf("[TRANSCRIBE] No speech detected\n\n");
        }
    } else {
        printf("[TRANSCRIBE] ERROR: Whisper failed with code %d\n\n", result);
    }

    // Keep last 1 second for context
    if (buffer_pos > WHISPER_SAMPLE_RATE) {
        std::copy(audio_buffer.begin() + buffer_pos - WHISPER_SAMPLE_RATE,
                  audio_buffer.begin() + buffer_pos,
                  audio_buffer.begin());
        buffer_pos = WHISPER_SAMPLE_RATE;
    } else {
        buffer_pos = 0;
    }
}

// Audio capture thread
HRESULT capture_audio() {
    HRESULT hr;
    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioClient* pAudioClient = nullptr;
    IAudioCaptureClient* pCaptureClient = nullptr;
    WAVEFORMATEX* pwfx = nullptr;
    UINT32 bufferFrameCount;
    REFERENCE_TIME hnsRequestedDuration = 10000000;  // 1 second

    // Initialize COM
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM" << std::endl;
        return hr;
    }

    // Create device enumerator
    hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
                          IID_IMMDeviceEnumerator, (void**)&pEnumerator);
    if (FAILED(hr)) {
        std::cerr << "Failed to create device enumerator" << std::endl;
        goto Exit;
    }

    // Get default capture device
    hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
    if (FAILED(hr)) {
        std::cerr << "Failed to get default audio endpoint" << std::endl;
        goto Exit;
    }

    // Activate audio client
    hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);
    if (FAILED(hr)) {
        std::cerr << "Failed to activate audio client" << std::endl;
        goto Exit;
    }

    // Get audio format
    hr = pAudioClient->GetMixFormat(&pwfx);
    if (FAILED(hr)) {
        std::cerr << "Failed to get mix format" << std::endl;
        goto Exit;
    }

    std::cout << "Audio format: " << pwfx->nSamplesPerSec << " Hz, "
              << pwfx->nChannels << " channels, "
              << pwfx->wBitsPerSample << " bits" << std::endl;

    // Store capture sample rate for resampling
    capture_sample_rate = pwfx->nSamplesPerSec;
    std::cout << "Will resample from " << capture_sample_rate << " Hz to " 
              << WHISPER_SAMPLE_RATE << " Hz" << std::endl;

    // Initialize audio client
    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                   0,
                                   hnsRequestedDuration,
                                   0,
                                   pwfx,
                                   NULL);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize audio client" << std::endl;
        goto Exit;
    }

    // Get buffer size
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    if (FAILED(hr)) {
        std::cerr << "Failed to get buffer size" << std::endl;
        goto Exit;
    }

    // Get capture client
    hr = pAudioClient->GetService(IID_IAudioCaptureClient, (void**)&pCaptureClient);
    if (FAILED(hr)) {
        std::cerr << "Failed to get capture client" << std::endl;
        goto Exit;
    }

    // Start audio capture
    hr = pAudioClient->Start();
    if (FAILED(hr)) {
        std::cerr << "Failed to start audio capture" << std::endl;
        goto Exit;
    }

    std::cout << "Audio capture started. Speak into the microphone..." << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;
    std::cout << "------------------------------------------------------------" << std::endl;

    // Capture loop
    while (running) {
        Sleep(100);  // Sleep for 100ms between reads

        UINT32 packetLength = 0;
        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        if (FAILED(hr)) {
            break;
        }

        while (packetLength != 0) {
            BYTE* pData;
            UINT32 numFramesAvailable;
            DWORD flags;

            hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
            if (FAILED(hr)) {
                break;
            }

            if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                // Silent packet
                pData = NULL;
            }

            // Convert to float and add to buffer
            if (pData != NULL && buffer_pos + numFramesAvailable < BUFFER_SIZE) {
                std::vector<float> temp_float(numFramesAvailable);
                
                if (pwfx->wBitsPerSample == 16) {
                    // 16-bit PCM
                    pcm16_to_float((int16_t*)pData, temp_float.data(), numFramesAvailable);
                } else if (pwfx->wBitsPerSample == 32 && pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
                    // 32-bit float (WASAPI default)
                    pcm32f_to_float((float*)pData, temp_float.data(), numFramesAvailable, pwfx->nChannels);
                } else if (pwfx->wBitsPerSample == 32) {
                    // 32-bit PCM integer
                    const int32_t* pcm32 = (int32_t*)pData;
                    for (size_t i = 0; i < numFramesAvailable; i++) {
                        temp_float[i] = (float)pcm32[i] / 2147483648.0f;
                    }
                } else {
                    // Unsupported format - skip this packet
                    hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
                    if (FAILED(hr)) {
                        break;
                    }
                    continue;
                }

                // Calculate RMS for this packet
                float packet_rms = calculate_rms(temp_float.data(), numFramesAvailable);
                
                // Resample to 16kHz for whisper
                std::vector<float> resampled;
                resample_audio(temp_float.data(), numFramesAvailable, capture_sample_rate, resampled);
                
                // Only show packets with significant audio (reduce spam)
                if (packet_rms > 0.2f) {
                    printf("[AUDIO] Captured %d frames (%.1fms) -> %zu resampled, RMS: %.4f\n", 
                           numFramesAvailable, 
                           (float)numFramesAvailable * 1000 / capture_sample_rate,
                           resampled.size(),
                           packet_rms);
                }

                // Copy resampled audio to main buffer
                if (buffer_pos + resampled.size() < BUFFER_SIZE) {
                    std::copy(resampled.begin(), resampled.end(), 
                             audio_buffer.begin() + buffer_pos);
                    buffer_pos += resampled.size();
                    
                    // Show buffer status every 1 second of data
                    static size_t last_report = 0;
                    if (buffer_pos / WHISPER_SAMPLE_RATE > last_report) {
                        last_report = buffer_pos / WHISPER_SAMPLE_RATE;
                        printf("[BUFFER] %zu / %d samples (%.1f / 3.0 seconds)\n",
                               buffer_pos, BUFFER_SIZE, (float)buffer_pos / WHISPER_SAMPLE_RATE);
                    }
                } else {
                    printf("[BUFFER] Buffer full (%zu samples), triggering transcription\n", buffer_pos);
                }

                // Transcribe every 3 seconds
                if (buffer_pos >= BUFFER_SIZE) {
                    transcribe_audio();
                }
            }

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
            if (FAILED(hr)) {
                break;
            }

            hr = pCaptureClient->GetNextPacketSize(&packetLength);
            if (FAILED(hr)) {
                break;
            }
        }
    }

    // Stop capture
    pAudioClient->Stop();

Exit:
    CoTaskMemFree(pwfx);
    if (pCaptureClient) pCaptureClient->Release();
    if (pAudioClient) pAudioClient->Release();
    if (pDevice) pDevice->Release();
    if (pEnumerator) pEnumerator->Release();
    CoUninitialize();

    return hr;
}

// Ctrl+C handler
BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        std::cout << "\nStopping transcription..." << std::endl;
        running = false;
        return TRUE;
    }
    return FALSE;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <model_path>" << std::endl;
        std::cerr << "Example: " << argv[0] << " whisper.cpp/models/ggml-base.bin" << std::endl;
        return 1;
    }

    const char* model_path = argv[1];

    std::cout << "========================================" << std::endl;
    std::cout << "Whisper Real-Time Transcription" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Load whisper model
    std::cout << "Loading model: " << model_path << std::endl;
    
    struct whisper_context_params cparams = whisper_context_default_params();
    cparams.use_gpu = true;
    
    ctx = whisper_init_from_file_with_params(model_path, cparams);
    if (ctx == nullptr) {
        std::cerr << "Failed to load model" << std::endl;
        return 1;
    }

    std::cout << "Model loaded successfully" << std::endl;
    std::cout << std::endl;

    // Initialize audio buffer
    audio_buffer.resize(BUFFER_SIZE);
    buffer_pos = 0;

    // Set Ctrl+C handler
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        std::cerr << "Failed to set control handler" << std::endl;
        return 1;
    }

    // Start audio capture
    HRESULT hr = capture_audio();

    // Cleanup
    whisper_free(ctx);

    if (FAILED(hr)) {
        std::cerr << "Audio capture failed with error: 0x" << std::hex << hr << std::endl;
        return 1;
    }

    std::cout << "Transcription stopped." << std::endl;
    return 0;
}
