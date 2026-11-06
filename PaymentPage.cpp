#include "PaymentPage.h"
#include "Theme.h"
#include "raygui.h"
#include <cstring>
#include <string>
#include <ctime>
#include <fstream>

// --- HÀM PHỤ TRỢ: THÊM DẤU PHẨY VÀO SỐ TIỀN ---
std::string FormatMoney(double amount) {
    std::string s = std::to_string((long long)amount);
    int n = s.length();
    for (int i = n - 3; i > 0; i -= 3) {
        s.insert(i, ",");
    }
    return s + " VNĐ";
}

void SavePayments(GymList<PaymentRecord>& payList) {
    std::ofstream file("data/payments.txt");
    for (int i = 0; i < payList.size(); i++) {
        char amtBuf[64]; snprintf(amtBuf, sizeof(amtBuf), "%.0f", payList[i].getAmount()); // Vẫn lưu số gốc vào file
        file << payList[i].getPaymentId() << "|" << payList[i].getMemberId() << "|" 
             << amtBuf << "|" << payList[i].getDate() << "|" << payList[i].getStatus() << "\n";
    }
}

static bool showPayModal = false; static bool isEditMode = false; static int selectedPayIndex = -1;
static char inputPayId[64] = ""; static char inputPayAmount[64] = ""; static char inputPayDate[64] = "";
static bool editPayAmount = false, editPayDate = false;
static int activeMemberIndex = 0; static int previousMemberIndex = -1;  static bool editMemberDropdown = false;
static float scrollOffset = 0.0f; static std::string errorMessage = ""; static int errorTimer = 0;               
static PaymentRecord lastDeletedPay; static int lastDeletedPayIndex = -1; static bool canUndoPay = false; static int undoPayTimer = 0;

static const char* statusOptions = "Đã thu (Tiền mặt);Đã thu (VNPAY);Đã thu (ZaloPay);Chờ xử lý";
static int activeStatusIndex = 0;       
static bool editStatusDropdown = false; 

static void VeChu(const char* text, float x, float y, float size, Color color) { DrawTextEx(modernFont, text, {x, y}, size, 1, color); }

void DrawPaymentPage(GymList<PaymentRecord>& payList, GymList<Member>& memberList) {
    VeChu("QUẢN LÝ THANH TOÁN", 280, 110, 32, textWhite);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER); GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
    if (GuiButton({ 740, 160, 130, 45 }, "TẠO HÓA ĐƠN") && !showPayModal) { 
        showPayModal = true; isEditMode = false; errorMessage = ""; editMemberDropdown = false;
        int maxIdNum = 0;
        for (int i = 0; i < payList.size(); i++) {
            std::string numStr = ""; std::string pid = payList[i].getPaymentId();
            for (char c : pid) if (isdigit(c)) numStr += c;
            if (!numStr.empty()) { int num = std::stoi(numStr); if (num > maxIdNum) maxIdNum = num; }
        }
        snprintf(inputPayId, sizeof(inputPayId), "HD%03d", maxIdNum + 1);
        time_t now = time(0); tm *ltm = localtime(&now);
        snprintf(inputPayDate, sizeof(inputPayDate), "%02d/%02d/%04d", ltm->tm_mday, 1 + ltm->tm_mon, 1900 + ltm->tm_year);
        memset(inputPayAmount, 0, sizeof(inputPayAmount));
        activeMemberIndex = 0; previousMemberIndex = -1; 
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 124, 45, 18, 255 })); 
    if (GuiButton({ 880, 160, 130, 45 }, "CHỈNH SỬA") && !showPayModal) { 
        if (selectedPayIndex != -1 && selectedPayIndex < payList.size()) { 
            showPayModal = true; isEditMode = true; errorMessage = ""; editMemberDropdown = false;
            strncpy(inputPayId, payList[selectedPayIndex].getPaymentId().c_str(), 63);
            
            // Format số tiền khi load vào form chỉnh sửa
            std::string formattedAmt = std::to_string((long long)payList[selectedPayIndex].getAmount());
            int n = formattedAmt.length();
            for (int i = n - 3; i > 0; i -= 3) formattedAmt.insert(i, ",");
            strncpy(inputPayAmount, formattedAmt.c_str(), 63); 
            
            strncpy(inputPayDate, payList[selectedPayIndex].getDate().c_str(), 63);
            
            activeMemberIndex = 0;
            for (int i = 0; i < memberList.size(); i++) {
                if (payList[selectedPayIndex].getMemberId() == memberList[i].getName()) { activeMemberIndex = i; break; }
            }
            previousMemberIndex = activeMemberIndex;
            
            const char* statusArr[] = {"Đã thu (Tiền mặt)", "Đã thu (VNPAY)", "Đã thu (ZaloPay)", "Chờ xử lý"};
            activeStatusIndex = 0;
            for (int i = 0; i < 4; i++) if (payList[selectedPayIndex].getStatus() == statusArr[i]) activeStatusIndex = i;
        }
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 127, 29, 29, 255 })); 
    if (GuiButton({ 1020, 160, 130, 45 }, "HỦY HÓA ĐƠN") && !showPayModal) { 
        if (selectedPayIndex != -1 && selectedPayIndex < payList.size()) {
            lastDeletedPay = payList[selectedPayIndex]; lastDeletedPayIndex = selectedPayIndex;
            canUndoPay = true; undoPayTimer = 300; 
            payList.erase(selectedPayIndex); selectedPayIndex = -1; SavePayments(payList);
        }
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    DrawRectangleRounded({ 280, 230, 870, 530 }, 0.05f, 10, bgCard);
    VeChu("MÃ GD", 310, 250, 18, textGray); VeChu("HỘI VIÊN", 430, 250, 18, textGray);
    VeChu("SỐ TIỀN", 730, 250, 18, textGray); VeChu("NGÀY LẬP", 950, 250, 18, textGray);
    DrawLine(280, 280, 1150, 280, {81, 99, 122, 255});

    Rectangle listArea = { 280, 281, 870, 479 }; float itemHeight = 55.0f; float totalListHeight = payList.size() * itemHeight;
    if (!showPayModal && CheckCollisionPointRec(GetMousePosition(), listArea)) scrollOffset += GetMouseWheelMove() * 30.0f; 
    float maxScroll = listArea.height - totalListHeight;
    if (maxScroll > 0) maxScroll = 0; if (scrollOffset < maxScroll) scrollOffset = maxScroll; if (scrollOffset > 0) scrollOffset = 0;

    BeginScissorMode(listArea.x, listArea.y, listArea.width, listArea.height);
    float startY = 300 + scrollOffset;
    for (int i = 0; i < payList.size(); i++) {
        if (startY + 45 >= listArea.y && startY - 10 <= listArea.y + listArea.height) {
            Rectangle rowRect = { 280, startY - 10, 870, 45 };
            if (!showPayModal && CheckCollisionPointRec(GetMousePosition(), rowRect)) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selectedPayIndex = i; 
            }
            if (selectedPayIndex == i) { DrawRectangleRec(rowRect, bgCardHover); DrawRectangle(280, startY - 10, 4, 45, neonGreen); }

            VeChu(payList[i].getPaymentId().c_str(), 310, startY, 20, textWhite);
            VeChu(payList[i].getMemberId().c_str(), 430, startY, 20, textWhite); 
            
            // GỌI HÀM FORMAT TIỀN Ở ĐÂY
            std::string amountDisplay = FormatMoney(payList[i].getAmount());
            VeChu(amountDisplay.c_str(), 730, startY, 20, neonBlue);
            
            VeChu(payList[i].getDate().c_str(), 950, startY, 20, textWhite);
            DrawLine(280, startY + 40, 1150, startY + 40, {51, 65, 85, 255});
        }
        startY += itemHeight;
    }
    EndScissorMode();

    if (canUndoPay && undoPayTimer > 0 && !showPayModal) {
        undoPayTimer--; DrawRectangleRounded({ 700, 680, 430, 60 }, 0.2f, 10, { 15, 23, 42, 230 });
        DrawRectangleRoundedLines({ 700, 680, 430, 60 }, 0.2f, 10, neonRed);
        std::string msg = "Da huy HD: " + lastDeletedPay.getPaymentId(); VeChu(msg.c_str(), 720, 700, 18, textWhite);
        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER); GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
        if (GuiButton({ 990, 690, 120, 40 }, "HOAN TAC")) {
            if (lastDeletedPayIndex >= 0 && lastDeletedPayIndex <= payList.size()) payList.insert(lastDeletedPayIndex, lastDeletedPay);
            else payList.push_back(lastDeletedPay); 
            canUndoPay = false; undoPayTimer = 0; selectedPayIndex = lastDeletedPayIndex; SavePayments(payList);
        }
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); if (undoPayTimer <= 0) canUndoPay = false; 
    }

    if (showPayModal) {
        DrawRectangle(0, 0, 1200, 800, { 0, 0, 0, 150 });
        DrawRectangleRounded({ 400, 130, 450, 560 }, 0.05f, 10, bgMain); DrawRectangleRoundedLines({ 400, 130, 450, 560 }, 0.05f, 10, bgSidebar);
        VeChu(isEditMode ? "CHỈNH SỬA HÓA ĐƠN" : "TẠO HÓA ĐƠN MỚI", 520, 160, 24, neonGreen);

        std::string memberOptionsStr = "";
        if (memberList.empty()) memberOptionsStr = "Chua co Hoi vien";
        else {
            for (int i = 0; i < memberList.size(); i++) {
                memberOptionsStr += memberList[i].getId() + " - " + memberList[i].getName(); 
                if (i < memberList.size() - 1) memberOptionsStr += ";";
            }
        }
        if (activeMemberIndex >= memberList.size()) activeMemberIndex = 0;
        
        // Tự động điền giá tiền KÈM DẤU PHẨY
        if (!memberList.empty() && activeMemberIndex != previousMemberIndex) {
            std::string selectedPlan = memberList[activeMemberIndex].getType(); 
            if (selectedPlan == "Basic 1 Thang") strncpy(inputPayAmount, "500,000", 63);
            else if (selectedPlan == "Basic 3 Thang") strncpy(inputPayAmount, "1,200,000", 63);
            else if (selectedPlan == "VIP 6 Thang") strncpy(inputPayAmount, "2,200,000", 63);
            else if (selectedPlan == "VIP 12 Thang") strncpy(inputPayAmount, "3,500,000", 63);
            else strncpy(inputPayAmount, "0", 63);
            previousMemberIndex = activeMemberIndex;
        }

        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
        VeChu("Mã GD:", 450, 230, 18, textWhite); DrawRectangleRounded({550, 220, 250, 40}, 0.1f, 10, {51, 65, 85, 255}); VeChu(inputPayId, 560, 230, 20, textGray);
        VeChu("Hội Viên:", 450, 290, 18, textWhite); if (GuiDropdownBox({ 550, 280, 250, 40 }, memberOptionsStr.c_str(), &activeMemberIndex, editMemberDropdown)) editMemberDropdown = !editMemberDropdown; 
        VeChu("Số Tiền:", 450, 350, 18, textWhite); if (GuiTextBox({ 550, 340, 250, 40 }, inputPayAmount, 64, editPayAmount)) editPayAmount = !editPayAmount;
        VeChu("Ngày Lập:", 450, 410, 18, textWhite); if (GuiTextBox({ 550, 400, 250, 40 }, inputPayDate, 64, editPayDate)) editPayDate = !editPayDate;
        
        if (errorTimer > 0) { VeChu(errorMessage.c_str(), 450, 535, 16, neonRed); errorTimer--; }

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
        if (GuiButton({ 480, 560, 130, 45 }, "LƯU LẠI") && !editMemberDropdown && !editStatusDropdown) {
            if (strlen(inputPayAmount) == 0) { errorMessage = "Vui long nhap so tien!"; errorTimer = 180; } 
            else if (memberList.empty()) { errorMessage = "Khong co Hoi Vien nao!"; errorTimer = 180; } 
            else {
                // Thuật toán bóc tách dấu phẩy khi người dùng bấm Lưu
                std::string rawAmt = inputPayAmount;
                std::string cleanAmt = "";
                for (char c : rawAmt) if (isdigit(c)) cleanAmt += c;
                
                double amtDouble = 0.0; try { amtDouble = std::stod(cleanAmt); } catch (...) { amtDouble = 0.0; }
                std::string selectedMemberId = memberList[activeMemberIndex].getName(); 
                const char* statusArr[] = {"Đã thu (Tiền mặt)", "Đã thu (VNPAY)", "Đã thu (ZaloPay)", "Chờ xử lý"};
                
                if (isEditMode) {
                    payList.erase(selectedPayIndex); 
                    payList.insert(selectedPayIndex, PaymentRecord(inputPayId, selectedMemberId, amtDouble, inputPayDate, statusArr[activeStatusIndex])); 
                } else payList.push_back(PaymentRecord(inputPayId, selectedMemberId, amtDouble, inputPayDate, statusArr[activeStatusIndex]));
                SavePayments(payList); showPayModal = false; 
            }
        }
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); 
        if (GuiButton({ 640, 560, 130, 45 }, "HỦY") && !editMemberDropdown && !editStatusDropdown) showPayModal = false;

        VeChu("Trạng Thái:", 450, 470, 18, textWhite);
        GuiSetStyle(DROPDOWNBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
        if (GuiDropdownBox({ 550, 460, 250, 40 }, statusOptions, &activeStatusIndex, editStatusDropdown)) { editStatusDropdown = !editStatusDropdown; editMemberDropdown = false; }
    }
}