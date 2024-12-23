/*
 * @Author: wds-Ubuntu22-cqu wdsnpshy@163.com
 * @Date: 2024-12-10 16:05:48
 * @Description: 麦克风录音模块
 * 邮箱：wdsnpshy@163.com 
 * Copyright (c) 2024 by ${wds-Ubuntu22-cqu}, All Rights Reserved. 
 */


/*
 * 音频处理函数：
 * 1. calculateRMS: 计算音频数据的 RMS 值
 * 2. applyGain: 对音频数据应用增益
 * 3. normalizeBuffer: 对音频数据进行归一化
 * 4. convertInt16ToFloat: 将 int16_t 数据转换为浮点数(-1.0~1.0)
 */

#ifndef MICRECORDER_HPP
#define MICRECORDER_HPP

#include <Arduino.h>
#include <math.h>
#include "driver/i2s.h"
#include "PINS.h"

// 默认参数定义
#define DEFAULT_I2S_NUM         I2S_NUM_0   //esp32 s3 一共有两个I2S接口
#define DEFAULT_SAMPLE_RATE     16000      // 讯飞 STT 推荐的采样率为 16kHz
#define DEFAULT_BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT   // 16位,讯飞 STT 推荐的位宽为 16 位
#define DEFAULT_CHANNEL_FORMAT  I2S_CHANNEL_FMT_ONLY_LEFT   // 只使用左声道
#define DEFAULT_COMM_FORMAT I2S_COMM_FORMAT_STAND_I2S   //定义为 I2S 标准模式
#define DEFAULT_DMA_BUF_COUNT   16
#define DEFAULT_DMA_BUF_LEN     64

class MicRecorder {
public:
    // 构造函数

    /**
     * @description: 麦克风录音模块构造函数
     * @param {uint32_t} sampleRate 采样率
     * @param {i2s_bits_per_sample_t} bitsPerSample 位深
     * @param {i2s_channel_fmt_t} channelFormat 通道格式
     * @param {i2s_comm_format_t} commFormat 通信格式
     * @param {int} bckPin BCK 引脚
     * @param {int} wsPin WS 引脚
     * @param {int} dataInPin DATA IN 引脚
     * @return {*} 
     */    
    MicRecorder(uint32_t sampleRate = DEFAULT_SAMPLE_RATE,
                i2s_bits_per_sample_t bitsPerSample = DEFAULT_BITS_PER_SAMPLE,
                i2s_channel_fmt_t channelFormat = DEFAULT_CHANNEL_FORMAT,
                i2s_comm_format_t commFormat = DEFAULT_COMM_FORMAT,
                int bckPin = DEFAULT_BCK_PIN,
                int wsPin = DEFAULT_WS_PIN,
                int dataInPin = DEFAULT_DATA_IN_PIN);

    // 初始化 I2S
    bool begin();
 
    size_t readPCM(int16_t* buffer, size_t maxSamples);

    // 设置参数函数
    void setSampleRate(uint32_t sampleRate);
    void setBitsPerSample(i2s_bits_per_sample_t bitsPerSample);
    void setChannelFormat(i2s_channel_fmt_t channelFormat);
    void setCommFormat(i2s_comm_format_t commFormat);
    void setPins(int bckPin, int wsPin, int dataInPin);

    // 音频信号处理函数
    float calculateRMS(const int16_t* samples, size_t sampleCount);
    void applyGain(int16_t* samples, size_t sampleCount, float gain);
    void normalizeBuffer(int16_t* samples, size_t sampleCount, int16_t maxAmplitude = 32767);
    void convertInt16ToFloat(const int16_t* input, float* output, size_t sampleCount);

private:
    i2s_port_t _i2s_num;
    uint32_t _sampleRate;
    i2s_bits_per_sample_t _bitsPerSample;
    i2s_channel_fmt_t _channelFormat;
    i2s_comm_format_t _commFormat;
    int _bckPin;
    int _wsPin;
    int _dataInPin;
};

// 实现部分


MicRecorder::MicRecorder(uint32_t sampleRate,
                         i2s_bits_per_sample_t bitsPerSample,
                         i2s_channel_fmt_t channelFormat,
                         i2s_comm_format_t commFormat,
                         int bckPin,
                         int wsPin,
                         int dataInPin)
    : _i2s_num(DEFAULT_I2S_NUM),
      _sampleRate(sampleRate),
      _bitsPerSample(bitsPerSample),
      _channelFormat(channelFormat),
      _commFormat(commFormat),
      _bckPin(bckPin),
      _wsPin(wsPin),
      _dataInPin(dataInPin) {
    // pass
}

// 初始化 I2S 实现
bool MicRecorder::begin() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = _sampleRate,
        .bits_per_sample = _bitsPerSample,
        .channel_format = _channelFormat,
        .communication_format = _commFormat,
        .intr_alloc_flags = 0,
        .dma_buf_count = DEFAULT_DMA_BUF_COUNT,
        .dma_buf_len = DEFAULT_DMA_BUF_LEN,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = _bckPin,
        .ws_io_num = _wsPin,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = _dataInPin
    };

    esp_err_t err = i2s_driver_install(_i2s_num, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.println("Failed to install I2S driver");
        return false;
    }

    err = i2s_set_pin(_i2s_num, &pin_config);
    if (err != ESP_OK) {
        Serial.println("Failed to set I2S pins");
        return false;
    }

    return true;
}

// 读取 PCM 数据实现
size_t MicRecorder::readPCM(int16_t* buffer, size_t maxSamples) {
    size_t bytesRead = 0;
    size_t bytesToRead = maxSamples * sizeof(int16_t);

    esp_err_t err = i2s_read(_i2s_num, (void*)buffer, bytesToRead, &bytesRead, portMAX_DELAY);
    if (err != ESP_OK) {
        Serial.println("I2S read failed");
        return 0;
    }

    return bytesRead / sizeof(int16_t);
}

// 设置参数实现
void MicRecorder::setSampleRate(uint32_t sampleRate) {
    _sampleRate = sampleRate;
}
void MicRecorder::setBitsPerSample(i2s_bits_per_sample_t bitsPerSample) {
    _bitsPerSample = bitsPerSample;
}
void MicRecorder::setChannelFormat(i2s_channel_fmt_t channelFormat) {
    _channelFormat = channelFormat;
}
void MicRecorder::setCommFormat(i2s_comm_format_t commFormat) {
    _commFormat = commFormat;
}
void MicRecorder::setPins(int bckPin, int wsPin, int dataInPin) {
    _bckPin = bckPin;
    _wsPin = wsPin;
    _dataInPin = dataInPin;
}

// 音频信号处理函数实现

// 1. calculateRMS: 计算音频数据的 RMS 值
float MicRecorder::calculateRMS(const int16_t* samples, size_t sampleCount) {
    double sum = 0.0;
    for (size_t i = 0; i < sampleCount; i++) {
        float val = (float)samples[i];
        sum += val * val;
    }
    double mean = sum / sampleCount;
    return sqrt(mean);
}

// 2. applyGain: 对音频数据应用增益
void MicRecorder::applyGain(int16_t* samples, size_t sampleCount, float gain) {
    for (size_t i = 0; i < sampleCount; i++) {
        int32_t temp = (int32_t)(samples[i] * gain);
        // 限幅，防止溢出
        if (temp > 32767) temp = 32767;
        if (temp < -32768) temp = -32768;
        samples[i] = (int16_t)temp;
    }
}

// 3. normalizeBuffer: 对音频数据进行归一化
void MicRecorder::normalizeBuffer(int16_t* samples, size_t sampleCount, int16_t maxAmplitude) {
    if (sampleCount == 0) return;
    
    int16_t maxVal = 0;
    for (size_t i = 0; i < sampleCount; i++) {
        int16_t val = samples[i];
        if (abs(val) > maxVal) {
            maxVal = abs(val);
        }
    }

    if (maxVal == 0) return; // 避免除零

    float scale = (float)maxAmplitude / (float)maxVal;
    applyGain(samples, sampleCount, scale);
}

// 4. convertInt16ToFloat: 将 int16_t 数据转换为浮点数(-1.0~1.0)
void MicRecorder::convertInt16ToFloat(const int16_t* input, float* output, size_t sampleCount) {
    for (size_t i = 0; i < sampleCount; i++) {
        output[i] = (float)input[i] / 32768.0f; // 32768 = 2^15
    }
}

#endif // MICRECORDER_HPP