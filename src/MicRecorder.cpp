#include "MicRecorder/MicRecorder.hpp"

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

