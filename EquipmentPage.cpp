#include "EquipmentPage.h"
#include "Theme.h"
#include "raygui.h"
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>

const char* EQUIPMENT_FILE = "data/equipment.txt";

void SaveEquipment(const std::vector<Equipment>& eqList) {
    std::ofstream file(EQUIPMENT_FILE);
    if (file.is_open()) {
        for (const auto& e : eqList) {
            file << e.id << "|" << e.name << "|" << e.type << "|" << e.quantity << "|" << e.status << "\n";
        }
        file.close();
    }
}

void LoadEquipment(std::vector<Equipment>& eqList) {
    std::ifstream file(EQUIPMENT_FILE);
    if (file.is_open()) {
        eqList.clear();
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            Equipment e;
            std::getline(ss, e.id, '|');
            std::getline(ss, e.name, '|');
            std::getline(ss, e.type, '|');
            std::getline(ss, e.quantity, '|');
            std::getline(ss, e.status, '|');
            if (!e.id.empty()) eqList.push_back(e);
        }
        file.close();
    } else {
        eqList = {
            {"TB001", "Máy chạy bộ Elip", "Máy Cardio", "5", "Đang hoạt động"},
            {"TB002", "Dàn tạ đa năng", "Máy tập Cơ", "2", "Cần bảo trì"},
            {"TB003", "Tạ đơn 10kg", "Tạ tự do", "20", "Đang hoạt động"}
        };
        SaveEquipment(eqList);
    }
}

// --- BIẾN TRẠNG THÁI GIAO DIỆN ---
static bool showEqModal = false;
static bool isEditMode = false;
static int selectedEqIndex = -1;

static char inputEqId[64] = "";
static char inputEqName[64] = "";
static char inputEqQty[64] = "";
static bool editEqName = false, editEqQty = false;

// Dropdown Phân loại
static const char* typeOptions = "Máy Cardio;Máy tập Cơ;Tạ tự do;Phụ kiện Gym;Khác";
static int activeTypeIndex = 0;
static bool editTypeDropdown = false;

// Dropdown Trạng thái
static const char* statusOptions = "Đang hoạt động;Cần bảo trì;Đã hỏng/Thanh lý";
static int activeStatusIndex = 0;
static bool editStatusDropdown = false;

// Cuộn & Lỗi
static float scrollOffset = 0.0f;
static std::string errorMessage = "";
static int errorTimer = 0;

// Hoàn tác (Undo)
static Equipment lastDeletedEq;
static int lastDeletedEqIndex = -1;
static bool canUndoEq = false;
static int undoEqTimer = 0;

static void VeChu(const char* text, float x, float y, float size, Color color) {
    DrawTextEx(modernFont, text, {x, y}, size, 1, color);
}

void DrawEquipmentPage(std::vector<Equipment>& eqList) {
    VeChu("QUẢN LÝ THIẾT BỊ", 280, 110, 32, textWhite);

    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    // --- NÚT THÊM MỚI ---
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
    if (GuiButton({ 740, 160, 130, 45 }, "THÊM THIẾT BỊ") && !showEqModal) { 
        showEqModal = true;
        isEditMode = false;
        errorMessage = "";
        editTypeDropdown = editStatusDropdown = false;
        
        // Auto ID
        int maxIdNum = 0;
        for (const auto& e : eqList) {
            std::string numStr = "";
            for (char c : e.id) if (isdigit(c)) numStr += c;
            if (!numStr.empty()) {
                int num = std::stoi(numStr);
                if (num > maxIdNum) maxIdNum = num;
            }
        }
        snprintf(inputEqId, sizeof(inputEqId), "TB%03d", maxIdNum + 1);

        memset(inputEqName, 0, sizeof(inputEqName));
        memset(inputEqQty, 0, sizeof(inputEqQty));
        activeTypeIndex = 0;
        activeStatusIndex = 0;
    }

    // --- NÚT CHỈNH SỬA ---
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 124, 45, 18, 255 })); 
    if (GuiButton({ 880, 160, 130, 45 }, "CHỈNH SỬA") && !showEqModal) { 
        if (selectedEqIndex != -1 && selectedEqIndex < eqList.size()) { 
            showEqModal = true;
            isEditMode = true;
            errorMessage = "";
            editTypeDropdown = editStatusDropdown = false;
            
            strncpy(inputEqId, eqList[selectedEqIndex].id.c_str(), 63);
            strncpy(inputEqName, eqList[selectedEqIndex].name.c_str(), 63);
            strncpy(inputEqQty, eqList[selectedEqIndex].quantity.c_str(), 63);
            
            // Map Dropdown Loại
            const char* typeArr[] = {"Máy Cardio", "Máy tập Cơ", "Tạ tự do", "Phụ kiện Gym", "Khác"};
            activeTypeIndex = 0;
            for (int i=0; i<5; i++) if (eqList[selectedEqIndex].type == typeArr[i]) activeTypeIndex = i;

            // Map Dropdown Trạng thái
            const char* stArr[] = {"Đang hoạt động", "Cần bảo trì", "Đã hỏng/Thanh lý"};
            activeStatusIndex = 0;
            for (int i=0; i<3; i++) if (eqList[selectedEqIndex].status == stArr[i]) activeStatusIndex = i;
        }
    }

    // --- NÚT XÓA ---
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 127, 29, 29, 255 })); 
    if (GuiButton({ 1020, 160, 130, 45 }, "XÓA") && !showEqModal) { 
        if (selectedEqIndex != -1 && selectedEqIndex < eqList.size()) {
            lastDeletedEq = eqList[selectedEqIndex];
            lastDeletedEqIndex = selectedEqIndex;
            canUndoEq = true;
            undoEqTimer = 300; 

            eqList.erase(eqList.begin() + selectedEqIndex);
            selectedEqIndex = -1; 
            SaveEquipment(eqList);
        }
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar));
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    // --- BẢNG DỮ LIỆU ---
    DrawRectangleRounded({ 280, 230, 870, 530 }, 0.05f, 10, bgCard);
    VeChu("MÃ TB", 310, 250, 18, textGray);
    VeChu("TÊN THIẾT BỊ", 430, 250, 18, textGray);
    VeChu("PHÂN LOẠI", 670, 250, 18, textGray);
    VeChu("SL", 840, 250, 18, textGray);
    VeChu("TRẠNG THÁI", 940, 250, 18, textGray);
    DrawLine(280, 280, 1150, 280, {81, 99, 122, 255});

    // --- SCROLLING (CUỘN CHUỘT) ---
    Rectangle listArea = { 280, 281, 870, 479 }; 
    float itemHeight = 55.0f;
    float totalListHeight = eqList.size() * itemHeight;
    
    if (!showEqModal && CheckCollisionPointRec(GetMousePosition(), listArea)) {
        scrollOffset += GetMouseWheelMove() * 30.0f; 
    }
    
    float maxScroll = listArea.height - totalListHeight;
    if (maxScroll > 0) maxScroll = 0; 
    if (scrollOffset < maxScroll) scrollOffset = maxScroll;
    if (scrollOffset > 0) scrollOffset = 0;

    BeginScissorMode(listArea.x, listArea.y, listArea.width, listArea.height);
    float startY = 300 + scrollOffset;
    for (size_t i = 0; i < eqList.size(); i++) {
        if (startY + 45 >= listArea.y && startY - 10 <= listArea.y + listArea.height) {
            Rectangle rowRect = { 280, startY - 10, 870, 45 };
            
            if (!showEqModal && CheckCollisionPointRec(GetMousePosition(), rowRect)) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selectedEqIndex = (int)i; 
            }

            if (selectedEqIndex == (int)i) {
                DrawRectangleRec(rowRect, bgCardHover);
                DrawRectangle(280, startY - 10, 4, 45, neonGreen); 
            }

            VeChu(eqList[i].id.c_str(), 310, startY, 20, textWhite);
            VeChu(eqList[i].name.c_str(), 430, startY, 20, textWhite);
            VeChu(eqList[i].type.c_str(), 670, startY, 18, neonBlue);
            VeChu(eqList[i].quantity.c_str(), 845, startY, 20, textWhite);

            Color stColor = textWhite;
            if (eqList[i].status == "Đang hoạt động") stColor = neonGreen;
            else if (eqList[i].status == "Cần bảo trì") stColor = neonOrange;
            else stColor = { 239, 68, 68, 255 }; // Đỏ hỏng hóc
            
            VeChu(eqList[i].status.c_str(), 940, startY, 18, stColor);

            DrawLine(280, startY + 40, 1150, startY + 40, {51, 65, 85, 255});
        }
        startY += itemHeight;
    }
    EndScissorMode(); 

    // --- TOAST HOÀN TÁC ---
    if (canUndoEq && undoEqTimer > 0 && !showEqModal) {
        undoEqTimer--;
        DrawRectangleRounded({ 700, 680, 430, 60 }, 0.2f, 10, { 15, 23, 42, 230 });
        DrawRectangleRoundedLines({ 700, 680, 430, 60 }, 0.2f, 10, neonRed);

        std::string msg = "Da xoa: " + lastDeletedEq.name;
        VeChu(msg.c_str(), 720, 700, 18, textWhite);

        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 

        if (GuiButton({ 990, 690, 120, 40 }, "HOAN TAC")) {
            if (lastDeletedEqIndex >= 0 && lastDeletedEqIndex <= eqList.size()) {
                eqList.insert(eqList.begin() + lastDeletedEqIndex, lastDeletedEq);
            } else {
                eqList.push_back(lastDeletedEq); 
            }
            SaveEquipment(eqList); 
            canUndoEq = false; 
            undoEqTimer = 0;
            selectedEqIndex = lastDeletedEqIndex; 
        }
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); 
        if (undoEqTimer <= 0) canUndoEq = false; 
    }

    // --- POPUP MODAL THÊM / SỬA ---
    if (showEqModal) {
        DrawRectangle(0, 0, 1200, 800, { 0, 0, 0, 150 }); 
        DrawRectangleRounded({ 400, 130, 450, 560 }, 0.05f, 10, bgMain);
        DrawRectangleRoundedLines({ 400, 130, 450, 560 }, 0.05f, 10, bgSidebar);

        VeChu(isEditMode ? "CHỈNH SỬA THIẾT BỊ" : "THÊM THIẾT BỊ MỚI", 500, 160, 24, neonGreen);

        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

        // Khóa ID
        VeChu("Mã TB:", 450, 230, 18, textWhite);
        DrawRectangleRounded({550, 220, 250, 40}, 0.1f, 10, {51, 65, 85, 255}); 
        VeChu(inputEqId, 560, 230, 20, textGray);

        VeChu("Tên TB:", 450, 290, 18, textWhite);
        if (GuiTextBox({ 550, 280, 250, 40 }, inputEqName, 64, editEqName)) editEqName = !editEqName;

        VeChu("Số lượng:", 450, 350, 18, textWhite);
        if (GuiTextBox({ 550, 340, 250, 40 }, inputEqQty, 64, editEqQty)) editEqQty = !editEqQty;

        if (errorTimer > 0) {
            VeChu(errorMessage.c_str(), 450, 535, 16, neonRed);
            errorTimer--;
        }

        // VẼ NÚT LƯU & HỦY TRƯỚC (Để Dropdown đè lên)
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
        if (GuiButton({ 480, 560, 130, 45 }, "LƯU LẠI") && !editTypeDropdown && !editStatusDropdown) {
            if (strlen(inputEqName) == 0 || strlen(inputEqQty) == 0) {
                errorMessage = "Vui long nhap Ten va So luong!";
                errorTimer = 180; 
            } 
            else {
                const char* typeArr[] = {"Máy Cardio", "Máy tập Cơ", "Tạ tự do", "Phụ kiện Gym", "Khác"};
                const char* stArr[] = {"Đang hoạt động", "Cần bảo trì", "Đã hỏng/Thanh lý"};
                
                if (isEditMode) {
                    eqList[selectedEqIndex].name = inputEqName;
                    eqList[selectedEqIndex].quantity = inputEqQty;
                    eqList[selectedEqIndex].type = typeArr[activeTypeIndex];
                    eqList[selectedEqIndex].status = stArr[activeStatusIndex];
                } else {
                    eqList.push_back({inputEqId, inputEqName, typeArr[activeTypeIndex], inputEqQty, stArr[activeStatusIndex]});
                }
                SaveEquipment(eqList); 
                showEqModal = false; 
            }
        }

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); 
        if (GuiButton({ 640, 560, 130, 45 }, "HỦY") && !editTypeDropdown && !editStatusDropdown) {
            showEqModal = false;
        }

        // VẼ DROPDOWN TRẠNG THÁI (Lớp đè 2)
        VeChu("Tình trạng:", 450, 470, 18, textWhite);
        GuiSetStyle(DROPDOWNBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
        if (GuiDropdownBox({ 550, 460, 250, 40 }, statusOptions, &activeStatusIndex, editStatusDropdown)) {
            editStatusDropdown = !editStatusDropdown; 
            editTypeDropdown = false; // Đóng cái kia nếu đang mở
        }

        // VẼ DROPDOWN PHÂN LOẠI (Lớp đè 1 - Trên cùng)
        VeChu("Phân loại:", 450, 410, 18, textWhite);
        if (GuiDropdownBox({ 550, 400, 250, 40 }, typeOptions, &activeTypeIndex, editTypeDropdown)) {
            editTypeDropdown = !editTypeDropdown; 
            editStatusDropdown = false; // Đóng cái kia nếu đang mở
        }
    }
}