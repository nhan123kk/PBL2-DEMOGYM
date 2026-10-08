#pragma once
#include <vector>
#include <string>

struct Equipment {
    std::string id;
    std::string name;
    std::string type;
    std::string quantity;
    std::string status;
};

void LoadEquipment(std::vector<Equipment>& eqList);
void SaveEquipment(const std::vector<Equipment>& eqList);
void DrawEquipmentPage(std::vector<Equipment>& eqList);