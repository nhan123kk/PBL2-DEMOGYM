#pragma once
#include <vector>
#include <string>

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
void DrawPaymentPage(std::vector<PaymentRecord>& payList);
