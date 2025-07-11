// === ConfigLoader.cpp ===
#include "ConfigLoader.h"
#include <fstream>
#include <iostream>

nlohmann::json ConfigLoader::config_;

bool ConfigLoader::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;
    try {
        file >> config_;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse config: " << e.what() << std::endl;
        return false;
    }
}

std::string ConfigLoader::getString(const std::string& key) {
    return config_.value(key, "");
}

int ConfigLoader::getInt(const std::string& key) {
    return config_.value(key, 0);
}
