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
            
            // Đọc trường cuối cùng
            std::getline(ss, e.status, '|');
            
            // --- ĐOẠN FIX LỖI: Cắt bỏ ký tự \r ẩn của Windows ---
            if (!e.status.empty() && e.status.back() == '\r') {
                e.status.pop_back();
            }
            // ---------------------------------------------------

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

// Biến cho Bộ lọc (Filter Dashboard)
static std::string currentFilter = "Tất cả"; 

// Dropdown Phân loại
static const char* typeOptions = "Máy Cardio;Máy tập Cơ;Tạ tự do;Phụ kiện & Khác";
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
    VeChu("QUẢN LÝ THIẾT BỊ", 280, 60, 32, textWhite);

    // --- THUẬT TOÁN THỐNG KÊ ĐỂ HIỂN THỊ LÊN DASHBOARD ---
    int countAll = (int)eqList.size();
    int countCardio = 0, countCo = 0, countTa = 0, countPhuKien = 0;
    
    for (const auto& e : eqList) {
        if (e.type == "Máy Cardio") countCardio++;
        else if (e.type == "Máy tập Cơ") countCo++;
        else if (e.type == "Tạ tự do") countTa++;
        else if (e.type == "Phụ kiện Gym" || e.type == "Khác" || e.type == "Phụ kiện & Khác") countPhuKien++;
    }

    // --- VẼ 5 Ô VUÔNG PHÂN LOẠI (DASHBOARD WIDGETS) ---
    struct DashboardCard {
        Rectangle rect;
        std::string title;
        std::string filterName;
        int count;
    };

    DashboardCard cards[5] = {
        {{280, 110, 160, 80}, "TẤT CẢ", "Tất cả", countAll},
        {{455, 110, 160, 80}, "MÁY CARDIO", "Máy Cardio", countCardio},
        {{630, 110, 160, 80}, "MÁY TẬP CƠ", "Máy tập Cơ", countCo},
        {{805, 110, 160, 80}, "TẠ TỰ DO", "Tạ tự do", countTa},
        {{980, 110, 160, 80}, "PHỤ KIỆN", "Phụ kiện & Khác", countPhuKien}
    };

    for (int i = 0; i < 5; i++) {
        // Đã sửa lỗi: Dùng trực tiếp Color{} thay vì ColorToInt()
        Color cardBg = (currentFilter == cards[i].filterName) ? Color{20, 83, 45, 255} : bgCard;
        DrawRectangleRounded(cards[i].rect, 0.1f, 10, cardBg);
        
        // Viền thẻ
        if (currentFilter == cards[i].filterName) DrawRectangleRoundedLines(cards[i].rect, 0.1f, 10, neonGreen);
        else DrawRectangleRoundedLines(cards[i].rect, 0.1f, 10, {51, 65, 85, 255});

        // Bắt sự kiện Click vào ô phân loại
        if (!showEqModal && CheckCollisionPointRec(GetMousePosition(), cards[i].rect)) {
            DrawRectangleRoundedLines(cards[i].rect, 0.1f, 10, textWhite);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentFilter = cards[i].filterName;
                scrollOffset = 0.0f; 
                selectedEqIndex = -1; 
            }
        }

        // Chữ trên thẻ
        VeChu(cards[i].title.c_str(), cards[i].rect.x + 15, cards[i].rect.y + 15, 16, textGray);
        VeChu(TextFormat("%d", cards[i].count), cards[i].rect.x + 15, cards[i].rect.y + 40, 28, textWhite);
    }

    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    // --- CÁC NÚT CHỨC NĂNG (Đẩy xuống dưới Dashboard) ---
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
    if (GuiButton({ 740, 210, 130, 45 }, "THÊM THIẾT BỊ") && !showEqModal) { 
        showEqModal = true;
        isEditMode = false;
        errorMessage = "";
        editTypeDropdown = editStatusDropdown = false;
        
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

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 124, 45, 18, 255 })); 
    if (GuiButton({ 880, 210, 130, 45 }, "CHỈNH SỬA") && !showEqModal) { 
        // Đã sửa lỗi Cảnh báo vàng: Thêm (int) trước eqList.size()
        if (selectedEqIndex != -1 && selectedEqIndex < (int)eqList.size()) { 
            showEqModal = true;
            isEditMode = true;
            errorMessage = "";
            editTypeDropdown = editStatusDropdown = false;
            
            strncpy(inputEqId, eqList[selectedEqIndex].id.c_str(), 63);
            strncpy(inputEqName, eqList[selectedEqIndex].name.c_str(), 63);
            strncpy(inputEqQty, eqList[selectedEqIndex].quantity.c_str(), 63);
            
            const char* typeArr[] = {"Máy Cardio", "Máy tập Cơ", "Tạ tự do", "Phụ kiện & Khác"};
            activeTypeIndex = 0;
            for (int i=0; i<4; i++) {
                if (eqList[selectedEqIndex].type == typeArr[i] || 
                   (eqList[selectedEqIndex].type == "Phụ kiện Gym" && i==3) || 
                   (eqList[selectedEqIndex].type == "Khác" && i==3)) {
                    activeTypeIndex = i;
                }
            }

            const char* stArr[] = {"Đang hoạt động", "Cần bảo trì", "Đã hỏng/Thanh lý"};
            activeStatusIndex = 0;
            for (int i=0; i<3; i++) if (eqList[selectedEqIndex].status == stArr[i]) activeStatusIndex = i;
        }
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 127, 29, 29, 255 })); 
    if (GuiButton({ 1020, 210, 130, 45 }, "XÓA") && !showEqModal) { 
        // Đã sửa lỗi Cảnh báo vàng: Thêm (int) trước eqList.size()
        if (selectedEqIndex != -1 && selectedEqIndex < (int)eqList.size()) {
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

    // --- BẢNG DỮ LIỆU ĐÃ LỌC ---
    DrawRectangleRounded({ 280, 280, 870, 490 }, 0.05f, 10, bgCard);
    VeChu("MÃ TB", 310, 300, 18, textGray);
    VeChu("TÊN THIẾT BỊ", 430, 300, 18, textGray);
    VeChu("PHÂN LOẠI", 670, 300, 18, textGray);
    VeChu("SL", 840, 300, 18, textGray);
    VeChu("TRẠNG THÁI", 940, 300, 18, textGray);
    DrawLine(280, 330, 1150, 330, {81, 99, 122, 255});

    // Thuật toán Lọc danh sách (Chỉ lấy các TB khớp với currentFilter)
    std::vector<int> filteredIndices;
    for (size_t i = 0; i < eqList.size(); i++) {
        bool match = false;
        if (currentFilter == "Tất cả") match = true;
        else if (eqList[i].type == currentFilter) match = true;
        else if (currentFilter == "Phụ kiện & Khác" && (eqList[i].type == "Phụ kiện Gym" || eqList[i].type == "Khác")) match = true;
        
        if (match) filteredIndices.push_back((int)i);
    }

    // --- SCROLLING CHO DANH SÁCH ĐÃ LỌC ---
    Rectangle listArea = { 280, 331, 870, 439 }; 
    float itemHeight = 55.0f;
    float totalListHeight = filteredIndices.size() * itemHeight;
    
    if (!showEqModal && CheckCollisionPointRec(GetMousePosition(), listArea)) {
        scrollOffset += GetMouseWheelMove() * 30.0f; 
    }
    
    float maxScroll = listArea.height - totalListHeight;
    if (maxScroll > 0) maxScroll = 0; 
    if (scrollOffset < maxScroll) scrollOffset = maxScroll;
    if (scrollOffset > 0) scrollOffset = 0;

    BeginScissorMode(listArea.x, listArea.y, listArea.width, listArea.height);
    float startY = 350 + scrollOffset;
    
    for (int idx : filteredIndices) {
        if (startY + 45 >= listArea.y && startY - 10 <= listArea.y + listArea.height) {
            Rectangle rowRect = { 280, startY - 10, 870, 45 };
            
            if (!showEqModal && CheckCollisionPointRec(GetMousePosition(), rowRect)) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selectedEqIndex = idx; 
            }

            if (selectedEqIndex == idx) {
                DrawRectangleRec(rowRect, bgCardHover);
                DrawRectangle(280, startY - 10, 4, 45, neonGreen); 
            }

            VeChu(eqList[idx].id.c_str(), 310, startY, 20, textWhite);
            VeChu(eqList[idx].name.c_str(), 430, startY, 20, textWhite);
            VeChu(eqList[idx].type.c_str(), 670, startY, 18, neonBlue);
            VeChu(eqList[idx].quantity.c_str(), 845, startY, 20, textWhite);

            Color stColor = textWhite;
            if (eqList[idx].status == "Đang hoạt động") stColor = neonGreen;
            else if (eqList[idx].status == "Cần bảo trì") stColor = neonOrange;
            else stColor = { 239, 68, 68, 255 }; 
            
            VeChu(eqList[idx].status.c_str(), 940, startY, 18, stColor);

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
            if (lastDeletedEqIndex >= 0 && lastDeletedEqIndex <= (int)eqList.size()) {
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

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
        if (GuiButton({ 480, 560, 130, 45 }, "LƯU LẠI") && !editTypeDropdown && !editStatusDropdown) {
            if (strlen(inputEqName) == 0 || strlen(inputEqQty) == 0) {
                errorMessage = "Vui long nhap Ten va So luong!";
                errorTimer = 180; 
            } 
            else {
                const char* typeArr[] = {"Máy Cardio", "Máy tập Cơ", "Tạ tự do", "Phụ kiện & Khác"};
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

        VeChu("Tình trạng:", 450, 470, 18, textWhite);
        GuiSetStyle(DROPDOWNBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
        if (GuiDropdownBox({ 550, 460, 250, 40 }, statusOptions, &activeStatusIndex, editStatusDropdown)) {
            editStatusDropdown = !editStatusDropdown; 
            editTypeDropdown = false; 
        }

        VeChu("Phân loại:", 450, 410, 18, textWhite);
        if (GuiDropdownBox({ 550, 400, 250, 40 }, typeOptions, &activeTypeIndex, editTypeDropdown)) {
            editTypeDropdown = !editTypeDropdown; 
            editStatusDropdown = false; 
        }
    }
}