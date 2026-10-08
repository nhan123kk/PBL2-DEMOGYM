#pragma once
#include <string>

// Cấu trúc lưu trữ cài đặt hệ thống
struct AppSettings {
    std::string gymName;
    std::string ownerName;
};

void LoadSettings(AppSettings& config);
void SaveSettings(const AppSettings& config);
void DrawSettingsPage(AppSettings& config);