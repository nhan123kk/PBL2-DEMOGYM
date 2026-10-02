#include "SchedulePage.h"
#include "Theme.h"
#include "raygui.h"
#include <fstream>
#include <sstream>
#include <cstring>

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
        // Dữ liệu mẫu ban đầu
        classList = {
            {"06:00 - 07:30", "Yoga Căn Bản", "Nguyễn Mai", "Studio 1", 0},
            {"15:00 - 16:30", "Boxing", "Lê Quyền", "Sân Võ Thuật", 2}
        };
        SaveSchedule(classList);
    }
}

// --- BIẾN TRẠNG THÁI GIAO DIỆN ---
static bool showClassModal = false;
static int selectedClassIndex = -1;
static char inputTime[64] = "";
static char inputClassName[64] = "";
static char inputTrainer[64] = "";
static char inputRoom[64] = "";
static bool editTime = false, editClassName = false, editTrainer = false, editRoom = false;

static void VeChu(const char* text, float x, float y, float size, Color color) {
    DrawTextEx(modernFont, text, {x, y}, size, 1, color);
}

void DrawSchedulePage(std::vector<GymClass>& classList) {
    VeChu("LỊCH TẬP & LỚP HỌC", 280, 110, 32, textWhite);
    
    // --- THANH CÔNG CỤ TÌM KIẾM ---
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 51, 65, 85, 255 }));
    if (GuiButton({ 280, 160, 120, 40 }, "HÔM NAY")) {}
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar));
    if (GuiButton({ 410, 160, 120, 40 }, "TUẦN NÀY")) {}
    if (GuiButton({ 540, 160, 120, 40 }, "THÁNG NÀY")) {}

    // Nút Thêm Lớp
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
    if (GuiButton({ 1020, 160, 130, 40 }, "+ THÊM LỚP") && !showClassModal) {
        showClassModal = true;
        memset(inputTime, 0, sizeof(inputTime));
        memset(inputClassName, 0, sizeof(inputClassName));
        memset(inputTrainer, 0, sizeof(inputTrainer));
        memset(inputRoom, 0, sizeof(inputRoom));
        selectedClassIndex = -1; // Bỏ chọn dòng
    }

    // Nút Xóa Lớp (Chỉ hoạt động khi có dòng được chọn)
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 127, 29, 29, 255 })); 
    if (GuiButton({ 880, 160, 130, 40 }, "XÓA LỚP") && !showClassModal) {
        if (selectedClassIndex != -1) {
            classList.erase(classList.begin() + selectedClassIndex);
            selectedClassIndex = -1;
            SaveSchedule(classList); // Lưu lại ngay vào ổ cứng
        }
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar));
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    // --- VẼ DANH SÁCH LỚP HỌC (CARD VIEW) ---
    int startY = 230;
    for (size_t i = 0; i < classList.size(); i++) {
        // Tự động phân màu dựa trên thứ tự
        Color cardColor = (classList[i].colorType == 0) ? neonGreen :
                          (classList[i].colorType == 1) ? neonOrange :
                          (classList[i].colorType == 2) ? neonRed : neonBlue;

        Rectangle rowRect = { 280, (float)startY, 870, 80 };
        
        // Logic Click chọn dòng
        if (!showClassModal && CheckCollisionPointRec(GetMousePosition(), rowRect)) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                selectedClassIndex = (int)i;
            }
        }

        // Vẽ thẻ (Tô màu khác đi nếu đang được click chọn)
        DrawRectangleRounded(rowRect, 0.1f, 10, (selectedClassIndex == (int)i) ? bgCardHover : bgCard);
        DrawRectangleRounded({ 280, (float)startY, 10, 80 }, 0.1f, 10, cardColor); 
        
        VeChu(classList[i].time.c_str(), 310, startY + 25, 22, cardColor);
        VeChu(classList[i].name.c_str(), 500, startY + 15, 24, textWhite);
        
        std::string subText = "HLV: " + classList[i].trainer + " | Phong: " + classList[i].room;
        VeChu(subText.c_str(), 500, startY + 45, 16, textGray);
        
        startY += 100; // Khoảng cách giữa các thẻ
    }

    // --- CỬA SỔ POPUP THÊM LỚP ---
    if (showClassModal) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0, 0, 0, 150 });
        DrawRectangleRounded({ 400, 150, 450, 500 }, 0.05f, 10, bgMain);
        DrawRectangleRoundedLines({ 400, 150, 450, 500 }, 0.05f, 10, bgSidebar);

        VeChu("THÊM LỚP HỌC MỚI", 500, 180, 24, neonGreen);

        GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

        VeChu("Giờ (VD: 06:00-07:00):", 450, 240, 16, textWhite);
        if (GuiTextBox({ 450, 260, 350, 40 }, inputTime, 64, editTime)) editTime = !editTime;

        VeChu("Tên Lớp (VD: Yoga):", 450, 310, 16, textWhite);
        if (GuiTextBox({ 450, 330, 350, 40 }, inputClassName, 64, editClassName)) editClassName = !editClassName;

        VeChu("Huấn luyện viên:", 450, 380, 16, textWhite);
        if (GuiTextBox({ 450, 400, 165, 40 }, inputTrainer, 64, editTrainer)) editTrainer = !editTrainer;

        VeChu("Phòng tập:", 635, 380, 16, textWhite);
        if (GuiTextBox({ 635, 400, 165, 40 }, inputRoom, 64, editRoom)) editRoom = !editRoom;

        // Nút Lưu (Tự sinh màu ngẫu nhiên cho lớp mới)
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({ 20, 83, 45, 255 })); 
        if (GuiButton({ 480, 520, 130, 45 }, "LƯU LẠI")) {
            int randomColor = classList.size() % 4; // Sinh số 0->3
            classList.push_back({inputTime, inputClassName, inputTrainer, inputRoom, randomColor});
            SaveSchedule(classList);
            showClassModal = false; 
        }

        // Nút Hủy
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(bgSidebar)); 
        if (GuiButton({ 640, 520, 130, 45 }, "HỦY")) {
            showClassModal = false;
        }
    }
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT); // Reset style
}