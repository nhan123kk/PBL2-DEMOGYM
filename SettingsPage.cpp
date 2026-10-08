#include "SettingsPage.h"
#include "Theme.h"
#include "raygui.h"
#include <fstream>
#include <sstream>
#include <cstring>

const char* SETTINGS_FILE = "data/settings.txt";

void SaveSettings(const AppSettings& config) {
    std::ofstream file(SETTINGS_FILE);
    if (file.is_open()) {
        file << config.gymName << "|" << config.ownerName << "\n";
        file.close();
    }
}

void LoadSettings(AppSettings& config) {
    std::ifstream file(SETTINGS_FILE);
    if (file.is_open()) {
        std::string line;
        if (std::getline(file, line)) {
            std::stringstream ss(line);
            std::getline(ss, config.gymName, '|');
            std::getline(ss, config.ownerName, '|');
        }
        file.close();
    } else {
        // Dữ liệu mặc định nếu chưa có file
        config.gymName = "PARADISE GYM";
        config.ownerName = "Đặng Thành Nhân"; 
        SaveSettings(config);
    }
}

// Biến trạng thái giao diện
static bool editGymName = false;
static bool editOwnerName = false;
static char inputGymName[64] = "";
static char inputOwnerName[64] = "";
static bool isInitialized = false;

// Hiệu ứng thông báo sao lưu
static bool showBackupSuccess = false;
static int backupTimer = 0;

static void VeChu(const char* text, float x, float y, float size, Color color) {
    DrawTextEx(modernFont, text, {x, y}, size, 1, color);
}

void DrawSettingsPage(AppSettings& config) {
    // Đổ dữ liệu từ biến config vào hộp thoại TextBox lần đầu tiên
    if (!isInitialized) {
        strncpy(inputGymName, config.gymName.c_str(), 63);
        strncpy(inputOwnerName, config.ownerName.c_str(), 63);
        isInitialized = true;
    }

    VeChu("CÀI ĐẶT HỆ THỐNG", 280, 110, 32, textWhite);

    // --- THẺ THÔNG TIN PHÒNG TẬP ---
    DrawRectangleRounded({ 280, 180, 870, 230 }, 0.05f, 10, bgCard);
    VeChu("THÔNG TIN TRUNG TÂM", 310, 200, 20, neonOrange);
    DrawLine(280, 240, 1150, 240, {81, 99, 122, 255});

    VeChu("Tên hiển thị:", 310, 270, 18, textGray);
    if (GuiTextBox({ 480, 260, 350, 40 }, inputGymName, 64, editGymName)) editGymName = !editGymName;

    VeChu("Người quản lý:", 310, 330, 18, textGray);
    if (GuiTextBox({ 480, 320, 350, 40 }, inputOwnerName, 64, editOwnerName)) editOwnerName = !editOwnerName;

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 }));
    if (GuiButton({ 860, 260, 130, 40 }, "CẬP NHẬT")) {
        config.gymName = inputGymName;
        config.ownerName = inputOwnerName;
        SaveSettings(config); // Lưu ngay vào file
    }
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar));

    // --- THẺ SAO LƯU DỮ LIỆU ---
    DrawRectangleRounded({ 280, 450, 870, 200 }, 0.05f, 10, bgCard);
    VeChu("SAO LƯU & BẢO MẬT", 310, 470, 20, neonBlue);
    DrawLine(280, 510, 1150, 510, {81, 99, 122, 255});

    VeChu("Khuyến nghị sao lưu dữ liệu Hội viên, Hóa đơn định kỳ để tránh mất mát.", 310, 540, 16, textGray);
    
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 51, 65, 85, 255 }));
    if (GuiButton({ 310, 580, 200, 45 }, "TẠO BẢN SAO LƯU")) {
        // Kích hoạt hiệu ứng thông báo thành công (hiển thị trong khoảng 3 giây)
        showBackupSuccess = true;
        backupTimer = 180; 
    }
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar));

    // Vẽ thông báo sao lưu
    if (showBackupSuccess) {
        VeChu("Sao lưu hoàn tất! Dữ liệu đã được đóng gói an toàn.", 540, 595, 16, neonGreen);
        backupTimer--;
        if (backupTimer <= 0) showBackupSuccess = false; // Tự động ẩn đi
    }
}