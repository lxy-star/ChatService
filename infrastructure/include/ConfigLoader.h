// === ConfigLoader.h ===
#pragma once
#include <string>
#include <nlohmann/json.hpp>

class ConfigLoader {
public:
    static bool load(const std::string& filename);
    static std::string getString(const std::string& key);
    static int getInt(const std::string& key);

private:
    static nlohmann::json config_;
};
