#include "SettingsPage.h"
#include "Theme.h"
#include "raygui.h"
#include <fstream>
#include <iostream>
#include <ctime>

// --- THUẬT TOÁN SAO LƯU FILE C++ ---
bool CopyFile(const char* source, const char* dest) {
    std::ifstream src(source, std::ios::binary);
    if (!src) return false;
    std::ofstream dst(dest, std::ios::binary);
    if (!dst) return false;
    dst << src.rdbuf(); 
    return true;
}

// Trạng thái thông báo
static std::string statusMsg = "";
static Color statusColor = textWhite;
static int statusTimer = 0;

static void VeChu(const char* text, float x, float y, float size, Color color) {
    DrawTextEx(modernFont, text, {x, y}, size, 1, color);
}

void DrawSettingsPage() {
    // Đã hạ tọa độ Y xuống 100 để không đè lên chữ "Welcome back"
    VeChu("CÀI ĐẶT HỆ THỐNG", 280, 100, 32, textWhite);

    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

    // ========================================================================
    // --- KHỐI 1: QUẢN LÝ DỮ LIỆU (BACKUP & RESTORE) ---
    // ========================================================================
    // Dịch toàn bộ khối này xuống bắt đầu từ y = 160
    DrawRectangleRounded({ 280, 160, 870, 200 }, 0.05f, 10, bgCard);
    VeChu("SAO LƯU & PHỤC HỒI DỮ LIỆU", 310, 180, 22, neonBlue);
    DrawLine(310, 215, 1120, 215, {51, 65, 85, 255});
    
    VeChu("Bảo vệ an toàn cho toàn bộ dữ liệu file .txt (Thiết bị, Lịch học, Hội viên...)", 310, 235, 16, textGray);
    VeChu("Hãy sao lưu thường xuyên để phòng trường hợp lỗi file hoặc mất điện.", 310, 260, 16, textGray);

    // Nút Sao Lưu
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
    if (GuiButton({ 310, 295, 180, 40 }, "SAO LƯU DỮ LIỆU")) {
        bool ok1 = CopyFile("data/equipment.txt", "data/backup_equipment.txt");
        bool ok2 = CopyFile("data/schedule.txt", "data/backup_schedule.txt");
        
        if (ok1 || ok2) {
            time_t now = time(0);
            tm *ltm = localtime(&now);
            char timeStr[64];
            sprintf(timeStr, "Sao luu thanh cong luc %02d:%02d:%02d!", ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
            statusMsg = timeStr;
            statusColor = neonGreen;
            statusTimer = 300;
        } else {
            statusMsg = "Loi: Khong the sao luu du lieu!";
            statusColor = neonRed;
            statusTimer = 300;
        }
    }

    // Nút Phục Hồi
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 124, 45, 18, 255 })); 
    if (GuiButton({ 510, 295, 180, 40 }, "PHỤC HỒI DỮ LIỆU")) {
        bool ok1 = CopyFile("data/backup_equipment.txt", "data/equipment.txt");
        bool ok2 = CopyFile("data/backup_schedule.txt", "data/schedule.txt");
        
        if (ok1 || ok2) {
            statusMsg = "Da phuc hoi du lieu tu ban Backup! Vui long khoi dong lai ung dung.";
            statusColor = neonOrange;
            statusTimer = 400;
        } else {
            statusMsg = "Loi: Khong tim thay ban Backup nao!";
            statusColor = neonRed;
            statusTimer = 300;
        }
    }
    
    // Hiển thị thông báo (Toast)
    if (statusTimer > 0) {
        VeChu(statusMsg.c_str(), 720, 305, 16, statusColor);
        statusTimer--;
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); 
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
}