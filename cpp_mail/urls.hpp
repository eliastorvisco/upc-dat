#ifndef URLS_HPP
#define URLS_HPP
#include <string>

extern std::string APPROOT;

const std::string LOGIN_URL = "/login";
const std::string LOGOUT_URL = "/logout";
const std::string INBOX_URL = "/inbox";
const std::string SEND_URL = "/send";

inline const std::string MSG_URL(std::string mid) { return "/inbox/" + mid; }

#endif