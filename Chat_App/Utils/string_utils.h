#pragma once
#include <string>
#include <algorithm> 
#include <cctype>
#include <sstream>
#include <regex>

namespace utils {

    // trim from start (in place)
    inline void ltrim(std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
            }));
    }

    // trim from end (in place)
    inline void rtrim(std::string& s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
            }).base(), s.end());
    }

    inline void trim(std::string& s) {
        ltrim(s);
        rtrim(s);
    }
    

    /*bool parseMessage(const std::string& message, const std::string& command, const std::string& username, const std::string& password, const std::string& email) {
        std::istringstream iss(message);
        if (!(iss >> command >> username >> password >> email)) {
            return false;
        }
        return isValidEmail(email);
    }*/

    inline bool isValidEmail(const std::string& email) {
        // Simple regex for validating an email
        const std::regex pattern(R"((\w+)(\.{1}\w+)*@(\w+)(\.\w+)+)");
        return std::regex_match(email, pattern);
    }
}
