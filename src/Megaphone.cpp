#include "Megaphone/Megaphone.hpp"

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
      _writerTaskHandle(nullptr),
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
      _compressorRelease(0.1f)
{
}

Megaphone::~Megaphone()
{
    stopWriterTask(); // 确保后台任务先停止
    if (_audioQueue)
    {
        vQueueDelete(_audioQueue);
        _audioQueue = nullptr;
    }
    i2s_driver_uninstall(_i2s_num);
}

// ------------ 初始化 I2S ------------
bool Megaphone::begin()
{
    if (!initI2S())
        return false;

    // 创建队列: 可容纳 QUEUE_LEN 个 AudioChunk
    _audioQueue = xQueueCreate(QUEUE_LEN, sizeof(AudioChunk));
    if (!_audioQueue)
    {
        Serial.println("Megaphone: Failed to create audio queue!");
        return false;
    }
    Serial.println("Megaphone: begin() done. Please call startWriterTask() to run background playback task.");
    return true;
}

bool Megaphone::initI2S()
{
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
        .fixed_mclk = 0};

    i2s_pin_config_t pinConfig = {
        .bck_io_num = _bckPin,
        .ws_io_num = _wsPin,
        .data_out_num = _dataOutPin,
        .data_in_num = I2S_PIN_NO_CHANGE};

    // 安装 I2S 驱动
    esp_err_t err = i2s_driver_install(_i2s_num, &i2sConfig, 0, NULL);
    if (err != ESP_OK)
    {
        Serial.println("Megaphone: Failed to install I2S driver");
        return false;
    }

    // 设置引脚
    err = i2s_set_pin(_i2s_num, &pinConfig);
    if (err != ESP_OK)
    {
        Serial.println("Megaphone: Failed to set I2S pins");
        i2s_driver_uninstall(_i2s_num);
        return false;
    }

    Serial.println("Megaphone: I2S initialized successfully");
    return true;
}

// ------------ 后台任务的启动和停止 ------------
bool Megaphone::startWriterTask()
{
    if (_writerTaskHandle)
    {
        Serial.println("Megaphone: i2sWriterTask already running!");
        return false;
    }
    if (!_audioQueue)
    {
        Serial.println("Megaphone: No queue created, call begin() first!");
        return false;
    }
    xTaskCreatePinnedToCore(
        i2sWriterTask,
        "i2sWriterTask",
        4096,
        this,
        1, // 优先级
        &_writerTaskHandle,
        1 // Core ID
    );
    if (_writerTaskHandle)
    {
        Serial.println("Megaphone: i2sWriterTask started!");
        return true;
    }
    Serial.println("Megaphone: Failed to create i2sWriterTask!");
    return false;
}

bool Megaphone::stopWriterTask()
{
    if (_writerTaskHandle)
    {
        vTaskDelete(_writerTaskHandle);
        _writerTaskHandle = nullptr;
        // 清空队列
        AudioChunk chunk;
        while (xQueueReceive(_audioQueue, &chunk, 0) == pdTRUE)
        {
            free(chunk.data);
        }
        Serial.println("Megaphone: i2sWriterTask stopped!");
        return true;
    }
    return false;
}

// ------------ 播放 PCM 数据（原始阻塞）会附加增益 ------------
size_t Megaphone::playPCM(const int16_t *buffer, size_t sampleCount)
{
    if (!buffer || sampleCount == 0)
        return 0;

    size_t bytesToWrite = sampleCount * sizeof(int16_t);
    size_t bytesWritten = 0;
    esp_err_t err = i2s_write(_i2s_num, (const char *)buffer, bytesToWrite, &bytesWritten, portMAX_DELAY);
    if (err != ESP_OK)
    {
        Serial.println("Megaphone: I2S write error");
        return 0;
    }
    return bytesWritten;
}

// ------------ 播放 PCM 数据（内部处理+阻塞） ------------
size_t Megaphone::playPCMProcessed(const int16_t *buffer, size_t sampleCount)
{
    if (!buffer || sampleCount == 0)
        return 0;
    int16_t *tmp = (int16_t *)malloc(sampleCount * sizeof(int16_t));
    if (!tmp)
        return 0;

    memcpy(tmp, buffer, sampleCount * sizeof(int16_t));
    processAudioBuffer(tmp, sampleCount);

    size_t written = playPCM(tmp, sampleCount);
    free(tmp);
    return written;
}

// ------------ 非阻塞队列接口 ------------
size_t Megaphone::queuePCM(const int16_t *buffer, size_t sampleCount, bool isLast)
{
    if (!_audioQueue || !buffer || sampleCount == 0)
        return 0;

    int16_t *dataCopy = (int16_t *)malloc(sampleCount * sizeof(int16_t));
    if (!dataCopy)
    {
        Serial.println("Megaphone: Malloc failed in queuePCM!");
        return 0;
    }
    memcpy(dataCopy, buffer, sampleCount * sizeof(int16_t));

    AudioChunk chunk;
    chunk.data = dataCopy;
    chunk.size = sampleCount;
    chunk.isLast = isLast;

    // 这里使用0超时(不阻塞)或portMAX_DELAY都可，看你需求
    // 若用0则队列满时立即失败
    if (xQueueSend(_audioQueue, &chunk, 0) == pdTRUE)
    {
        return sampleCount;
    }
    else
    {
        free(dataCopy);
        return 0;
    }
}

// ------------ 从文件读取并播放(阻塞示例) ------------
void Megaphone::playFromFile(const char *filename)
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("Megaphone: Failed to mount SPIFFS");
        return;
    }

    File audioFile = SPIFFS.open(filename, "r");
    if (!audioFile)
    {
        Serial.println("Megaphone: Failed to open audio file");
        SPIFFS.end();
        return;
    }

    Serial.println("Megaphone: Starting playback from file...");
    int16_t buffer[256];
    size_t bytesRead;

    while ((bytesRead = audioFile.read((uint8_t *)buffer, sizeof(buffer))) > 0)
    {
        size_t samples = bytesRead / sizeof(int16_t);
        // 阻塞播放 (也可改用 queuePCM 让后台播放)
        AudioProcessor::applyGain(buffer, samples, _ampGain);
        playPCM(buffer, samples);
    }

    audioFile.close();
    SPIFFS.end();
    Serial.println("Megaphone: Playback finished.");
}

// ------------ 参数设置 ------------
void Megaphone::setSampleRate(uint32_t sampleRate)
{
    _sampleRate = sampleRate;
}
void Megaphone::setBitsPerSample(i2s_bits_per_sample_t bitsPerSample)
{
    _bitsPerSample = bitsPerSample;
}
void Megaphone::setChannelFormat(i2s_channel_fmt_t channelFormat)
{
    _channelFormat = channelFormat;
}
void Megaphone::setCommFormat(i2s_comm_format_t commFormat)
{
    _commFormat = commFormat;
}
void Megaphone::setPins(int bckPin, int wsPin, int dataOutPin)
{
    _bckPin = bckPin;
    _wsPin = wsPin;
    _dataOutPin = dataOutPin;
}
void Megaphone::setVolume(float gain)
{
    _ampGain = gain;
}

// ------------ 播放控制(isPlaying标志) ------------
bool Megaphone::isPlaying() const
{
    return _isPlaying;
}

// ============ 音频效果开关 ============
void Megaphone::enableEcho(bool enable, float delaySeconds, float decay)
{
    _echoEnabled = enable;
    _echoDelay = delaySeconds;
    _echoDecay = decay;
}
void Megaphone::enableReverb(bool enable, const float *ir, size_t irLen)
{
    _reverbEnabled = enable;
    _reverbImpulse = ir;
    _reverbLen = irLen;
}
void Megaphone::enableCompressor(bool enable, float threshold, float ratio, float attack, float release)
{
    _compressorEnabled = enable;
    _compressorThreshold = threshold;
    _compressorRatio = ratio;
    _compressorAttack = attack;
    _compressorRelease = release;
}

// ------------ 清空DMA缓冲 ------------
void Megaphone::clearBuffer()
{
    i2s_zero_dma_buffer(_i2s_num);
    Serial.println("Megaphone: DMA buffer cleared.");
}

// ------------ 获取队列可用空间 ------------
size_t Megaphone::getBufferFree() const
{
    if (!_audioQueue)
        return 0;
    return uxQueueSpacesAvailable(_audioQueue);
}

// ------------ 回调 ------------
void Megaphone::setPlaybackCallback(PlaybackCallback callback, void *context)
{
    _callback = callback;
    _callbackContext = context;
}

// ------------ 音频处理(音量+效果等) ------------
void Megaphone::processAudioBuffer(int16_t *buffer, size_t sampleCount)
{
    // 1. 音量增益
    if (_ampGain != 1.0f)
    {
        AudioProcessor::applyGain(buffer, sampleCount, _ampGain);
    }
    // 2. 压缩
    if (_compressorEnabled)
    {
        AudioProcessor::applyCompressor(buffer, sampleCount,
                                        _compressorThreshold, _compressorRatio,
                                        _compressorAttack, _compressorRelease);
    }
    // 3. 回声
    if (_echoEnabled)
    {
        AudioProcessor::applyEcho(buffer, sampleCount, _echoDelay, _echoDecay);
    }
    // 4. 混响
    if (_reverbEnabled && _reverbImpulse && _reverbLen > 0)
    {
        AudioProcessor::applyReverb(buffer, sampleCount, _reverbImpulse, _reverbLen);
    }
}


// ------------ 后台任务 ------------
void Megaphone::i2sWriterTask(void *parameter)
{
    Megaphone *self = static_cast<Megaphone *>(parameter);
    int star_pal = 0;
    while (true)
    {
        AudioChunk chunk;
        // 当队列中有十个数据是，才开始播放

        if (self->getBufferFree() < 41)
        {
            star_pal = 1;
        }
        if (star_pal == 1)
        {

            if (xQueueReceive(self->_audioQueue, &chunk, portMAX_DELAY) == pdTRUE)
            {
                {
                    if (!chunk.data || chunk.size == 0)
                    {
                        if (chunk.data)
                            free(chunk.data);
                        continue;
                    }

                    // 处理(音量/效果等)
                    // self->processAudioBuffer(chunk.data, chunk.size);

                    // 写I2S(阻塞)
                    size_t bytesWritten = 0;
                    // 应用增益
                    AudioProcessor::applyGain(chunk.data, chunk.size, self->_ampGain);
                    self->playPCM(chunk.data, chunk.size);

                    free(chunk.data);

                    // 如果是最后一块，则触发回调(如果已设置)
                    if (chunk.isLast && self->_callback)
                    {
                        self->_isPlaying = false;

                        self->_callback(self->_callbackContext);
                    }
                }
            }
        }
    }
}
