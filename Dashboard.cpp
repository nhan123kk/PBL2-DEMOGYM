#include "Dashboard.h"
#include "Theme.h"
#include <ctime>
#include <cstdio>
#include <string>
#include <cctype>

static void VeChu(const char* text, float x, float y, float size, Color color) {
    DrawTextEx(modernFont, text, {x, y}, size, 1, color);
}

// BỎ CHỮ const ĐỂ KHỚP VỚI KHAI BÁO TRONG main.cpp VÀ Dashboard.h
void DrawDashboard(GymList<Member>& memberList, 
                   GymList<GymClass>& classList, 
                   GymList<Equipment>& eqList, 
                   GymList<PaymentRecord>& payList) {
    
    // --- 1. THUẬT TOÁN ĐỒNG BỘ DỮ LIỆU ---
    std::string totalMembers = std::to_string(memberList.size());

    int activeEq = 0;
    for(const auto& eq : eqList) {
        // Dùng Getter thay vì truy cập trực tiếp
        if(eq.getStatus() == "Hoạt động" || eq.getStatus() == "Đang hoạt động") activeEq++;
    }
    std::string eqStr = std::to_string(eqList.size());
    std::string eqSubText = "Đang hoạt động: " + std::to_string(activeEq);

    double totalRevenue = 0;
    for (const auto& p : payList) {
        // Giả sử cứ có hóa đơn là tính doanh thu (vì cấu trúc mới không có thuộc tính status cho hóa đơn)
        totalRevenue += p.getAmount(); 
    }
    
    char revStr[32];
    if (totalRevenue >= 1000000) sprintf(revStr, "%.1fM", totalRevenue / 1000000.0);
    else if (totalRevenue > 0) sprintf(revStr, "%.0fK", totalRevenue / 1000.0);
    else sprintf(revStr, "0");

    // --- 2. VẼ 3 THẺ TỔNG QUAN ---
    DrawRectangleRounded({ 280, 120, 280, 130 }, 0.1f, 10, bgCard);
    VeChu("TỔNG HỘI VIÊN", 300, 140, 18, textGray);
    VeChu(totalMembers.c_str(), 300, 165, 48, textWhite); 
    VeChu("Đã đồng bộ thời gian thực", 300, 215, 14, neonGreen);

    DrawRectangleRounded({ 580, 120, 280, 130 }, 0.1f, 10, bgCard);
    VeChu("TỔNG THIẾT BỊ", 600, 140, 18, textGray);
    VeChu(eqStr.c_str(), 600, 165, 48, textWhite);
    VeChu(eqSubText.c_str(), 600, 215, 14, neonOrange);

    DrawRectangleRounded({ 880, 120, 280, 130 }, 0.1f, 10, bgCard);
    VeChu("TỔNG DOANH THU", 900, 140, 18, textGray);
    VeChu(revStr, 900, 165, 48, textWhite);
    VeChu("Đã tổng hợp toàn hệ thống", 900, 215, 14, neonGreen);

    // --- 3. BIỂU ĐỒ ---
    DrawRectangleRounded({ 280, 280, 580, 320 }, 0.05f, 10, bgCard);
    VeChu("TĂNG TRƯỞNG HỘI VIÊN", 300, 300, 20, textWhite);
    DrawLine(320, 550, 820, 550, textGray); 
    DrawLine(320, 350, 320, 550, textGray); 
    DrawLineEx({ 320, 520 }, { 400, 480 }, 4, neonGreen); DrawCircle(400, 480, 5, textWhite);
    DrawLineEx({ 400, 480 }, { 500, 500 }, 4, neonGreen); DrawCircle(500, 500, 5, textWhite);
    DrawLineEx({ 500, 500 }, { 600, 420 }, 4, neonGreen); DrawCircle(600, 420, 5, textWhite);
    DrawLineEx({ 600, 420 }, { 700, 380 }, 4, neonGreen); DrawCircle(700, 380, 5, textWhite);
    DrawLineEx({ 700, 380 }, { 800, 400 }, 4, neonGreen); DrawCircle(800, 400, 5, neonOrange); 

    // --- 4. DANH SÁCH LỚP HỌC ---
    DrawRectangleRounded({ 880, 280, 280, 320 }, 0.05f, 10, bgCard);
    VeChu("LỚP HỌC HÔM NAY", 900, 300, 20, textWhite);
    
    int classY = 345;
    int maxClasses = (classList.size() > 3) ? 3 : classList.size();
    
    for (int i = 0; i < maxClasses; i++) {
        DrawRectangleRounded({ 900, (float)classY, 240, 65 }, 0.2f, 10, bgMain);
        
        Color cColor = neonBlue; // Mặc định màu xanh

        std::string fullTime = classList[i].getTimeSlot();
        std::string shortTime = fullTime.length() >= 5 ? fullTime.substr(0, 5) : fullTime;

        VeChu(shortTime.c_str(), 915, classY + 20, 20, cColor);
        VeChu(classList[i].getClassName().c_str(), 985, classY + 10, 18, textWhite);
        VeChu(classList[i].getTrainerName().c_str(), 985, classY + 35, 14, textGray); // Dùng Trainer thay cho Room
        classY += 80;
    }

    if (maxClasses == 0) VeChu("Chưa có lớp học nào...", 920, 400, 16, textGray);

    // --- 5. ĐỒNG HỒ HỆ THỐNG ---
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char timeStr[64]; char dateStr[64];
    sprintf(timeStr, "%02d:%02d:%02d", ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    const char* days[] = {"Chủ Nhật", "Thứ Hai", "Thứ Ba", "Thứ Tư", "Thứ Năm", "Thứ Sáu", "Thứ Bảy"};
    sprintf(dateStr, "%s, %02d/%02d/%04d", days[ltm->tm_wday], ltm->tm_mday, 1 + ltm->tm_mon, 1900 + ltm->tm_year);

    DrawRectangleRounded({ 280, 620, 580, 95 }, 0.1f, 10, bgCard);
    VeChu("THỜI GIAN HỆ THỐNG", 300, 635, 16, textGray);
    VeChu(timeStr, 300, 655, 45, neonBlue); 
    VeChu(dateStr, 480, 670, 22, textWhite);
}
