#pragma once
#include <vector>
#include <string>

// Cấu trúc dữ liệu của một Thiết bị
struct Equipment {
    std::string id;
    std::string name;
    std::string category; // VD: Cardio, Tạ, Máy khối...
    std::string status;   // VD: Hoạt động, Bảo trì, Hỏng
};

// Khai báo các hàm xử lý
void LoadEquipment(std::vector<Equipment>& eqList);
void SaveEquipment(const std::vector<Equipment>& eqList);
void DrawEquipmentPage(std::vector<Equipment>& eqList);