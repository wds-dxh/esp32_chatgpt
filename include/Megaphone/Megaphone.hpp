#ifndef MEGAPHONE_HPP
#define MEGAPHONE_HPP

#include <Arduino.h>
#include "driver/i2s.h"
#include "SPIFFS.h"    // 用于文件系统操作
#include "PINS.h"      // 确保 PINS.h 中定义了 DEFAULT_BCK_PIN、DEFAULT_WS_PIN、DEFAULT_DATA_OUT_PIN

// ------------------- 默认参数定义 -------------------
#ifndef Megaphone_DEFAULT_I2S_NUM
#define Megaphone_DEFAULT_I2S_NUM         I2S_NUM_1   // ESP32 S3 一共有两个 I2S 接口
#endif

#ifndef Megaphone_DEFAULT_SAMPLE_RATE
#define Megaphone_DEFAULT_SAMPLE_RATE     16000       // 默认采样率为 24kHz
#endif

#ifndef Megaphone_DEFAULT_BITS_PER_SAMPLE
#define Megaphone_DEFAULT_BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT // 默认位深为 16 位
#endif

#ifndef Megaphone_DEFAULT_CHANNEL_FORMAT
#define Megaphone_DEFAULT_CHANNEL_FORMAT  I2S_CHANNEL_FMT_ONLY_LEFT  // 只使用左声道
#endif

#ifndef Megaphone_DEFAULT_COMM_FORMAT
#define Megaphone_DEFAULT_COMM_FORMAT     I2S_COMM_FORMAT_STAND_I2S  // 标准 I2S 通信模式
#endif

#ifndef Megaphone_DEFAULT_DMA_BUF_COUNT
#define Megaphone_DEFAULT_DMA_BUF_COUNT   16  // DMA 缓冲区数量
#endif

#ifndef Megaphone_DEFAULT_DMA_BUF_LEN
#define Megaphone_DEFAULT_DMA_BUF_LEN     64  // DMA 缓冲区长度
#endif

/**
 * @brief 麦克风放大器控制模块
 */
class Megaphone {
public:
    /**
     * @brief 构造函数
     * 
     * @param sampleRate     采样率 (默认: Megaphone_DEFAULT_SAMPLE_RATE)
     * @param bitsPerSample  位深 (默认: Megaphone_DEFAULT_BITS_PER_SAMPLE)
     * @param channelFormat  通道格式 (默认: Megaphone_DEFAULT_CHANNEL_FORMAT)
     * @param commFormat     通信格式 (默认: Megaphone_DEFAULT_COMM_FORMAT)
     * @param bckPin         BCK 引脚 (默认: DEFAULT_BCK_PIN)
     * @param wsPin          WS 引脚 (默认: DEFAULT_WS_PIN)
     * @param dataOutPin     数据输出引脚 (默认: DEFAULT_DATA_OUT_PIN)
     * @param dmaBufCount    DMA 缓冲区数量 (默认: Megaphone_DEFAULT_DMA_BUF_COUNT)
     * @param dmaBufLen      DMA 缓冲区长度 (默认: Megaphone_DEFAULT_DMA_BUF_LEN)
     * @param i2sNum         I2S 端口号 (默认: Megaphone_DEFAULT_I2S_NUM)
     */
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

    /**
     * @brief 析构函数，卸载 I2S 驱动
     */
    ~Megaphone();

    /**
     * @brief 初始化 I2S
     * 
     * @return true 初始化成功
     * @return false 初始化失败
     */
    bool begin();

    /**
     * @brief 播放 PCM 数据
     * 
     * @param buffer      音频采样数据
     * @param sampleCount 采样总数
     * @return size_t     实际写入的字节数
     */
    size_t playPCM(const int16_t* buffer, size_t sampleCount);

    /**
     * @brief 从文件读取音频数据并播放
     * 
     * @param filename 音频文件名 (路径以斜杠开头，例如 "/audio.pcm")
     */
    void playFromFile(const char *filename);

    // ------------------- 设置参数函数 -------------------
    void setSampleRate(uint32_t sampleRate);
    void setBitsPerSample(i2s_bits_per_sample_t bitsPerSample);
    void setChannelFormat(i2s_channel_fmt_t channelFormat);
    void setCommFormat(i2s_comm_format_t commFormat);
    void setPins(int bckPin, int wsPin, int dataOutPin);
    void setVolume(float gain);  // 设置音量增益

    // ------------------- 音频信号处理函数 -------------------
    void applyGain(int16_t* samples, size_t sampleCount, float gain);
    void normalizeBuffer(int16_t* samples, size_t sampleCount, int16_t maxAmplitude = 32767);   //默认最大幅度为32767
    void convertFloatToInt16(const float* input, int16_t* output, size_t sampleCount);

private:
    // I2S 配置相关
    i2s_port_t              _i2s_num;       // I2S 端口号
    uint32_t                _sampleRate;    // 采样率
    i2s_bits_per_sample_t   _bitsPerSample; // 位深
    i2s_channel_fmt_t       _channelFormat; // 声道格式
    i2s_comm_format_t       _commFormat;    // 通信格式
    int                     _bckPin;        // BCK 引脚
    int                     _wsPin;         // WS 引脚
    int                     _dataOutPin;    // 数据输出引脚
    int                     _dmaBufCount;   // DMA 缓冲区数量
    int                     _dmaBufLen;     // DMA 缓冲区长度
    float                   _ampGain = 1;       // 音量增益
};

// ====================== 实现部分 ======================

// 构造函数
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
      _ampGain(1.0f)  // 默认增益为1.0
{
}

// 析构函数
Megaphone::~Megaphone() {
    i2s_driver_uninstall(_i2s_num);
}

// 初始化 I2S
bool Megaphone::begin()
{
    // I2S 配置
    i2s_config_t i2sConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX), // 主机 + 发送模式
        .sample_rate = _sampleRate,                          // 采样率
        .bits_per_sample = _bitsPerSample,                   // 位深
        .channel_format = _channelFormat,                    // 单声道或双声道
        .communication_format = _commFormat,                 // 标准 I2S 通信格式
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,            // 中断优先级
        .dma_buf_count = _dmaBufCount,                       // DMA 缓冲区数量
        .dma_buf_len = _dmaBufLen,                           // 每个 DMA 缓冲区大小
        .use_apll = false,                                    // 不使用 APLL
        .tx_desc_auto_clear = true,                           // 打开自动清除选项（防止出现全 0 发送阻塞）
        .fixed_mclk = 0                                       // 固定 MCLK，默认 0
    };

    // I2S 引脚配置
    i2s_pin_config_t pinConfig = {
        .bck_io_num = _bckPin,
        .ws_io_num = _wsPin,
        .data_out_num = _dataOutPin,
        .data_in_num = I2S_PIN_NO_CHANGE // 不使用输入
    };

    // 安装 I2S 驱动
    esp_err_t err = i2s_driver_install(_i2s_num, &i2sConfig, 0, NULL);
    if (err != ESP_OK) {
        Serial.println("Megaphone: Failed to install I2S driver");
        return false;
    }

    // 设置 I2S 引脚
    err = i2s_set_pin(_i2s_num, &pinConfig);
    if (err != ESP_OK) {
        Serial.println("Megaphone: Failed to set I2S pins");
        i2s_driver_uninstall(_i2s_num);
        return false;
    }

    Serial.println("Megaphone: I2S initialized successfully");
    return true;
}

// 播放 PCM 数据
size_t Megaphone::playPCM(const int16_t* buffer, size_t sampleCount)
{
    if (!buffer || sampleCount == 0) return 0;

    size_t bytesToWrite = sampleCount * sizeof(int16_t);
    size_t bytesWritten = 0;
    esp_err_t err = i2s_write(_i2s_num, (const char*)buffer, bytesToWrite, &bytesWritten, portMAX_DELAY);

    if (err != ESP_OK) {
        Serial.println("Megaphone: I2S write error");
        return 0;
    }

    return bytesWritten;    // 返回实际写入的字节数
}

// 从文件读取音频数据并播放
void Megaphone::playFromFile(const char *filename) {
    if (!SPIFFS.begin(true)) {  // 初始化 SPIFFS 文件系统，格式化如果挂载失败
        Serial.println("Megaphone: Failed to mount SPIFFS");
        return;
    }

    File audioFile = SPIFFS.open(filename, "r");  // 打开文件进行读取
    if (!audioFile) {  // 文件打开失败
        Serial.println("Megaphone: Failed to open audio file");
        SPIFFS.end();
        return;
    }

    Serial.println("Megaphone: Starting playback...");

    int16_t buffer[256];  // 存储音频数据的缓冲区
    size_t samplesRead;    // 每次读取的采样点数

    // 循环读取文件数据并播放
    while ((samplesRead = audioFile.read((uint8_t*)buffer, sizeof(buffer))) > 0) {
        // 根据音量增益调节音频样本的音量
        applyGain(buffer, samplesRead / sizeof(int16_t), _ampGain);

        // 播放 PCM 数据
        size_t bytesWritten = playPCM(buffer, samplesRead / sizeof(int16_t));
        if (bytesWritten == 0) {
            Serial.println("Megaphone: Playback error");
            break;
        }
    }

    Serial.println("Megaphone: Playback finished.");
    audioFile.close();  // 关闭文件
    SPIFFS.end();       // 结束 SPIFFS 操作
}

// 设置采样率
void Megaphone::setSampleRate(uint32_t sampleRate)
{
    _sampleRate = sampleRate;
}

// 设置位深
void Megaphone::setBitsPerSample(i2s_bits_per_sample_t bitsPerSample)
{
    _bitsPerSample = bitsPerSample;
}

// 设置声道格式
void Megaphone::setChannelFormat(i2s_channel_fmt_t channelFormat)
{
    _channelFormat = channelFormat;
}

// 设置通信格式
void Megaphone::setCommFormat(i2s_comm_format_t commFormat)
{
    _commFormat = commFormat;
}

// 设置引脚
void Megaphone::setPins(int bckPin, int wsPin, int dataOutPin)
{
    _bckPin = bckPin;
    _wsPin = wsPin;
    _dataOutPin = dataOutPin;
}

// 设置音量增益
void Megaphone::setVolume(float gain)
{
    _ampGain = gain;
}

// 应用增益
void Megaphone::applyGain(int16_t* samples, size_t sampleCount, float gain)
{
    if (!samples) return;
    for (size_t i = 0; i < sampleCount; ++i) {
        float temp = samples[i] * gain;
        // 防止溢出，进行限幅
        if (temp > 32767.0f) temp = 32767.0f;
        if (temp < -32768.0f) temp = -32768.0f;
        samples[i] = static_cast<int16_t>(temp);
    }
}

// 归一化缓冲区至指定的最大幅度
void Megaphone::normalizeBuffer(int16_t* samples, size_t sampleCount, int16_t maxAmplitude)
{
    if (!samples || sampleCount == 0) return;

    // 找到当前缓冲区中绝对值最大的样本
    int16_t maxVal = 0;
    for (size_t i = 0; i < sampleCount; i++) {
        int16_t val = abs(samples[i]);
        if (val > maxVal) {
            maxVal = val;
        }
    }

    // 计算缩放比例
    if (maxVal == 0) return;  // 避免除以 0
    float scale = static_cast<float>(maxAmplitude) / maxVal;

    // 应用缩放
    applyGain(samples, sampleCount, scale);
}

// 将 float 数组转换为 int16_t
void Megaphone::convertFloatToInt16(const float* input, int16_t* output, size_t sampleCount)
{
    if (!input || !output) return;

    for (size_t i = 0; i < sampleCount; ++i) {
        // 假设 input 的范围是 -1.0 ~ +1.0
        float temp = input[i] * 32767.0f;
        if (temp > 32767.0f) temp = 32767.0f;
        if (temp < -32768.0f) temp = -32768.0f;
        output[i] = static_cast<int16_t>(temp);
    }
}

#endif // MEGAPHONE_HPP