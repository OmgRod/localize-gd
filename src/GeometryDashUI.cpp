#include <Geode/Geode.hpp>
#include <Geode/modify/LoadingLayer.hpp>
#include <localize.hpp>

using namespace geode::prelude;

class $modify(MyLoadingLayer, LoadingLayer) {
    const char* getLoadingString() {
        int num = geode::utils::random::generate(1, 101);
        thread_local static std::string str;
        const char* defaultString = LoadingLayer::getLoadingString();
        str = getLanguageString(fmt::format("gd.loading.message.{}", fmt::to_string(num)));
        if (str == fmt::format("gd.loading.message.{}", fmt::to_string(num))) {
            return defaultString;
        }
        return str.c_str();
    }
};
