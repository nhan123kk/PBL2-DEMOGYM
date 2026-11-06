#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "Member.h"

void DrawDashboard(GymList<Member>& memberList, GymList<GymClass>& classList, GymList<Equipment>& eqList, GymList<PaymentRecord>& payList);

#endif