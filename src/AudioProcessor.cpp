#include "AudioProcessor/AudioProcessor.hpp"
#include <vector>
#include <string.h> // for memset, memcpy

// ======================== 基础音频处理 ========================
void AudioProcessor::applyGain(int16_t* samples, size_t sampleCount, float gain) {
    if (!samples || sampleCount == 0) return;
    for (size_t i = 0; i < sampleCount; i++) {
        float temp = samples[i] * gain;
        if (temp > 32767.0f)  temp = 32767.0f;
        if (temp < -32768.0f) temp = -32768.0f;
        samples[i] = static_cast<int16_t>(temp);
    }
}

void AudioProcessor::normalize(int16_t* samples, size_t sampleCount, int16_t maxAmplitude) {
    if (!samples || sampleCount == 0) return;
    int16_t maxVal = 0;
    for (size_t i = 0; i < sampleCount; i++) {
        int16_t absVal = abs(samples[i]);
        if (absVal > maxVal) maxVal = absVal;
    }
    if (maxVal == 0) return; // 全是0，不需要归一化

    float scale = static_cast<float>(maxAmplitude) / maxVal;
    applyGain(samples, sampleCount, scale);
}

float AudioProcessor::calculateRMS(const int16_t* samples, size_t sampleCount) {
    if (!samples || sampleCount == 0) return 0.0f;
    double sum = 0.0;
    for (size_t i = 0; i < sampleCount; i++) {
        float val = static_cast<float>(samples[i]);
        sum += val * val;
    }
    return sqrt(sum / sampleCount);
}

float AudioProcessor::calculatePeak(const int16_t* samples, size_t sampleCount) {
    if (!samples || sampleCount == 0) return 0.0f;
    float peak = 0.0f;
    for (size_t i = 0; i < sampleCount; i++) {
        float absVal = fabs(static_cast<float>(samples[i]));
        if (absVal > peak) peak = absVal;
    }
    return peak;
}

// ======================== 格式转换 ========================
void AudioProcessor::convertInt16ToFloat(const int16_t* input, float* output, size_t sampleCount) {
    if (!input || !output || sampleCount == 0) return;
    for (size_t i = 0; i < sampleCount; i++) {
        output[i] = static_cast<float>(input[i]) / 32768.0f; // -1.0 ~ +1.0
    }
}

void AudioProcessor::convertFloatToInt16(const float* input, int16_t* output, size_t sampleCount) {
    if (!input || !output || sampleCount == 0) return;
    for (size_t i = 0; i < sampleCount; i++) {
        float temp = input[i] * 32767.0f;
        if (temp > 32767.0f)  temp = 32767.0f;
        if (temp < -32768.0f) temp = -32768.0f;
        output[i] = static_cast<int16_t>(temp);
    }
}

// ======================== 声音效果处理 ========================
void AudioProcessor::applyEcho(int16_t* samples, size_t sampleCount, float delay, float decay) {    // delay: 延迟时间(秒), decay: 衰减系数(0~1)
    if (!samples || sampleCount == 0) return;

    // 假设采样率 44.1kHz，delay秒 -> delayInSamples
    int delayInSamples = static_cast<int>(delay * 44100.0f);
    if (delayInSamples <= 0) return;

    // 建立一个环形缓冲用来实现延迟
    std::vector<int16_t> buffer(delayInSamples, 0);
    std::vector<int16_t> tempOutput(sampleCount);

    for (size_t i = 0; i < sampleCount; i++) {
        tempOutput[i] = samples[i];
        if (i >= (size_t)delayInSamples) {
            float echoed = buffer[i % delayInSamples] * decay;
            float mixed  = tempOutput[i] + echoed;
            // 防止溢出
            if (mixed > 32767.0f)   mixed = 32767.0f;
            if (mixed < -32768.0f) mixed = -32768.0f;
            tempOutput[i] = static_cast<int16_t>(mixed);
        }
        // 更新环形缓冲
        buffer[i % delayInSamples] = samples[i];
    }
    memcpy(samples, tempOutput.data(), sampleCount * sizeof(int16_t));
}

void AudioProcessor::applyReverb(int16_t* samples, size_t sampleCount, 
                                 const float* impulseResponse, size_t irLength)
{
    if (!samples || sampleCount == 0 || !impulseResponse || irLength == 0) return;

    // 卷积结果长度 = sampleCount + irLength - 1
    size_t convLength = sampleCount + irLength - 1;
    std::vector<float> convResult(convLength, 0.0f);

    // 1. 转成 float
    std::vector<float> floatInput(sampleCount);
    for (size_t i = 0; i < sampleCount; i++) {
        floatInput[i] = static_cast<float>(samples[i]);
    }

    // 2. 做直接卷积 (O(N*M)) -- 简化示例
    for (size_t n = 0; n < sampleCount; n++) {
        for (size_t k = 0; k < irLength; k++) {
            convResult[n + k] += floatInput[n] * impulseResponse[k];
        }
    }

    // 3. 将卷积结果前 sampleCount 个样本写回(或做混合)
    for (size_t i = 0; i < sampleCount; i++) {
        float val = convResult[i];
        if (val > 32767.0f)  val = 32767.0f;
        if (val < -32768.0f) val = -32768.0f;
        samples[i] = static_cast<int16_t>(val);
    }
}

void AudioProcessor::applyCompressor(int16_t* samples, size_t sampleCount, 
                                     float threshold, float ratio, float attack, float release)
{
    if (!samples || sampleCount == 0) return;
    if (ratio < 1.0f) return; // ratio=1 => 不压缩

    // threshold 是0~1之间时，需要映射到 int16 量级
    float threshold_int16 = threshold * 32767.0f;

    // 极简示例：没有真正使用 attack/release 做平滑包络
    for (size_t i = 0; i < sampleCount; i++) {
        float sampleVal = static_cast<float>(samples[i]);
        float absVal    = fabs(sampleVal);
        if (absVal > threshold_int16) {
            // 超过阈值, 进行压缩
            float over = absVal - threshold_int16;
            over = over / ratio;
            float newVal = (sampleVal >= 0.0f) 
                           ? (threshold_int16 + over) 
                           : -(threshold_int16 + over);

            if (newVal > 32767.0f)   newVal = 32767.0f;
            if (newVal < -32768.0f) newVal = -32768.0f;
            samples[i] = static_cast<int16_t>(newVal);
        }
        // 否则：未超阈值，不做处理
    }
}

// ======================== 滤波器 ========================
void AudioProcessor::applyLowPassFilter(int16_t* samples, size_t sampleCount, float cutoffFreq, float sampleRate) {
    if (!samples || sampleCount == 0) return;

    // 一阶低通: y[n] = y[n-1] + alpha * (x[n] - y[n-1])
    float dt = 1.0f / sampleRate;
    float rc = 1.0f / (2.0f * M_PI * cutoffFreq);
    float alpha = dt / (rc + dt);

    float lastOutput = samples[0];
    for (size_t i = 1; i < sampleCount; i++) {
        float input = static_cast<float>(samples[i]);
        lastOutput  = lastOutput + alpha * (input - lastOutput);
        samples[i]  = static_cast<int16_t>(lastOutput);
    }
}

void AudioProcessor::applyHighPassFilter(int16_t* samples, size_t sampleCount, float cutoffFreq, float sampleRate) {
    if (!samples || sampleCount == 0) return;

    // 一阶高通: y[n] = α * ( y[n-1] + x[n] - x[n-1] )
    // 其中 α = RC / (RC + dt), RC = 1 / (2πfc)
    float dt = 1.0f / sampleRate;
    float rc = 1.0f / (2.0f * M_PI * cutoffFreq);
    float alpha = rc / (rc + dt);

    float x_prev = static_cast<float>(samples[0]);
    float y_prev = x_prev;
    for (size_t i = 1; i < sampleCount; i++) {
        float x_curr = static_cast<float>(samples[i]);
        float y_curr = alpha * (y_prev + x_curr - x_prev);
        samples[i]   = static_cast<int16_t>(y_curr);

        x_prev = x_curr;
        y_prev = y_curr;
    }
}

void AudioProcessor::applyBandPassFilter(int16_t* samples, size_t sampleCount, 
                                         float lowCutoff, float highCutoff, float sampleRate)
{
    if (!samples || sampleCount == 0) return;
    // 简易实现：先高通(滤除低频)，再低通(滤除高频)
    applyHighPassFilter(samples, sampleCount, lowCutoff, sampleRate);
    applyLowPassFilter(samples, sampleCount, highCutoff, sampleRate);
}

// ======================== 频谱分析 ========================
void AudioProcessor::calculateFFT(const int16_t* samples, size_t sampleCount, float* magnitudes, float* phases) {
    if (!samples || sampleCount == 0 || !magnitudes || !phases) return;

    // 占位示例：把采样值简单复制到 magnitudes 并令 phase=0
    // 实际需要真正 FFT 算法(例如 KISS FFT / ESP-DSP / etc.)
    for (size_t i = 0; i < sampleCount; i++) {
        float val = static_cast<float>(samples[i]);
        magnitudes[i] = fabs(val); 
        phases[i] = 0.0f;         // Phase 未计算
    }
}

void AudioProcessor::inverseFft(const float* magnitudes, const float* phases, int16_t* output, size_t sampleCount) {
    if (!magnitudes || !phases || !output || sampleCount == 0) return;

    // 占位示例：将 magnitudes 直接写回 output
    for (size_t i = 0; i < sampleCount; i++) {
        float val = magnitudes[i];
        if (val > 32767.0f)   val = 32767.0f;
        if (val < -32768.0f)  val = -32768.0f;
        output[i] = static_cast<int16_t>(val);
    }
}

// ======================== 噪声处理 ========================
void AudioProcessor::applyNoiseGate(int16_t* samples, size_t sampleCount, float threshold) {
    if (!samples || sampleCount == 0) return;
    // 1. 计算整段RMS
    float rms = calculateRMS(samples, sampleCount);
    // 2. 如果RMS<阈值，则整段设为0(硬门限)
    if (rms < threshold) {
        memset(samples, 0, sampleCount * sizeof(int16_t));
    }
}

void AudioProcessor::applyNoiseReduction(int16_t* samples, size_t sampleCount, float threshold) {
    if (!samples || sampleCount == 0) return;

    // 简易示例：如果 |sample| < threshold_int16，则衰减
    float threshold_int16 = threshold * 32767.0f;
    for (size_t i = 0; i < sampleCount; i++) {
        float val = static_cast<float>(samples[i]);
        if (fabs(val) < threshold_int16) {
            val *= 0.5f; // 简单衰减 50%
        }
        samples[i] = static_cast<int16_t>(val);
    }
}

// ======================== 音高和速度调整 ========================
void AudioProcessor::pitchShift(const int16_t* input, int16_t* output, size_t sampleCount, float pitchFactor) {
    if (!input || !output || sampleCount == 0) return;

    // 简易做法：线性插值重新采样
    std::vector<float> floatIn(sampleCount);
    for (size_t i = 0; i < sampleCount; i++) {
        floatIn[i] = static_cast<float>(input[i]);
    }

    size_t outIdx = 0;
    float inPos = 0.0f;
    while (true) {
        size_t posInt = static_cast<size_t>(inPos);
        if (posInt >= sampleCount - 1) break; // 超出则结束

        float frac    = inPos - posInt;
        float sampleA = floatIn[posInt];
        float sampleB = floatIn[posInt + 1];
        float interp  = sampleA + (sampleB - sampleA) * frac;

        // 写入
        if (outIdx < sampleCount) {
            if (interp > 32767.0f)   interp = 32767.0f;
            if (interp < -32768.0f) interp = -32768.0f;
            output[outIdx] = static_cast<int16_t>(interp);
            outIdx++;
        } else {
            break;
        }
        inPos += pitchFactor;
    }
    // 如果输出比输入短，则补零
    for (; outIdx < sampleCount; outIdx++) {
        output[outIdx] = 0;
    }
}

void AudioProcessor::timeStretch(const int16_t* input, int16_t* output, 
                                 size_t inputCount, size_t& outputCount, float speedFactor)
{
    if (!input || !output || inputCount == 0 || speedFactor <= 0.0f) {
        outputCount = 0;
        return;
    }

    // 简易做法：类似pitchShift，但将目标长度= inputCount / speedFactor
    // speedFactor>1 => 播放更快 => 输出帧更少
    // speedFactor<1 => 播放更慢 => 输出帧更多

    size_t targetLength = static_cast<size_t>(inputCount / speedFactor);
    if (targetLength == 0) {
        outputCount = 0;
        return;
    }

    // 线性插值
    std::vector<float> floatIn(inputCount);
    for (size_t i = 0; i < inputCount; i++) {
        floatIn[i] = static_cast<float>(input[i]);
    }

    float pos   = 0.0f;
    float step  = static_cast<float>(inputCount) / targetLength;
    size_t oIdx = 0;

    while (oIdx < targetLength) {
        size_t posInt = static_cast<size_t>(pos);
        if (posInt >= inputCount - 1) break;

        float frac    = pos - posInt;
        float sampleA = floatIn[posInt];
        float sampleB = floatIn[posInt + 1];
        float interp  = sampleA + (sampleB - sampleA) * frac;

        // 写入
        if (interp > 32767.0f)   interp = 32767.0f;
        if (interp < -32768.0f) interp = -32768.0f;
        output[oIdx] = static_cast<int16_t>(interp);

        oIdx++;
        pos += step;
    }

    // 剩余全置 0
    outputCount = oIdx;
    for (; oIdx < targetLength; oIdx++) {
        output[oIdx] = 0;
    }
}

// ======================== VAD (Voice Activity Detection) ========================
bool AudioProcessor::detectVoiceActivity(const int16_t* samples, size_t sampleCount, float threshold) {
    if (!samples || sampleCount == 0) return false;

    float rms  = calculateRMS(samples, sampleCount);
    float zcr  = calculateZeroCrossingRate(samples, sampleCount);   // 计算过零率

    // Serial.printf("RMS: %.2f, ZCR: %.2f\n", rms, zcr);  //用于调试阈值
    // 简单逻辑：RMS>threshold + ZCR在一个合适范围  => 有语音
    return (rms > threshold) && (zcr >= 0.0f && zcr <= 0.9f); 
    }

// ======================== 音频分析 ========================
float AudioProcessor::calculateZeroCrossingRate(const int16_t* samples, size_t sampleCount) {
    if (!samples || sampleCount <= 1) return 0.0f;

    int crossings = 0;
    for (size_t i = 1; i < sampleCount; i++) {
        if ((samples[i] >= 0 && samples[i-1] < 0) ||
            (samples[i] < 0 && samples[i-1] >= 0)) {
            crossings++;
        }
    }
    return static_cast<float>(crossings) / (sampleCount - 1);
}

float AudioProcessor::calculateSpectralCentroid(const float* magnitudes, size_t count, float sampleRate) {
    if (!magnitudes || count == 0 || sampleRate <= 0.0f) return 0.0f;

    // 频谱质心 = (∑(f_k * |X_k|)) / (∑|X_k|)
    // k在[0, count-1], f_k = k*(sampleRate / count)
    double numerator   = 0.0;
    double denominator = 0.0;
    for (size_t k = 0; k < count; k++) {
        float freq = (static_cast<float>(k) * sampleRate) / (float)count;
        float mag  = magnitudes[k];
        numerator   += freq * mag;
        denominator += mag;
    }
    if (denominator == 0.0) return 0.0f;
    return static_cast<float>(numerator / denominator);
}

void AudioProcessor::calculateMFCC(const int16_t* samples, size_t sampleCount, float* mfccCoeffs, int numCoeffs) {
    if (!samples || sampleCount == 0 || !mfccCoeffs || numCoeffs <= 0) return;

    // 这里仅做示例；真正的 MFCC 需要：分帧->加窗->FFT->Mel滤波器组->Log->DCT
    // 我们给个占位示例，每个系数都设0
    for (int i = 0; i < numCoeffs; i++) {
        mfccCoeffs[i] = 0.0f;
    }
}

// ======================== 私有实用工具函数 ========================
float AudioProcessor::hzToRad(float hz, float sampleRate) {
    return 2.0f * M_PI * hz / sampleRate;
}

void AudioProcessor::applyWindow(float* buffer, size_t size, int windowType) {
    // 简化: 示例使用 Hanning 窗
    // w[n] = 0.5 * (1 - cos(2πn / (N-1)))
    for (size_t i = 0; i < size; i++) {
        float multiplier = 0.5f * (1.0f - cosf(2.0f * M_PI * i / (size - 1)));
        buffer[i] *= multiplier;
    }
}

float AudioProcessor::calculateEnvelope(const int16_t* samples, size_t sampleCount, int windowSize) {
    if (!samples || sampleCount == 0 || windowSize <= 0) return 0.0f;

    // 示例：计算一段音频的平均绝对值 (仅用于示例)
    double sumAbs = 0.0;
    for (size_t i = 0; i < sampleCount && i < (size_t)windowSize; i++) {
        sumAbs += fabs((float)samples[i]);
    }
    return static_cast<float>(sumAbs / windowSize);
}
