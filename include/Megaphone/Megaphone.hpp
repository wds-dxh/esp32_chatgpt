#ifndef MEGAPHONE_HPP
#define MEGAPHONE_HPP

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "driver/i2s.h"
#include "SPIFFS.h"
#include "PINS.h"             // 确保这里定义了 Megaphone_DEFAULT_BCK_PIN、Megaphone_DEFAULT_WS_PIN、Megaphone_DEFAULT_DATA_OUT_PIN
#include "AudioProcessor/AudioProcessor.hpp"

// ------------------- 默认参数定义 -------------------
#define Megaphone_DEFAULT_I2S_NUM         I2S_NUM_1
#define Megaphone_DEFAULT_SAMPLE_RATE     16000
#define Megaphone_DEFAULT_BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT
#define Megaphone_DEFAULT_CHANNEL_FORMAT  I2S_CHANNEL_FMT_ONLY_LEFT
#define Megaphone_DEFAULT_COMM_FORMAT     I2S_COMM_FORMAT_STAND_I2S
#define Megaphone_DEFAULT_DMA_BUF_COUNT   16
#define Megaphone_DEFAULT_DMA_BUF_LEN     64

// ========== 用于后台播放的音频数据包 ==========
struct AudioChunk {
    int16_t* data;    // 动态分配的采样数据指针
    size_t   size;    // 采样点数量
    bool     isLast;  // 是否是最后一个数据块，用于触发回调
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

    // ------------------- 原始播放(阻塞) -------------------
    size_t playPCM(const int16_t* buffer, size_t sampleCount);

    // 带内部处理(阻塞)
    size_t playPCMProcessed(const int16_t* buffer, size_t sampleCount);

    // ------------------- 队列(非阻塞)播放接口 -------------------
    /**
     * @brief 把一段 PCM 数据放入队列，后台任务会自动播放
     * @param buffer PCM 数据
     * @param sampleCount 采样点数量
     * @param isLast 是否为此段音频的最后一包，用于在播放结束后触发回调
     * @return 实际放入队列的采样点数量(=0 表示队列已满或分配失败)
     */
    size_t queuePCM(const int16_t* buffer, size_t sampleCount, bool isLast = false);

    // ------------------- 从文件读取并播放 -------------------
    // 以下演示仍是阻塞式读取文件，但可将数据用 queuePCM 推送到队列。示例不做循环队列式文件读取。
    void playFromFile(const char* filename);

    // ------------------- 设置参数函数 -------------------
    void setSampleRate(uint32_t sampleRate);
    void setBitsPerSample(i2s_bits_per_sample_t bitsPerSample);
    void setChannelFormat(i2s_channel_fmt_t channelFormat);
    void setCommFormat(i2s_comm_format_t commFormat);
    void setPins(int bckPin, int wsPin, int dataOutPin);
    void setVolume(float gain);

    // ------------------- 音频处理快速接口 -------------------
    void applyGain(int16_t* samples, size_t sampleCount, float gain) {
        AudioProcessor::applyGain(samples, sampleCount, gain);
    }

    // ... 还可直接调用 AudioProcessor::applyEqualizer(...) 等

    // ------------------- 播放控制接口 -------------------
    bool startPlaying();
    bool stopPlaying();
    bool isPlaying() const;

    // ------------------- 音频效果和均衡器 -------------------
    // 根据实际需求添加一些开关/参数，用于 processAudioBuffer() 内部调用
    // 这里示例用布尔量来决定是否使用 Echo / Reverb / Compressor / EQ
    void enableEcho(bool enable, float delaySeconds = 0.3f, float decay = 0.5f);
    void enableReverb(bool enable, const float* ir = nullptr, size_t irLen = 0);
    void enableCompressor(bool enable, float threshold=0.1f, float ratio=2.0f, float attack=0.01f, float release=0.1f);
    // void enableEqualizer(bool enable, const float* eqBands = nullptr, size_t eqBandCount = 0);      // 未实现

    // ------------------- 缓冲区控制 -------------------
    void clearBuffer();

    /**
     * @brief 获取队列可用空间(还可放多少个 AudioChunk)
     * @return 当前可用队列槽位数
     */
    size_t getBufferFree() const;

    // ------------------- 事件回调 -------------------
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

    bool   _eqEnabled;
    const float* _eqBands;
    size_t       _eqBandCount;

    // 私有方法
    bool initI2S();

    /**
     * @brief 后台音频处理函数（增益、效果等）
     */
    void processAudioBuffer(int16_t* buffer, size_t sampleCount);

    /**
     * @brief 后台任务: 从队列取音频块 -> 处理 -> i2s_write -> 释放
     */
    static void i2sWriterTask(void* parameter);
};


// ====================== 实现部分 ======================

// ------------ 构造 & 析构 ------------
Megaphone::Megaphone(uint32_t sampleRate,
                     i2s_bits_per_sample_t bitsPerSample,
                     i2s_channel_fmt_t channelFormat,
                     i2s_comm_format_t commFormat,
                     int bckPin,
                     int wsPin,
                     int dataOutPin,
                     int dmaBufCount,
                     int dmaBufLen,
                     i2s_port_t i2sNum)
    : _i2s_num(i2sNum),
      _sampleRate(sampleRate),
      _bitsPerSample(bitsPerSample),
      _channelFormat(channelFormat),
      _commFormat(commFormat),
      _bckPin(bckPin),
      _wsPin(wsPin),
      _dataOutPin(dataOutPin),
      _dmaBufCount(dmaBufCount),
      _dmaBufLen(dmaBufLen),
      _ampGain(1.0f),
      _isPlaying(false),
      _callback(nullptr),
      _callbackContext(nullptr),
      _audioQueue(nullptr),
      _echoEnabled(false),
      _echoDelay(0.3f),
      _echoDecay(0.5f),
      _reverbEnabled(false),
      _reverbImpulse(nullptr),
      _reverbLen(0),
      _compressorEnabled(false),
      _compressorThreshold(0.1f),
      _compressorRatio(2.0f),
      _compressorAttack(0.01f),
      _compressorRelease(0.1f),
      _eqEnabled(false),
      _eqBands(nullptr),
      _eqBandCount(0)
{
}

Megaphone::~Megaphone() {
    // 结束任务 & 删除队列
    if (_audioQueue) {
        vQueueDelete(_audioQueue);
        _audioQueue = nullptr;
    }
    i2s_driver_uninstall(_i2s_num);
}

// ------------ 初始化 I2S ------------
bool Megaphone::begin() {
    if (!initI2S()) return false;

    // 创建队列: 可容纳 QUEUE_LEN 个 AudioChunk
    _audioQueue = xQueueCreate(QUEUE_LEN, sizeof(AudioChunk));
    if (!_audioQueue) {
        Serial.println("Megaphone: Failed to create audio queue!");
        return false;
    }

    // 创建后台任务，用于从队列读取数据并播放
    xTaskCreatePinnedToCore(i2sWriterTask, 
                            "i2sWriterTask", 
                            4096,  // 堆栈可根据需求调大
                            this, 
                            1,     // 优先级
                            NULL, 
                            0);    // 绑定到Core 0(或1)

    return true;
}

bool Megaphone::initI2S() {
    i2s_config_t i2sConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = _sampleRate,
        .bits_per_sample = _bitsPerSample,
        .channel_format = _channelFormat,
        .communication_format = _commFormat,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = _dmaBufCount,
        .dma_buf_len = _dmaBufLen,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pinConfig = {
        .bck_io_num = _bckPin,
        .ws_io_num = _wsPin,
        .data_out_num = _dataOutPin,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    // 安装 I2S 驱动
    esp_err_t err = i2s_driver_install(_i2s_num, &i2sConfig, 0, NULL);
    if (err != ESP_OK) {
        Serial.println("Megaphone: Failed to install I2S driver");
        return false;
    }

    // 设置引脚
    err = i2s_set_pin(_i2s_num, &pinConfig);
    if (err != ESP_OK) {
        Serial.println("Megaphone: Failed to set I2S pins");
        i2s_driver_uninstall(_i2s_num);
        return false;
    }

    Serial.println("Megaphone: I2S initialized successfully");
    return true;
}

// ------------ 播放 PCM 数据（原始阻塞） ------------
size_t Megaphone::playPCM(const int16_t* buffer, size_t sampleCount) {
    if (!buffer || sampleCount == 0) return 0;

    size_t bytesToWrite = sampleCount * sizeof(int16_t);
    size_t bytesWritten = 0;
    esp_err_t err = i2s_write(_i2s_num, (const char*)buffer, bytesToWrite, &bytesWritten, portMAX_DELAY);
    if (err != ESP_OK) {
        Serial.println("Megaphone: I2S write error");
        return 0;
    }
    return bytesWritten; 
}

// ------------ 播放 PCM 数据（内部处理+阻塞） ------------
size_t Megaphone::playPCMProcessed(const int16_t* buffer, size_t sampleCount) {
    if (!buffer || sampleCount == 0) return 0;
    int16_t* tmp = (int16_t*)malloc(sampleCount * sizeof(int16_t));
    if (!tmp) return 0;

    memcpy(tmp, buffer, sampleCount * sizeof(int16_t));
    processAudioBuffer(tmp, sampleCount);

    size_t written = playPCM(tmp, sampleCount);
    free(tmp);  // 释放临时内存
    return written;
}

// ------------ 非阻塞队列接口 ------------
size_t Megaphone::queuePCM(const int16_t* buffer, size_t sampleCount, bool isLast) {
    if (!_audioQueue || !buffer || sampleCount == 0) return 0;

    // 分配内存拷贝数据
    int16_t* dataCopy = (int16_t*)malloc(sampleCount * sizeof(int16_t));
    if (!dataCopy) {
        Serial.println("Megaphone: Malloc failed in queuePCM!");
        return 0;
    }
    memcpy(dataCopy, buffer, sampleCount*sizeof(int16_t));

    AudioChunk chunk;   
    chunk.data  = dataCopy;
    chunk.size  = sampleCount;
    chunk.isLast = isLast;

    // 尝试入队(立即返回，不阻塞)
    if (xQueueSend(_audioQueue, &chunk, 0) == pdTRUE) {
        // 入队成功
        return sampleCount;
    } else {
        // 队列满了
        free(dataCopy);
        return 0;
    }
}

// ------------ 从文件读取并播放(阻塞示例) ------------
void Megaphone::playFromFile(const char* filename) {
    if (!SPIFFS.begin(true)) {
        Serial.println("Megaphone: Failed to mount SPIFFS");
        return;
    }

    File audioFile = SPIFFS.open(filename, "r");
    if (!audioFile) {
        Serial.println("Megaphone: Failed to open audio file");
        SPIFFS.end();
        return;
    }

    Serial.println("Megaphone: Starting playback from file...");
    int16_t buffer[256];
    size_t bytesRead;

    while ((bytesRead = audioFile.read((uint8_t*)buffer, sizeof(buffer))) > 0) {
        size_t samples = bytesRead / sizeof(int16_t);
        // 调用阻塞接口(或你也可以用 queuePCM 推送到后台)
        applyGain(buffer, samples, _ampGain);
        playPCM(buffer, samples);
    }

    audioFile.close();
    SPIFFS.end();
    Serial.println("Megaphone: Playback finished.");
}

// ------------ 参数设置 ------------
void Megaphone::setSampleRate(uint32_t sampleRate) {
    _sampleRate = sampleRate;
}
void Megaphone::setBitsPerSample(i2s_bits_per_sample_t bitsPerSample) {
    _bitsPerSample = bitsPerSample;
}
void Megaphone::setChannelFormat(i2s_channel_fmt_t channelFormat) {
    _channelFormat = channelFormat;
}
void Megaphone::setCommFormat(i2s_comm_format_t commFormat) {
    _commFormat = commFormat;
}
void Megaphone::setPins(int bckPin, int wsPin, int dataOutPin) {
    _bckPin     = bckPin;
    _wsPin      = wsPin;
    _dataOutPin = dataOutPin;
}
void Megaphone::setVolume(float gain) {
    _ampGain = gain;
}

// ------------ 播放控制 ------------
bool Megaphone::startPlaying() {
    _isPlaying = true;
    return true;
}
bool Megaphone::stopPlaying() {
    _isPlaying = false;
    return true;
}
bool Megaphone::isPlaying() const {
    return _isPlaying;
}

// ============ 音频效果开关 ============ 
void Megaphone::enableEcho(bool enable, float delaySeconds, float decay) {
    _echoEnabled = enable;
    _echoDelay   = delaySeconds;
    _echoDecay   = decay;
}

void Megaphone::enableReverb(bool enable, const float* ir, size_t irLen) {
    _reverbEnabled  = enable;
    _reverbImpulse  = ir;
    _reverbLen      = irLen;
}

void Megaphone::enableCompressor(bool enable, float threshold, float ratio, float attack, float release) {
    _compressorEnabled  = enable;
    _compressorThreshold= threshold;
    _compressorRatio    = ratio;
    _compressorAttack   = attack;
    _compressorRelease  = release;
}

// void Megaphone::enableEqualizer(bool enable, const float* eqBands, size_t eqBandCount) { // 未实现
//     _eqEnabled   = enable;
//     _eqBands     = eqBands;
//     _eqBandCount = eqBandCount;
// }

// ------------ 清空DMA缓冲 ------------
void Megaphone::clearBuffer() {
    i2s_zero_dma_buffer(_i2s_num);
    Serial.println("Megaphone: DMA buffer cleared.");
}

// ------------ 获取队列可用空间 ------------
size_t Megaphone::getBufferFree() const {
    if (!_audioQueue) return 0;
    // 返回当前队列中还可放多少个 AudioChunk
    return uxQueueSpacesAvailable(_audioQueue);
}

// ------------ 回调 ------------
void Megaphone::setPlaybackCallback(PlaybackCallback callback, void* context) {
    _callback = callback;
    _callbackContext = context;
}

// ------------ 音频处理(音量+效果等) ------------
void Megaphone::processAudioBuffer(int16_t* buffer, size_t sampleCount) {
    // 1. 音量增益
    if (_ampGain != 1.0f) {
        AudioProcessor::applyGain(buffer, sampleCount, _ampGain);
    }

    // // 2. 均衡器
    // if (_eqEnabled && _eqBands && _eqBandCount > 0) {
    //     AudioProcessor::applyEqualizer(buffer, sampleCount, _eqBands, _eqBandCount);
    // }

    // 3. 压缩
    if (_compressorEnabled) {
        AudioProcessor::applyCompressor(buffer, sampleCount, 
                                        _compressorThreshold, _compressorRatio, 
                                        _compressorAttack, _compressorRelease);
    }

    // 4. 回声
    if (_echoEnabled) {
        AudioProcessor::applyEcho(buffer, sampleCount, _echoDelay, _echoDecay);
    }

    // 5. 混响
    if (_reverbEnabled && _reverbImpulse && _reverbLen > 0) {
        AudioProcessor::applyReverb(buffer, sampleCount, _reverbImpulse, _reverbLen);
    }
}

// ------------ 后台任务: 从队列取数据 -> process -> i2s_write ------------
void Megaphone::i2sWriterTask(void* parameter) {
    Megaphone* self = static_cast<Megaphone*>(parameter);
    while (true) {
        AudioChunk chunk;
        // 等待队列里有数据
        if (xQueueReceive(self->_audioQueue, &chunk, portMAX_DELAY) == pdTRUE) {
            if (!chunk.data || chunk.size == 0) {
                // 如果数据异常，直接丢弃
                if (chunk.data) free(chunk.data);
                continue;
            }
            // 处理(音量/效果等)
            self->processAudioBuffer(chunk.data, chunk.size);

            // 写I2S(阻塞)
            size_t bytesWritten = 0;
            i2s_write(self->_i2s_num, chunk.data, chunk.size*sizeof(int16_t), &bytesWritten, portMAX_DELAY);

            // 释放堆内存
            free(chunk.data);

            // 如果是最后一块，则触发回调(如果已设置)
            if (chunk.isLast && self->_callback) {
                self->_callback(self->_callbackContext);
            }
        }
    }
}

#endif // MEGAPHONE_HPP
