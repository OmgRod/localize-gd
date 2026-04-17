#pragma once

#include <Geode/Geode.hpp>
#include <matjson.hpp>

using namespace geode::prelude;

template <typename... Keys>
std::string getLanguageString(Keys&&... keys) {
    auto language = Mod::get()->getSettingValue<std::string>("language");
    static const std::unordered_map<std::string, std::string> languageMap = {
        {"Arabic", "ar"},
        {"German", "de-DE"},
        {"English", "en"},
        {"Spanish", "es"},
        {"French", "fr"},
        {"Japanese", "ja"},
        {"Korean", "ko-KR"},
        {"Portuguese", "pt"},
        {"Russian", "ru"},
    };

    auto it = languageMap.find(language);
    std::string langCode = "en";
    if (it != languageMap.end()) {
        langCode = it->second;
    }

    auto path = Mod::get()->getResourcesDir() / (langCode + ".json");
    log::info("Opening language file: {}", path.string());
    std::ifstream file(path);
    if (!file.is_open()) {
        log::error("Failed to open file: {}", path.string());
        return "Unknown";
    }
    auto result = matjson::parse(file);
    if (!result) {
        log::error("Failed to parse JSON: {}", result.unwrapErr().message);
        return "Unknown";
    }
    auto json = result.unwrap();

    auto traverse = [](matjson::Value* value, auto&&... ks) -> matjson::Value* {
        std::vector<std::string> keyList{ks...};
        for (const auto& key : keyList) {
            if (!value) {
                log::error("Null value before key: {}", key);
                return nullptr;
            }
            auto res = value->get(key);
            if (!res) {
                log::error("Key '{}' not found", key);
                return nullptr;
            }
            value = &res.unwrap();
            log::info("Traversed key: {}", key);
        }
        return value;
    };

    matjson::Value* found = traverse(&json, std::forward<Keys>(keys)...);
    if (!found) {
        log::error("Could not find value for given keys");
        return "Unknown";
    }
    auto strResult = found->asString();
    if (!strResult) {
        log::error("Value is not a string");
        return "Unknown";
    }
    log::info("Found string: {}", strResult.unwrap());
    return strResult.unwrap();
}