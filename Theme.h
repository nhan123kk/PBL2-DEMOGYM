#pragma once
#include "raylib.h"

// 1. Khai báo danh sách các trang (Pages)
enum Page { DASHBOARD, MEMBERS, SCHEDULE, EQUIPMENT, PAYMENT, SETTINGS };

// 2. Khai báo bộ mã màu Dark Fitness dùng chung toàn dự án
const Color bgMain = { 15, 23, 42, 255 };
const Color bgSidebar = { 30, 41, 59, 255 };
const Color bgCard = { 30, 41, 59, 255 };
const Color bgCardHover = { 51, 65, 85, 255 };
const Color textWhite = { 248, 250, 252, 255 };
const Color textGray = { 148, 163, 184, 255 };
const Color neonGreen = { 34, 197, 94, 255 };
const Color neonOrange = { 249, 115, 22, 255 };
const Color neonRed = { 239, 68, 68, 255 };
const Color neonBlue = { 56, 189, 248, 255 };

// 3. Khai báo biến Font chữ dùng chung (sẽ được tải ở main.cpp)
extern Font modernFont;