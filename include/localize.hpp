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
    {"Serbian (Latin)", "sr-Latn"},
};

inline std::unordered_map<std::string, std::unordered_set<std::string>>& getMissingTranslationCache() {
    static std::unordered_map<std::string, std::unordered_set<std::string>> cache;
    return cache;
}

inline bool findKeyByValueRecursive(const matjson::Value& json, const std::string& target, std::string& foundPath, std::string currentPath = "") {
    if (json.isObject()) {
        for (const auto& entry : json) {
            auto keyOpt = entry.getKey();
            if (!keyOpt) continue;
            const std::string& key = *keyOpt;
            std::string nextPath = currentPath.empty() ? key : currentPath + "." + key;
            if (findKeyByValueRecursive(entry, target, foundPath, nextPath)) {
                return true;
            }
        }
    } else if (json.isArray()) {
        size_t idx = 0;
        for (const auto& entry : json) {
            std::string nextPath = currentPath + "[" + std::to_string(idx) + "]";
            if (findKeyByValueRecursive(entry, target, foundPath, nextPath)) {
                return true;
            }
            ++idx;
        }
    } else if (json.isString()) {
        auto strResult = json.asString();
        if (strResult.ok() && strResult.unwrap() == target) {
            foundPath = currentPath;
            return true;
        }
    }
    return false;
}

inline const matjson::Value* getValueByPath(const matjson::Value& json, const std::string& path) {
    const matjson::Value* current = &json;
    size_t pos = 0;
    while (pos < path.size()) {
        if (path[pos] == '[') {
            size_t end = path.find(']', pos);
            if (end == std::string::npos) return nullptr;
            int idx = 0;
            {
                std::string idxStr = path.substr(pos + 1, end - pos - 1);
                idx = 0;
                bool negative = false;
                size_t i = 0;
                if (!idxStr.empty() && idxStr[0] == '-') { negative = true; i = 1; }
                for (; i < idxStr.size(); ++i) {
                    if (idxStr[i] < '0' || idxStr[i] > '9') return nullptr;
                    idx = idx * 10 + (idxStr[i] - '0');
                }
                if (negative) idx = -idx;
            }
            if (!current->isArray()) return nullptr;
            size_t arrSize = 0;
            for (auto it = current->begin(); it != current->end(); ++it) ++arrSize;
            if (idx < 0 || static_cast<size_t>(idx) >= arrSize) return nullptr;
            auto it = current->begin();
            std::advance(it, idx);
            current = &(*it);
            pos = end + 1;
        } else {
            size_t dot = path.find('.', pos);
            size_t bracket = path.find('[', pos);
            size_t next = std::min(dot == std::string::npos ? path.size() : dot, bracket == std::string::npos ? path.size() : bracket);
            std::string key = path.substr(pos, next - pos);
            if (!current->isObject()) return nullptr;
            bool found = false;
            for (const auto& entry : *current) {
                auto keyOpt = entry.getKey();
                if (keyOpt && *keyOpt == key) {
                    current = &entry;
                    found = true;
                    break;
                }
            }
            if (!found) return nullptr;
            pos = next;
            if (pos < path.size() && path[pos] == '.') ++pos;
        }
    }
    return current;
}


inline std::string getLanguageString(const std::string& value) {
    for (const auto& pair : languageMap) {
        if (pair.first == value) return value;
    }

    auto language = Mod::get()->getSettingValue<std::string>("language");
    auto it = languageMap.find(language);
    std::string langCode = "en";
    if (it != languageMap.end()) langCode = it->second;

    // Early return if language is English
    if (langCode == "en") return value;

    auto& cache = getMissingTranslationCache();
    if (cache[langCode].count(value)) return value;
    if (cache["en"].count(value)) return value;

    auto enPath = Mod::get()->getResourcesDir() / "en.json";
    std::ifstream enFile(enPath);
    if (!enFile.is_open()) {
        log::error("Failed to open English translation file: {}", enPath.string());
        return value;
    }
    auto enResult = matjson::parse(enFile);
    if (!enResult) {
        log::error("Failed to parse English translation file: {}", enResult.unwrapErr().message);
        return value;
    }
    auto enJson = enResult.unwrap();
    std::string foundPath;
    if (!findKeyByValueRecursive(enJson, value, foundPath)) {
        // log::error("Value '{}' not found in English translation file (recursive)", value);
        cache["en"].insert(value);
        return value;
    }

    auto langPath = Mod::get()->getResourcesDir() / (langCode + ".json");
    std::ifstream langFile(langPath);
    if (!langFile.is_open()) {
        log::warn("Could not open translation file for '{}': {}", language, langPath.string());
        return value;
    }
    auto langResult = matjson::parse(langFile);
    if (!langResult) {
        log::warn("Failed to parse translation file for '{}': {}", language, langPath.string());
        return value;
    }
    auto langJson = langResult.unwrap();
    const matjson::Value* translated = getValueByPath(langJson, foundPath);
    if (translated && translated->isString()) {
        auto strResult = translated->asString();
        if (strResult.ok()) return strResult.unwrap();
    }
    cache[langCode].insert(value);
    return value;
}


inline bool hasTranslationKey(const std::string& value) {
    for (const auto& pair : languageMap) {
        if (pair.first == value) return false;
    }

    auto language = Mod::get()->getSettingValue<std::string>("language");
    auto it = languageMap.find(language);
    std::string langCode = "en";
    if (it != languageMap.end()) langCode = it->second;

    auto& cache = getMissingTranslationCache();
    if (cache[langCode].count(value)) return false;
    if (langCode != "en" && cache["en"].count(value)) return false;

    auto enPath = Mod::get()->getResourcesDir() / "en.json";
    std::ifstream enFile(enPath);
    if (!enFile.is_open()) {
        log::warn("Could not open English translation file: {}", enPath.string());
        return false;
    }
    auto enResult = matjson::parse(enFile);
    if (!enResult) {
        log::warn("Failed to parse English translation file: {}", enPath.string());
        return false;
    }
    auto enJson = enResult.unwrap();
    std::string foundPath;
    if (!findKeyByValueRecursive(enJson, value, foundPath)) {
        // log::info("Value '{}' not found in English translation file (recursive).", value);
        cache["en"].insert(value);
        return false;
    }

    if (langCode == "en") return true;
    auto langPath = Mod::get()->getResourcesDir() / (langCode + ".json");
    std::ifstream langFile(langPath);
    if (!langFile.is_open()) {
        log::warn("Could not open translation file for '{}': {}", language, langPath.string());
        return false;
    }
    auto langResult = matjson::parse(langFile);
    if (!langResult) {
        log::warn("Failed to parse translation file for '{}': {}", language, langPath.string());
        return false;
    }
    auto langJson = langResult.unwrap();
    const matjson::Value* translated = getValueByPath(langJson, foundPath);
    if (translated && translated->isString()) return true;
    cache[langCode].insert(value);
    return false;
}