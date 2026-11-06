#include "SchedulePage.h"
#include "Theme.h"
#include "raygui.h"
#include <cstring>
#include <string>
#include <cctype>
#include <fstream>

void SaveSchedule(GymList<GymClass>& schList) {
    std::ofstream file("data/schedule.txt");
    for (int i = 0; i < schList.size(); i++) {
        file << schList[i].getTimeSlot() << "|" << schList[i].getClassName() << "|" 
             << schList[i].getTrainerName() << "|" << schList[i].getRoom() << "|" << schList[i].getStatus() << "\n";
    }
}

float ParseTime(std::string t, bool isEnd) {
    size_t dash = t.find('-'); std::string part = t;
    if (dash != std::string::npos) { if (isEnd) part = t.substr(dash + 1); else part = t.substr(0, dash); }
    int h = 0, m = 0; std::string num = ""; bool isMin = false;
    for (char c : part) {
        if (isdigit(c)) num += c;
        else if (c == ':') { if (!num.empty()) h = std::stoi(num); num = ""; isMin = true; }
    }
    if (!num.empty() && isMin) m = std::stoi(num); else if (!num.empty() && !isMin) h = std::stoi(num);
    if (h < 5) h = 5; if (h > 23) h = 23;
    return h + (m / 60.0f);
}

static bool showSchModal = false; static bool isEditMode = false; static int selectedSchIndex = -1;
static char inputSchTime[64] = ""; static char inputSchClass[64] = ""; static char inputSchTrainer[64] = "";
static bool editSchTime = false, editSchClass = false, editSchTrainer = false;
static const char* roomOptions = "Phòng Yoga;Phòng Group X;Khu Kickboxing;Khu Cardio;Khu Tạ Tự Do";
static int activeRoomIndex = 0; static bool editRoomDropdown = false;
static const char* statusOptions = "Sắp diễn ra;Đang diễn ra;Đã kết thúc;Đã hủy";
static int activeStatusIndex = 0; static bool editStatusDropdown = false;
static float scrollOffset = 0.0f; static std::string errorMessage = ""; static int errorTimer = 0;
static void VeChu(const char* text, float x, float y, float size, Color color) { DrawTextEx(modernFont, text, {x, y}, size, 1, color); }

void DrawSchedulePage(GymList<GymClass>& schList) {
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER); GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
    if (GuiButton({ 740, 80, 130, 40 }, "THÊM LỊCH") && !showSchModal) { 
        showSchModal = true; isEditMode = false; errorMessage = "";
        memset(inputSchTime, 0, sizeof(inputSchTime)); memset(inputSchClass, 0, sizeof(inputSchClass)); memset(inputSchTrainer, 0, sizeof(inputSchTrainer));
        activeRoomIndex = activeStatusIndex = 0;
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 124, 45, 18, 255 })); 
    if (GuiButton({ 880, 80, 130, 40 }, "CHỈNH SỬA") && !showSchModal) { 
        if (selectedSchIndex != -1 && selectedSchIndex < schList.size()) { 
            showSchModal = true; isEditMode = true; errorMessage = "";
            strncpy(inputSchTime, schList[selectedSchIndex].getTimeSlot().c_str(), 63);
            strncpy(inputSchClass, schList[selectedSchIndex].getClassName().c_str(), 63);
            strncpy(inputSchTrainer, schList[selectedSchIndex].getTrainerName().c_str(), 63);
            
            const char* roomArr[] = {"Phòng Yoga", "Phòng Group X", "Khu Kickboxing", "Khu Cardio", "Khu Tạ Tự Do"};
            activeRoomIndex = 0; for (int i=0; i<5; i++) if (schList[selectedSchIndex].getRoom() == roomArr[i]) activeRoomIndex = i;
            const char* stArr[] = {"Sắp diễn ra", "Đang diễn ra", "Đã kết thúc", "Đã hủy"};
            activeStatusIndex = 0; for (int i=0; i<4; i++) if (schList[selectedSchIndex].getStatus() == stArr[i]) activeStatusIndex = i;
        }
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 127, 29, 29, 255 })); 
    if (GuiButton({ 1020, 80, 130, 40 }, "XÓA LỚP") && !showSchModal) { 
        if (selectedSchIndex != -1 && selectedSchIndex < schList.size()) {
            schList.erase(selectedSchIndex); selectedSchIndex = -1; SaveSchedule(schList);
        }
    }
    
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar));
    DrawRectangleRounded({ 280, 135, 870, 635 }, 0.05f, 10, bgCard);
    
    const char* cols[] = {"Phòng Yoga", "Phòng Group X", "Khu Kickboxing", "Khu Cardio", "Khu Tạ Tự Do"};
    float colWidth = (870.0f - 70.0f) / 5.0f; float startX = 350.0f;
    for (int i = 0; i < 5; i++) {
        VeChu(cols[i], startX + (i * colWidth) + 20, 150, 18, neonBlue);
        if (i > 0) DrawLine(startX + (i * colWidth), 135, startX + (i * colWidth), 770, {51, 65, 85, 255}); 
    }
    DrawLine(280, 180, 1150, 180, {81, 99, 122, 255}); 

    Rectangle gridArea = { 280, 181, 870, 589 }; float hourHeight = 65.0f; int startHour = 6; int endHour = 21;   
    float totalGridHeight = (endHour - startHour + 1) * hourHeight;
    if (!showSchModal && CheckCollisionPointRec(GetMousePosition(), gridArea)) scrollOffset += GetMouseWheelMove() * 40.0f; 
    float maxScroll = gridArea.height - totalGridHeight;
    if (maxScroll > 0) maxScroll = 0; if (scrollOffset < maxScroll) scrollOffset = maxScroll; if (scrollOffset > 0) scrollOffset = 0;

    BeginScissorMode(gridArea.x, gridArea.y, gridArea.width, gridArea.height);
    for (int h = startHour; h <= endHour; h++) {
        float yPos = 195 + (h - startHour) * hourHeight + scrollOffset;
        if (yPos > 135 && yPos < 800) { VeChu(TextFormat("%02d:00", h), 290, yPos - 10, 16, textGray); DrawLine(350, yPos, 1150, yPos, {51, 65, 85, 150}); }
    }

    for (int i = 0; i < schList.size(); i++) {
        int colIdx = 4; // Mặc định cột cuối
        for (int c = 0; c < 5; c++) { if (schList[i].getRoom() == cols[c]) { colIdx = c; break; } }
        float sTime = ParseTime(schList[i].getTimeSlot(), false); float eTime = ParseTime(schList[i].getTimeSlot(), true);
        float evY = 195 + (sTime - startHour) * hourHeight + scrollOffset; float evHeight = (eTime - sTime) * hourHeight;
        
        if (evY + evHeight < gridArea.y || evY > gridArea.y + gridArea.height) continue;
        Rectangle evRect = { startX + (colIdx * colWidth) + 3, evY, colWidth - 6, evHeight - 2 };

        if (!showSchModal && CheckCollisionPointRec(GetMousePosition(), evRect)) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selectedSchIndex = i;
        }

        Color baseColor = { 59, 130, 246, 255 }; 
        if (schList[i].getStatus() == "Đang diễn ra") baseColor = { 34, 197, 94, 255 }; 
        else if (schList[i].getStatus() == "Đã kết thúc") baseColor = { 107, 114, 128, 255 }; 
        else if (schList[i].getStatus() == "Đã hủy") baseColor = { 239, 68, 68, 255 }; 

        Color bgColor = ColorAlpha(baseColor, 0.2f); if (selectedSchIndex == i) bgColor = ColorAlpha(baseColor, 0.6f); 
        DrawRectangleRounded(evRect, 0.15f, 5, bgColor); DrawRectangle(evRect.x, evRect.y, 4, evRect.height, baseColor); 
        if (selectedSchIndex == i) DrawRectangleRoundedLines(evRect, 0.15f, 5, textWhite); 

        if (evHeight >= 40) {
            VeChu(schList[i].getClassName().c_str(), evRect.x + 8, evRect.y + 5, 15, textWhite);
            VeChu(schList[i].getTimeSlot().c_str(), evRect.x + 8, evRect.y + 24, 13, textGray);
            if (evHeight >= 60) VeChu(schList[i].getTrainerName().c_str(), evRect.x + 8, evRect.y + 40, 13, baseColor);
        } else VeChu(schList[i].getClassName().c_str(), evRect.x + 8, evRect.y + 5, 14, textWhite); 
    }
    EndScissorMode();

    if (showSchModal) {
        DrawRectangle(0, 0, 1200, 800, { 0, 0, 0, 150 }); 
        DrawRectangleRounded({ 400, 100, 450, 620 }, 0.05f, 10, bgMain); DrawRectangleRoundedLines({ 400, 100, 450, 620 }, 0.05f, 10, bgSidebar);
        VeChu(isEditMode ? "CHỈNH SỬA LỊCH HỌC" : "THÊM LỊCH HỌC MỚI", 500, 130, 24, neonGreen);

        VeChu("Khung giờ (VD: 06:00 - 07:30):", 450, 180, 16, textWhite); if (GuiTextBox({ 450, 210, 350, 40 }, inputSchTime, 64, editSchTime)) editSchTime = !editSchTime;
        VeChu("Tên Lớp (VD: Yoga, Zumba):", 450, 270, 16, textWhite); if (GuiTextBox({ 450, 300, 350, 40 }, inputSchClass, 64, editSchClass)) editSchClass = !editSchClass;
        VeChu("Huấn Luyện Viên:", 450, 360, 16, textWhite); if (GuiTextBox({ 450, 390, 350, 40 }, inputSchTrainer, 64, editSchTrainer)) editSchTrainer = !editSchTrainer;

        if (errorTimer > 0) { VeChu(errorMessage.c_str(), 450, 615, 16, neonRed); errorTimer--; }

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
        if (GuiButton({ 480, 640, 130, 45 }, "LƯU LẠI") && !editRoomDropdown && !editStatusDropdown) {
            if (strlen(inputSchTime) == 0 || strlen(inputSchClass) == 0) { errorMessage = "Vui long nhap du thong tin!"; errorTimer = 180; } 
            else {
                const char* roomArr[] = {"Phòng Yoga", "Phòng Group X", "Khu Kickboxing", "Khu Cardio", "Khu Tạ Tự Do"};
                const char* stArr[] = {"Sắp diễn ra", "Đang diễn ra", "Đã kết thúc", "Đã hủy"};
                if (isEditMode) {
                    schList.erase(selectedSchIndex);
                    schList.insert(selectedSchIndex, GymClass(inputSchTime, inputSchClass, inputSchTrainer, roomArr[activeRoomIndex], stArr[activeStatusIndex]));
                } else schList.push_back(GymClass(inputSchTime, inputSchClass, inputSchTrainer, roomArr[activeRoomIndex], stArr[activeStatusIndex]));
                SaveSchedule(schList); showSchModal = false; 
            }
        }

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); 
        if (GuiButton({ 640, 640, 130, 45 }, "HỦY") && !editRoomDropdown && !editStatusDropdown) showSchModal = false;
        
        VeChu("Trạng thái:", 450, 540, 16, textWhite);
        GuiSetStyle(DROPDOWNBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
        if (GuiDropdownBox({ 550, 530, 250, 40 }, statusOptions, &activeStatusIndex, editStatusDropdown)) { editStatusDropdown = !editStatusDropdown; editRoomDropdown = false; }
        VeChu("Phòng Tập:", 450, 470, 16, textWhite);
        if (GuiDropdownBox({ 550, 460, 250, 40 }, roomOptions, &activeRoomIndex, editRoomDropdown)) { editRoomDropdown = !editRoomDropdown; editStatusDropdown = false; }
    }
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
}