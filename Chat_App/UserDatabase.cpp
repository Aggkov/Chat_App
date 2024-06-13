#include "includes/UserDatabase.h"

UserDatabase& UserDatabase::get_instance() {
    static UserDatabase instance;
    return instance;
}

std::unordered_map<SOCKET, User> UserDatabase::get_users() const {
    return m_users;
}
