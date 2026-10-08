#include "PaymentPage.h"
#include "Theme.h"
#include "raygui.h"
#include <fstream>
#include <sstream>
#include <cstring>
#include <ctime>

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
        payList = {
            {"HD001", "Dang Thanh Nhan", "3,500,000", "01/10/2026", "Đã thu (VNPAY)"},
            {"HD002", "Nguyen Van A", "1,200,000", "02/10/2026", "Chờ xử lý"}
        };
        SavePayments(payList);
    }
}

static bool showPayModal = false;
static bool isEditMode = false; // BIẾN MỚI CHO TÍNH NĂNG CHỈNH SỬA
static int selectedPayIndex = -1;
static char inputPayId[64] = "";
static char inputPayAmount[64] = "";
static char inputPayDate[64] = "";
static bool editPayAmount = false, editPayDate = false;

// Biến cho Dropdown Trạng Thái
static const char* statusOptions = "Đã thu (Tiền mặt);Đã thu (VNPAY);Đã thu (ZaloPay);Chờ xử lý";
static int activeStatusIndex = 0;       
static bool editStatusDropdown = false; 

// Biến cho Dropdown Hội Viên
static int activeMemberIndex = 0;
static int previousMemberIndex = -1;  
static bool editMemberDropdown = false;

static float scrollOffset = 0.0f;        
static std::string errorMessage = "";    
static int errorTimer = 0;               

static PaymentRecord lastDeletedPay;
static int lastDeletedPayIndex = -1;
static bool canUndoPay = false;
static int undoPayTimer = 0;

static void VeChu(const char* text, float x, float y, float size, Color color) {
    DrawTextEx(modernFont, text, {x, y}, size, 1, color);
}

void DrawPaymentPage(std::vector<PaymentRecord>& payList, const std::vector<Member>& memberList) {
    VeChu("QUẢN LÝ THANH TOÁN", 280, 110, 32, textWhite);

    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    
    // --- NÚT TẠO HÓA ĐƠN ---
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
    if (GuiButton({ 740, 160, 130, 45 }, "TẠO HÓA ĐƠN") && !showPayModal) { 
        showPayModal = true;
        isEditMode = false;
        errorMessage = "";
        editStatusDropdown = false;
        editMemberDropdown = false;
        
        int maxIdNum = 0;
        for (const auto& p : payList) {
            std::string numStr = "";
            for (char c : p.id) if (isdigit(c)) numStr += c;
            if (!numStr.empty()) {
                int num = std::stoi(numStr);
                if (num > maxIdNum) maxIdNum = num;
            }
        }
        snprintf(inputPayId, sizeof(inputPayId), "HD%03d", maxIdNum + 1);

        time_t now = time(0);
        tm *ltm = localtime(&now);
        snprintf(inputPayDate, sizeof(inputPayDate), "%02d/%02d/%04d", ltm->tm_mday, 1 + ltm->tm_mon, 1900 + ltm->tm_year);

        memset(inputPayAmount, 0, sizeof(inputPayAmount));
        activeStatusIndex = 0;
        activeMemberIndex = 0;
        previousMemberIndex = -1; 
    }

    // --- NÚT CHỈNH SỬA ---
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 124, 45, 18, 255 })); 
    if (GuiButton({ 880, 160, 130, 45 }, "CHỈNH SỬA") && !showPayModal) { 
        if (selectedPayIndex != -1 && selectedPayIndex < payList.size()) { 
            showPayModal = true;
            isEditMode = true;
            errorMessage = "";
            editStatusDropdown = false;
            editMemberDropdown = false;
            
            // Tải dữ liệu cũ lên textbox
            strncpy(inputPayId, payList[selectedPayIndex].id.c_str(), 63);
            strncpy(inputPayAmount, payList[selectedPayIndex].amount.c_str(), 63);
            strncpy(inputPayDate, payList[selectedPayIndex].date.c_str(), 63);

            // Tự động map Trạng thái thanh toán cũ vào Dropdown
            const char* statusArr[] = {"Đã thu (Tiền mặt)", "Đã thu (VNPAY)", "Đã thu (ZaloPay)", "Chờ xử lý"};
            activeStatusIndex = 0;
            for (int i = 0; i < 4; i++) {
                if (payList[selectedPayIndex].status == statusArr[i]) activeStatusIndex = i;
            }

            // Tự động map Hội viên cũ vào Dropdown
            activeMemberIndex = 0;
            for (size_t i = 0; i < memberList.size(); i++) {
                if (payList[selectedPayIndex].memberName == memberList[i].name) {
                    activeMemberIndex = i;
                    break;
                }
            }
            // Khóa không cho tính năng Auto-amount nhảy loạn lên khi vừa mở bảng Edit
            previousMemberIndex = activeMemberIndex;
        }
    }

    // --- NÚT HỦY HÓA ĐƠN ---
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 127, 29, 29, 255 })); 
    if (GuiButton({ 1020, 160, 130, 45 }, "HỦY HÓA ĐƠN") && !showPayModal) { 
        if (selectedPayIndex != -1 && selectedPayIndex < payList.size()) {
            lastDeletedPay = payList[selectedPayIndex];
            lastDeletedPayIndex = selectedPayIndex;
            canUndoPay = true;
            undoPayTimer = 300; 

            payList.erase(payList.begin() + selectedPayIndex);
            selectedPayIndex = -1; 
            SavePayments(payList);
        }
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar));
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    // --- BẢNG DANH SÁCH ---
    DrawRectangleRounded({ 280, 230, 870, 530 }, 0.05f, 10, bgCard);
    VeChu("MÃ GD", 310, 250, 18, textGray);
    VeChu("HỘI VIÊN", 430, 250, 18, textGray);
    VeChu("SỐ TIỀN", 630, 250, 18, textGray);
    VeChu("NGÀY LẬP", 800, 250, 18, textGray);
    VeChu("TRẠNG THÁI", 980, 250, 18, textGray);
    DrawLine(280, 280, 1150, 280, {81, 99, 122, 255});

    Rectangle listArea = { 280, 281, 870, 479 }; 
    float itemHeight = 55.0f;
    float totalListHeight = payList.size() * itemHeight;
    
    if (!showPayModal && CheckCollisionPointRec(GetMousePosition(), listArea)) {
        scrollOffset += GetMouseWheelMove() * 30.0f; 
    }
    
    float maxScroll = listArea.height - totalListHeight;
    if (maxScroll > 0) maxScroll = 0; 
    if (scrollOffset < maxScroll) scrollOffset = maxScroll;
    if (scrollOffset > 0) scrollOffset = 0;

    BeginScissorMode(listArea.x, listArea.y, listArea.width, listArea.height);
    float startY = 300 + scrollOffset;
    for (size_t i = 0; i < payList.size(); i++) {
        if (startY + 45 >= listArea.y && startY - 10 <= listArea.y + listArea.height) {
            Rectangle rowRect = { 280, startY - 10, 870, 45 };
            if (!showPayModal && CheckCollisionPointRec(GetMousePosition(), rowRect)) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selectedPayIndex = (int)i; 
            }
            if (selectedPayIndex == (int)i) {
                DrawRectangleRec(rowRect, bgCardHover);
                DrawRectangle(280, startY - 10, 4, 45, neonGreen); 
            }

            VeChu(payList[i].id.c_str(), 310, startY, 20, textWhite);
            VeChu(payList[i].memberName.c_str(), 430, startY, 20, textWhite);
            
            std::string amountDisplay = payList[i].amount;
            if (amountDisplay.find("VN") == std::string::npos) amountDisplay += " VNĐ";
            VeChu(amountDisplay.c_str(), 630, startY, 20, neonBlue);
            
            VeChu(payList[i].date.c_str(), 800, startY, 20, textWhite);

            Color statusColor = textWhite;
            if (payList[i].status.find("Đã thu") != std::string::npos) statusColor = neonGreen;
            else if (payList[i].status.find("Chờ") != std::string::npos) statusColor = neonOrange;
            else statusColor = { 239, 68, 68, 255 }; 
            
            VeChu(payList[i].status.c_str(), 980, startY, 20, statusColor);
            DrawLine(280, startY + 40, 1150, startY + 40, {51, 65, 85, 255});
        }
        startY += itemHeight;
    }
    EndScissorMode();

    if (canUndoPay && undoPayTimer > 0 && !showPayModal) {
        undoPayTimer--;
        DrawRectangleRounded({ 700, 680, 430, 60 }, 0.2f, 10, { 15, 23, 42, 230 });
        DrawRectangleRoundedLines({ 700, 680, 430, 60 }, 0.2f, 10, neonRed);
        std::string msg = "Da huy HD: " + lastDeletedPay.id;
        VeChu(msg.c_str(), 720, 700, 18, textWhite);

        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
        if (GuiButton({ 990, 690, 120, 40 }, "HOAN TAC")) {
            if (lastDeletedPayIndex >= 0 && lastDeletedPayIndex <= payList.size()) payList.insert(payList.begin() + lastDeletedPayIndex, lastDeletedPay);
            else payList.push_back(lastDeletedPay); 
            SavePayments(payList); 
            canUndoPay = false; 
            undoPayTimer = 0;
            selectedPayIndex = lastDeletedPayIndex; 
        }
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); 
        if (undoPayTimer <= 0) canUndoPay = false; 
    }

    // --- POPUP TẠO / SỬA HÓA ĐƠN ---
    if (showPayModal) {
        DrawRectangle(0, 0, 1200, 800, { 0, 0, 0, 150 });
        DrawRectangleRounded({ 400, 130, 450, 560 }, 0.05f, 10, bgMain);
        DrawRectangleRoundedLines({ 400, 130, 450, 560 }, 0.05f, 10, bgSidebar);

        VeChu(isEditMode ? "CHỈNH SỬA HÓA ĐƠN" : "TẠO HÓA ĐƠN MỚI", 520, 160, 24, neonGreen);

        std::string memberOptionsStr = "";
        if (memberList.empty()) {
            memberOptionsStr = "Chua co Hoi vien";
        } else {
            for (size_t i = 0; i < memberList.size(); i++) {
                memberOptionsStr += memberList[i].id + " - " + memberList[i].name; 
                if (i < memberList.size() - 1) memberOptionsStr += ";";
            }
        }

        if (activeMemberIndex >= memberList.size()) activeMemberIndex = 0;

        // Auto điền giá tiền (chỉ nhảy khi người dùng thực sự click đổi Hội viên khác)
        if (!memberList.empty() && activeMemberIndex != previousMemberIndex) {
            std::string selectedPlan = memberList[activeMemberIndex].plan;
            
            if (selectedPlan == "Basic 1 Thang") strncpy(inputPayAmount, "500,000", 63);
            else if (selectedPlan == "Basic 3 Thang") strncpy(inputPayAmount, "1,200,000", 63);
            else if (selectedPlan == "VIP 6 Thang") strncpy(inputPayAmount, "2,200,000", 63);
            else if (selectedPlan == "VIP 12 Thang") strncpy(inputPayAmount, "3,500,000", 63);
            else strncpy(inputPayAmount, "0", 63);
            
            previousMemberIndex = activeMemberIndex;
        }

        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

        VeChu("Mã GD:", 450, 230, 18, textWhite);
        DrawRectangleRounded({550, 220, 250, 40}, 0.1f, 10, {51, 65, 85, 255}); 
        VeChu(inputPayId, 560, 230, 20, textGray);

        VeChu("Số Tiền:", 450, 350, 18, textWhite);
        if (GuiTextBox({ 550, 340, 250, 40 }, inputPayAmount, 64, editPayAmount)) editPayAmount = !editPayAmount;

        VeChu("Ngày Lập:", 450, 410, 18, textWhite);
        if (GuiTextBox({ 550, 400, 250, 40 }, inputPayDate, 64, editPayDate)) editPayDate = !editPayDate;

        if (errorTimer > 0) {
            VeChu(errorMessage.c_str(), 450, 535, 16, neonRed);
            errorTimer--;
        }

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
        if (GuiButton({ 480, 560, 130, 45 }, "LƯU LẠI") && !editStatusDropdown && !editMemberDropdown) {
            if (strlen(inputPayAmount) == 0) {
                errorMessage = "Vui long nhap so tien!";
                errorTimer = 180;
            } else if (memberList.empty()) {
                errorMessage = "Khong co Hoi Vien nao!";
                errorTimer = 180;
            } else {
                const char* statusArr[] = {"Đã thu (Tiền mặt)", "Đã thu (VNPAY)", "Đã thu (ZaloPay)", "Chờ xử lý"};
                std::string selectedStatusStr = statusArr[activeStatusIndex];
                
                std::string selectedMemberStr = memberList[activeMemberIndex].name;

                if (isEditMode) {
                    payList[selectedPayIndex].memberName = selectedMemberStr;
                    payList[selectedPayIndex].amount = inputPayAmount;
                    payList[selectedPayIndex].date = inputPayDate;
                    payList[selectedPayIndex].status = selectedStatusStr;
                } else {
                    payList.push_back({inputPayId, selectedMemberStr, inputPayAmount, inputPayDate, selectedStatusStr});
                }

                SavePayments(payList);
                showPayModal = false; 
            }
        }

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); 
        if (GuiButton({ 640, 560, 130, 45 }, "HỦY") && !editStatusDropdown && !editMemberDropdown) {
            showPayModal = false;
        }

        VeChu("Trạng Thái:", 450, 470, 18, textWhite);
        GuiSetStyle(DROPDOWNBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
        if (GuiDropdownBox({ 550, 460, 250, 40 }, statusOptions, &activeStatusIndex, editStatusDropdown)) {
            editStatusDropdown = !editStatusDropdown; 
            editMemberDropdown = false; 
        }

        VeChu("Hội Viên:", 450, 290, 18, textWhite);
        if (GuiDropdownBox({ 550, 280, 250, 40 }, memberOptionsStr.c_str(), &activeMemberIndex, editMemberDropdown)) {
            editMemberDropdown = !editMemberDropdown; 
            editStatusDropdown = false; 
        }
    }
}