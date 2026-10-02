#include "PaymentPage.h"
#include "Theme.h"
#include "raygui.h"
#include <fstream>
#include <sstream>
#include <cstring>

const char* PAYMENT_FILE = "data/payments.txt";

void SavePayments(const std::vector<PaymentRecord>& payList) {
    std::ofstream file(PAYMENT_FILE);
    if (file.is_open()) {
        for (const auto& p : payList) {
            file << p.id << "|" << p.memberName << "|" << p.amount << "|" << p.date << "|" << p.status << "\n";
        }
        file.close();
    }
}

void LoadPayments(std::vector<PaymentRecord>& payList) {
    std::ifstream file(PAYMENT_FILE);
    if (file.is_open()) {
        payList.clear();
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            PaymentRecord p;
            std::getline(ss, p.id, '|');
            std::getline(ss, p.memberName, '|');
            std::getline(ss, p.amount, '|');
            std::getline(ss, p.date, '|');
            std::getline(ss, p.status, '|');
            if (!p.id.empty()) payList.push_back(p);
        }
        file.close();
    } else {
        // Dữ liệu mẫu ban đầu
        payList = {
            {"HD001", "Đặng Thành Nhân", "3,500,000 VNĐ", "25/09/2026", "Đã thu (VNPAY)"},
            {"HD002", "Nguyễn Văn A", "1,200,000 VNĐ", "24/09/2026", "Chờ xử lý"},
            {"HD003", "Trần Thị B", "500,000 VNĐ", "23/09/2026", "Đã thu (ZaloPay)"}
        };
        SavePayments(payList);
    }
}

static bool showPayModal = false;
static int selectedPayIndex = -1;
static char inputPayId[64] = "";
static char inputPayName[64] = "";
static char inputPayAmount[64] = "";
static char inputPayDate[64] = "";
static char inputPayStatus[64] = "Đã thu (Tiền mặt)";
static bool editPayId = false, editPayName = false, editPayAmount = false, editPayDate = false, editPayStatus = false;

static void VeChu(const char* text, float x, float y, float size, Color color) {
    DrawTextEx(modernFont, text, {x, y}, size, 1, color);
}

void DrawPaymentPage(std::vector<PaymentRecord>& payList) {
    VeChu("QUẢN LÝ THANH TOÁN", 280, 110, 32, textWhite);

    // --- THANH CÔNG CỤ ---
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
    if (GuiButton({ 740, 160, 130, 45 }, "TẠO HÓA ĐƠN") && !showPayModal) { 
        showPayModal = true;
        memset(inputPayId, 0, sizeof(inputPayId));
        memset(inputPayName, 0, sizeof(inputPayName));
        memset(inputPayAmount, 0, sizeof(inputPayAmount));
        memset(inputPayDate, 0, sizeof(inputPayDate));
        strncpy(inputPayStatus, "Đã thu (Tiền mặt)", 63);
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 127, 29, 29, 255 })); 
    if (GuiButton({ 880, 160, 130, 45 }, "HỦY HÓA ĐƠN") && !showPayModal) { 
        if (selectedPayIndex != -1) {
            payList.erase(payList.begin() + selectedPayIndex);
            selectedPayIndex = -1; 
            SavePayments(payList);
        }
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar));
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    // --- BẢNG DANH SÁCH GIAO DỊCH ---
    DrawRectangleRounded({ 280, 230, 870, 530 }, 0.05f, 10, bgCard);
    VeChu("MÃ GD", 310, 250, 18, textGray);
    VeChu("HỘI VIÊN", 430, 250, 18, textGray);
    VeChu("SỐ TIỀN", 630, 250, 18, textGray);
    VeChu("NGÀY LẬP", 800, 250, 18, textGray);
    VeChu("TRẠNG THÁI", 980, 250, 18, textGray);
    DrawLine(280, 280, 1150, 280, {81, 99, 122, 255});

    int startY = 300;
    for (size_t i = 0; i < payList.size(); i++) {
        Rectangle rowRect = { 280, (float)startY - 10, 870, 45 };
        
        if (!showPayModal && CheckCollisionPointRec(GetMousePosition(), rowRect)) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selectedPayIndex = (int)i; 
        }

        if (selectedPayIndex == (int)i) {
            DrawRectangleRec(rowRect, bgCardHover);
            DrawRectangle(280, startY - 10, 4, 45, neonGreen); 
        }

        VeChu(payList[i].id.c_str(), 310, startY, 20, textWhite);
        VeChu(payList[i].memberName.c_str(), 430, startY, 20, textWhite);
        VeChu(payList[i].amount.c_str(), 630, startY, 20, textWhite);
        VeChu(payList[i].date.c_str(), 800, startY, 20, textWhite);

        // Hiệu ứng màu sắc cho trạng thái
        Color statusColor = textWhite;
        if (payList[i].status.find("Đã thu") != std::string::npos) statusColor = neonGreen;
        else if (payList[i].status.find("Chờ") != std::string::npos) statusColor = neonOrange;
        else statusColor = { 239, 68, 68, 255 }; 
        
        VeChu(payList[i].status.c_str(), 980, startY, 20, statusColor);

        DrawLine(280, startY + 40, 1150, startY + 40, {51, 65, 85, 255});
        startY += 55;
    }

    // --- POPUP TẠO HÓA ĐƠN ---
    if (showPayModal) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0, 0, 0, 150 });
        DrawRectangleRounded({ 400, 130, 450, 560 }, 0.05f, 10, bgMain);
        DrawRectangleRoundedLines({ 400, 130, 450, 560 }, 0.05f, 10, bgSidebar);

        VeChu("TẠO HÓA ĐƠN MỚI", 520, 160, 24, neonGreen);

        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

        VeChu("Mã GD:", 450, 230, 18, textWhite);
        if (GuiTextBox({ 550, 220, 250, 40 }, inputPayId, 64, editPayId)) editPayId = !editPayId;

        VeChu("Hội Viên:", 450, 290, 18, textWhite);
        if (GuiTextBox({ 550, 280, 250, 40 }, inputPayName, 64, editPayName)) editPayName = !editPayName;

        VeChu("Số Tiền:", 450, 350, 18, textWhite);
        if (GuiTextBox({ 550, 340, 250, 40 }, inputPayAmount, 64, editPayAmount)) editPayAmount = !editPayAmount;

        VeChu("Ngày Lập:", 450, 410, 18, textWhite);
        if (GuiTextBox({ 550, 400, 250, 40 }, inputPayDate, 64, editPayDate)) editPayDate = !editPayDate;

        VeChu("Trạng Thái:", 450, 470, 18, textWhite);
        if (GuiTextBox({ 550, 460, 250, 40 }, inputPayStatus, 64, editPayStatus)) editPayStatus = !editPayStatus;

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
        if (GuiButton({ 480, 530, 130, 45 }, "LƯU LẠI")) {
            payList.push_back({inputPayId, inputPayName, inputPayAmount, inputPayDate, inputPayStatus});
            SavePayments(payList);
            showPayModal = false; 
        }

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); 
        if (GuiButton({ 640, 530, 130, 45 }, "HỦY")) showPayModal = false;
    }
}