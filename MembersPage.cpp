#include "MembersPage.h"
#include "Theme.h"
#include "raygui.h"
#include <string>
#include <cstring>

// Các biến lưu trạng thái của trang này (dùng 'static' để giữ nguyên giá trị khi lật trang)
static char searchBox[64] = "";
static bool searchEditMode = false;
static int selectedMemberIndex = -1; 
static bool showModal = false;
static bool isEditMode = false;      

static char inputId[64] = "";
static char inputName[64] = "";
static char inputPhone[64] = "";
static char inputPlan[64] = "";
static bool editId = false, editName = false, editPhone = false, editPlan = false;

static void VeChu(const char* text, float x, float y, float size, Color color) {
    DrawTextEx(modernFont, text, {x, y}, size, 1, color);
}

void DrawMembersPage(std::vector<Member>& memberList) {
    VeChu("QUẢN LÝ HỘI VIÊN", 280, 110, 32, textWhite);

    GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    
    // THANH TÌM KIẾM
    if (GuiTextBox({ 280, 160, 300, 45 }, searchBox, 64, searchEditMode)) searchEditMode = !searchEditMode;
    VeChu("Nhập tên / mã...", 290, 172, 16, searchBox[0] == '\0' ? textGray : BLANK);

    // NÚT THÊM MỚI
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
    if (GuiButton({ 740, 160, 130, 45 }, "THÊM MỚI") && !showModal) { 
        showModal = true;
        isEditMode = false;
        memset(inputId, 0, sizeof(inputId));
        memset(inputName, 0, sizeof(inputName));
        memset(inputPhone, 0, sizeof(inputPhone));
        memset(inputPlan, 0, sizeof(inputPlan));
    }

    // NÚT CHỈNH SỬA
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 124, 45, 18, 255 })); 
    if (GuiButton({ 880, 160, 130, 45 }, "CHỈNH SỬA") && !showModal) { 
        if (selectedMemberIndex != -1) { 
            showModal = true;
            isEditMode = true;
            strncpy(inputId, memberList[selectedMemberIndex].id.c_str(), 63);
            strncpy(inputName, memberList[selectedMemberIndex].name.c_str(), 63);
            strncpy(inputPhone, memberList[selectedMemberIndex].phone.c_str(), 63);
            strncpy(inputPlan, memberList[selectedMemberIndex].plan.c_str(), 63);
        }
    }

    // NÚT XÓA 
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 127, 29, 29, 255 })); 
    if (GuiButton({ 1020, 160, 130, 45 }, "XÓA") && !showModal) { 
        if (selectedMemberIndex != -1) {
            memberList.erase(memberList.begin() + selectedMemberIndex);
            selectedMemberIndex = -1; 
            SaveMembers(memberList); // Lưu ngay vào ổ cứng
        }
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar));
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    // BẢNG DỮ LIỆU
    DrawRectangleRounded({ 280, 230, 870, 530 }, 0.05f, 10, bgCard);
    VeChu("MÃ HV", 310, 250, 18, textGray);
    VeChu("HỌ VÀ TÊN", 430, 250, 18, textGray);
    VeChu("SỐ ĐIỆN THOẠI", 700, 250, 18, textGray);
    VeChu("GÓI TẬP", 880, 250, 18, textGray);
    VeChu("TRẠNG THÁI", 1040, 250, 18, textGray);
    DrawLine(280, 280, 1150, 280, {81, 99, 122, 255});

    int startY = 300;
    std::string searchStr(searchBox);
    
    for (size_t i = 0; i < memberList.size(); i++) {
        if (!searchStr.empty()) {
            if (memberList[i].name.find(searchStr) == std::string::npos && 
                memberList[i].id.find(searchStr) == std::string::npos) {
                continue; 
            }
        }

        Rectangle rowRect = { 280, (float)startY - 10, 870, 45 };
        if (!showModal && CheckCollisionPointRec(GetMousePosition(), rowRect)) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                selectedMemberIndex = (int)i; 
            }
        }

        if (selectedMemberIndex == (int)i) {
            DrawRectangleRec(rowRect, bgCardHover);
            DrawRectangle(280, startY - 10, 4, 45, neonGreen); 
        }

        VeChu(memberList[i].id.c_str(), 310, startY, 20, textWhite);
        VeChu(memberList[i].name.c_str(), 430, startY, 20, textWhite);
        VeChu(memberList[i].phone.c_str(), 700, startY, 20, textWhite);
        VeChu(memberList[i].plan.c_str(), 880, startY, 20, textWhite);

        if (memberList[i].isActive) {
            DrawRectangleRounded({ 1040, startY - 2, 90, 26 }, 0.5f, 10, { 20, 83, 45, 255 });
            VeChu("Active", 1055, startY, 16, neonGreen);
        } else {
            DrawRectangleRounded({ 1040, startY - 2, 90, 26 }, 0.5f, 10, { 71, 85, 105, 255 });
            VeChu("Expired", 1053, startY, 16, textWhite);
        }
        
        DrawLine(280, startY + 40, 1150, startY + 40, {51, 65, 85, 255});
        startY += 55;
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
        if (GuiTextBox({ 550, 250, 250, 40 }, inputId, 64, editId)) editId = !editId;

        VeChu("Họ Tên:", 450, 320, 20, textWhite);
        if (GuiTextBox({ 550, 310, 250, 40 }, inputName, 64, editName)) editName = !editName;

        VeChu("SĐT:", 450, 380, 20, textWhite);
        if (GuiTextBox({ 550, 370, 250, 40 }, inputPhone, 64, editPhone)) editPhone = !editPhone;

        VeChu("Gói Tập:", 450, 440, 20, textWhite);
        if (GuiTextBox({ 550, 430, 250, 40 }, inputPlan, 64, editPlan)) editPlan = !editPlan;

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
        if (GuiButton({ 480, 520, 130, 45 }, "LƯU LẠI")) {
            if (isEditMode) {
                memberList[selectedMemberIndex].id = inputId;
                memberList[selectedMemberIndex].name = inputName;
                memberList[selectedMemberIndex].phone = inputPhone;
                memberList[selectedMemberIndex].plan = inputPlan;
            } else {
                memberList.push_back({inputId, inputName, inputPhone, inputPlan, true});
            }
            SaveMembers(memberList); // LƯU VÀO Ổ CỨNG NGAY KHI BẤM LƯU
            showModal = false; 
        }

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); 
        if (GuiButton({ 640, 520, 130, 45 }, "HỦY")) {
            showModal = false;
        }
    }
}