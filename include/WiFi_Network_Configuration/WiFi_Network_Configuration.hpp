/*
 * @Author: wds-Ubuntu22-cqu wdsnpshy@163.com
 * @Date: 2024-12-08 12:57:58
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2024-12-25 11:07:19
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

#include "static/html.h"        // 提供html表单

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

// 构造函数,初始化ap的ssid和password
WiFi_Network_Configuration::WiFi_Network_Configuration(String apSsid, String apPassword)
{
    this->apSsid = apSsid;
    this->apPassword = apPassword;
}

bool WiFi_Network_Configuration::connectWifi()
{   
    loadWifiFromFlash();  // 加载WiFi信息到缓存
    
    for(const auto& wifi : cache.wifiCredentials) {
        WiFi.begin(wifi.first.c_str(), wifi.second.c_str());
        int count = 0;
        while (WiFi.status() != WL_CONNECTED && count < 10) {
            vTaskDelay(100);
            count++;
        }
        if(WiFi.status() == WL_CONNECTED) {
            return true;
        }
    }
    return false;
}

void WiFi_Network_Configuration::openweb(bool Dns)
{
    // 设置 AP 模式
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSsid.c_str(), apPassword.c_str());
    
    if(Dns)
    {
        dnsServer = new DNSServer();
        dnsServer->start(53, "*", WiFi.softAPIP());
    }
    
    // 各个路由的处理函数
    server.on("/", HTTP_GET, std::bind(&WiFi_Network_Configuration::rootPage, this, std::placeholders::_1));
    server.on("/wifi", HTTP_GET, std::bind(&WiFi_Network_Configuration::wifiPage, this, std::placeholders::_1));
    
    // server.on("/saveWifi", HTTP_POST, std::bind(&WiFi_Network_Configuration::saveWifi, this, std::placeholders::_1));    // 保存wifi信息的路由
    server.on("/saveWifiSuccessfully", HTTP_POST, std::bind(&WiFi_Network_Configuration::saveWifiSuccessfully, this, std::placeholders::_1));

    // server.on("/deleteWifi", HTTP_POST, std::bind(&WiFi_Network_Configuration::deleteWifi, this, std::placeholders::_1));    // 删除wifi信息的路由
    server.on("/deleteWifiSuccessfully", HTTP_POST, std::bind(&WiFi_Network_Configuration::deleteWifiSuccessfully, this, std::placeholders::_1));  

    // server.on("/deleteAllWifi", HTTP_POST, std::bind(&WiFi_Network_Configuration::deleteAllWifi, this, std::placeholders::_1));    // 删除所有wifi信息的路由
    server.on("/deleteAllWifiSuccessfully", HTTP_POST, std::bind(&WiFi_Network_Configuration::deleteAllWifiSuccessfully, this, std::placeholders::_1));

    server.on("/loadWifi", HTTP_GET, std::bind(&WiFi_Network_Configuration::loadWifi, this, std::placeholders::_1)); // 显示wifi列表的路由(显示本地已经保存的wifi信息)
    server.begin();
    
    // 如果启用了 DNS 服务器，需要在主循环中处理 DNS 请求
    if(Dns) {
        xTaskCreate(
            [](void* parameter) {
                WiFi_Network_Configuration* serverInstance = (WiFi_Network_Configuration*)parameter;
                while(1) {
                    if(serverInstance->dnsServer) {
                        serverInstance->dnsServer->processNextRequest();
                    }
                    vTaskDelay(10);
                }
            },
            "dns_task",
            2048,
            this,
            1,
            &dnsTaskHandle
        );
    }
}

void WiFi_Network_Configuration::rootPage(AsyncWebServerRequest *request)
{
    request->send(200, "text/html", ROOT_PAGE); // 返回一个美化后的HTML表单
}

void WiFi_Network_Configuration::wifiPage(AsyncWebServerRequest *request)
{
    request->send(200, "text/html", WIFI_PAGE); // 返回一个美化的HTML表单
}

void WiFi_Network_Configuration::saveWifiSuccessfully(AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
        String ssid = request->getParam("ssid", true)->value();
        String password = request->getParam("password", true)->value();
        
        loadWifiFromFlash();  // 确保缓存是最新的
        
        Preferences preferences;
        preferences.begin("wifi", false);
        
        // 更新缓存
        cache.wifiCredentials.push_back({ssid, password});
        cache.numWifi++;
        
        // 一次性写入flash
        preferences.putString(("ssid" + String(cache.numWifi - 1)).c_str(), ssid);
        preferences.putString(("password" + String(cache.numWifi - 1)).c_str(), password);
        preferences.putInt("numWifi", cache.numWifi);
        preferences.end();
        
        WiFi.begin(ssid.c_str(), password.c_str());
    }
    request->send(200, "text/html", SAVE_WIFI_SUCCESSFULLY_PAGE);
}

void WiFi_Network_Configuration::deleteWifiSuccessfully(AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true)) {
        String ssid = request->getParam("ssid", true)->value();
        
        loadWifiFromFlash();  // 确保缓存是最新的
        
        // 在缓存中查找并删除
        for (size_t i = 0; i < cache.wifiCredentials.size(); i++) {
            if (cache.wifiCredentials[i].first == ssid) {
                cache.wifiCredentials.erase(cache.wifiCredentials.begin() + i);
                cache.numWifi--;
                
                // 一次性重写所有数据到flash
                Preferences preferences;
                preferences.begin("wifi", false);
                for (size_t j = 0; j < cache.wifiCredentials.size(); j++) {
                    preferences.putString(("ssid" + String(j)).c_str(), cache.wifiCredentials[j].first);
                    preferences.putString(("password" + String(j)).c_str(), cache.wifiCredentials[j].second);
                }
                preferences.putInt("numWifi", cache.numWifi);
                preferences.end();
                break;
            }
        }
    }
    request->send(200, "text/html", DELETE_WIFI_SUCCESSFULLY_PAGE);
}

void WiFi_Network_Configuration::deleteAllWifiSuccessfully(AsyncWebServerRequest *request) {
    // 清空缓存
    cache.wifiCredentials.clear();
    cache.numWifi = 0;
    cache.isDirty = false;
    
    // 清空flash
    Preferences preferences;
    preferences.begin("wifi", false);
    preferences.clear();
    preferences.end();
    
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSsid.c_str(), apPassword.c_str());
    
    request->send(200, "text/html", DELETE_ALL_WIFI_SUCCESSFULLY_PAGE);
}

void WiFi_Network_Configuration::loadWifi(AsyncWebServerRequest *request) {
    loadWifiFromFlash();  // 从flash加载到缓存
    
    String wifiList = "";
    for(const auto& wifi : cache.wifiCredentials) {
        wifiList += "<div class='wifi-item'>SSID: " + wifi.first + "</div>";
    }
    
    String html = WIFI_LIST_PAGE;
    html.replace("%WIFI_LIST%", wifiList);
    request->send(200, "text/html", html);
}

#endif
