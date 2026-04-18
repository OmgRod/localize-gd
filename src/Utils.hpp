#pragma once

#include <Geode/Geode.hpp>
#include <matjson.hpp>

using namespace geode::prelude;

static const std::unordered_map<std::string, std::string> languageMap = {
    {"English", "en"},
    {"Spanish", "es"},
    {"French", "fr"},
    {"Portuguese", "pt"},
    {"Russian", "ru"},
    {"Italian", "it"},
    {"Polish", "pl"},
    {"Ukrainian", "uk"},
    {"Bulgarian", "bg"},
    {"Serbian", "sr"},
};

inline std::string getLanguageString(const std::string& key) {
    auto language = Mod::get()->getSettingValue<std::string>("language");

    auto it = languageMap.find(language);
    std::string langCode = "en";
    if (it != languageMap.end()) {
        langCode = it->second;
    }

    auto path = Mod::get()->getResourcesDir() / (langCode + ".json");
    std::ifstream file(path);
    if (!file.is_open()) {
        log::error("Failed to open file: {}", path.string());
        return key;
    }
    auto result = matjson::parse(file);
    if (!result) {
        log::error("Failed to parse JSON: {}", result.unwrapErr().message);
        return key;
    }
    auto json = result.unwrap();
    auto valResult = json.get(key);
    if (!valResult) {
        log::error("Key '{}' not found in translation file", key);
        return key;
    }
    auto& val = valResult.unwrap();
    auto strResult = val.asString();
    if (!strResult) {
        log::error("Value for key '{}' is not a string", key);
        return key;
    }
    return strResult.unwrap();
}

inline bool hasTranslationKey(const std::string& key) {
    auto language = Mod::get()->getSettingValue<std::string>("language");

    log::info("[hasTranslationKey] Checking key '{}' for language '{}'...", key, language);

    auto enPath = Mod::get()->getResourcesDir() / "en.json";
    std::ifstream enFile(enPath);
    if (!enFile.is_open()) {
        log::warn("[hasTranslationKey] Could not open English translation file: {}", enPath.string());
        return false;
    }
    auto enResult = matjson::parse(enFile);
    if (!enResult) {
        log::warn("[hasTranslationKey] Failed to parse English translation file: {}", enPath.string());
        return false;
    }
    auto enJson = enResult.unwrap();
    if (!enJson.contains(key)) {
        log::info("[hasTranslationKey] Key '{}' not found in English translation file.", key);
        return false;
    }
    log::info("[hasTranslationKey] Key '{}' found in English translation file.", key);

    if (language == "English") {
        log::info("[hasTranslationKey] Language is English, key '{}' is valid.", key);
        return true;
    }

    auto it = languageMap.find(language);
    std::string langCode = "en";
    if (it != languageMap.end()) {
        langCode = it->second;
    }
    if (langCode == "en") {
        log::info("[hasTranslationKey] Language code is 'en', key '{}' is valid.", key);
        return true;
    }
    auto langPath = Mod::get()->getResourcesDir() / (langCode + ".json");
    std::ifstream langFile(langPath);
    if (!langFile.is_open()) {
        log::warn("[hasTranslationKey] Could not open translation file for '{}': {}", language, langPath.string());
        return false;
    }
    auto langResult = matjson::parse(langFile);
    if (!langResult) {
        log::warn("[hasTranslationKey] Failed to parse translation file for '{}': {}", language, langPath.string());
        return false;
    }
    auto langJson = langResult.unwrap();
    if (!langJson.contains(key)) {
        log::info("[hasTranslationKey] Key '{}' not found in translation file for '{}'.", key, language);
        return false;
    }
    log::info("[hasTranslationKey] Key '{}' found in translation file for '{}'.", key, language);
    return true;
}