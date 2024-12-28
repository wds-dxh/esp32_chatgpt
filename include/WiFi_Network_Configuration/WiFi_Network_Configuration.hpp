/*
 * @Author: wds-Ubuntu22-cqu wdsnpshy@163.com
 * @Date: 2024-12-08 12:57:58
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2024-12-29 04:44:54
 * @FilePath: /arduino-esp32/include/ESPAsyncWebServer/web_server.hpp
 * @Description: 
 * 微信: 15310638214 
 * 邮箱：wdsnpshy@163.com 
 * Copyright (c) 2024 by ${wds-Ubuntu22-cqu}, All Rights Reserved. 
 */

/*
1. 根路由：提供一个根路由，返回一个美化后的HTML表单。显示多个功能（现在主要是wifi）
2. wifi路由：处理wifi的连接和断开，并返回当前wifi状态
*/
#ifndef WEB_SERVER_HPP
#define WEB_SERVER_HPP
#include <ESPAsyncWebServer.h>
#include <Preferences.h>        // 用于存储和读取ESP32的闪存(flash)
#include <Arduino.h>            // 提供Arduino库
#include <DNSServer.h>          // 提供DNS库
#include <WiFi.h>               // 提供WiFi库



class WiFi_Network_Configuration
{
private:
    AsyncWebServer server{80};         // 创建AsyncWebServer对象，监听80端口
    DNSServer* dnsServer = nullptr;    // 添加 DNS 服务器指针,用于拦截DNS请求
    TaskHandle_t dnsTaskHandle = nullptr;

    // 定义wifi,保存当前接的wifi信息
    String ssid;
    String password;

    // 定义AP的ssid和password
    String apSsid = "ESP32-AP";
    String apPassword = "12345678";

    // 定义路由返回的函数
    void rootPage(AsyncWebServerRequest *request);                      // 提供函数返回一个美化后的HTML表单
    void wifiPage(AsyncWebServerRequest *request);                      // 提供函数返回一个美化后的HTML表单---主要是wifi相关的设置

    // void saveWifi(AsyncWebServerRequest *request);                    // 提供函数保存wifi信息到闪存,保存后自动连接wifi
    void saveWifiSuccessfully(AsyncWebServerRequest *request);           // 保存wifi成功后返回的路由

    // void deleteWifi(AsyncWebServerRequest *request);                  // 提供函数删除wifi信息
    void deleteWifiSuccessfully(AsyncWebServerRequest *request);         // 删除wifi成功后返回的路由

    // void deleteAllWifi(AsyncWebServerRequest *request);               // 提供函数删除所有wifi信息
    void deleteAllWifiSuccessfully(AsyncWebServerRequest *request);      // 删除所有wifi成功后返回的路由

    void loadWifi(AsyncWebServerRequest *request);                        // 显示wifi列表的路由

    // 添加缓存存储
    struct {
        int numWifi = 0;
        std::vector<std::pair<String, String>> wifiCredentials;
        bool isDirty = true;  // 标记是否需要重新从flash读取
    } cache;

    // 新增方法：从flash加载所有WiFi信息到缓存
    void loadWifiFromFlash() {
        if (!cache.isDirty) return;  // 如果缓存有效，直接返回
        
        Preferences preferences;
        preferences.begin("wifi", true);  // 只读模式
        cache.numWifi = preferences.getInt("numWifi", 0);
        cache.wifiCredentials.clear();
        
        for(int i = 0; i < cache.numWifi; i++) {
            String ssid = preferences.getString(("ssid" + String(i)).c_str(), "");
            String password = preferences.getString(("password" + String(i)).c_str(), "");
            cache.wifiCredentials.push_back({ssid, password});
        }
        preferences.end();
        cache.isDirty = false;
    }

public:
    WiFi_Network_Configuration(String apSsid, String apPassword);          // 构造函数

    bool connectWifi();                                                    // 提供函数链接wifi
    void openweb(bool Dns);                                                // 提供函数打开web服务,参数用于开启dns拦截，让浏览器访问任何地址都指向esp32

    ~WiFi_Network_Configuration() {                                       // 添加析构函数
        if (dnsTaskHandle) {                                               // 删除dns任务
            vTaskDelete(dnsTaskHandle);
            dnsTaskHandle = nullptr;
        }
        if (dnsServer) {                                                   // 删除dns服务器
            delete dnsServer;
            dnsServer = nullptr;
        }
    }
};

#endif
