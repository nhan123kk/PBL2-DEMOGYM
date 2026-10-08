#include "MembersPage.h"
#include "Theme.h"
#include "raygui.h"
#include <string>
#include <cstring>
#include <vector>

// Các biến lưu trạng thái của trang
static char searchBox[64] = "";
static bool searchEditMode = false;
static int selectedMemberIndex = -1; 
static bool showModal = false;
static bool isEditMode = false;      

static char inputId[64] = "";
static char inputName[64] = "";
static char inputPhone[64] = "";
static bool editName = false, editPhone = false;

// Biến cho Dropdown Gói Tập
static const char* planOptions = "Basic 1 Thang;Basic 3 Thang;VIP 6 Thang;VIP 12 Thang";
static int activePlanIndex = 0;       
static bool editPlanDropdown = false; 

// Biến cho Cuộn chuột và Báo lỗi
static float scrollOffset = 0.0f;        
static std::string errorMessage = "";    
static int errorTimer = 0;               

// --- BIẾN CHO TÍNH NĂNG HOÀN TÁC (UNDO) ---
static Member lastDeletedMember;         // Lưu bản sao của hội viên vừa xóa
static int lastDeletedIndex = -1;        // Lưu vị trí cũ để chèn lại đúng chỗ
static bool canUndo = false;             // Trạng thái cho phép hoàn tác
static int undoTimer = 0;                // Đồng hồ đếm ngược 5 giây

static void VeChu(const char* text, float x, float y, float size, Color color) {
    DrawTextEx(modernFont, text, {x, y}, size, 1, color);
}

void DrawMembersPage(std::vector<Member>& memberList) {
    VeChu("QUẢN LÝ HỘI VIÊN", 280, 110, 32, textWhite);

    GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    
    // --- THANH TÌM KIẾM ---
    if (GuiTextBox({ 280, 160, 300, 45 }, searchBox, 64, searchEditMode)) searchEditMode = !searchEditMode;
    VeChu("Nhập tên / mã...", 290, 172, 16, searchBox[0] == '\0' ? textGray : BLANK);

    // --- NÚT THÊM MỚI ---
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
    if (GuiButton({ 740, 160, 130, 45 }, "THÊM MỚI") && !showModal) { 
        showModal = true;
        isEditMode = false;
        errorMessage = ""; 
        editPlanDropdown = false; 
        
        int maxIdNum = 0;
        for (const auto& m : memberList) {
            std::string numStr = "";
            for (char c : m.id) if (isdigit(c)) numStr += c;
            if (!numStr.empty()) {
                int num = std::stoi(numStr);
                if (num > maxIdNum) maxIdNum = num;
            }
        }
        snprintf(inputId, sizeof(inputId), "HV%03d", maxIdNum + 1);

        memset(inputName, 0, sizeof(inputName));
        memset(inputPhone, 0, sizeof(inputPhone));
        activePlanIndex = 0; 
    }

    // --- NÚT CHỈNH SỬA ---
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 124, 45, 18, 255 })); 
    if (GuiButton({ 880, 160, 130, 45 }, "CHỈNH SỬA") && !showModal) { 
        if (selectedMemberIndex != -1 && selectedMemberIndex < memberList.size()) { 
            showModal = true;
            isEditMode = true;
            errorMessage = ""; 
            editPlanDropdown = false;
            
            strncpy(inputId, memberList[selectedMemberIndex].id.c_str(), 63);
            strncpy(inputName, memberList[selectedMemberIndex].name.c_str(), 63);
            strncpy(inputPhone, memberList[selectedMemberIndex].phone.c_str(), 63);
            
            std::string currentPlan = memberList[selectedMemberIndex].plan;
            if (currentPlan == "Basic 1 Thang") activePlanIndex = 0;
            else if (currentPlan == "Basic 3 Thang") activePlanIndex = 1;
            else if (currentPlan == "VIP 6 Thang") activePlanIndex = 2;
            else if (currentPlan == "VIP 12 Thang") activePlanIndex = 3;
            else activePlanIndex = 0; 
        }
    }

    // --- NÚT XÓA (CÓ TÍCH HỢP LƯU LẠI ĐỂ UNDO) ---
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 127, 29, 29, 255 })); 
    if (GuiButton({ 1020, 160, 130, 45 }, "XÓA") && !showModal) { 
        if (selectedMemberIndex != -1 && selectedMemberIndex < memberList.size()) {
            // 1. Lưu bản sao vào bộ nhớ tạm trước khi xóa
            lastDeletedMember = memberList[selectedMemberIndex];
            lastDeletedIndex = selectedMemberIndex;
            canUndo = true;
            undoTimer = 300; // Khởi động đồng hồ đếm ngược 5 giây (60fps * 5)

            // 2. Tiến hành xóa khỏi danh sách
            memberList.erase(memberList.begin() + selectedMemberIndex);
            selectedMemberIndex = -1; 
            SaveMembers(memberList); // Lưu vào ổ cứng (coi như đã xóa thật)
        }
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar));
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    // --- BẢNG DỮ LIỆU & TIÊU ĐỀ ---
    DrawRectangleRounded({ 280, 230, 870, 530 }, 0.05f, 10, bgCard);
    VeChu("MÃ HV", 310, 250, 18, textGray);
    VeChu("HỌ VÀ TÊN", 430, 250, 18, textGray);
    VeChu("SỐ ĐIỆN THOẠI", 700, 250, 18, textGray);
    VeChu("GÓI TẬP", 880, 250, 18, textGray);
    VeChu("TRẠNG THÁI", 1040, 250, 18, textGray);
    DrawLine(280, 280, 1150, 280, {81, 99, 122, 255});

    // --- THUẬT TOÁN TÌM KIẾM ---
    std::string searchStr(searchBox);
    std::vector<int> filteredIndices; 
    for (size_t i = 0; i < memberList.size(); i++) {
        if (searchStr.empty() || 
            memberList[i].name.find(searchStr) != std::string::npos || 
            memberList[i].id.find(searchStr) != std::string::npos) {
            filteredIndices.push_back(i);
        }
    }

    // --- THUẬT TOÁN CUỘN CHUỘT ---
    Rectangle listArea = { 280, 281, 870, 479 }; 
    float itemHeight = 55.0f;
    float totalListHeight = filteredIndices.size() * itemHeight;
    
    if (!showModal && CheckCollisionPointRec(GetMousePosition(), listArea)) {
        scrollOffset += GetMouseWheelMove() * 30.0f; 
    }
    
    float maxScroll = listArea.height - totalListHeight;
    if (maxScroll > 0) maxScroll = 0; 
    if (scrollOffset < maxScroll) scrollOffset = maxScroll;
    if (scrollOffset > 0) scrollOffset = 0;

    BeginScissorMode(listArea.x, listArea.y, listArea.width, listArea.height);

    float startY = 300 + scrollOffset;
    for (size_t i = 0; i < filteredIndices.size(); i++) {
        int actualIndex = filteredIndices[i];
        
        if (startY + 45 >= listArea.y && startY - 10 <= listArea.y + listArea.height) {
            Rectangle rowRect = { 280, startY - 10, 870, 45 };
            
            if (!showModal && CheckCollisionPointRec(GetMousePosition(), rowRect)) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selectedMemberIndex = actualIndex; 
            }

            if (selectedMemberIndex == actualIndex) {
                DrawRectangleRec(rowRect, bgCardHover);
                DrawRectangle(280, startY - 10, 4, 45, neonGreen); 
            }

            VeChu(memberList[actualIndex].id.c_str(), 310, startY, 20, textWhite);
            VeChu(memberList[actualIndex].name.c_str(), 430, startY, 20, textWhite);
            VeChu(memberList[actualIndex].phone.c_str(), 700, startY, 20, textWhite);
            VeChu(memberList[actualIndex].plan.c_str(), 880, startY, 20, textWhite);

            if (memberList[actualIndex].isActive) {
                DrawRectangleRounded({ 1040, startY - 2, 90, 26 }, 0.5f, 10, { 20, 83, 45, 255 });
                VeChu("Active", 1055, startY, 16, neonGreen);
            } else {
                DrawRectangleRounded({ 1040, startY - 2, 90, 26 }, 0.5f, 10, { 71, 85, 105, 255 });
                VeChu("Expired", 1053, startY, 16, textWhite);
            }
            
            DrawLine(280, startY + 40, 1150, startY + 40, {51, 65, 85, 255});
        }
        startY += itemHeight;
    }
    EndScissorMode(); 

    // ==========================================
    // VẼ THANH THÔNG BÁO HOÀN TÁC (TOAST)
    // ==========================================
    if (canUndo && undoTimer > 0 && !showModal) {
        undoTimer--;

        // Vẽ khung đen báo hiệu
        DrawRectangleRounded({ 700, 680, 430, 60 }, 0.2f, 10, { 15, 23, 42, 230 });
        DrawRectangleRoundedLines({ 700, 680, 430, 60 }, 0.2f, 10, neonOrange);

        std::string msg = "Da xoa: " + lastDeletedMember.name;
        VeChu(msg.c_str(), 720, 700, 18, textWhite);

        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 

        // Nút Hoàn Tác
        if (GuiButton({ 990, 690, 120, 40 }, "HOAN TAC")) {
            // Chèn lại hội viên vào đúng vị trí cũ
            if (lastDeletedIndex >= 0 && lastDeletedIndex <= memberList.size()) {
                memberList.insert(memberList.begin() + lastDeletedIndex, lastDeletedMember);
            } else {
                memberList.push_back(lastDeletedMember); // Dự phòng
            }
            
            SaveMembers(memberList); // Lưu lại vào ổ cứng
            
            canUndo = false; 
            undoTimer = 0;
            selectedMemberIndex = lastDeletedIndex; // Highlight lại người vừa khôi phục
        }
        
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); // Trả lại màu cũ cho nút bấm
        
        if (undoTimer <= 0) canUndo = false; // Hết giờ thì tắt thông báo
    }

    // ==========================================
    // VẼ CỬA SỔ POPUP MODAL (THÊM / SỬA)
    // ==========================================
    if (showModal) {
        DrawRectangle(0, 0, 1200, 800, { 0, 0, 0, 150 }); 
        DrawRectangleRounded({ 400, 150, 450, 500 }, 0.05f, 10, bgMain);
        DrawRectangleRoundedLines({ 400, 150, 450, 500 }, 0.05f, 10, bgSidebar);

        VeChu(isEditMode ? "CHỈNH SỬA HỘI VIÊN" : "THÊM HỘI VIÊN MỚI", 520, 180, 24, neonGreen);

        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

        VeChu("Mã HV:", 450, 260, 20, textWhite);
        DrawRectangleRounded({550, 250, 250, 40}, 0.1f, 10, {51, 65, 85, 255}); 
        VeChu(inputId, 560, 260, 20, textGray);

        VeChu("Họ Tên:", 450, 320, 20, textWhite);
        if (GuiTextBox({ 550, 310, 250, 40 }, inputName, 64, editName)) editName = !editName;

        VeChu("SĐT:", 450, 380, 20, textWhite);
        if (GuiTextBox({ 550, 370, 250, 40 }, inputPhone, 64, editPhone)) editPhone = !editPhone;

        if (errorTimer > 0) {
            VeChu(errorMessage.c_str(), 450, 485, 16, neonRed);
            errorTimer--;
        }

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
        if (GuiButton({ 480, 520, 130, 45 }, "LƯU LẠI") && !editPlanDropdown) {
            if (strlen(inputName) == 0 || strlen(inputPhone) == 0) {
                errorMessage = "Vui long nhap day du Ho Ten va SDT!";
                errorTimer = 180; 
            } 
            else {
                const char* planArr[] = {"Basic 1 Thang", "Basic 3 Thang", "VIP 6 Thang", "VIP 12 Thang"};
                std::string selectedPlanStr = planArr[activePlanIndex];

                if (isEditMode) {
                    memberList[selectedMemberIndex].name = inputName;
                    memberList[selectedMemberIndex].phone = inputPhone;
                    memberList[selectedMemberIndex].plan = selectedPlanStr;
                } else {
                    memberList.push_back({inputId, inputName, inputPhone, selectedPlanStr, true});
                }
                SaveMembers(memberList); 
                showModal = false; 
            }
        }

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); 
        if (GuiButton({ 640, 520, 130, 45 }, "HỦY") && !editPlanDropdown) {
            showModal = false;
        }

        VeChu("Gói Tập:", 450, 440, 20, textWhite);
        GuiSetStyle(DROPDOWNBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
        if (GuiDropdownBox({ 550, 430, 250, 40 }, planOptions, &activePlanIndex, editPlanDropdown)) {
            editPlanDropdown = !editPlanDropdown; 
        }
    }
}