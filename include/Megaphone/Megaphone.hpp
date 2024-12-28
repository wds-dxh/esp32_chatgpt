#ifndef MEGAPHONE_HPP
#define MEGAPHONE_HPP

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "driver/i2s.h"
#include "SPIFFS.h"
#include "PINS.h"
#include "AudioProcessor/AudioProcessor.hpp"

// ------------------- 默认参数定义 -------------------
#define Megaphone_DEFAULT_I2S_NUM         I2S_NUM_1
#define Megaphone_DEFAULT_SAMPLE_RATE     16000
#define Megaphone_DEFAULT_BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT
#define Megaphone_DEFAULT_CHANNEL_FORMAT  I2S_CHANNEL_FMT_ONLY_LEFT
#define Megaphone_DEFAULT_COMM_FORMAT     I2S_COMM_FORMAT_STAND_I2S
#define Megaphone_DEFAULT_DMA_BUF_COUNT   16
#define Megaphone_DEFAULT_DMA_BUF_LEN     64

// 用于后台播放的音频数据包
struct AudioChunk {
    int16_t* data;   // 动态分配的采样数据指针
    size_t   size;   // 采样点数量
    bool     isLast; // 是否是最后一个数据块，用于触发回调
};

/**
 * @brief 麦克风放大器控制模块（输出到扬声器或耳机），带非阻塞播放
 */
class Megaphone {
public:
    // 构造 & 析构
    Megaphone(uint32_t sampleRate = Megaphone_DEFAULT_SAMPLE_RATE,
              i2s_bits_per_sample_t bitsPerSample = Megaphone_DEFAULT_BITS_PER_SAMPLE,
              i2s_channel_fmt_t channelFormat = Megaphone_DEFAULT_CHANNEL_FORMAT,
              i2s_comm_format_t commFormat = Megaphone_DEFAULT_COMM_FORMAT,
              int bckPin = Megaphone_DEFAULT_BCK_PIN,
              int wsPin = Megaphone_DEFAULT_WS_PIN,
              int dataOutPin = Megaphone_DEFAULT_DATA_OUT_PIN,
              int dmaBufCount = Megaphone_DEFAULT_DMA_BUF_COUNT,
              int dmaBufLen = Megaphone_DEFAULT_DMA_BUF_LEN,
              i2s_port_t i2sNum = Megaphone_DEFAULT_I2S_NUM);

    ~Megaphone();

    bool begin();

    // 后台任务显式接口
    bool startWriterTask();
    bool stopWriterTask();

    // ------------------- 原始播放(阻塞) -------------------
    size_t playPCM(const int16_t* buffer, size_t sampleCount);

    // 带内部处理(阻塞)
    size_t playPCMProcessed(const int16_t* buffer, size_t sampleCount);

    // ------------------- 队列(非阻塞)播放接口 -------------------
    size_t queuePCM(const int16_t* buffer, size_t sampleCount, bool isLast = false);

    // ------------------- 从文件读取并播放(阻塞) -------------------
    void playFromFile(const char* filename);

    // ------------------- 设置参数函数 -------------------
    void setSampleRate(uint32_t sampleRate);
    void setBitsPerSample(i2s_bits_per_sample_t bitsPerSample);
    void setChannelFormat(i2s_channel_fmt_t channelFormat);
    void setCommFormat(i2s_comm_format_t commFormat);
    void setPins(int bckPin, int wsPin, int dataOutPin);
    void setVolume(float gain);

    // 播放控制接口（结合_isPlaying标志）
    bool isPlaying() const;

    // 音频效果开关
    void enableEcho(bool enable, float delaySeconds = 0.3f, float decay = 0.5f);
    void enableReverb(bool enable, const float* ir = nullptr, size_t irLen = 0);
    void enableCompressor(bool enable, float threshold=0.1f, float ratio=2.0f, float attack=0.01f, float release=0.1f);

    // 缓冲区控制
    void clearBuffer();
    size_t getBufferFree() const;

    // 事件回调
    using PlaybackCallback = void (*)(void* context);
    void setPlaybackCallback(PlaybackCallback callback, void* context = nullptr);

private:
    // I2S配置
    i2s_port_t            _i2s_num;
    uint32_t              _sampleRate;
    i2s_bits_per_sample_t _bitsPerSample;
    i2s_channel_fmt_t     _channelFormat;
    i2s_comm_format_t     _commFormat;
    int _bckPin, _wsPin, _dataOutPin;
    int _dmaBufCount, _dmaBufLen;

    // 播放状态
    float _ampGain;
    bool  _isPlaying;

    // 回调
    PlaybackCallback _callback;
    void*            _callbackContext;

    // ============ 后台队列相关 ============
    QueueHandle_t _audioQueue;       // 存储 AudioChunk 的队列
    static const int QUEUE_LEN = 8;  // 队列最大长度(可根据需要调整)

    // 后台任务
    TaskHandle_t _writerTaskHandle;

    // ============ 音效相关标志及参数 ============
    bool   _echoEnabled;
    float  _echoDelay;
    float  _echoDecay;

    bool   _reverbEnabled;
    const float* _reverbImpulse;
    size_t       _reverbLen;

    bool   _compressorEnabled;
    float  _compressorThreshold;
    float  _compressorRatio;
    float  _compressorAttack;
    float  _compressorRelease;

    bool initI2S();

    /**
     * @brief 后台音频处理函数（增益、效果等）
     */
    void processAudioBuffer(int16_t* buffer, size_t sampleCount);

    /**
     * @brief 后台任务: 从队列取音频块 -> 在isPlaying为true时播放 -> 释放
     */ 
    static void i2sWriterTask(void* parameter);
};


#endif // MEGAPHONE_HPP
