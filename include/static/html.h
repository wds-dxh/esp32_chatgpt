/*
 * @Author: wds-Ubuntu22-cqu wdsnpshy@163.com
 * @Date: 2024-12-08 17:32:50
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2024-12-30 12:29:04
 * @FilePath: /arduino-esp32/include/ESPAsyncWebServer/html.h
 * @Description: 提供各个路由的html，简化web_server.hpp的代码
 * 微信: 15310638214 
 * 邮箱：wdsnpshy@163.com 
 * Copyright (c) 2024 by ${wds-Ubuntu22-cqu}, All Rights Reserved. 
 */
#pragma once

#include <Arduino.h>

// 提供根路由的HTML表单: 1.跳转到wifi设置页面
const char* ROOT_PAGE = R"(
<!DOCTYPE html>
<html lang="zh">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 控制面板</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f0f2f5;
        }
        .container {
            max-width: 600px;
            margin: 0 auto;
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 30px;
        }
        .menu-item {
            display: block;
            padding: 15px 20px;
            margin: 10px 0;
            background-color: #1890ff;
            color: white;
            text-decoration: none;
            border-radius: 5px;
            text-align: center;
            transition: background-color 0.3s;
        }
        .menu-item:hover {
            background-color: #40a9ff;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 控制面板</h1>
        <a href="/wifi" class="menu-item">WiFi 设置</a>
    </div>
</body>
</html>
)";

// 提供wifi设置的HTML表单：1. 保存wifi信息 2. 删除wifi信息 3. 删除所有wifi信息
const char* WIFI_PAGE = R"(
<!DOCTYPE html>
<html lang="zh">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WiFi 设置</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f0f2f5;
        }
        .container {
            max-width: 600px;
            margin: 0 auto;
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        h1, h2 {
            color: #333;
            text-align: center;
        }
        .form-group {
            margin-bottom: 15px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            color: #666;
        }
        input[type="text"],
        input[type="password"] {
            width: 100%;
            padding: 8px;
            border: 1px solid #ddd;
            border-radius: 4px;
            box-sizing: border-box;
        }
        .btn {
            display: inline-block;
            padding: 10px 20px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 14px;
            margin: 5px;
            text-align: center;
            text-decoration: none;
            transition: background-color 0.3s;
        }
        .btn-primary {
            background-color: #1890ff;
            color: white;
        }
        .btn-danger {
            background-color: #ff4d4f;
            color: white;
        }
        .btn-primary:hover {
            background-color: #40a9ff;
        }
        .btn-danger:hover {
            background-color: #ff7875;
        }
        .wifi-list {
            margin: 20px 0;
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        .back-link {
            display: block;
            text-align: center;
            margin-top: 20px;
            color: #1890ff;
            text-decoration: none;
        }
        .back-link:hover {
            text-decoration: underline;
        }
        .button-group {
            display: flex;
            justify-content: center;
            gap: 10px;
            margin-top: 15px;
        }
        .delete-all-btn {
            width: 100%;
            margin-top: 30px;
        }
        .wifi-item {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px;
            border-bottom: 1px solid #eee;
        }
        .wifi-item:last-child {
            border-bottom: none;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>WiFi 设置</h1>
        
        <!-- 查看WiFi列表按钮 -->
        <a href="/loadWifi" class="btn btn-primary" style="display: block; text-align: center; margin-bottom: 20px;">
            查看已保存的WiFi
        </a>

        <!-- WiFi设置表单 -->
        <form id="wifiForm">
            <div class="form-group">
                <label for="ssid">WiFi名称(删除wifi时只需要输入wifi名称):</label>
                
                <input type="text" id="ssid" name="ssid" required>
            </div>
            <div class="form-group">
                <label for="password">WiFi密码:</label>
                <input type="password" id="password" name="password">
            </div>
            
            <!-- 保存和删除按钮组 -->
            <div class="button-group">
                <button type="submit" class="btn btn-primary" formaction="/saveWifiSuccessfully" formmethod="POST">保存WiFi</button>
                <button type="submit" class="btn btn-danger" formaction="/deleteWifiSuccessfully" formmethod="POST">删除WiFi</button>
            </div>
        </form>

        <!-- 删除所有WiFi -->
        <form action="/deleteAllWifiSuccessfully" method="POST">
            <button type="submit" class="btn btn-danger delete-all-btn">删除所有WiFi</button>
        </form>

        <a href="/" class="back-link">返回主页</a>
    </div>
</body>
</html>
)";

// 新增：WiFi列表页面
const char* WIFI_LIST_PAGE = R"(
<!DOCTYPE html>
<html lang="zh">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>已保存的WiFi列表</title>
    <style>
        /* 复用之前的��式 */
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f0f2f5;
        }
        .container {
            max-width: 600px;
            margin: 0 auto;
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            text-align: center;
        }
        .wifi-list {
            margin: 20px 0;
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        .wifi-item {
            padding: 10px;
            border-bottom: 1px solid #eee;
        }
        .wifi-item:last-child {
            border-bottom: none;
        }
        .back-link {
            display: block;
            text-align: center;
            margin-top: 20px;
            color: #1890ff;
            text-decoration: none;
        }
        .back-link:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>已保存的WiFi列表</h1>
        <div class="wifi-list">
            %WIFI_LIST%
        </div>
        <a href="/wifi" class="back-link">返回WiFi设置</a>
    </div>
</body>
</html>
)";


// 新增：保存wifi成功后返回的路由
const char* SAVE_WIFI_SUCCESSFULLY_PAGE = R"(
<!DOCTYPE html>
<html lang="zh">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WiFi保存成功</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f0f2f5;
        }
        .container {
            max-width: 600px;
            margin: 0 auto;
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            text-align: center;
        }
        .success-icon {
            color: #52c41a;
            font-size: 48px;
            margin-bottom: 20px;
        }
        .message {
            color: #333;
            margin-bottom: 20px;
        }
        .back-link {
            display: inline-block;
            padding: 10px 20px;
            background-color: #1890ff;
            color: white;
            text-decoration: none;
            border-radius: 4px;
            transition: background-color 0.3s;
        }
        .back-link:hover {
            background-color: #40a9ff;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="success-icon">✓</div>
        <h2 class="message">WiFi信息保存成功！</h2>
        <p>系统将尝试连接到新的WiFi网络</p>
        <a href="/wifi" class="back-link">返回WiFi设置</a>
    </div>
</body>
</html>
)";

// 新增：删除wifi成功后返回的路由
const char* DELETE_WIFI_SUCCESSFULLY_PAGE = R"(
<!DOCTYPE html>
<html lang="zh">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WiFi删除成功</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f0f2f5;
        }
        .container {
            max-width: 600px;
            margin: 0 auto;
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            text-align: center;
        }
        .success-icon {
            color: #52c41a;
            font-size: 48px;
            margin-bottom: 20px;
        }
        .message {
            color: #333;
            margin-bottom: 20px;
        }
        .back-link {
            display: inline-block;
            padding: 10px 20px;
            background-color: #1890ff;
            color: white;
            text-decoration: none;
            border-radius: 4px;
            transition: background-color 0.3s;
        }
        .back-link:hover {
            background-color: #40a9ff;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="success-icon">✓</div>
        <h2 class="message">WiFi信息删除成功！</h2>
        <a href="/wifi" class="back-link">返回WiFi设置</a>
    </div>
</body>
</html>
)";

// 新增：删除所有wifi成功后返回的路由
const char* DELETE_ALL_WIFI_SUCCESSFULLY_PAGE = R"(
<!DOCTYPE html>
<html lang="zh">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>所有WiFi删除成功</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f0f2f5;
        }
        .container {
            max-width: 600px;
            margin: 0 auto;
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            text-align: center;
        }
        .success-icon {
            color: #52c41a;
            font-size: 48px;
            margin-bottom: 20px;
        }
        .message {
            color: #333;
            margin-bottom: 20px;
        }
        .back-link {
            display: inline-block;
            padding: 10px 20px;
            background-color: #1890ff;
            color: white;
            text-decoration: none;
            border-radius: 4px;
            transition: background-color 0.3s;
        }
        .back-link:hover {
            background-color: #40a9ff;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="success-icon">✓</div>
        <h2 class="message">所有WiFi信息已清除！</h2>
        <p>设备将重置为AP模式</p>
        <a href="/wifi" class="back-link">返回WiFi设置</a>
    </div>
</body>
</html>
)";




