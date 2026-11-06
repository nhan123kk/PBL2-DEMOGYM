#include "SchedulePage.h"
#include "Theme.h"
#include "raygui.h"
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>
#include <string>
#include <cctype>

const char* SCHEDULE_FILE = "data/schedule.txt";

float ParseTime(std::string t, bool isEnd) {
    size_t dash = t.find('-');
    std::string part = t;
    if (dash != std::string::npos) {
        if (isEnd) part = t.substr(dash + 1);
        else part = t.substr(0, dash);
    }
    
    int h = 0, m = 0;
    std::string num = "";
    bool isMin = false;
    for (char c : part) {
        if (isdigit(c)) num += c;
        else if (c == ':') {
            if (!num.empty()) h = std::stoi(num);
            num = "";
            isMin = true;
        }
    }
    if (!num.empty() && isMin) m = std::stoi(num);
    else if (!num.empty() && !isMin) h = std::stoi(num);

    if (h < 5) h = 5; 
    if (h > 23) h = 23;
    return h + (m / 60.0f);
}

std::string MapStatus(const std::string& rawStatus) {
    if (rawStatus == "0") return "Sắp diễn ra";
    if (rawStatus == "1") return "Đang diễn ra";
    if (rawStatus == "2") return "Đã kết thúc";
    if (rawStatus == "3") return "Đã hủy";
    return rawStatus; 
}

void SaveSchedule(const std::vector<GymClass>& schList) {
    std::ofstream file(SCHEDULE_FILE);
    if (file.is_open()) {
        for (const auto& s : schList) {
            file << s.time << "|" << s.className << "|" << s.trainer << "|" << s.room << "|" << s.status << "\n";
        }
        file.close();
    }
}

void LoadSchedule(std::vector<GymClass>& schList) {
    std::ifstream file(SCHEDULE_FILE);
    if (file.is_open()) {
        schList.clear();
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            GymClass s;
            std::getline(ss, s.time, '|');
            std::getline(ss, s.className, '|');
            std::getline(ss, s.trainer, '|');
            std::getline(ss, s.room, '|');
            std::getline(ss, s.status, '|');
            
            if (!s.status.empty() && s.status.back() == '\r') s.status.pop_back();
            s.status = MapStatus(s.status); 

            if (!s.time.empty()) schList.push_back(s);
        }
        file.close();
    } else {
        // DỮ LIỆU MẪU ĐÃ ĐƯỢC THAY ĐỔI THEO THỰC TẾ
        schList = {
            {"06:00 - 07:30", "Yoga Co Ban", "HLV Nguyen Mai", "Phòng Yoga", "Đã kết thúc"},
            {"08:00 - 09:30", "HIIT Dot Mo", "HLV Tuan Anh", "Khu Cardio", "Đang diễn ra"},
            {"17:30 - 19:00", "Zumba Dance", "HLV Minh Anh", "Phòng Group X", "Sắp diễn ra"},
            {"19:30 - 21:00", "Kickboxing", "HLV Tien Dung", "Khu Kickboxing", "Sắp diễn ra"}
        };
        SaveSchedule(schList);
    }
}

static bool showSchModal = false;
static bool isEditMode = false;
static int selectedSchIndex = -1;

static char inputSchTime[64] = "";
static char inputSchClass[64] = "";
static char inputSchTrainer[64] = "";
static bool editSchTime = false, editSchClass = false, editSchTrainer = false;

// ĐỒNG BỘ DROPDOWN TÊN PHÒNG
static const char* roomOptions = "Phòng Yoga;Phòng Group X;Khu Kickboxing;Khu Cardio;Khu Tạ Tự Do";
static int activeRoomIndex = 0;
static bool editRoomDropdown = false;

static const char* statusOptions = "Sắp diễn ra;Đang diễn ra;Đã kết thúc;Đã hủy";
static int activeStatusIndex = 0;
static bool editStatusDropdown = false;

static float scrollOffset = 0.0f;
static std::string errorMessage = "";
static int errorTimer = 0;

static void VeChu(const char* text, float x, float y, float size, Color color) {
    DrawTextEx(modernFont, text, {x, y}, size, 1, color);
}

void DrawSchedulePage(std::vector<GymClass>& schList) {
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
    if (GuiButton({ 740, 80, 130, 40 }, "THÊM LỊCH") && !showSchModal) { 
        showSchModal = true;
        isEditMode = false;
        errorMessage = "";
        editRoomDropdown = editStatusDropdown = false;
        memset(inputSchTime, 0, sizeof(inputSchTime));
        memset(inputSchClass, 0, sizeof(inputSchClass));
        memset(inputSchTrainer, 0, sizeof(inputSchTrainer));
        activeRoomIndex = activeStatusIndex = 0;
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 124, 45, 18, 255 })); 
    if (GuiButton({ 880, 80, 130, 40 }, "CHỈNH SỬA") && !showSchModal) { 
        if (selectedSchIndex != -1 && selectedSchIndex < (int)schList.size()) { 
            showSchModal = true;
            isEditMode = true;
            errorMessage = "";
            editRoomDropdown = editStatusDropdown = false;
            
            strncpy(inputSchTime, schList[selectedSchIndex].time.c_str(), 63);
            strncpy(inputSchClass, schList[selectedSchIndex].className.c_str(), 63);
            strncpy(inputSchTrainer, schList[selectedSchIndex].trainer.c_str(), 63);
            
            // ĐỒNG BỘ ARRAY TÊN PHÒNG KHI SỬA
            const char* roomArr[] = {"Phòng Yoga", "Phòng Group X", "Khu Kickboxing", "Khu Cardio", "Khu Tạ Tự Do"};
            activeRoomIndex = 0;
            for (int i=0; i<5; i++) if (schList[selectedSchIndex].room == roomArr[i]) activeRoomIndex = i;

            const char* stArr[] = {"Sắp diễn ra", "Đang diễn ra", "Đã kết thúc", "Đã hủy"};
            activeStatusIndex = 0;
            for (int i=0; i<4; i++) if (schList[selectedSchIndex].status == stArr[i]) activeStatusIndex = i;
        }
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 127, 29, 29, 255 })); 
    if (GuiButton({ 1020, 80, 130, 40 }, "XÓA LỚP") && !showSchModal) { 
        if (selectedSchIndex != -1 && selectedSchIndex < (int)schList.size()) {
            schList.erase(schList.begin() + selectedSchIndex);
            selectedSchIndex = -1; 
            SaveSchedule(schList);
        }
    }
    
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar));

    // ========================================================================
    // --- KHU VỰC VẼ GOOGLE CALENDAR GRID ---
    // ========================================================================
    
    DrawRectangleRounded({ 280, 135, 870, 635 }, 0.05f, 10, bgCard);
    
    // ĐỒNG BỘ TIÊU ĐỀ CỘT LƯỚI
    const char* cols[] = {"Phòng Yoga", "Phòng Group X", "Khu Kickboxing", "Khu Cardio", "Khu Tạ Tự Do"};
    float colWidth = (870.0f - 70.0f) / 5.0f; 
    float startX = 350.0f;

    for (int i = 0; i < 5; i++) {
        VeChu(cols[i], startX + (i * colWidth) + 20, 150, 18, neonBlue);
        if (i > 0) DrawLine(startX + (i * colWidth), 135, startX + (i * colWidth), 770, {51, 65, 85, 255}); 
    }
    DrawLine(280, 180, 1150, 180, {81, 99, 122, 255}); 

    Rectangle gridArea = { 280, 181, 870, 589 };
    float hourHeight = 65.0f; 
    int startHour = 6;  
    int endHour = 21;   
    float totalGridHeight = (endHour - startHour + 1) * hourHeight;
    
    if (!showSchModal && CheckCollisionPointRec(GetMousePosition(), gridArea)) {
        scrollOffset += GetMouseWheelMove() * 40.0f; 
    }
    float maxScroll = gridArea.height - totalGridHeight;
    if (maxScroll > 0) maxScroll = 0; 
    if (scrollOffset < maxScroll) scrollOffset = maxScroll;
    if (scrollOffset > 0) scrollOffset = 0;

    BeginScissorMode(gridArea.x, gridArea.y, gridArea.width, gridArea.height);
    
    for (int h = startHour; h <= endHour; h++) {
        float yPos = 195 + (h - startHour) * hourHeight + scrollOffset;
        if (yPos > 135 && yPos < 800) {
            VeChu(TextFormat("%02d:00", h), 290, yPos - 10, 16, textGray);
            DrawLine(350, yPos, 1150, yPos, {51, 65, 85, 150}); 
        }
    }

    for (size_t i = 0; i < schList.size(); i++) {
        int colIdx = 4; 
        for (int c = 0; c < 5; c++) {
            if (schList[i].room == cols[c]) { colIdx = c; break; }
        }

        float sTime = ParseTime(schList[i].time, false);
        float eTime = ParseTime(schList[i].time, true);
        
        float evY = 195 + (sTime - startHour) * hourHeight + scrollOffset;
        float evHeight = (eTime - sTime) * hourHeight;
        
        if (evY + evHeight < gridArea.y || evY > gridArea.y + gridArea.height) continue;

        Rectangle evRect = { startX + (colIdx * colWidth) + 3, evY, colWidth - 6, evHeight - 2 };

        if (!showSchModal && CheckCollisionPointRec(GetMousePosition(), evRect)) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selectedSchIndex = (int)i;
        }

        Color baseColor = { 59, 130, 246, 255 }; 
        if (schList[i].status == "Đang diễn ra") baseColor = { 34, 197, 94, 255 }; 
        else if (schList[i].status == "Đã kết thúc") baseColor = { 107, 114, 128, 255 }; 
        else if (schList[i].status == "Đã hủy") baseColor = { 239, 68, 68, 255 }; 

        Color bgColor = ColorAlpha(baseColor, 0.2f); 
        if (selectedSchIndex == (int)i) bgColor = ColorAlpha(baseColor, 0.6f); 

        DrawRectangleRounded(evRect, 0.15f, 5, bgColor);
        DrawRectangle(evRect.x, evRect.y, 4, evRect.height, baseColor); 
        
        if (selectedSchIndex == (int)i) {
            DrawRectangleRoundedLines(evRect, 0.15f, 5, textWhite); 
        }

        if (evHeight >= 40) {
            VeChu(schList[i].className.c_str(), evRect.x + 8, evRect.y + 5, 15, textWhite);
            VeChu(schList[i].time.c_str(), evRect.x + 8, evRect.y + 24, 13, textGray);
            if (evHeight >= 60) VeChu(schList[i].trainer.c_str(), evRect.x + 8, evRect.y + 40, 13, baseColor);
        } else {
            VeChu(schList[i].className.c_str(), evRect.x + 8, evRect.y + 5, 14, textWhite); 
        }
    }
    EndScissorMode();

    // ========================================================================
    // --- MODAL THÊM / SỬA ---
    // ========================================================================
    if (showSchModal) {
        DrawRectangle(0, 0, 1200, 800, { 0, 0, 0, 150 }); 
        DrawRectangleRounded({ 400, 100, 450, 620 }, 0.05f, 10, bgMain);
        DrawRectangleRoundedLines({ 400, 100, 450, 620 }, 0.05f, 10, bgSidebar);

        VeChu(isEditMode ? "CHỈNH SỬA LỊCH HỌC" : "THÊM LỊCH HỌC MỚI", 500, 130, 24, neonGreen);

        VeChu("Khung giờ (VD: 06:00 - 07:30):", 450, 180, 16, textWhite);
        if (GuiTextBox({ 450, 210, 350, 40 }, inputSchTime, 64, editSchTime)) editSchTime = !editSchTime;

        // ĐỒNG BỘ TEXT HƯỚNG DẪN NHẬP
        VeChu("Tên Lớp (VD: Yoga, Zumba, Kickboxing):", 450, 270, 16, textWhite);
        if (GuiTextBox({ 450, 300, 350, 40 }, inputSchClass, 64, editSchClass)) editSchClass = !editSchClass;

        VeChu("Huấn Luyện Viên:", 450, 360, 16, textWhite);
        if (GuiTextBox({ 450, 390, 350, 40 }, inputSchTrainer, 64, editSchTrainer)) editSchTrainer = !editSchTrainer;

        if (errorTimer > 0) {
            VeChu(errorMessage.c_str(), 450, 615, 16, neonRed);
            errorTimer--;
        }

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
        if (GuiButton({ 480, 640, 130, 45 }, "LƯU LẠI") && !editRoomDropdown && !editStatusDropdown) {
            if (strlen(inputSchTime) == 0 || strlen(inputSchClass) == 0) {
                errorMessage = "Vui long nhap du thong tin!";
                errorTimer = 180; 
            } else {
                // ĐỒNG BỘ ARRAY TÊN PHÒNG KHI LƯU
                const char* roomArr[] = {"Phòng Yoga", "Phòng Group X", "Khu Kickboxing", "Khu Cardio", "Khu Tạ Tự Do"};
                const char* stArr[] = {"Sắp diễn ra", "Đang diễn ra", "Đã kết thúc", "Đã hủy"};
                if (isEditMode) {
                    schList[selectedSchIndex].time = inputSchTime;
                    schList[selectedSchIndex].className = inputSchClass;
                    schList[selectedSchIndex].trainer = inputSchTrainer;
                    schList[selectedSchIndex].room = roomArr[activeRoomIndex];
                    schList[selectedSchIndex].status = stArr[activeStatusIndex];
                } else {
                    schList.push_back({inputSchTime, inputSchClass, inputSchTrainer, roomArr[activeRoomIndex], stArr[activeStatusIndex]});
                }
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