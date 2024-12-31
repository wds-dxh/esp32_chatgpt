/*
 * @Author: wds-Ubuntu22-cqu wdsnpshy@163.com
 * @Date: 2024-12-10 16:05:48
 * @Description: 麦克风录音模块（将声明和实现都放在同一个 .hpp 文件中）
 * 邮箱：wdsnpshy@163.com
 */
#pragma once

#include <Arduino.h>
#include <math.h>
#include "driver/i2s.h"
#include "AudioProcessor/AudioProcessor.hpp"


// 如果你有自己的 PINS.h，用于定义引脚，可保留此处
#include "PINS.h" 


#define MicRecorder_DEFAULT_I2S_NUM         I2S_NUM_0   // ESP32 S3 一共有两个 I2S 接口
#define MicRecorder_DEFAULT_SAMPLE_RATE     8000       // 16kHz，一般的语音识别采样率
#define MicRecorder_DEFAULT_BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT // 16位，一般的语音识别位深
#define MicRecorder_DEFAULT_CHANNEL_FORMAT  I2S_CHANNEL_FMT_ONLY_LEFT  // 只使用左声道
#define MicRecorder_DEFAULT_COMM_FORMAT     I2S_COMM_FORMAT_STAND_I2S  // 标准 I2S 通信模式
#define MicRecorder_DEFAULT_DMA_BUF_COUNT   16  // DMA 缓冲区数量
#define MicRecorder_DEFAULT_DMA_BUF_LEN     64  // DMA 缓冲区长度


/**
 * @brief 麦克风录音模块
 */
class MicRecorder {
public:
    /**
     * @description: 麦克风录音模块构造函数
     * @param {i2s_port_t} i2sNum       I2S 端口号 (默认 I2S_NUM_0)
     * @param {uint32_t} sampleRate     采样率 (默认 16kHz)
     * @param {i2s_bits_per_sample_t} bitsPerSample 位深 (默认 16bit)
     * @param {i2s_channel_fmt_t} channelFormat     通道格式 (默认只使用左声道)
     * @param {i2s_comm_format_t} commFormat        通信格式 (默认标准 I2S)
     * @param {int} dmaBufCount         DMA 缓冲区数量 (默认 16)
     * @param {int} dmaBufLen           DMA 缓冲区长度 (默认 64)
     * @param {int} bckPin              BCK 引脚
     * @param {int} wsPin               WS 引脚
     * @param {int} dataInPin           DATA IN 引脚
     */
    MicRecorder(i2s_port_t i2sNum = MicRecorder_DEFAULT_I2S_NUM,
                uint32_t sampleRate = MicRecorder_DEFAULT_SAMPLE_RATE,
                i2s_bits_per_sample_t bitsPerSample = MicRecorder_DEFAULT_BITS_PER_SAMPLE,
                i2s_channel_fmt_t channelFormat = MicRecorder_DEFAULT_CHANNEL_FORMAT,
                i2s_comm_format_t commFormat = MicRecorder_DEFAULT_COMM_FORMAT,
                int dmaBufCount = MicRecorder_DEFAULT_DMA_BUF_COUNT,
                int dmaBufLen = MicRecorder_DEFAULT_DMA_BUF_LEN,
                int bckPin = MicRecorder_DEFAULT_BCK_PIN,
                int wsPin = MicRecorder_DEFAULT_WS_PIN,
                int dataInPin = MicRecorder_DEFAULT_DATA_IN_PIN);

    bool begin();
    ~MicRecorder(); // 析构函数

    /**
     * @brief 从 I2S DMA 中读取 PCM 数据
     * @param[out] buffer 存放读取到的数据（int16_t 类型）
     * @param[in]  maxSamples buffer 能够容纳的最大采样数量（单位：采样点）
     * @return 实际读取到的采样数量
     */
    size_t readPCM(int16_t* buffer, size_t maxSamples);

        // 音频数据读取
    size_t readPCMProcessed(int16_t* buffer, size_t maxSamples, bool autoGain = true);
    
    // 音频监测
    float getCurrentVolume();  // 获取当前音量
    bool isVoiceDetected();   // 检测是否有声音输入 使用rms和zcr

    // ------------------- 设置参数函数 -------------------
    void setSampleRate(uint32_t sampleRate);
    void setBitsPerSample(i2s_bits_per_sample_t bitsPerSample);
    void setChannelFormat(i2s_channel_fmt_t channelFormat);
    void setCommFormat(i2s_comm_format_t commFormat);
    void setPins(int bckPin, int wsPin, int dataInPin);
    // 设置参数
    void setGain(float gain);  // 设置增益
    void setVoiceDetectionThreshold(float threshold);   

    // ------------------- 音频信号处理函数 -------------------
    /**
     * @brief 计算给定音频数据的 RMS 值
     * @param samples 输入的 int16_t 音频数据
     * @param sampleCount 音频数据的采样数量
     * @return 计算得到的 RMS 值
     */
    inline float calculateRMS(const int16_t* samples, size_t sampleCount) {
        return AudioProcessor::calculateRMS(samples, sampleCount);
    }

    /**
     * @brief 对音频数据应用增益
     * @param samples 需要处理的 int16_t 音频数据
     * @param sampleCount 音频数据的采样数量
     * @param gain 增益值 (如 0.5, 2.0)
     */
    inline void applyGain(int16_t* samples, size_t sampleCount, float gain) {
        AudioProcessor::applyGain(samples, sampleCount, gain);
    }

    /**
     * @brief 对音频数据进行归一化
     * @param samples 需要处理的 int16_t 音频数据
     * @param sampleCount 音频数据的采样数量
     * @param maxAmplitude 归一化目标最大值，默认为 32767
     */
    inline void normalizeBuffer(int16_t* samples, size_t sampleCount, int16_t maxAmplitude = 32767) {
        AudioProcessor::normalize(samples, sampleCount, maxAmplitude);
    }

    /**
     * @brief 将 int16_t 数据转换为浮点数(-1.0~1.0)
     * @param input 输入的 int16_t 音频数据
     * @param output 输出的 float 音频数据
     * @param sampleCount 音频数据的采样数量
     */
    inline void convertInt16ToFloat(const int16_t* input, float* output, size_t sampleCount) {
        if (!input || !output || sampleCount == 0) return;

        for (size_t i = 0; i < sampleCount; i++) {
            output[i] = (float)input[i] / 32768.0f; // 32768 = 2^15
        }
    }

    // 优化的音频采集接口,待完善，还没有实现逻辑部分！ 
    bool startRecording();  // 开始录音
    bool stopRecording();   // 停止录音
    bool isRecording() const; // 是否正在录音
    
private:
    // I2S 配置相关
    i2s_port_t             _i2s_num;       // I2S 端口号
    uint32_t               _sampleRate;    // 采样率
    i2s_bits_per_sample_t  _bitsPerSample; // 位深
    i2s_channel_fmt_t      _channelFormat; // 通道格式
    i2s_comm_format_t      _commFormat;    // 通信格式
    int                    _dmaBufCount;   // DMA 缓冲区数量
    int                    _dmaBufLen;     // DMA 缓冲区长度

    // 引脚配置
    int                    _bckPin;        // BCK 引脚
    int                    _wsPin;         // WS 引脚
    int                    _dataInPin;     // DATA IN 引脚

    // 成员变量
    bool _isRecording;  // 是否正在录音
    float _gain;        // 增益 
    float _voiceThreshold;  // 语音检测阈值

    // 私有工具方法
    bool initI2S();
    void processAudioBuffer(int16_t* buffer, size_t sampleCount);
};

