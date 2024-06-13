#pragma once
#include <unordered_map>
#include <string>
#include "User.h"

class UserDatabase {
public:
    static UserDatabase& get_instance();

    /*void addUser(const std::string& username, const User& user);
    User getUser(const std::string& username) const;
    bool removeUser(const std::string& username);
    bool userExists(const std::string& username) const;*/
    std::unordered_map<SOCKET, User> get_users() const;

private:
    UserDatabase() = default; // Private constructor
    ~UserDatabase() = default;
    UserDatabase(const UserDatabase&) = delete;
    UserDatabase& operator=(const UserDatabase&) = delete;
    UserDatabase(UserDatabase&&) = delete; // Deleted move constructor
    UserDatabase& operator=(UserDatabase&&) = delete; // Deleted move assignment operator

    std::unordered_map<SOCKET, User> m_users;
};
