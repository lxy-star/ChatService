// === JsonHelper.h ===
#pragma once
#include <string>
#include <nlohmann/json.hpp>

class JsonHelper {
public:
    static std::string buildLoginResult(bool success, const std::string& reason = "",std::string type = "login");
    static std::string buildRegisterResult(bool success, const std::string& reason = "",std::string type = "register");
    static std::string buildUnkownTypeResult(bool success, const std::string& reason = "",std::string type = "Unknown");
    static nlohmann::json parseJson(const std::string& data);
};