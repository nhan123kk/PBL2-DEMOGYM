#include "EquipmentPage.h"
#include "Theme.h"
#include "raygui.h"
#include <cstring>
#include <string>
#include <fstream>

void SaveEquipment(GymList<Equipment>& eqList) {
    std::ofstream file("data/equipment.txt");
    for (int i = 0; i < eqList.size(); i++) {
        file << eqList[i].getId() << "|" << eqList[i].getName() << "|" 
             << eqList[i].getType() << "|" << eqList[i].getQuantity() << "|" << eqList[i].getStatus() << "\n";
    }
}

static bool showEqModal = false; static bool isEditMode = false; static int selectedEqIndex = -1;
static char inputEqId[64] = ""; static char inputEqName[64] = ""; static char inputEqQty[64] = "";
static bool editEqName = false, editEqQty = false;
static std::string currentFilter = "Tất cả"; 
static const char* typeOptions = "Máy Cardio;Máy tập Cơ;Tạ tự do;Phụ kiện & Khác";
static int activeTypeIndex = 0; static bool editTypeDropdown = false;
static const char* statusOptions = "Đang hoạt động;Cần bảo trì;Đã hỏng/Thanh lý";
static int activeStatusIndex = 0; static bool editStatusDropdown = false;
static float scrollOffset = 0.0f; static std::string errorMessage = ""; static int errorTimer = 0;
static Equipment lastDeletedEq; static int lastDeletedEqIndex = -1; static bool canUndoEq = false; static int undoEqTimer = 0;

static void VeChu(const char* text, float x, float y, float size, Color color) { DrawTextEx(modernFont, text, {x, y}, size, 1, color); }

void DrawEquipmentPage(GymList<Equipment>& eqList) {
    VeChu("QUẢN LÝ THIẾT BỊ", 280, 60, 32, textWhite);
    int countAll = eqList.size(); int countCardio = 0, countCo = 0, countTa = 0, countPhuKien = 0;
    
    // Sửa lỗi: Thống kê chuẩn số lượng theo loại để lên thẻ Dashboard
    for (int i = 0; i < eqList.size(); i++) {
        if (eqList[i].getType() == "Máy Cardio") countCardio++;
        else if (eqList[i].getType() == "Máy tập Cơ") countCo++;
        else if (eqList[i].getType() == "Tạ tự do") countTa++;
        else countPhuKien++;
    }

    struct DashboardCard { Rectangle rect; std::string title; std::string filterName; int count; };
    DashboardCard cards[5] = {
        {{280, 110, 160, 80}, "TẤT CẢ", "Tất cả", countAll}, {{455, 110, 160, 80}, "MÁY CARDIO", "Máy Cardio", countCardio},
        {{630, 110, 160, 80}, "MÁY TẬP CƠ", "Máy tập Cơ", countCo}, {{805, 110, 160, 80}, "TẠ TỰ DO", "Tạ tự do", countTa},
        {{980, 110, 160, 80}, "PHỤ KIỆN", "Phụ kiện & Khác", countPhuKien}
    };

    for (int i = 0; i < 5; i++) {
        Color cardBg = (currentFilter == cards[i].filterName) ? Color{20, 83, 45, 255} : bgCard;
        DrawRectangleRounded(cards[i].rect, 0.1f, 10, cardBg);
        if (currentFilter == cards[i].filterName) DrawRectangleRoundedLines(cards[i].rect, 0.1f, 10, neonGreen);
        else DrawRectangleRoundedLines(cards[i].rect, 0.1f, 10, {51, 65, 85, 255});
        if (!showEqModal && CheckCollisionPointRec(GetMousePosition(), cards[i].rect)) {
            DrawRectangleRoundedLines(cards[i].rect, 0.1f, 10, textWhite);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) { currentFilter = cards[i].filterName; scrollOffset = 0.0f; selectedEqIndex = -1; }
        }
        VeChu(cards[i].title.c_str(), cards[i].rect.x + 15, cards[i].rect.y + 15, 16, textGray);
        VeChu(TextFormat("%d", cards[i].count), cards[i].rect.x + 15, cards[i].rect.y + 40, 28, textWhite);
    }

    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER); GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
    if (GuiButton({ 740, 210, 130, 45 }, "THÊM THIẾT BỊ") && !showEqModal) { 
        showEqModal = true; isEditMode = false; errorMessage = ""; editTypeDropdown = editStatusDropdown = false;
        int maxIdNum = 0;
        for (int i = 0; i < eqList.size(); i++) {
            std::string numStr = ""; std::string idStr = eqList[i].getId();
            for (char c : idStr) if (isdigit(c)) numStr += c;
            if (!numStr.empty()) { int num = std::stoi(numStr); if (num > maxIdNum) maxIdNum = num; }
        }
        snprintf(inputEqId, sizeof(inputEqId), "TB%03d", maxIdNum + 1);
        memset(inputEqName, 0, sizeof(inputEqName)); memset(inputEqQty, 0, sizeof(inputEqQty));
        activeTypeIndex = 0; activeStatusIndex = 0;
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 124, 45, 18, 255 })); 
    if (GuiButton({ 880, 210, 130, 45 }, "CHỈNH SỬA") && !showEqModal) { 
        if (selectedEqIndex != -1 && selectedEqIndex < eqList.size()) { 
            showEqModal = true; isEditMode = true; errorMessage = ""; editTypeDropdown = editStatusDropdown = false;
            strncpy(inputEqId, eqList[selectedEqIndex].getId().c_str(), 63);
            strncpy(inputEqName, eqList[selectedEqIndex].getName().c_str(), 63);
            strncpy(inputEqQty, eqList[selectedEqIndex].getQuantity().c_str(), 63);
            
            const char* typeArr[] = {"Máy Cardio", "Máy tập Cơ", "Tạ tự do", "Phụ kiện & Khác"};
            activeTypeIndex = 0; for (int i=0; i<4; i++) if (eqList[selectedEqIndex].getType() == typeArr[i]) activeTypeIndex = i;
            const char* stArr[] = {"Đang hoạt động", "Cần bảo trì", "Đã hỏng/Thanh lý"};
            activeStatusIndex = 0; for (int i=0; i<3; i++) if (eqList[selectedEqIndex].getStatus() == stArr[i]) activeStatusIndex = i;
        }
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 127, 29, 29, 255 })); 
    if (GuiButton({ 1020, 210, 130, 45 }, "XÓA") && !showEqModal) { 
        if (selectedEqIndex != -1 && selectedEqIndex < eqList.size()) {
            lastDeletedEq = eqList[selectedEqIndex]; lastDeletedEqIndex = selectedEqIndex;
            canUndoEq = true; undoEqTimer = 300; 
            eqList.erase(selectedEqIndex); selectedEqIndex = -1; SaveEquipment(eqList);
        }
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    DrawRectangleRounded({ 280, 280, 870, 490 }, 0.05f, 10, bgCard);
    VeChu("MÃ TB", 310, 300, 18, textGray); VeChu("TÊN THIẾT BỊ", 430, 300, 18, textGray);
    VeChu("TRẠNG THÁI", 940, 300, 18, textGray); DrawLine(280, 330, 1150, 330, {81, 99, 122, 255});

    GymList<int> filteredIndices;
    for (int i = 0; i < eqList.size(); i++) {
        bool match = false;
        if (currentFilter == "Tất cả") match = true;
        else if (eqList[i].getType() == currentFilter) match = true;
        else if (currentFilter == "Phụ kiện & Khác" && (eqList[i].getType() == "Phụ kiện Gym" || eqList[i].getType() == "Khác")) match = true;
        if (match) filteredIndices.push_back(i);
    }

    Rectangle listArea = { 280, 331, 870, 439 }; float itemHeight = 55.0f; float totalListHeight = filteredIndices.size() * itemHeight;
    if (!showEqModal && CheckCollisionPointRec(GetMousePosition(), listArea)) scrollOffset += GetMouseWheelMove() * 30.0f; 
    float maxScroll = listArea.height - totalListHeight;
    if (maxScroll > 0) maxScroll = 0; if (scrollOffset < maxScroll) scrollOffset = maxScroll; if (scrollOffset > 0) scrollOffset = 0;

    BeginScissorMode(listArea.x, listArea.y, listArea.width, listArea.height);
    float startY = 350 + scrollOffset;
    for (int j = 0; j < filteredIndices.size(); j++) {
        int idx = filteredIndices[j];
        if (startY + 45 >= listArea.y && startY - 10 <= listArea.y + listArea.height) {
            Rectangle rowRect = { 280, startY - 10, 870, 45 };
            if (!showEqModal && CheckCollisionPointRec(GetMousePosition(), rowRect)) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selectedEqIndex = idx; 
            }
            if (selectedEqIndex == idx) { DrawRectangleRec(rowRect, bgCardHover); DrawRectangle(280, startY - 10, 4, 45, neonGreen); }

            VeChu(eqList[idx].getId().c_str(), 310, startY, 20, textWhite);
            VeChu(eqList[idx].getName().c_str(), 430, startY, 20, textWhite);

            Color stColor = textWhite;
            if (eqList[idx].getStatus() == "Đang hoạt động") stColor = neonGreen;
            else if (eqList[idx].getStatus() == "Cần bảo trì") stColor = neonOrange;
            else stColor = { 239, 68, 68, 255 }; 
            VeChu(eqList[idx].getStatus().c_str(), 940, startY, 18, stColor);
            DrawLine(280, startY + 40, 1150, startY + 40, {51, 65, 85, 255});
        }
        startY += itemHeight;
    }
    EndScissorMode(); 

    if (canUndoEq && undoEqTimer > 0 && !showEqModal) {
        undoEqTimer--; DrawRectangleRounded({ 700, 680, 430, 60 }, 0.2f, 10, { 15, 23, 42, 230 });
        DrawRectangleRoundedLines({ 700, 680, 430, 60 }, 0.2f, 10, neonRed);
        std::string msg = "Da xoa: " + lastDeletedEq.getName(); VeChu(msg.c_str(), 720, 700, 18, textWhite);
        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER); GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
        if (GuiButton({ 990, 690, 120, 40 }, "HOAN TAC")) {
            if (lastDeletedEqIndex >= 0 && lastDeletedEqIndex <= eqList.size()) eqList.insert(lastDeletedEqIndex, lastDeletedEq);
            else eqList.push_back(lastDeletedEq);
            canUndoEq = false; undoEqTimer = 0; selectedEqIndex = lastDeletedEqIndex; SaveEquipment(eqList);
        }
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); if (undoEqTimer <= 0) canUndoEq = false; 
    }

    if (showEqModal) {
        DrawRectangle(0, 0, 1200, 800, { 0, 0, 0, 150 }); 
        DrawRectangleRounded({ 400, 130, 450, 560 }, 0.05f, 10, bgMain); DrawRectangleRoundedLines({ 400, 130, 450, 560 }, 0.05f, 10, bgSidebar);
        VeChu(isEditMode ? "CHỈNH SỬA THIẾT BỊ" : "THÊM THIẾT BỊ MỚI", 500, 160, 24, neonGreen);

        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
        VeChu("Mã TB:", 450, 230, 18, textWhite); DrawRectangleRounded({550, 220, 250, 40}, 0.1f, 10, {51, 65, 85, 255}); VeChu(inputEqId, 560, 230, 20, textGray);
        VeChu("Tên TB:", 450, 290, 18, textWhite); if (GuiTextBox({ 550, 280, 250, 40 }, inputEqName, 64, editEqName)) editEqName = !editEqName;
        VeChu("Số lượng:", 450, 350, 18, textWhite); if (GuiTextBox({ 550, 340, 250, 40 }, inputEqQty, 64, editEqQty)) editEqQty = !editEqQty;

        if (errorTimer > 0) { VeChu(errorMessage.c_str(), 450, 535, 16, neonRed); errorTimer--; }

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
        if (GuiButton({ 480, 560, 130, 45 }, "LƯU LẠI") && !editTypeDropdown && !editStatusDropdown) {
            if (strlen(inputEqName) == 0) { errorMessage = "Vui long nhap Ten thiet bi!"; errorTimer = 180; } 
            else {
                const char* typeArr[] = {"Máy Cardio", "Máy tập Cơ", "Tạ tự do", "Phụ kiện & Khác"};
                const char* stArr[] = {"Đang hoạt động", "Cần bảo trì", "Đã hỏng/Thanh lý"};
                if (isEditMode) {
                    eqList.erase(selectedEqIndex);
                    eqList.insert(selectedEqIndex, Equipment(inputEqId, inputEqName, typeArr[activeTypeIndex], inputEqQty, stArr[activeStatusIndex]));
                } else eqList.push_back(Equipment(inputEqId, inputEqName, typeArr[activeTypeIndex], inputEqQty, stArr[activeStatusIndex]));
                SaveEquipment(eqList); showEqModal = false; 
            }
        }

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); 
        if (GuiButton({ 640, 560, 130, 45 }, "HỦY") && !editTypeDropdown && !editStatusDropdown) showEqModal = false;

        VeChu("Tình trạng:", 450, 470, 18, textWhite);
        GuiSetStyle(DROPDOWNBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
        if (GuiDropdownBox({ 550, 460, 250, 40 }, statusOptions, &activeStatusIndex, editStatusDropdown)) { editStatusDropdown = !editStatusDropdown; editTypeDropdown = false; }
        VeChu("Phân loại:", 450, 410, 18, textWhite);
        if (GuiDropdownBox({ 550, 400, 250, 40 }, typeOptions, &activeTypeIndex, editTypeDropdown)) { editTypeDropdown = !editTypeDropdown; editStatusDropdown = false; }
    }
}