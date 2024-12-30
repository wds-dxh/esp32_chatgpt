#pragma once



#include <Arduino.h>
#include <math.h>

/**
 * @brief 通用音频处理类：包含增益、滤波、混响、压缩、FFT 等示例函数
 */
class AudioProcessor {
public:
    // ======================== 基础音频处理 ========================
    static void applyGain(int16_t* samples, size_t sampleCount, float gain);
    static void normalize(int16_t* samples, size_t sampleCount, int16_t maxAmplitude = 32767);
    static float calculateRMS(const int16_t* samples, size_t sampleCount);
    static float calculatePeak(const int16_t* samples, size_t sampleCount);

    // ======================== 格式转换 ========================
    static void convertInt16ToFloat(const int16_t* input, float* output, size_t sampleCount);
    static void convertFloatToInt16(const float* input, int16_t* output, size_t sampleCount);

    // ======================== 声音效果处理 ========================
    static void applyEcho(int16_t* samples, size_t sampleCount, float delay, float decay);
    static void applyReverb(int16_t* samples, size_t sampleCount, 
                            const float* impulseResponse, size_t irLength);
                            
    static void applyCompressor(int16_t* samples, size_t sampleCount, 
                                float threshold, float ratio, float attack, float release);

    // ======================== 滤波器 ========================
    static void applyLowPassFilter(int16_t* samples, size_t sampleCount, float cutoffFreq, float sampleRate);
    static void applyHighPassFilter(int16_t* samples, size_t sampleCount, float cutoffFreq, float sampleRate);
    static void applyBandPassFilter(int16_t* samples, size_t sampleCount, 
                                    float lowCutoff, float highCutoff, float sampleRate);

    // ======================== 频谱分析 ========================
    static void calculateFFT(const int16_t* samples, size_t sampleCount, float* magnitudes, float* phases);
    static void inverseFft(const float* magnitudes, const float* phases, int16_t* output, size_t sampleCount);

    // ======================== 噪声处理 ========================
    static void applyNoiseGate(int16_t* samples, size_t sampleCount, float threshold);
    static void applyNoiseReduction(int16_t* samples, size_t sampleCount, float threshold);

    // ======================== 音高和速度调整 ========================
    static void pitchShift(const int16_t* input, int16_t* output, size_t sampleCount, float pitchFactor);
    static void timeStretch(const int16_t* input, int16_t* output, 
                            size_t inputCount, size_t& outputCount, float speedFactor);

    // ======================== VAD (Voice Activity Detection) ========================
    static bool detectVoiceActivity(const int16_t* samples, size_t sampleCount, float threshold);

    // ======================== 音频分析 ========================
    static float calculateZeroCrossingRate(const int16_t* samples, size_t sampleCount);
    static float calculateSpectralCentroid(const float* magnitudes, size_t count, float sampleRate);
    static void calculateMFCC(const int16_t* samples, size_t sampleCount, float* mfccCoeffs, int numCoeffs);

private:
    // ======================== 私有实用工具函数 ========================
    static float hzToRad(float hz, float sampleRate);
    static void applyWindow(float* buffer, size_t size, int windowType);
    static float calculateEnvelope(const int16_t* samples, size_t sampleCount, int windowSize);
};

