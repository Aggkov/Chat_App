#include "includes/User.h"
#include "includes/network_headers.h"


std::string User::get_username() const {
    return m_username;
}

std::string User::get_password() const {
    return m_password;
}

std::string User::get_email() const {
    return m_email;
}

SOCKET User::get_socket() const {
    return m_socket;
}