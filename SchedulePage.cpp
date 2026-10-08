#include "SchedulePage.h"
#include "Theme.h"
#include "raygui.h"
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>

const char* SCHEDULE_FILE = "data/schedule.txt";

// --- HÀM LƯU & ĐỌC FILE ---
void SaveSchedule(const std::vector<GymClass>& classList) {
    std::ofstream file(SCHEDULE_FILE);
    if (file.is_open()) {
        for (const auto& c : classList) {
            file << c.time << "|" << c.name << "|" << c.trainer << "|" << c.room << "|" << c.colorType << "\n";
        }
        file.close();
    }
}

void LoadSchedule(std::vector<GymClass>& classList) {
    std::ifstream file(SCHEDULE_FILE);
    if (file.is_open()) {
        classList.clear();
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string item;
            GymClass c;
            std::getline(ss, c.time, '|');
            std::getline(ss, c.name, '|');
            std::getline(ss, c.trainer, '|');
            std::getline(ss, c.room, '|');
            std::getline(ss, item, '|');
            c.colorType = std::stoi(item.empty() ? "0" : item);
            if (!c.time.empty()) classList.push_back(c);
        }
        file.close();
    } else {
        classList = {
            {"06:00 - 07:30", "Yoga Căn Bản", "Nguyễn Mai", "Studio 1", 0},
            {"15:00 - 16:30", "Boxing", "Lê Quyền", "Sân Võ Thuật", 2}
        };
        SaveSchedule(classList);
    }
}

// --- BIẾN TRẠNG THÁI GIAO DIỆN ---
static bool showClassModal = false;
static bool isEditMode = false;
static int selectedClassIndex = -1;

static char inputClassName[64] = "";
static char inputTrainer[64] = "";
static bool editClassName = false, editTrainer = false;

// Biến cho Dropdown
static const char* timeOptions = "06:00 - 07:30;08:00 - 09:30;15:00 - 16:30;17:30 - 19:00;19:30 - 21:00";
static int activeTimeIndex = 0;
static bool editTimeDropdown = false;

static const char* roomOptions = "Studio 1;Studio 2;Sân Võ Thuật;Khu Tạ Nặng;Hồ Bơi";
static int activeRoomIndex = 0;
static bool editRoomDropdown = false;

static const char* colorOptions = "Xanh lá (Yoga/Zen);Cam (Cardio/Đạp xe);Đỏ (Võ thuật/Sức mạnh);Xanh dương (Bơi/Hồi phục)";
static int activeColorIndex = 0;
static bool editColorDropdown = false;

// Biến cho Cuộn chuột & Báo lỗi
static float scrollOffset = 0.0f;
static std::string errorMessage = "";
static int errorTimer = 0;

// Biến cho Undo
static GymClass lastDeletedClass;
static int lastDeletedClassIndex = -1;
static bool canUndoClass = false;
static int undoClassTimer = 0;

static void VeChu(const char* text, float x, float y, float size, Color color) {
    DrawTextEx(modernFont, text, {x, y}, size, 1, color);
}

void DrawSchedulePage(std::vector<GymClass>& classList) {
    VeChu("LỊCH TẬP & LỚP HỌC", 280, 110, 32, textWhite);
    
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    // --- THANH LỌC THỜI GIAN (DUMMY UI) ---
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 51, 65, 85, 255 }));
    if (GuiButton({ 280, 160, 120, 40 }, "HÔM NAY")) {}
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar));
    if (GuiButton({ 410, 160, 120, 40 }, "TUẦN NÀY")) {}
    if (GuiButton({ 540, 160, 120, 40 }, "THÁNG NÀY")) {}

    // --- NÚT THÊM LỚP ---
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
    if (GuiButton({ 1020, 160, 130, 40 }, "+ THÊM LỚP") && !showClassModal) {
        showClassModal = true;
        isEditMode = false;
        errorMessage = "";
        editTimeDropdown = false;
        editRoomDropdown = false;
        editColorDropdown = false;

        memset(inputClassName, 0, sizeof(inputClassName));
        memset(inputTrainer, 0, sizeof(inputTrainer));
        activeTimeIndex = 0;
        activeRoomIndex = 0;
        activeColorIndex = 0;
    }

    // --- NÚT CHỈNH SỬA ---
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 124, 45, 18, 255 })); 
    if (GuiButton({ 880, 160, 130, 40 }, "CHỈNH SỬA") && !showClassModal) {
        if (selectedClassIndex != -1 && selectedClassIndex < classList.size()) {
            showClassModal = true;
            isEditMode = true;
            errorMessage = "";
            editTimeDropdown = editRoomDropdown = editColorDropdown = false;

            strncpy(inputClassName, classList[selectedClassIndex].name.c_str(), 63);
            strncpy(inputTrainer, classList[selectedClassIndex].trainer.c_str(), 63);
            
            // Map dữ liệu cũ vào Dropdown
            const char* timeArr[] = {"06:00 - 07:30", "08:00 - 09:30", "15:00 - 16:30", "17:30 - 19:00", "19:30 - 21:00"};
            const char* roomArr[] = {"Studio 1", "Studio 2", "Sân Võ Thuật", "Khu Tạ Nặng", "Hồ Bơi"};
            
            activeTimeIndex = 0;
            for (int i=0; i<5; i++) if (classList[selectedClassIndex].time == timeArr[i]) activeTimeIndex = i;
            
            activeRoomIndex = 0;
            for (int i=0; i<5; i++) if (classList[selectedClassIndex].room == roomArr[i]) activeRoomIndex = i;
            
            activeColorIndex = classList[selectedClassIndex].colorType;
            if (activeColorIndex > 3 || activeColorIndex < 0) activeColorIndex = 0;
        }
    }

    // --- NÚT XÓA LỚP ---
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 127, 29, 29, 255 })); 
    if (GuiButton({ 740, 160, 130, 40 }, "XÓA LỚP") && !showClassModal) {
        if (selectedClassIndex != -1 && selectedClassIndex < classList.size()) {
            lastDeletedClass = classList[selectedClassIndex];
            lastDeletedClassIndex = selectedClassIndex;
            canUndoClass = true;
            undoClassTimer = 300; 

            classList.erase(classList.begin() + selectedClassIndex);
            selectedClassIndex = -1;
            SaveSchedule(classList); 
        }
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar));
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    // --- VẼ DANH SÁCH LỚP HỌC (CARD VIEW VỚI SCISSOR MODE) ---
    Rectangle listArea = { 280, 230, 870, 530 }; 
    float itemHeight = 100.0f; // Mỗi thẻ cao 80 + 20 khoảng cách
    float totalListHeight = classList.size() * itemHeight;
    
    if (!showClassModal && CheckCollisionPointRec(GetMousePosition(), listArea)) {
        scrollOffset += GetMouseWheelMove() * 40.0f; 
    }
    
    float maxScroll = listArea.height - totalListHeight;
    if (maxScroll > 0) maxScroll = 0; 
    if (scrollOffset < maxScroll) scrollOffset = maxScroll;
    if (scrollOffset > 0) scrollOffset = 0;

    BeginScissorMode(listArea.x, listArea.y, listArea.width, listArea.height);

    float startY = 230 + scrollOffset;
    for (size_t i = 0; i < classList.size(); i++) {
        if (startY + 80 >= listArea.y && startY <= listArea.y + listArea.height) {
            
            Color cardColor = (classList[i].colorType == 0) ? neonGreen :
                              (classList[i].colorType == 1) ? neonOrange :
                              (classList[i].colorType == 2) ? neonRed : neonBlue;

            Rectangle rowRect = { 280, startY, 870, 80 };
            
            if (!showClassModal && CheckCollisionPointRec(GetMousePosition(), rowRect)) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selectedClassIndex = (int)i;
            }

            DrawRectangleRounded(rowRect, 0.1f, 10, (selectedClassIndex == (int)i) ? bgCardHover : bgCard);
            DrawRectangleRounded({ 280, startY, 10, 80 }, 0.1f, 10, cardColor); 
            
            VeChu(classList[i].time.c_str(), 310, startY + 25, 22, cardColor);
            VeChu(classList[i].name.c_str(), 500, startY + 15, 24, textWhite);
            
            std::string subText = "HLV: " + classList[i].trainer + "   |   Phòng: " + classList[i].room;
            VeChu(subText.c_str(), 500, startY + 45, 16, textGray);
        }
        startY += itemHeight; 
    }
    EndScissorMode();

    // --- VẼ TOAST THÔNG BÁO HOÀN TÁC ---
    if (canUndoClass && undoClassTimer > 0 && !showClassModal) {
        undoClassTimer--;
        DrawRectangleRounded({ 700, 680, 430, 60 }, 0.2f, 10, { 15, 23, 42, 230 });
        DrawRectangleRoundedLines({ 700, 680, 430, 60 }, 0.2f, 10, neonRed);

        std::string msg = "Da xoa: " + lastDeletedClass.name;
        VeChu(msg.c_str(), 720, 700, 18, textWhite);

        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 

        if (GuiButton({ 990, 690, 120, 40 }, "HOAN TAC")) {
            if (lastDeletedClassIndex >= 0 && lastDeletedClassIndex <= classList.size()) {
                classList.insert(classList.begin() + lastDeletedClassIndex, lastDeletedClass);
            } else {
                classList.push_back(lastDeletedClass); 
            }
            SaveSchedule(classList); 
            canUndoClass = false; 
            undoClassTimer = 0;
            selectedClassIndex = lastDeletedClassIndex; 
        }
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); 
        if (undoClassTimer <= 0) canUndoClass = false; 
    }

    // --- CỬA SỔ POPUP THÊM / SỬA LỚP ---
    if (showClassModal) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0, 0, 0, 150 });
        DrawRectangleRounded({ 400, 120, 450, 580 }, 0.05f, 10, bgMain);
        DrawRectangleRoundedLines({ 400, 120, 450, 580 }, 0.05f, 10, bgSidebar);

        VeChu(isEditMode ? "CHỈNH SỬA LỚP HỌC" : "THÊM LỚP HỌC MỚI", 500, 150, 24, neonGreen);

        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

        // Khung nhập Tên lớp và HLV
        VeChu("Tên Lớp (VD: Yoga, Boxing):", 450, 210, 16, textWhite);
        if (GuiTextBox({ 450, 230, 350, 40 }, inputClassName, 64, editClassName)) editClassName = !editClassName;

        VeChu("Huấn luyện viên:", 450, 290, 16, textWhite);
        if (GuiTextBox({ 450, 310, 350, 40 }, inputTrainer, 64, editTrainer)) editTrainer = !editTrainer;

        if (errorTimer > 0) {
            VeChu(errorMessage.c_str(), 450, 565, 16, neonRed);
            errorTimer--;
        }

        // VẼ NÚT LƯU LẠI VÀ HỦY (Vẽ trước để Dropdown đè lên)
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
        if (GuiButton({ 480, 590, 130, 45 }, "LƯU LẠI") && !editTimeDropdown && !editRoomDropdown && !editColorDropdown) {
            if (strlen(inputClassName) == 0 || strlen(inputTrainer) == 0) {
                errorMessage = "Vui long nhap Ten lop va HLV!";
                errorTimer = 180; 
            } else {
                const char* timeArr[] = {"06:00 - 07:30", "08:00 - 09:30", "15:00 - 16:30", "17:30 - 19:00", "19:30 - 21:00"};
                const char* roomArr[] = {"Studio 1", "Studio 2", "Sân Võ Thuật", "Khu Tạ Nặng", "Hồ Bơi"};
                
                std::string selTime = timeArr[activeTimeIndex];
                std::string selRoom = roomArr[activeRoomIndex];

                if (isEditMode) {
                    classList[selectedClassIndex].time = selTime;
                    classList[selectedClassIndex].name = inputClassName;
                    classList[selectedClassIndex].trainer = inputTrainer;
                    classList[selectedClassIndex].room = selRoom;
                    classList[selectedClassIndex].colorType = activeColorIndex;
                } else {
                    classList.push_back({selTime, inputClassName, inputTrainer, selRoom, activeColorIndex});
                }
                SaveSchedule(classList);
                showClassModal = false; 
            }
        }

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); 
        if (GuiButton({ 640, 590, 130, 45 }, "HỦY") && !editTimeDropdown && !editRoomDropdown && !editColorDropdown) {
            showClassModal = false;
        }

        // VẼ DROPDOWN THEO THỨ TỰ TỪ DƯỚI LÊN ĐỂ LỚP ĐÈ CHUẨN XÁC
        
        // 3. Dropdown Màu sắc (Dưới cùng)
        VeChu("Phân loại màu thẻ:", 450, 480, 16, textWhite);
        GuiSetStyle(DROPDOWNBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
        if (GuiDropdownBox({ 450, 500, 350, 40 }, colorOptions, &activeColorIndex, editColorDropdown)) {
            editColorDropdown = !editColorDropdown;
            editTimeDropdown = editRoomDropdown = false;
        }

        // 2. Dropdown Phòng Tập (Giữa)
        VeChu("Phòng tập:", 635, 380, 16, textWhite);
        if (GuiDropdownBox({ 635, 400, 165, 40 }, roomOptions, &activeRoomIndex, editRoomDropdown)) {
            editRoomDropdown = !editRoomDropdown;
            editTimeDropdown = editColorDropdown = false;
        }

        // 1. Dropdown Giờ học (Trên cùng)
        VeChu("Giờ học:", 450, 380, 16, textWhite);
        if (GuiDropdownBox({ 450, 400, 165, 40 }, timeOptions, &activeTimeIndex, editTimeDropdown)) {
            editTimeDropdown = !editTimeDropdown;
            editRoomDropdown = editColorDropdown = false;
        }
    }
}