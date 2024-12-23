#include <Arduino.h>
#include <SPIFFS.h>
#include "driver/i2s.h"

class SpeakerController {
public:
    SpeakerController() : ampGain(1) {}
    ~SpeakerController() {
        i2s_driver_uninstall(i2sPort); // 卸载 I2S 驱动
    }

    // 初始化 I2S，设置采样率、位深和通道
    int begin(uint32_t sampleRate = 24000, i2s_bits_per_sample_t bitsPerSample = I2S_BITS_PER_SAMPLE_16BIT) {
        esp_err_t err;
        
        // 配置 I2S
        const i2s_config_t i2sConfig = {
            .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),  // 主模式，数据传输
            .sample_rate = sampleRate,  // 采样率，例如 24000Hz
            .bits_per_sample = bitsPerSample,  // 每个样本的位数
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,  // 单声道
            .communication_format = I2S_COMM_FORMAT_STAND_I2S, 
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,  // 中断级别
            .dma_buf_count = 16,  // DMA 缓冲区数量
            .dma_buf_len = 60,  // 每个 DMA 缓冲区的长度
            .use_apll = false  // 不使用 APLL（音频 PLL）
        };

        // 配置 I2S 引脚
        const i2s_pin_config_t pinConfig = {
            .bck_io_num = 26,  // BCLK 引脚
            .ws_io_num = 27,   // LRC 引脚
            .data_out_num = 25,// 数据输出引脚
            .data_in_num = -1  // 数据输入不使用
        };

        // 安装 I2S 驱动
        err = i2s_driver_install(i2sPort, &i2sConfig, 0, NULL);
        if (err != ESP_OK) return -1;

        // 配置 I2S 引脚
        err = i2s_set_pin(i2sPort, &pinConfig);
        if (err != ESP_OK) return -2;

        return 0;
    }

    // 设置音量增益
    void setVolume(uint8_t gain) {
        ampGain = gain;
    }

    // 从文件读取音频数据并播放
    void playFromFile(const char *filename) {
        if (!SPIFFS.begin()) {  // 初始化 SPIFFS 文件系统
            Serial.println("Failed to mount SPIFFS");  // 文件系统挂载失败
            return;
        }

        File audioFile = SPIFFS.open(filename, "r");  // 打开文件进行读取
        if (!audioFile) {  // 文件打开失败
            Serial.println("Failed to open audio file");
            return;
        }

        Serial.println("Starting playback...");
        
        int16_t buffer[256];  // 存储音频数据的缓冲区
        size_t bytesRead;  // 每次读取的字节数

        // 循环读取文件数据并播放
        while ((bytesRead = audioFile.readBytes((char *)buffer, sizeof(buffer))) > 0) {
            // 根据音量增益调节音频样本的音量
            for (size_t i = 0; i < bytesRead / 2; ++i) {  // 每个样本是 2 字节（16 位）
                buffer[i] = buffer[i] * ampGain;  // 应用音量增益
            }
            
            size_t bytesWritten;  // 实际写入的字节数
            i2s_write(i2sPort, buffer, bytesRead, &bytesWritten, portMAX_DELAY);  // 写入数据到 I2S 播放
        }

        Serial.println("Playback finished.");
        audioFile.close();  // 关闭文件
        SPIFFS.end();  // 结束 SPIFFS 操作
    }

private:
    uint8_t ampGain;  // 音量增益
    const i2s_port_t i2sPort = I2S_NUM_0;  // I2S 端口
};

// 创建一个 SpeakerController 实例
SpeakerController speaker;

void setup() {
    Serial.begin(115200);  // 初始化串口

    // 初始化扬声器
    if (speaker.begin(24000, I2S_BITS_PER_SAMPLE_16BIT) != 0) {  // 设置为 24kHz 采样率和 16-bit 位深
        Serial.println("Failed to initialize speaker");
        return;
    }

    speaker.setVolume(1);  // 设置音量增益为 1

    // 播放存储在 Flash 中的 PCM 文件
    speaker.playFromFile("/output.pcm");  // 替换为您上传的音频文件名称
}

void loop() {
    // 循环中不需要做任何操作
}
