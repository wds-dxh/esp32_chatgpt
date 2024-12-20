/*
 * @Author: wds-Ubuntu22-cqu wdsnpshy@163.com
 * @Date: 2024-12-08 12:57:58
 * @LastEditors: wds-Ubuntu22-cqu wdsnpshy@163.com
 * @LastEditTime: 2024-12-08 19:00:55
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
#include <Arduino.h>           // 提供Arduino库
#include <DNSServer.h>         // 提供DNS库
#include <WiFi.h>              // 提供WiFi库


#include "static/html.h"              // 提供html表单
class WebServer
{
private:
    AsyncWebServer server{80};  // 创建AsyncWebServer对象，监听80端口
    DNSServer* dnsServer = nullptr;  // 添加 DNS 服务器指针,用于拦截DNS请求
    TaskHandle_t dnsTaskHandle = nullptr;

    //定义wifi,保存当前接的wifi信息
    String ssid;
    String password;

    //定义AP的ssid和password
    String apSsid = "ESP32-AP";
    String apPassword = "12345678";

    //定义路由返回的函数
    void rootPage(AsyncWebServerRequest *request); // 提供函数返回一个美化后的HTML表单
    void wifiPage(AsyncWebServerRequest *request); // 提供函数返回一个美化后的HTML表单---主要是wifi相关的设置

    // void saveWifi(AsyncWebServerRequest *request); // 提供函数保存wifi信息到闪存,保存后自动连接wifi
    void saveWifiSuccessfully(AsyncWebServerRequest *request); // 保存wifi成功后返回的路由

    // void deleteWifi(AsyncWebServerRequest *request); // 提供函数删除wifi信息
    void deleteWifiSuccessfully(AsyncWebServerRequest *request); // 删除wifi成功后返回的路由

    // void deleteAllWifi(AsyncWebServerRequest *request); // 提供函数删除所有wifi信息
    void deleteAllWifiSuccessfully(AsyncWebServerRequest *request); // 删除所有wifi成功后返回的路由

    void loadWifi(AsyncWebServerRequest *request); // 显示wifi列表的路由

    
public:
    WebServer(String apSsid, String apPassword); //构造函数
    
    bool connectWifi(); //提供函数链接wifi
    void openweb(bool Dns); //提供函数打开web服务,参数用于开启dns拦截，让浏览器访问任何地址都指向esp32

    ~WebServer() {  // 添加析构函数
        if (dnsTaskHandle) {    // 删除dns任务
            vTaskDelete(dnsTaskHandle);
            dnsTaskHandle = nullptr;
        }
        if (dnsServer) {    // 删除dns服务器
            delete dnsServer;
            dnsServer = nullptr;
        }
    }
};

WebServer::WebServer(String apSsid, String apPassword)  //构造函数,初始化ap的ssid和password
{
    this->apSsid = apSsid;
    this->apPassword = apPassword;
}

bool WebServer::connectWifi()
{   
    Preferences preferences; // 用于存储和读取ESP32的闪存(flash)
    //从闪存中读取wifi信息
    preferences.begin("wifi", false);
    int numWifi = preferences.getInt("numWifi", 0);
    
    //读取wifi信息
    for(int i = 0; i < numWifi; i++)
    {
        ssid = preferences.getString(("ssid" + String(i)).c_str(), ""); //参数1：key，参数2：默认值
        password = preferences.getString(("password" + String(i)).c_str(), "");
        // Serial.println("ssid: " + ssid + " password: " + password);
        //连接wifi
        WiFi.begin(ssid.c_str(), password.c_str());
        //等待连接成功
        int count = 0;
        while (WiFi.status() != WL_CONNECTED)
        {
            vTaskDelay(100);    // 等待100ms
            count++;
            if(count > 10)
            {
                Serial.println("连接wifi失败");
                break;
            }
        }
        if(WiFi.status() == WL_CONNECTED)
        {
            return true;
        }
    }
    return false;
}

void WebServer::openweb(bool Dns)
{
    // 设置 AP 模式
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSsid.c_str(), apPassword.c_str());
    
    if(Dns)
    {
        dnsServer = new DNSServer();
        dnsServer->start(53, "*", WiFi.softAPIP());
    }
    
    //各个路由的处理函数
    server.on("/", HTTP_GET, std::bind(&WebServer::rootPage, this, std::placeholders::_1));
    server.on("/wifi", HTTP_GET, std::bind(&WebServer::wifiPage, this, std::placeholders::_1));
    
    // server.on("/saveWifi", HTTP_POST, std::bind(&WebServer::saveWifi, this, std::placeholders::_1));    // 保存wifi信息的路由
    server.on("/saveWifiSuccessfully", HTTP_POST, std::bind(&WebServer::saveWifiSuccessfully, this, std::placeholders::_1));

    // server.on("/deleteWifi", HTTP_POST, std::bind(&WebServer::deleteWifi, this, std::placeholders::_1));    // 删除wifi信息的路由
    server.on("/deleteWifiSuccessfully", HTTP_POST, std::bind(&WebServer::deleteWifiSuccessfully, this, std::placeholders::_1));  

    // server.on("/deleteAllWifi", HTTP_POST, std::bind(&WebServer::deleteAllWifi, this, std::placeholders::_1));    // 删除所有wifi信息的路由
    server.on("/deleteAllWifiSuccessfully", HTTP_POST, std::bind(&WebServer::deleteAllWifiSuccessfully, this, std::placeholders::_1));

    server.on("/loadWifi", HTTP_GET, std::bind(&WebServer::loadWifi, this, std::placeholders::_1)); // 显示wifi列表的路由(显示本地已经保存的wifi信息)
    server.begin();
    
    // 如果启用了 DNS 服务器，需要在主循环中处理 DNS 请求
    if(Dns) {
        xTaskCreate(
            [](void* parameter) {
                WebServer* server = (WebServer*)parameter;
                while(1) {
                    if(server->dnsServer) {
                        server->dnsServer->processNextRequest();
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

void WebServer::rootPage(AsyncWebServerRequest *request)
{
    request->send(200, "text/html", ROOT_PAGE); //返回一个美化后的HTML表单
}

void WebServer::wifiPage(AsyncWebServerRequest *request)
{
    request->send(200, "text/html", WIFI_PAGE); //返回一个美化的HTML表单
}

void WebServer::saveWifiSuccessfully(AsyncWebServerRequest *request) {
    // 从请求中获取WiFi信息
    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
        String ssid = request->getParam("ssid", true)->value();
        String password = request->getParam("password", true)->value();
        
        // 保存到Preferences
        Preferences preferences;
        preferences.begin("wifi", false);
        int numWifi = preferences.getInt("numWifi", 0);
        
        // 保存新的WiFi信息
        preferences.putString(("ssid" + String(numWifi)).c_str(), ssid);
        preferences.putString(("password" + String(numWifi)).c_str(), password);
        preferences.putInt("numWifi", numWifi + 1);
        preferences.end();
        
        // 尝试连接新的WiFi
        WiFi.begin(ssid.c_str(), password.c_str());
    }
    
    request->send(200, "text/html", SAVE_WIFI_SUCCESSFULLY_PAGE);
}

void WebServer::deleteWifiSuccessfully(AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true)) {
        String ssid = request->getParam("ssid", true)->value();
        
        // 从Preferences中删除指定的WiFi
        Preferences preferences;
        preferences.begin("wifi", false);
        int numWifi = preferences.getInt("numWifi", 0);
        
        // 查找并删除指定的WiFi
        for (int i = 0; i < numWifi; i++) {
            String currentSsid = preferences.getString(("ssid" + String(i)).c_str(), "");
            if (currentSsid == ssid) {
                // 移动后面的WiFi信息前移
                for (int j = i; j < numWifi - 1; j++) {
                    String nextSsid = preferences.getString(("ssid" + String(j + 1)).c_str(), "");
                    String nextPassword = preferences.getString(("password" + String(j + 1)).c_str(), "");
                    preferences.putString(("ssid" + String(j)).c_str(), nextSsid);
                    preferences.putString(("password" + String(j)).c_str(), nextPassword);
                }
                preferences.putInt("numWifi", numWifi - 1);
                break;
            }
        }
        preferences.end();
    }
    
    request->send(200, "text/html", DELETE_WIFI_SUCCESSFULLY_PAGE);
}

void WebServer::deleteAllWifiSuccessfully(AsyncWebServerRequest *request) {
    // 除所有保存的WiFi信息
    Preferences preferences;
    preferences.begin("wifi", false);
    preferences.clear();
    preferences.end();
    
    // 断开当前WiFi连接
    WiFi.disconnect();
    
    // 设置为AP模式
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSsid.c_str(), apPassword.c_str());
        
    request->send(200, "text/html", DELETE_ALL_WIFI_SUCCESSFULLY_PAGE);
}

void WebServer::loadWifi(AsyncWebServerRequest *request) {
    Preferences preferences;
    preferences.begin("wifi", true);  // 只读模式打开
    int numWifi = preferences.getInt("numWifi", 0);
    
    String wifiList = "";
    for(int i = 0; i < numWifi; i++) {  
        String ssid = preferences.getString(("ssid" + String(i)).c_str(), "");
        wifiList += "<div class='wifi-item'>SSID: " + ssid + "</div>";
    }
    preferences.end();
    
    String html = WIFI_LIST_PAGE;
    html.replace("%WIFI_LIST%", wifiList);  // 替换html中的%WIFI_LIST%为wifiList
    request->send(200, "text/html", html);
}

#endif

