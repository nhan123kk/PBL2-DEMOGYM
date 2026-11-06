#ifndef PAYMENTPAGE_H
#define PAYMENTPAGE_H

#include "Member.h"

// Chỉ giữ lại hàm vẽ giao diện để main.cpp gọi
void DrawPaymentPage(GymList<PaymentRecord>& payList, GymList<Member>& memberList);

#endif