#include "raylib.h"
#include <algorithm> 
#include <fstream>   // Thư viện đọc/ghi file
#include <sstream>   // Thư viện xử lý chuỗi
#include <stdexcept> // Thư viện xử lý ngoại lệ Exception

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// =========================================================
// NHÚNG CÁC MODULE 
// =========================================================
#include "Theme.h"
#include "Member.h"  // <--- File này giờ đã chứa GymList, Person, Member, Equipment, GymClass, PaymentRecord

#include "Dashboard.h"
#include "MembersPage.h"
#include "SchedulePage.h"
#include "EquipmentPage.h"
#include "PaymentPage.h"
#include "SettingsPage.h"
#include "AccountManager.h"
#include "LoginPage.h"

using namespace std;

// Biến toàn cục
Account* currentUser = nullptr;
Font modernFont;

// =================================================================
// CÁC HÀM ĐỌC DỮ LIỆU TỪ FILE TXT (Áp dụng Try-Catch)
// =================================================================

void LoadMembers(GymList<Member>& list) {
    try {
        ifstream file("data/members.txt"); 
        if (!file.is_open()) return;
        string line, id, name, phone, plan, activeStr;
        while (getline(file, line)) {
            stringstream ss(line);
            getline(ss, id, '|'); getline(ss, name, '|'); getline(ss, phone, '|');
            getline(ss, plan, '|'); getline(ss, activeStr, '|');
            if (!activeStr.empty() && activeStr.back() == '\r') activeStr.pop_back();
            if (!id.empty()) {
                bool isActive = (activeStr == "1" || activeStr == "true" || activeStr.find("Active") != string::npos);
                list.push_back(Member(id, name, phone, plan, isActive));
            }
        }
        file.close();
    } catch (...) {}
}

void LoadSchedule(GymList<GymClass>& list) {
    try {
        ifstream file("data/schedule.txt");
        if (!file.is_open()) return;
        string line, time, name, trainer, room, status;
        while (getline(file, line)) {
            stringstream ss(line);
            getline(ss, time, '|'); getline(ss, name, '|'); 
            getline(ss, trainer, '|'); getline(ss, room, '|'); getline(ss, status, '|');
            if (!status.empty() && status.back() == '\r') status.pop_back();
            if (!time.empty()) list.push_back(GymClass(time, name, trainer, room, status));
        }
        file.close();
    } catch (...) {}
}

void LoadEquipment(GymList<Equipment>& list) {
    try {
        ifstream file("data/equipment.txt");
        if (!file.is_open()) return;
        string line, id, name, type, qty, status;
        while (getline(file, line)) {
            stringstream ss(line);
            getline(ss, id, '|'); getline(ss, name, '|'); 
            getline(ss, type, '|'); getline(ss, qty, '|'); getline(ss, status, '|');
            if (!status.empty() && status.back() == '\r') status.pop_back();
            if (!id.empty()) list.push_back(Equipment(id, name, type, qty, status));
        }
        file.close();
    } catch (...) {}
}

void LoadPayments(GymList<PaymentRecord>& list) {
    try {
        ifstream file("data/payments.txt");
        if (!file.is_open()) return;
        string line, pid, mid, amtStr, date, status;
        while (getline(file, line)) {
            stringstream ss(line);
            getline(ss, pid, '|'); getline(ss, mid, '|'); 
            getline(ss, amtStr, '|'); getline(ss, date, '|'); getline(ss, status, '|');
            if (!status.empty() && status.back() == '\r') status.pop_back();
            
            if (!pid.empty()) {
                string cleanAmt = "";
                // Thuật toán bóc tách dấu phẩy (,) ra khỏi chuỗi tiền tệ
                for (char c : amtStr) if (isdigit(c) || c == '.') cleanAmt += c;
                double amount = cleanAmt.empty() ? 0.0 : stod(cleanAmt);
                list.push_back(PaymentRecord(pid, mid, amount, date, status));
            }
        }
        file.close();
    } catch (...) {}
}

// ==========================================
// HÀM MAIN CHÍNH
// ==========================================
int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1200, 800, "Paradise Gym - Modern Dark Fitness");
    SetWindowMinSize(1200, 800);
    SetTargetFPS(60);

    AccountManager accountManager;
    accountManager.addAccount("admin", "123456", ADMIN);
    accountManager.addAccount("staff", "123", STAFF);

    LoginPage loginPage;

    const int virtualWidth = 1200;
    const int virtualHeight = 800;
    RenderTexture2D target = LoadRenderTexture(virtualWidth, virtualHeight);

    const char* vnChars = "aáàảãạăắằẳẵặâấầẩẫậeéèẻẽẹêếềểễệiíìỉĩịoóòỏõọôốồổỗộơớờởỡợuúùủũụưứừửữựyýỳỷỹỵđAÁÀẢÃẠĂẮẰẲẴẶÂẤẦẨẪẬEÉÈẺẼẸÊẾỀỂỄỆIÍÌỈĨỊOÓÒỎÕỌÔỐỒỔỖỘƠỚỜỞỠỢUÚÙỦŨỤƯỨỪỬỮỰYÝỲỶỸỴĐ 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()-=_+[]{}|;':\",./<>?`~";
    int codepointCount = 0;
    int* codepoints = LoadCodepoints(vnChars, &codepointCount);
    modernFont = LoadFontEx("C:/Windows/Fonts/segoeuib.ttf", 72, codepoints, codepointCount);
    UnloadCodepoints(codepoints);
    SetTextureFilter(modernFont.texture, TEXTURE_FILTER_BILINEAR);

    GuiSetFont(modernFont);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 0);
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar));
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(bgCardHover));
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    Page currentPage = DASHBOARD; 
    
    // Khởi tạo các mảng động theo chuẩn Template GymList và đọc file
    GymList<Member> memberList; LoadMembers(memberList);
    GymList<GymClass> scheduleList; LoadSchedule(scheduleList);
    GymList<Equipment> eqList; LoadEquipment(eqList);
    GymList<PaymentRecord> payList; LoadPayments(payList);
   
    auto VeChu = [&](const char* text, float x, float y, float size, Color color) {
        DrawTextEx(modernFont, text, {x, y}, size, 1, color);
    };

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_F11)) ToggleFullscreen(); 

        float scale = std::min((float)GetScreenWidth() / virtualWidth, (float)GetScreenHeight() / virtualHeight);
        SetMouseOffset(-(GetScreenWidth() - (virtualWidth * scale)) * 0.5f, -(GetScreenHeight() - (virtualHeight * scale)) * 0.5f);
        SetMouseScale(1.0f / scale, 1.0f / scale);

        BeginTextureMode(target);
        ClearBackground(bgMain);

        if (currentUser == nullptr) {
            currentUser = loginPage.Draw(accountManager);
        } 
        else {
            DrawRectangle(0, 0, 240, virtualHeight, bgSidebar);
            VeChu("PARADISE GYM", 20, 30, 24, neonGreen);
            
            if (GuiButton({ 10, 110, 220, 50 }, "     DASHBOARD")) currentPage = DASHBOARD;
            if (GuiButton({ 10, 170, 220, 50 }, "     MEMBERS"))   currentPage = MEMBERS;
            if (GuiButton({ 10, 230, 220, 50 }, "     SCHEDULE"))  currentPage = SCHEDULE;
            if (GuiButton({ 10, 290, 220, 50 }, "     EQUIPMENT")) currentPage = EQUIPMENT;
            if (GuiButton({ 10, 350, 220, 50 }, "     PAYMENT"))   currentPage = PAYMENT;
            
            if (currentUser->isAdmin()) {
                if (GuiButton({ 10, 410, 220, 50 }, "     SETTINGS")) currentPage = SETTINGS;
            } else {
                GuiDisable(); 
                GuiButton({ 10, 410, 220, 50 }, "     SETTINGS (LOCKED)");
                GuiEnable();  
            }

            VeChu("Welcome back", 280, 25, 30, textWhite);
            DrawRectangleRounded({ 1010, 25, 150, 50 }, 0.5f, 10, bgSidebar);
            DrawCircle(1035, 50, 15, neonGreen);
            VeChu(currentUser->getUsername().c_str(), 1065, 38, 20, textWhite);

            // GỌI CÁC TRANG (Truyền GymList vào các hàm vẽ giao diện)
            if (currentPage == DASHBOARD) {
                DrawDashboard(memberList, scheduleList, eqList, payList);
            } else if (currentPage == MEMBERS) {
                DrawMembersPage(memberList);
            } else if (currentPage == SCHEDULE) {
                DrawSchedulePage(scheduleList);
            } else if (currentPage == EQUIPMENT) {
                DrawEquipmentPage(eqList);
            } else if (currentPage == PAYMENT) {
                DrawPaymentPage(payList, memberList);
            } else if (currentPage == SETTINGS) {
                DrawSettingsPage();
            } else {
                VeChu("Chức năng đang phát triển...", 280, 150, 24, textGray);
            }

            GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
            if (GuiButton({ 10, (float)virtualHeight - 60, 220, 40 }, "LOG OUT")) {
                currentUser = nullptr; 
                currentPage = DASHBOARD; 
            }
            GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT); 
        }

        EndTextureMode(); 

        BeginDrawing();
        ClearBackground({ 10, 15, 25, 255 }); 
        DrawTexturePro(target.texture, 
            { 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
            { (GetScreenWidth() - ((float)virtualWidth * scale)) * 0.5f, 
              (GetScreenHeight() - ((float)virtualHeight * scale)) * 0.5f,
              (float)virtualWidth * scale, (float)virtualHeight * scale },
            { 0, 0 }, 0.0f, WHITE);
        EndDrawing();
    }
    
    UnloadRenderTexture(target);
    UnloadFont(modernFont);
    CloseWindow();
    return 0;
}