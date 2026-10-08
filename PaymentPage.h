#pragma once
#include <vector>
#include <string>
#include "Member.h" // Nhúng thư viện Hội viên để liên kết dữ liệu

// Cấu trúc dữ liệu của một Giao dịch/Hóa đơn
struct PaymentRecord {
    std::string id;
    std::string memberName;
    std::string amount;
    std::string date;
    std::string status; 
};

// Khai báo các hàm xử lý
void LoadPayments(std::vector<PaymentRecord>& payList);
void SavePayments(const std::vector<PaymentRecord>& payList);

// Bổ sung const std::vector<Member>& memberList vào tham số
void DrawPaymentPage(std::vector<PaymentRecord>& payList, const std::vector<Member>& memberList);