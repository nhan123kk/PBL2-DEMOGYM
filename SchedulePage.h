#pragma once
#pragma once
#include <vector>
#include <string>

// Cấu trúc dữ liệu của 1 buổi tập
struct GymClass {
    std::string time;
    std::string name;
    std::string trainer;
    std::string room;
    int colorType; // 0: Xanh lá, 1: Cam, 2: Đỏ, 3: Xanh dương
};

// Khai báo các hàm xử lý
void LoadSchedule(std::vector<GymClass>& classList);
void SaveSchedule(const std::vector<GymClass>& classList);
void DrawSchedulePage(std::vector<GymClass>& classList);
