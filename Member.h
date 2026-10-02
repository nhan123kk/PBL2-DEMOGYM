#pragma once
#include <string>
#include <vector>

// Cấu trúc dữ liệu của một Hội viên
struct Member {
    std::string id;
    std::string name;
    std::string phone;
    std::string plan;
    bool isActive;
};

// Khai báo các hàm xử lý file để các file khác có thể gọi được
void SaveMembers(const std::vector<Member>& list);
void LoadMembers(std::vector<Member>& list);