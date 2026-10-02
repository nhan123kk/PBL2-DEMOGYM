#include "Member.h"
#include <fstream>
#include <sstream>

// Đường dẫn trỏ vào thư mục data bạn vừa tạo
const char* DATA_FILE = "data/members.txt";

void SaveMembers(const std::vector<Member>& list) {
    std::ofstream file(DATA_FILE);
    if (file.is_open()) {
        for (const auto& m : list) {
            file << m.id << "|" << m.name << "|" << m.phone << "|" << m.plan << "|" << m.isActive << "\n";
        }
        file.close();
    }
}

void LoadMembers(std::vector<Member>& list) {
    std::ifstream file(DATA_FILE);
    if (file.is_open()) {
        list.clear();
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string item;
            Member m;
            
            std::getline(ss, m.id, '|');
            std::getline(ss, m.name, '|');
            std::getline(ss, m.phone, '|');
            std::getline(ss, m.plan, '|');
            std::getline(ss, item, '|');
            
            m.isActive = (item == "1");
            if (!m.id.empty()) {
                list.push_back(m);
            }
        }
        file.close();
    } else {
        // Nếu file chưa tồn tại (lần chạy đầu tiên), tạo dữ liệu mẫu
        list = {
            {"HV001", "Dang Thanh Nhan", "0901234567", "VIP 12 Thang", true},
            {"HV002", "Nguyen Van A", "0911112222", "Basic 3 Thang", true}
        };
        SaveMembers(list);
    }
}