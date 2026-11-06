#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include "raylib.h"
#include "AccountManager.h"
#include <string>

class LoginPage {
private:
    char usernameInput[64];  // Mảng char lưu tên đăng nhập nhập vào
    char passwordInput[64];  // Mảng char lưu mật khẩu
    bool usernameEditMode;   // Trạng thái đang click vào ô username để gõ
    bool passwordEditMode;   // Trạng thái đang click vào ô password để gõ
    bool loginFailed;        // Biến cờ báo lỗi nếu đăng nhập sai

public:
    LoginPage();
    
    // Hàm vẽ giao diện. Sẽ trả về con trỏ Account nếu đăng nhập đúng, ngược lại trả về nullptr
    Account* Draw(AccountManager& accountManager); 
};

#endif