#pragma once
#include <string>
#include <vector>

// Trả lại tên GymClass để khớp với Dashboard và Main
struct GymClass {
    std::string time;
    std::string className;
    std::string trainer;
    std::string room;
    std::string status;
};

// Khai báo các hàm để main.cpp có thể gọi được
void LoadSchedule(std::vector<GymClass>& schList);
void SaveSchedule(const std::vector<GymClass>& schList);
void DrawSchedulePage(std::vector<GymClass>& schList);