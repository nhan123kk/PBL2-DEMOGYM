#include "LoginPage.h"
#include "raygui.h"
#include "Theme.h"    
#include <cstring>

// Khai báo biến font toàn cục từ main.cpp sang để dùng
extern Font modernFont; 

LoginPage::LoginPage() {
    memset(usernameInput, 0, 64);
    memset(passwordInput, 0, 64);
    usernameEditMode = false;
    passwordEditMode = false;
    loginFailed = false;
}

Account* LoginPage::Draw(AccountManager& accountManager) {
    Account* loggedInUser = nullptr;

    // Sử dụng kích thước của màn hình ảo (camera)
    int virtualWidth = 1200;
    int virtualHeight = 800;

    int boxWidth = 360;
    int boxHeight = 45;
    int startX = (virtualWidth - boxWidth) / 2;
    int startY = virtualHeight / 2 - 100;

    // 1. Vẽ Khung nền (Card) bo góc
    DrawRectangleRounded(Rectangle{(float)startX - 40, (float)startY - 100, (float)boxWidth + 80, 420}, 0.1f, 20, bgSidebar);

    // 2. Tính toán độ rộng chữ để căn giữa hoàn hảo
    Vector2 title1Size = MeasureTextEx(modernFont, "PARADISE GYM", 30, 1);
    Vector2 title2Size = MeasureTextEx(modernFont, "DANG NHAP HE THONG", 24, 1);
    
    float title1X = startX + (boxWidth / 2.0f) - (title1Size.x / 2.0f);
    float title2X = startX + (boxWidth / 2.0f) - (title2Size.x / 2.0f);

    // Vẽ Tiêu đề đã được căn giữa
    DrawTextEx(modernFont, "PARADISE GYM", {title1X, (float)startY - 70}, 30, 1, neonGreen);
    DrawTextEx(modernFont, "DANG NHAP HE THONG", {title2X, (float)startY - 30}, 24, 1, textWhite);

    // 3. Username
    DrawTextEx(modernFont, "Ten dang nhap", {(float)startX, (float)startY + 15}, 18, 1, textGray);
    GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    if (GuiTextBox(Rectangle{(float)startX, (float)startY + 40, (float)boxWidth, (float)boxHeight}, usernameInput, 64, usernameEditMode)) {
        usernameEditMode = !usernameEditMode;
        passwordEditMode = false;
    }

    // 4. Password
    DrawTextEx(modernFont, "Mat khau", {(float)startX, (float)startY + 100}, 18, 1, textGray);
    if (GuiTextBox(Rectangle{(float)startX, (float)startY + 125, (float)boxWidth, (float)boxHeight}, passwordInput, 64, passwordEditMode)) {
        passwordEditMode = !passwordEditMode;
        usernameEditMode = false;
    }

    // 5. Nút Bấm Đăng Nhập
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(neonGreen));
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(DARKGREEN));
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER); // Căn giữa chữ cho nút
    
    if (GuiButton(Rectangle{(float)startX, (float)startY + 200, (float)boxWidth, 50}, "DANG NHAP")) {
        string userStr(usernameInput);
        string passStr(passwordInput);

        loggedInUser = accountManager.login(userStr, passStr);

        if (loggedInUser == nullptr) {
            loginFailed = true;
        } else {
            loginFailed = false;
            memset(usernameInput, 0, 64);
            memset(passwordInput, 0, 64);
            usernameEditMode = false;
            passwordEditMode = false;
        }
    }
    
    // Trả lại style mặc định cho các nút bên trong app
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar));
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(bgCardHover));
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT); // Trả lại căn trái cho menu

    // 6. Hiển thị lỗi bằng chữ Đỏ (cũng căn giữa)
    if (loginFailed) {
        const char* errorMsg = "Sai ten dang nhap hoac mat khau!";
        Vector2 errorSize = MeasureTextEx(modernFont, errorMsg, 18, 1);
        float errorX = startX + (boxWidth / 2.0f) - (errorSize.x / 2.0f);
        DrawTextEx(modernFont, errorMsg, {errorX, (float)startY + 270}, 18, 1, RED);
    }

    return loggedInUser;
}