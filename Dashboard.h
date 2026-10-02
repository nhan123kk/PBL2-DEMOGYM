#pragma once
#include <vector>
#include "Member.h"
#include "SchedulePage.h"
#include "EquipmentPage.h"
#include "PaymentPage.h"

// Cấu hình hàm DrawDashboard nhận vào 4 danh sách dữ liệu
void DrawDashboard(const std::vector<Member>& memberList, 
                   const std::vector<GymClass>& classList, 
                   const std::vector<Equipment>& eqList, 
                   const std::vector<PaymentRecord>& payList);