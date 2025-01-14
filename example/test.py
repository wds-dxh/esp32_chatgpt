'''
Author: wds-dxh wdsnpshy@163.com
Date: 2024-12-09 01:45:20
LastEditors: Please set LastEditors
LastEditTime: 2025-01-14 17:59:58
FilePath: /arduino-esp32/test.py
Description: 第一个版本测试蓝牙配网
微信: 15310638214 
邮箱：wdsnpshy@163.com 
Copyright (c) 2024 by ${wds-dxh}, All Rights Reserved. 
'''
import asyncio
from bleak import BleakScanner, BleakClient
import json


class BLEConfig:
    def __init__(self):
        self.device = None
        self.client = None
        self.write_characteristic = None

    async def scan_device(self, device_name_prefix="ESP32-BLE"):
        """扫描ESP32设备"""
        print("正在扫描蓝牙设备...")
        devices = await BleakScanner.discover()

        for device in devices:
            if device.name and device.name.startswith(device_name_prefix):
                print(f"找到ESP32设备: {device.name}")
                print(f"地址: {device.address}")
                self.device = device
                return True
        print("未找到ESP32设备")
        return False

    async def connect_and_configure(self, wifi_ssid, wifi_password):
        """连接设备并发送WiFi配置"""
        if not self.device:
            print("未找到设备")
            return False

        try:
            # 连接到设备
            async with BleakClient(self.device.address) as client:
                print(f"已连接到 {self.device.name}")

                # 获取所有服务
                services = client.services
                write_characteristic = None

                # 查找具有写入权限的特征值
                for service in services:
                    for char in service.characteristics:
                        if "write" in char.properties:
                            write_characteristic = char
                            break
                    if write_characteristic:
                        break

                if not write_characteristic:
                    print("未找到可写入的特征值")
                    return False

                # 准备WiFi配置数据
                wifi_config = {
                    "ssid": wifi_ssid,
                    "password": wifi_password
                }

                # 发送配置
                print("正在发送WiFi配置...")
                await client.write_gatt_char(
                    write_characteristic.uuid,
                    json.dumps(wifi_config).encode()
                )
                print("WiFi配置已发送")
                return True

        except Exception as e:
            print(f"配置过程出错: {str(e)}")
            return False


async def main():
    # WiFi配置信息
    WIFI_SSID = "Your_WiFi_SSID"  # 替换为你的WiFi名称
    WIFI_PASSWORD = "Your_WiFi_Password"  # 替换为你的WiFi密码

    ble_config = BLEConfig()

    # 扫描设备
    if await ble_config.scan_device():
        # 发送配置
        success = await ble_config.connect_and_configure(WIFI_SSID, WIFI_PASSWORD)
        if success:
            print("配网成功！")
        else:
            print("配网失败！")

# 运行主程序
if __name__ == "__main__":
    # 安装方法：pip install bleak
    asyncio.run(main())
