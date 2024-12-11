# MicRecorder 类说明文档

## 概述

`MicRecorder` 类提供了一个封装的接口来与 I2S 麦克风进行交互，允许用户获取音频数据并进行基本的音频信号处理。该类隐藏了复杂的 I2S 初始化和数据采集细节，提供了简单易用的 API。

## 功能

- **初始化**：配置和初始化 I2S 驱动
- **数据采集**：读取 PCM 音频数据
- **音频处理**：提供 RMS 计算、增益控制、归一化等音频处理功能
- **参数配置**：支持采样率、位深度等参数的动态配置

## 主要函数介绍

### 构造函数

```cpp
MicRecorder::MicRecorder(uint32_t sampleRate = DEFAULT_SAMPLE_RATE,
                        i2s_bits_per_sample_t bitsPerSample = DEFAULT_BITS_PER_SAMPLE,
                        i2s_channel_fmt_t channelFormat = DEFAULT_CHANNEL_FORMAT,
                        i2s_comm_format_t commFormat = DEFAULT_COMM_FORMAT,
                        int bckPin = DEFAULT_BCK_PIN,
                        int wsPin = DEFAULT_WS_PIN,
                        int dataInPin = DEFAULT_DATA_IN_PIN)
```

- **简介**：初始化 MicRecorder 实例，设置音频采��参数
- **参数**：
  - `sampleRate`：采样率（默认 16kHz）
  - `bitsPerSample`：采样位深（默认 16 位）
  - `channelFormat`：通道格式（默认仅左声道）
  - `commFormat`：通信格式（默认 I2S 标准）
  - `bckPin`：I2S BCK 引脚
  - `wsPin`：I2S WS 引脚
  - `dataInPin`：I2S DATA 引脚

### 初始化函数

```cpp
bool begin()
```

- **简介**：初始化 I2S 驱动和硬件配置
- **返回值**：初始化成功返回 true，失败返回 false

### 数据采集函数

```cpp
size_t readPCM(int16_t* buffer, size_t maxSamples)
```

- **简介**：读取 PCM 音频数据
- **参数**：
  - `buffer`：存储音频数据的缓冲区
  - `maxSamples`：最大采样点数
- **返回值**：实际读取的采样点数

### 音频处理函数

```cpp
float calculateRMS(const int16_t* samples, size_t sampleCount)
void applyGain(int16_t* samples, size_t sampleCount, float gain)
void normalizeBuffer(int16_t* samples, size_t sampleCount, int16_t maxAmplitude = 32767)
void convertInt16ToFloat(const int16_t* input, float* output, size_t sampleCount)
```

- **calculateRMS**：计算音频数据的均方根值
- **applyGain**：对音频数据应用增益
- **normalizeBuffer**：对音频数据进行归一化处理
- **convertInt16ToFloat**：将整型音频数据转换为浮点数��式

## 示例代码

```cpp
#include "MicRecorder.hpp"

void setup() {
    Serial.begin(115200);
    
    MicRecorder mic;  // 使用默认参数创建实例
    
    if (!mic.begin()) {
        Serial.println("MicRecorder initialization failed!");
        while (1);
    }
    
    Serial.println("MicRecorder initialized successfully!");
}

void loop() {
    const size_t BUFFER_SIZE = 1024;
    int16_t buffer[BUFFER_SIZE];
    
    size_t samplesRead = mic.readPCM(buffer, BUFFER_SIZE);
    
    if (samplesRead > 0) {
        float rms = mic.calculateRMS(buffer, samplesRead);
        Serial.printf("Audio RMS: %.2f\n", rms);
    }
    
    delay(100);
}
```

## 注意事项

1. **硬件连接**：
   - 确保 I2S 麦克风模块正确连接到指定的引脚
   - 检查电源供应是否稳定

2. **采样率设置**：
   - 默认采样率为 16kHz，适合语音识别应用
   - 可根据需要调整采样率，但要注意 ESP32 的处理能力限制

3. **缓冲区管理**：
   - 读取数据时确保缓冲区大小足够
   - 避免缓冲区溢出

4. **性能考虑**：
   - 音频处理函数可能较为耗时，建议在必要时使用
   - 考虑使用任务调度来处理音频数据，避免阻塞主循环

5. **错误处理**：
   - 初始化失败时及时检查硬件连接和配置
   - 监控数据读取的返回值，确保采集正常