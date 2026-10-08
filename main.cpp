#include "raylib.h"
#include <algorithm> // Thư viện để dùng hàm tính tỷ lệ phóng to (std::min)

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// Nhúng các file module
#include "Theme.h"
#include "Member.h"
#include "Dashboard.h"
#include "MembersPage.h"
#include "SchedulePage.h"
#include "EquipmentPage.h"
#include "PaymentPage.h"
#include "SettingsPage.h"

Font modernFont;

int main() {
    // Cho phép người dùng thu phóng cửa sổ thoải mái
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1200, 800, "Paradise Gym - Modern Dark Fitness");
    SetWindowMinSize(1200, 800);
    SetTargetFPS(60);

    // ==========================================
    // KHỞI TẠO MÀN HÌNH ẢO ĐỂ PHÓNG TO GIAO DIỆN
    // ==========================================
    const int virtualWidth = 1200;
    const int virtualHeight = 800;
    RenderTexture2D target = LoadRenderTexture(virtualWidth, virtualHeight);

    // Load Font Tiếng Việt
    const char* vnChars = "aáàảãạăắằẳẵặâấầẩẫậeéèẻẽẹêếềểễệiíìỉĩịoóòỏõọôốồổỗộơớờởỡợuúùủũụưứừửữựyýỳỷỹỵđAÁÀẢÃẠĂẮẰẲẴẶÂẤẦẨẪẬEÉÈẺẼẸÊẾỀỂỄỆIÍÌỈĨỊOÓÒỎÕỌÔỐỒỔỖỘƠỚỜỞỠỢUÚÙỦŨỤƯỨỪỬỮỰYÝỲỶỸỴĐ 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()-=_+[]{}|;':\",./<>?`~";
    int codepointCount = 0;
    int* codepoints = LoadCodepoints(vnChars, &codepointCount);
    modernFont = LoadFontEx("C:/Windows/Fonts/segoeuib.ttf", 72, codepoints, codepointCount);
    UnloadCodepoints(codepoints);
    SetTextureFilter(modernFont.texture, TEXTURE_FILTER_BILINEAR);

    // Cấu hình Nút bấm Raygui
    GuiSetFont(modernFont);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 0);
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar));
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(bgCardHover));
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    // Load Dữ liệu
    Page currentPage = DASHBOARD; 
    std::vector<Member> memberList; LoadMembers(memberList);
    std::vector<GymClass> scheduleList; LoadSchedule(scheduleList);
    std::vector<Equipment> eqList; LoadEquipment(eqList);
    std::vector<PaymentRecord> payList; LoadPayments(payList);
    AppSettings config; 
    LoadSettings(config);

    auto VeChu = [&](const char* text, float x, float y, float size, Color color) {
        DrawTextEx(modernFont, text, {x, y}, size, 1, color);
    };

    while (!WindowShouldClose()) {
        // Nhấn F11 để Fullscreen tràn viền
        if (IsKeyPressed(KEY_F11)) ToggleFullscreen(); 

        // --- THUẬT TOÁN TÍNH TỶ LỆ PHÓNG TO ---
        float scale = std::min((float)GetScreenWidth() / virtualWidth, (float)GetScreenHeight() / virtualHeight);
        
        // Đồng bộ tọa độ chuột để click chuẩn xác khi màn hình bị kéo giãn
        SetMouseOffset(-(GetScreenWidth() - (virtualWidth * scale)) * 0.5f, -(GetScreenHeight() - (virtualHeight * scale)) * 0.5f);
        SetMouseScale(1.0f / scale, 1.0f / scale);

        // ==========================================
        // BƯỚC 1: VẼ TOÀN BỘ GIAO DIỆN VÀO MÀN HÌNH ẢO
        // ==========================================
        BeginTextureMode(target);
        ClearBackground(bgMain);

        // --- MENU BÊN TRÁI ---
        DrawRectangle(0, 0, 240, virtualHeight, bgSidebar);
        VeChu(config.gymName.c_str(), 20, 30, 24, neonGreen);
        VeChu(config.ownerName.c_str(), 1055, 38, 20, textWhite);
        
        if (GuiButton({ 10, 110, 220, 50 }, "     DASHBOARD")) currentPage = DASHBOARD;
        if (GuiButton({ 10, 170, 220, 50 }, "     MEMBERS"))   currentPage = MEMBERS;
        if (GuiButton({ 10, 230, 220, 50 }, "     SCHEDULE"))  currentPage = SCHEDULE;
        if (GuiButton({ 10, 290, 220, 50 }, "     EQUIPMENT")) currentPage = EQUIPMENT;
        if (GuiButton({ 10, 350, 220, 50 }, "     PAYMENT"))   currentPage = PAYMENT;
        if (GuiButton({ 10, 410, 220, 50 }, "     SETTINGS"))  currentPage = SETTINGS;

        // --- HEADER ---
        VeChu("Welcome back, Admin 👋", 280, 25, 30, textWhite);
        DrawRectangleRounded({ 1010, 25, 150, 50 }, 0.5f, 10, bgSidebar);
        DrawCircle(1035, 50, 15, neonGreen);
        VeChu("Admin", 1065, 38, 20, textWhite);

        // --- GỌI CÁC TRANG ---
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
             DrawSettingsPage(config);
        } else {
            VeChu("Chức năng đang phát triển...", 280, 150, 24, textGray);
        }
        EndTextureMode(); // Kết thúc vẽ vào màn hình ảo

        // ==========================================
        // BƯỚC 2: IN MÀN HÌNH ẢO ĐÓ RA MÀN HÌNH THẬT
        // ==========================================
        BeginDrawing();
        ClearBackground({ 10, 15, 25, 255 }); // Màu nền viền đen nếu tỷ lệ màn hình không khớp 16:10

        // Phóng to bức tranh ảo ra giữa màn hình
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