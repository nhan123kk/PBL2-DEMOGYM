#include "MembersPage.h"
#include "Theme.h"
#include "raygui.h"
#include <string>
#include <cstring>
#include <fstream>

void SaveMembers(GymList<Member>& list) {
    std::ofstream file("data/members.txt");
    for (int i = 0; i < list.size(); i++) {
        file << list[i].getId() << "|" << list[i].getName() << "|" << list[i].getPhone() << "|" 
             << list[i].getType() << "|" << (list[i].getIsActive() ? "1" : "0") << "\n";
    }
}

static char searchBox[64] = "";
static bool searchEditMode = false;
static int selectedMemberIndex = -1; 
static bool showModal = false;
static bool isEditMode = false;      

static char inputId[64] = "";
static char inputName[64] = "";
static char inputPhone[64] = "";
static bool editName = false, editPhone = false;
static const char* planOptions = "Basic 1 Thang;Basic 3 Thang;VIP 6 Thang;VIP 12 Thang";
static int activePlanIndex = 0;       
static bool editPlanDropdown = false; 
static float scrollOffset = 0.0f;        
static std::string errorMessage = "";    
static int errorTimer = 0;               

static Member lastDeletedMember;         
static int lastDeletedIndex = -1;        
static bool canUndo = false;             
static int undoTimer = 0;                

static void VeChu(const char* text, float x, float y, float size, Color color) { DrawTextEx(modernFont, text, {x, y}, size, 1, color); }

void DrawMembersPage(GymList<Member>& memberList) {
    VeChu("QUẢN LÝ HỘI VIÊN", 280, 110, 32, textWhite);
    GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    
    if (GuiTextBox({ 280, 160, 300, 45 }, searchBox, 64, searchEditMode)) searchEditMode = !searchEditMode;
    VeChu("Nhập tên / mã...", 290, 172, 16, searchBox[0] == '\0' ? textGray : BLANK);

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
    if (GuiButton({ 740, 160, 130, 45 }, "THÊM MỚI") && !showModal) { 
        showModal = true; isEditMode = false; errorMessage = ""; editPlanDropdown = false; 
        int maxIdNum = 0;
        for (int i = 0; i < memberList.size(); i++) {
            std::string numStr = ""; std::string currentId = memberList[i].getId();
            for (char c : currentId) if (isdigit(c)) numStr += c;
            if (!numStr.empty()) { int num = std::stoi(numStr); if (num > maxIdNum) maxIdNum = num; }
        }
        snprintf(inputId, sizeof(inputId), "HV%03d", maxIdNum + 1);
        memset(inputName, 0, sizeof(inputName)); memset(inputPhone, 0, sizeof(inputPhone));
        activePlanIndex = 0; 
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 124, 45, 18, 255 })); 
    if (GuiButton({ 880, 160, 130, 45 }, "CHỈNH SỬA") && !showModal) { 
        if (selectedMemberIndex != -1 && selectedMemberIndex < memberList.size()) { 
            showModal = true; isEditMode = true; errorMessage = ""; editPlanDropdown = false;
            strncpy(inputId, memberList[selectedMemberIndex].getId().c_str(), 63);
            strncpy(inputName, memberList[selectedMemberIndex].getName().c_str(), 63);
            strncpy(inputPhone, memberList[selectedMemberIndex].getPhone().c_str(), 63);
            std::string currentPlan = memberList[selectedMemberIndex].getType();
            if (currentPlan == "Basic 1 Thang") activePlanIndex = 0;
            else if (currentPlan == "Basic 3 Thang") activePlanIndex = 1;
            else if (currentPlan == "VIP 6 Thang") activePlanIndex = 2;
            else if (currentPlan == "VIP 12 Thang") activePlanIndex = 3;
            else activePlanIndex = 0; 
        }
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 127, 29, 29, 255 })); 
    if (GuiButton({ 1020, 160, 130, 45 }, "XÓA") && !showModal) { 
        if (selectedMemberIndex != -1 && selectedMemberIndex < memberList.size()) {
            lastDeletedMember = memberList[selectedMemberIndex]; lastDeletedIndex = selectedMemberIndex;
            canUndo = true; undoTimer = 300; 
            memberList.erase(selectedMemberIndex); selectedMemberIndex = -1; SaveMembers(memberList);
        }
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar));
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    DrawRectangleRounded({ 280, 230, 870, 530 }, 0.05f, 10, bgCard);
    VeChu("MÃ HV", 310, 250, 18, textGray); VeChu("HỌ VÀ TÊN", 430, 250, 18, textGray);
    VeChu("SỐ ĐIỆN THOẠI", 700, 250, 18, textGray); VeChu("GÓI TẬP", 880, 250, 18, textGray);
    VeChu("TRẠNG THÁI", 1040, 250, 18, textGray);
    DrawLine(280, 280, 1150, 280, {81, 99, 122, 255});

    std::string searchStr(searchBox); GymList<int> filteredIndices; 
    for (int i = 0; i < memberList.size(); i++) {
        if (searchStr.empty() || memberList[i].getName().find(searchStr) != std::string::npos || memberList[i].getId().find(searchStr) != std::string::npos) {
            filteredIndices.push_back(i);
        }
    }

    Rectangle listArea = { 280, 281, 870, 479 }; 
    float itemHeight = 55.0f; float totalListHeight = filteredIndices.size() * itemHeight;
    if (!showModal && CheckCollisionPointRec(GetMousePosition(), listArea)) scrollOffset += GetMouseWheelMove() * 30.0f; 
    float maxScroll = listArea.height - totalListHeight;
    if (maxScroll > 0) maxScroll = 0; 
    if (scrollOffset < maxScroll) scrollOffset = maxScroll;
    if (scrollOffset > 0) scrollOffset = 0;

    BeginScissorMode(listArea.x, listArea.y, listArea.width, listArea.height);
    float startY = 300 + scrollOffset;
    for (int i = 0; i < filteredIndices.size(); i++) {
        int actualIndex = filteredIndices[i];
        if (startY + 45 >= listArea.y && startY - 10 <= listArea.y + listArea.height) {
            Rectangle rowRect = { 280, startY - 10, 870, 45 };
            if (!showModal && CheckCollisionPointRec(GetMousePosition(), rowRect)) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selectedMemberIndex = actualIndex; 
            }
            if (selectedMemberIndex == actualIndex) {
                DrawRectangleRec(rowRect, bgCardHover); DrawRectangle(280, startY - 10, 4, 45, neonGreen); 
            }

            VeChu(memberList[actualIndex].getId().c_str(), 310, startY, 20, textWhite);
            VeChu(memberList[actualIndex].getName().c_str(), 430, startY, 20, textWhite);
            VeChu(memberList[actualIndex].getPhone().c_str(), 700, startY, 20, textWhite);
            VeChu(memberList[actualIndex].getType().c_str(), 880, startY, 20, textWhite);

            if (memberList[actualIndex].getIsActive()) {
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

    if (canUndo && undoTimer > 0 && !showModal) {
        undoTimer--;
        DrawRectangleRounded({ 700, 680, 430, 60 }, 0.2f, 10, { 15, 23, 42, 230 });
        DrawRectangleRoundedLines({ 700, 680, 430, 60 }, 0.2f, 10, neonOrange);
        std::string msg = "Da xoa: " + lastDeletedMember.getName(); VeChu(msg.c_str(), 720, 700, 18, textWhite);
        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
        if (GuiButton({ 990, 690, 120, 40 }, "HOAN TAC")) {
            if (lastDeletedIndex >= 0 && lastDeletedIndex <= memberList.size()) memberList.insert(lastDeletedIndex, lastDeletedMember);
            else memberList.push_back(lastDeletedMember); 
            canUndo = false; undoTimer = 0; selectedMemberIndex = lastDeletedIndex; SaveMembers(memberList);
        }
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); 
        if (undoTimer <= 0) canUndo = false; 
    }

    if (showModal) {
        DrawRectangle(0, 0, 1200, 800, { 0, 0, 0, 150 }); 
        DrawRectangleRounded({ 400, 150, 450, 500 }, 0.05f, 10, bgMain);
        DrawRectangleRoundedLines({ 400, 150, 450, 500 }, 0.05f, 10, bgSidebar);
        VeChu(isEditMode ? "CHỈNH SỬA HỘI VIÊN" : "THÊM HỘI VIÊN MỚI", 520, 180, 24, neonGreen);

        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
        VeChu("Mã HV:", 450, 260, 20, textWhite); DrawRectangleRounded({550, 250, 250, 40}, 0.1f, 10, {51, 65, 85, 255}); VeChu(inputId, 560, 260, 20, textGray);
        VeChu("Họ Tên:", 450, 320, 20, textWhite); if (GuiTextBox({ 550, 310, 250, 40 }, inputName, 64, editName)) editName = !editName;
        VeChu("SĐT:", 450, 380, 20, textWhite); if (GuiTextBox({ 550, 370, 250, 40 }, inputPhone, 64, editPhone)) editPhone = !editPhone;

        if (errorTimer > 0) { VeChu(errorMessage.c_str(), 450, 485, 16, neonRed); errorTimer--; }

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
        if (GuiButton({ 480, 520, 130, 45 }, "LƯU LẠI") && !editPlanDropdown) {
            if (strlen(inputName) == 0 || strlen(inputPhone) == 0) { errorMessage = "Vui long nhap day du Ho Ten va SDT!"; errorTimer = 180; } 
            else {
                const char* planArr[] = {"Basic 1 Thang", "Basic 3 Thang", "VIP 6 Thang", "VIP 12 Thang"};
                std::string selectedPlanStr = planArr[activePlanIndex];
                if (isEditMode) { memberList.erase(selectedMemberIndex); memberList.insert(selectedMemberIndex, Member(inputId, inputName, inputPhone, selectedPlanStr, true)); } 
                else memberList.push_back(Member(inputId, inputName, inputPhone, selectedPlanStr, true));
                SaveMembers(memberList); showModal = false; 
            }
        }
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); 
        if (GuiButton({ 640, 520, 130, 45 }, "HỦY") && !editPlanDropdown) showModal = false;

        VeChu("Gói Tập:", 450, 440, 20, textWhite);
        GuiSetStyle(DROPDOWNBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
        if (GuiDropdownBox({ 550, 430, 250, 40 }, planOptions, &activePlanIndex, editPlanDropdown)) editPlanDropdown = !editPlanDropdown; 
    }
}