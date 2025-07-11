// === JsonHelper.h ===
#pragma once
#include <string>
#include <nlohmann/json.hpp>

class JsonHelper {
public:
    static std::string buildLoginResult(bool success, const std::string& reason = "");
    static std::string buildRegisterResult(bool success, const std::string& reason = "");
    static nlohmann::json parseJson(const std::string& data);
};