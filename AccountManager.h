#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include "Account.h"

class AccountManager {
private:
    Account** accounts; // Mảng động chứa các con trỏ Account
    int capacity;       // Sức chứa hiện tại của mảng
    int count;          // Số lượng tài khoản thực tế đang có

    void resize();      // Hàm tự động tăng kích thước mảng khi đầy

public:
    AccountManager(int initialCapacity = 10);
    ~AccountManager();

    // Thêm tài khoản mới
    void addAccount(string username, string password, Role role);
    
    // Kiểm tra đăng nhập
    Account* login(string username, string password);
};

#endif