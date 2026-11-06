#ifndef MEMBER_H
#define MEMBER_H

#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;

template <class T>
class GymList {
private:
    T* data; int capacity; int current_size;
    void resize() {
        capacity *= 2; T* temp = new T[capacity];
        for (int i = 0; i < current_size; i++) temp[i] = data[i];
        delete[] data; data = temp;
    }
public:
    GymList() { capacity = 10; current_size = 0; data = new T[capacity]; }
    GymList(const GymList& other) {
        capacity = other.capacity; current_size = other.current_size; data = new T[capacity];
        for (int i = 0; i < current_size; i++) data[i] = other.data[i];
    }
    GymList& operator=(const GymList& other) {
        if (this != &other) {
            delete[] data; capacity = other.capacity; current_size = other.current_size; data = new T[capacity];
            for (int i = 0; i < current_size; i++) data[i] = other.data[i];
        } return *this;
    }
    ~GymList() { delete[] data; }

    void push_back(T element) { if (current_size == capacity) resize(); data[current_size] = element; current_size++; }
    void insert(int index, T element) {
        if (index < 0 || index > current_size) return;
        if (current_size == capacity) resize();
        for (int i = current_size; i > index; i--) data[i] = data[i - 1];
        data[index] = element; current_size++;
    }
    void erase(int index) {
        if (index < 0 || index >= current_size) return;
        for (int i = index; i < current_size - 1; i++) data[i] = data[i + 1];
        current_size--;
    }
    void erase(T* it) { erase(it - data); } 
    int size() const { return current_size; }
    bool empty() const { return current_size == 0; }
    void clear() { current_size = 0; }

    T& operator[](int index) { if (index < 0 || index >= current_size) throw out_of_range("Index error"); return data[index]; }
    const T& operator[](int index) const { if (index < 0 || index >= current_size) throw out_of_range("Index error"); return data[index]; }
    T* begin() { return data; }
    T* end() { return data + current_size; }
    const T* begin() const { return data; }
    const T* end() const { return data + current_size; }
};

class Member {
private:
    string id, name, phone, plan; bool isActive;
public:
    Member(string i="", string n="", string p="", string pl="", bool act=true) 
        : id(i), name(n), phone(p), plan(pl), isActive(act) {}
    string getId() const { return id; }
    string getName() const { return name; }
    string getPhone() const { return phone; }
    string getType() const { return plan; } // Hỗ trợ tương thích ngược với code cũ
    bool getIsActive() const { return isActive; }
};

class Equipment {
private:
    string id, name, type, quantity, status;
public:
    Equipment(string i="", string n="", string t="", string q="", string s="") 
        : id(i), name(n), type(t), quantity(q), status(s) {}
    string getId() const { return id; }
    string getName() const { return name; }
    string getType() const { return type; }
    string getQuantity() const { return quantity; }
    string getStatus() const { return status; }
};

class GymClass {
private:
    string time, className, trainer, room, status;
public:
    GymClass(string t="", string n="", string tr="", string r="", string s="") 
        : time(t), className(n), trainer(tr), room(r), status(s) {}
    string getTimeSlot() const { return time; }
    string getClassName() const { return className; }
    string getTrainerName() const { return trainer; }
    string getRoom() const { return room; }
    string getStatus() const { return status; }
};

class PaymentRecord {
private:
    string paymentId, memberRef; double amount; string date, status;
public:
    PaymentRecord(string pid="", string mRef="", double amt=0.0, string d="", string s="") 
        : paymentId(pid), memberRef(mRef), amount(amt), date(d), status(s) {}
    string getPaymentId() const { return paymentId; }
    string getMemberId() const { return memberRef; } // Lưu Name trực tiếp
    double getAmount() const { return amount; } 
    string getDate() const { return date; }
    string getStatus() const { return status; }
};

#endif