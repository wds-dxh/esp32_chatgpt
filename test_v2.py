import asyncio
from bleak import BleakScanner, BleakClient
import json
from enum import Enum

class Status(Enum):
    OK = "OK"
    NOT_OK = "NOT_OK"

class BLEConfig:
    def __init__(self):
        self.device = None
        self.client = None
        self.write_characteristic = None
        self.notify_characteristic = None
        self.notification_received = asyncio.Event()
        self.last_notification = None
        self.retry_count = 3

    def notification_handler(self, sender, data):
        """处理从设备接收到的通知"""
        try:
            message = json.loads(data.decode())
            print(f"收到通知: {message}")
            
            if message.get("type") == "wifi_config":
                status = message.get("status")
                error_msg = message.get("error_message", "")
                
                if status == Status.OK.value:
                    print("配网成功:", error_msg)
                else:
                    print("配网失败:", error_msg)
            
            self.last_notification = message
            self.notification_received.set()
            
        except json.JSONDecodeError:
            print("通知数据解析错误")

    async def scan_device(self, device_name="ai-toys"):
        """扫描指定名称的设备"""
        print(f"正在扫描设备 {device_name}...")
        devices = await BleakScanner.discover()
        
        for device in devices:
            if device.name and device.name == device_name:
                print(f"找到设备: {device.name}")
                print(f"地址: {device.address}")
                self.device = device
                return True
                
        print(f"未找到设备: {device_name}")
        return False

    async def find_characteristics(self, client):
        """动态查找可用的特征值"""
        for service in client.services:
            print(f"发现服务: {service.uuid}")
            for char in service.characteristics:
                print(f"  特征值: {char.uuid}")
                print(f"  属性: {char.properties}")
                
                # 查找具有写入权限的特征值
                if "write" in char.properties:
                    print(f"找到写入特征值: {char.uuid}")
                    self.write_characteristic = char
                    
                # 查找具有notify权限的特征值
                if "notify" in char.properties:
                    print(f"找到通知特征值: {char.uuid}")
                    self.notify_characteristic = char

        return self.write_characteristic is not None and self.notify_characteristic is not None

    async def connect_and_configure(self, wifi_ssid, wifi_password, timeout=30):
        """连接设备并发送WiFi配置"""
        if not self.device:
            print("未找到设备")
            return False

        for attempt in range(self.retry_count):
            try:
                print(f"尝试连接 (第{attempt + 1}次)")
                async with BleakClient(self.device.address, timeout=20) as client:
                    print(f"已连接到 {self.device.name}")
                    
                    # 等待服务发现完成
                    await asyncio.sleep(2)
                    
                    if not await self.find_characteristics(client):
                        print("未找到所需的特征值")
                        continue

                    # 配置通知前等待
                    await asyncio.sleep(2)
                    
                    try:
                        await client.start_notify(
                            self.notify_characteristic.uuid,
                            self.notification_handler
                        )
                        print("通知已启用")
                    except Exception as e:
                        print(f"启用通知失败: {e}")
                        continue

                    # 发送配置并等待响应
                    await self.send_config_and_wait(client, wifi_ssid, wifi_password, timeout)
                    return True

            except Exception as e:
                print(f"连接尝试 {attempt + 1} 失败: {e}")
                await asyncio.sleep(2)

        print("所有连接尝试均失败")
        return False

    async def send_config_and_wait(self, client, wifi_ssid, wifi_password, timeout):
        """发送配置并等待响应"""
        wifi_config = {
            "ssid": wifi_ssid,
            "password": wifi_password
        }
        
        self.notification_received.clear()
        print("正在发送WiFi配置...")
        
        await client.write_gatt_char(
            self.write_characteristic.uuid,
            json.dumps(wifi_config).encode()
        )

        try:
            await asyncio.wait_for(
                self.notification_received.wait(),
                timeout=timeout
            )
        except asyncio.TimeoutError:
            print(f"等待响应超时 ({timeout}秒)")
            return False

        return True

async def main():
    # WiFi配置信息
    WIFI_SSID = "wds"      # 替换为实际的WiFi名称
    WIFI_PASSWORD = "wds666666"     # 替换为实际的WiFi密码

    ble_config = BLEConfig()

    # 扫描并配网
    if await ble_config.scan_device():
        success = await ble_config.connect_and_configure(
            WIFI_SSID, 
            WIFI_PASSWORD
        )
        if success:
            print("配网流程完成")
        else:
            print("配网失败")

if __name__ == "__main__":
    asyncio.run(main())