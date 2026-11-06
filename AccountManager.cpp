#include "AccountManager.h"

// Khởi tạo mảng động
AccountManager::AccountManager(int initialCapacity) {
    capacity = initialCapacity;
    count = 0;
    accounts = new Account*[capacity];
}

// Hủy mảng và giải phóng bộ nhớ để tránh memory leak
AccountManager::~AccountManager() {
    for (int i = 0; i < count; ++i) {
        delete accounts[i];
    }
    delete[] accounts;
}

// Thuật toán nới rộng mảng động
void AccountManager::resize() {
    capacity *= 2;
    Account** newAccounts = new Account*[capacity];
    
    for (int i = 0; i < count; ++i) {
        newAccounts[i] = accounts[i];
    }
    
    delete[] accounts; // Xóa mảng cũ (chỉ xóa mảng con trỏ, không xóa object Account)
    accounts = newAccounts; // Trỏ sang mảng mới
}

// Thêm tài khoản
void AccountManager::addAccount(string username, string password, Role role) {
    if (count == capacity) {
        resize();
    }
    accounts[count] = new Account(username, password, role);
    count++;
}

// Xử lý đăng nhập
Account* AccountManager::login(string username, string password) {
    for (int i = 0; i < count; ++i) {
        if (accounts[i]->getUsername() == username && accounts[i]->getPassword() == password) {
            return accounts[i]; // Đăng nhập thành công, trả về con trỏ của tài khoản đó
        }
    }
    return nullptr; // Trả về con trỏ null nếu sai thông tin
}