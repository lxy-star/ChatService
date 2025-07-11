// JsonHelper.cpp
#include "JsonHelper.h"

std::string JsonHelper::buildLoginResult(bool success, const std::string& reason,std::string type) {
    nlohmann::json result;
    result["type"] = type + "_result";
    result["success"] = success;
    if (!success) result["reason"] = reason;
    return result.dump();
}

std::string JsonHelper::buildRegisterResult(bool success, const std::string& reason,std::string type) {
    nlohmann::json result;
    result["type"] = type + "_result";
    result["success"] = success;
    if (!success) result["reason"] = reason;
    return result.dump();
}

std::string JsonHelper::buildUnkownTypeResult(bool success, const std::string& reason,std::string type){
    nlohmann::json result;
    result["type"] = type + "_result";
    result["success"] = success;
    if (!success) result["reason"] = reason;
    return result.dump();
}

nlohmann::json JsonHelper::parseJson(const std::string& data) {
    return nlohmann::json::parse(data);
}