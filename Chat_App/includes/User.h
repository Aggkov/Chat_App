#pragma once
#include "network_headers.h"
#define MAX_REQUEST_SIZE 2047

class User {
public:
    enum class Role {
        ADMIN,
        USER,
    };
    struct connection_info {
        struct sockaddr_storage address;
        socklen_t address_length;
    };
    connection_info conn_info;
    // Default constructor
    User() = default;
    User(const std::string& username, const std::string& password, const std::string& email,
        SOCKET socket) : m_username(username), m_password(password), m_email(email), m_is_registered(false), m_is_logged_in(false), m_socket(socket) {};

    /*User(const std::string& username, const std::string& password, SOCKET socket) : m_username(username),
        m_password(password), m_is_registered(false), m_is_logged_in(false), m_socket(socket) {};*/

    // Getters
    std::string get_username() const;
    std::string get_password() const;
    std::string get_email() const;
    struct sockaddr_storage& get_address() { return conn_info.address; }
    socklen_t& get_address_length() { return conn_info.address_length; }
    SOCKET get_socket() const;
    bool get_is_registered() const {
        return m_is_registered;
    }
    bool get_is_logged_in() const {
        return m_is_logged_in;
    }

    // Setters
    void set_username(const std::string& username) {
        m_username = username;
    }

    void set_password(const std::string& password) {
        m_password = password;
    }

    void set_email(const std::string& email) {
        m_email = email;
    }

    void set_address(const struct sockaddr_storage& address) {
        conn_info.address = address;
    }

    void set_address_length(socklen_t address_length) {
        conn_info.address_length = address_length;
    }
   
    void set_is_registered(bool registered) {
        m_is_registered = registered;
    }
    void set_is_logged_in(bool logged) {
        m_is_logged_in = logged;
    }

private:
    std::string m_username;
    std::string m_password;
    std::string m_email;
    /*struct sockaddr_storage m_address;
    socklen_t m_address_length;*/
    SOCKET m_socket;
    bool m_is_registered;
    bool m_is_logged_in;
    Role m_role;
};