#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <iostream>
#include <string>

using namespace std;

// Định nghĩa 2 vai trò
enum Role {
    ADMIN,
    STAFF
};

class Account {
private:
    string username;
    string password;
    Role role;

public:
    // Constructor
    Account(string u = "", string p = "", Role r = STAFF) 
        : username(u), password(p), role(r) {}

    // Getters
    string getUsername() const { return username; }
    string getPassword() const { return password; }
    Role getRole() const { return role; }

    // Kiểm tra quyền
    bool isAdmin() const { return role == ADMIN; }
    bool isStaff() const { return role == STAFF; }
};

#endif