#include "EquipmentPage.h"
#include "Theme.h"
#include "raygui.h"
#include <fstream>
#include <sstream>
#include <cstring>

const char* EQUIPMENT_FILE = "data/equipment.txt";

void SaveEquipment(const std::vector<Equipment>& eqList) {
    std::ofstream file(EQUIPMENT_FILE);
    if (file.is_open()) {
        for (const auto& eq : eqList) {
            file << eq.id << "|" << eq.name << "|" << eq.category << "|" << eq.status << "\n";
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
            Equipment eq;
            std::getline(ss, eq.id, '|');
            std::getline(ss, eq.name, '|');
            std::getline(ss, eq.category, '|');
            std::getline(ss, eq.status, '|');
            if (!eq.id.empty()) eqList.push_back(eq);
        }
        file.close();
    } else {
        // Dữ liệu mẫu ban đầu
        eqList = {
            {"TB001", "Máy chạy bộ Treadmill X", "Cardio", "Hoạt động"},
            {"TB002", "Dàn tạ đa năng", "Thể lực", "Bảo trì"},
            {"TB003", "Xe đạp tập Elip", "Cardio", "Hoạt động"}
        };
        SaveEquipment(eqList);
    }
}

// Các biến trạng thái của UI
static bool showEqModal = false;
static int selectedEqIndex = -1;
static char inputEqId[64] = "";
static char inputEqName[64] = "";
static char inputEqCat[64] = "";
static char inputEqStatus[64] = "";
static bool editEqId = false, editEqName = false, editEqCat = false, editEqStatus = false;

static void VeChu(const char* text, float x, float y, float size, Color color) {
    DrawTextEx(modernFont, text, {x, y}, size, 1, color);
}

void DrawEquipmentPage(std::vector<Equipment>& eqList) {
    VeChu("QUẢN LÝ THIẾT BỊ", 280, 110, 32, textWhite);

    // --- THANH CÔNG CỤ ---
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    
    // Nút Thêm Mới
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
    if (GuiButton({ 740, 160, 130, 45 }, "THÊM MÁY") && !showEqModal) { 
        showEqModal = true;
        memset(inputEqId, 0, sizeof(inputEqId));
        memset(inputEqName, 0, sizeof(inputEqName));
        memset(inputEqCat, 0, sizeof(inputEqCat));
        memset(inputEqStatus, 0, sizeof(inputEqStatus));
    }

    // Nút Xóa
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 127, 29, 29, 255 })); 
    if (GuiButton({ 880, 160, 130, 45 }, "XÓA MÁY") && !showEqModal) { 
        if (selectedEqIndex != -1) {
            eqList.erase(eqList.begin() + selectedEqIndex);
            selectedEqIndex = -1; 
            SaveEquipment(eqList);
        }
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar));
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    // --- BẢNG DANH SÁCH THIẾT BỊ ---
    DrawRectangleRounded({ 280, 230, 870, 530 }, 0.05f, 10, bgCard);
    VeChu("MÃ TB", 310, 250, 18, textGray);
    VeChu("TÊN THIẾT BỊ", 450, 250, 18, textGray);
    VeChu("PHÂN LOẠI", 750, 250, 18, textGray);
    VeChu("TÌNH TRẠNG", 950, 250, 18, textGray);
    DrawLine(280, 280, 1150, 280, {81, 99, 122, 255});

    int startY = 300;
    for (size_t i = 0; i < eqList.size(); i++) {
        Rectangle rowRect = { 280, (float)startY - 10, 870, 45 };
        
        if (!showEqModal && CheckCollisionPointRec(GetMousePosition(), rowRect)) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selectedEqIndex = (int)i; 
        }

        if (selectedEqIndex == (int)i) {
            DrawRectangleRec(rowRect, bgCardHover);
            DrawRectangle(280, startY - 10, 4, 45, neonOrange); // Highlight viền cam
        }

        VeChu(eqList[i].id.c_str(), 310, startY, 20, textWhite);
        VeChu(eqList[i].name.c_str(), 450, startY, 20, textWhite);
        VeChu(eqList[i].category.c_str(), 750, startY, 20, textWhite);

        // Đổi màu text trạng thái
        Color statusColor = textWhite;
        if (eqList[i].status == "Hoạt động") statusColor = neonGreen;
        else if (eqList[i].status == "Bảo trì") statusColor = neonOrange;
        else statusColor = { 239, 68, 68, 255 }; // Hỏng (Màu đỏ)
        
        VeChu(eqList[i].status.c_str(), 950, startY, 20, statusColor);

        DrawLine(280, startY + 40, 1150, startY + 40, {51, 65, 85, 255});
        startY += 55;
    }

    // --- POPUP THÊM THIẾT BỊ ---
    if (showEqModal) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0, 0, 0, 150 });
        DrawRectangleRounded({ 400, 150, 450, 500 }, 0.05f, 10, bgMain);
        DrawRectangleRoundedLines({ 400, 150, 450, 500 }, 0.05f, 10, bgSidebar);

        VeChu("THÊM THIẾT BỊ MỚI", 520, 180, 24, neonOrange);

        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

        VeChu("Mã TB:", 450, 260, 20, textWhite);
        if (GuiTextBox({ 550, 250, 250, 40 }, inputEqId, 64, editEqId)) editEqId = !editEqId;

        VeChu("Tên máy:", 450, 320, 20, textWhite);
        if (GuiTextBox({ 550, 310, 250, 40 }, inputEqName, 64, editEqName)) editEqName = !editEqName;

        VeChu("Loại:", 450, 380, 20, textWhite);
        if (GuiTextBox({ 550, 370, 250, 40 }, inputEqCat, 64, editEqCat)) editEqCat = !editEqCat;

        VeChu("Tình trạng:", 450, 440, 20, textWhite);
        if (GuiTextBox({ 550, 430, 250, 40 }, inputEqStatus, 64, editEqStatus)) editEqStatus = !editEqStatus;

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
        if (GuiButton({ 480, 520, 130, 45 }, "LƯU LẠI")) {
            eqList.push_back({inputEqId, inputEqName, inputEqCat, inputEqStatus});
            SaveEquipment(eqList);
            showEqModal = false; 
        }

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); 
        if (GuiButton({ 640, 520, 130, 45 }, "HỦY")) showEqModal = false;
    }
}