/*
 * @Author: wds-Ubuntu22-cqu wdsnpshy@163.com
 * @Date: 2024-12-10 16:05:48
 * @Description: 麦克风录音模块（将声明和实现都放在同一个 .hpp 文件中）
 * 邮箱：wdsnpshy@163.com
 */

#ifndef MICRECORDER_HPP
#define MICRECORDER_HPP

#include <Arduino.h>
#include <math.h>
#include "driver/i2s.h"
#include "AudioProcessor/AudioProcessor.hpp"


// 如果你有自己的 PINS.h，用于定义引脚，可保留此处
#include "PINS.h" 


#define MicRecorder_DEFAULT_I2S_NUM         I2S_NUM_0   // ESP32 S3 一共有两个 I2S 接口
#define MicRecorder_DEFAULT_SAMPLE_RATE     16000       // 16kHz，一般的语音识别采样率
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

// ====================== 实现部分 ======================

MicRecorder::MicRecorder(i2s_port_t i2sNum,
                         uint32_t sampleRate,
                         i2s_bits_per_sample_t bitsPerSample,
                         i2s_channel_fmt_t channelFormat,
                         i2s_comm_format_t commFormat,
                         int dmaBufCount,
                         int dmaBufLen,
                         int bckPin,
                         int wsPin,
                         int dataInPin)
    : _i2s_num(i2sNum),
      _sampleRate(sampleRate),
      _bitsPerSample(bitsPerSample),
      _channelFormat(channelFormat),
      _commFormat(commFormat),
      _dmaBufCount(dmaBufCount),
      _dmaBufLen(dmaBufLen),
      _bckPin(bckPin),
      _wsPin(wsPin),
      _dataInPin(dataInPin),
      _isRecording(false),
      _gain(1.0f),
      _voiceThreshold(50.0f)
{
    // 构造函数中可进行一些自定义操作
}

bool MicRecorder::begin() {
    return initI2S();
}

MicRecorder::~MicRecorder() {
    i2s_driver_uninstall(_i2s_num);
}

bool MicRecorder::initI2S() {   
    // I2S 配置
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = _sampleRate,
        .bits_per_sample = _bitsPerSample,
        .channel_format = _channelFormat,
        .communication_format = _commFormat,
        .intr_alloc_flags = 0,
        .dma_buf_count = _dmaBufCount,
        .dma_buf_len = _dmaBufLen,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    // I2S 引脚配置
    i2s_pin_config_t pin_config = {
        .bck_io_num = _bckPin,
        .ws_io_num = _wsPin,
        .data_out_num = I2S_PIN_NO_CHANGE,  // 只录音，不输出
        .data_in_num = _dataInPin
    };

    // 安装 I2S 驱动
    esp_err_t err = i2s_driver_install(_i2s_num, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.println("MicRecorder: Failed to install I2S driver");
        return false;
    }

    // 配置 I2S 引脚
    err = i2s_set_pin(_i2s_num, &pin_config);
    if (err != ESP_OK) {
        Serial.println("MicRecorder: Failed to set I2S pins");
        i2s_driver_uninstall(_i2s_num); // 卸载已安装的 I2S 驱动，防止资源泄露
        return false;
    }

    Serial.println("MicRecorder: I2S initialized successfully.");
    return true;
}

bool MicRecorder::startRecording() {
    if (_isRecording) return false;
    _isRecording = true;
    return true;
}

bool MicRecorder::stopRecording() {
    if (!_isRecording) return false;
    _isRecording = false;
    return true;
}

bool MicRecorder::isRecording() const {
    return _isRecording;
}

size_t MicRecorder::readPCM(int16_t* buffer, size_t maxSamples) {
    if (!buffer || maxSamples == 0) return 0;

    size_t bytesRead = 0;
    size_t bytesToRead = maxSamples * sizeof(int16_t);

    // 读取数据到 buffer 中
    esp_err_t err = i2s_read(_i2s_num, (void*)buffer, bytesToRead, &bytesRead, portMAX_DELAY);
    if (err != ESP_OK) {
        Serial.println("MicRecorder: I2S read failed");
        return 0;
    }

    // 返回读取到的采样数量
    return bytesRead / sizeof(int16_t);
}

size_t MicRecorder::readPCMProcessed(int16_t* buffer, size_t maxSamples, bool autoGain) {
    size_t samplesRead = readPCM(buffer, maxSamples);
    if (samplesRead > 0) {
        processAudioBuffer(buffer, samplesRead);
        if (autoGain) {
            applyGain(buffer, samplesRead, _gain);
        }
    }
    return samplesRead;
}

void MicRecorder::processAudioBuffer(int16_t* buffer, size_t sampleCount) {
    // 使用 AudioProcessor 进行处理
    if (_gain != 1.0f) {
        AudioProcessor::applyGain(buffer, sampleCount, _gain);
    }
    
    // 添加降噪处理，很基本的降噪处理：低通滤波 + 噪声门限（低于阈值的部分置为0）
    AudioProcessor::applyNoiseGate(buffer, sampleCount, _voiceThreshold);
    
    // 添加低通滤波
    AudioProcessor::applyLowPassFilter(buffer, sampleCount, 8000.0f, _sampleRate);
}

float MicRecorder::getCurrentVolume() {
    int16_t buffer[256];
    size_t samplesRead = readPCM(buffer, 256);  //read 256 samples
    return AudioProcessor::calculateRMS(buffer, samplesRead);
}

bool MicRecorder::isVoiceDetected() {
    int16_t buffer[256];
    size_t samplesRead = readPCM(buffer, 256);
    return AudioProcessor::detectVoiceActivity(buffer, samplesRead, _voiceThreshold);
}

// ------------------- 设置参数函数 -------------------
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
void MicRecorder::setGain(float gain) {
    _gain = gain;
}
void MicRecorder::setVoiceDetectionThreshold(float threshold) {
    _voiceThreshold = threshold;
}

#endif // MICRECORDER_HPP
