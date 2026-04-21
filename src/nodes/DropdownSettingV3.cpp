
#include <Geode/loader/SettingV3.hpp>
#include <Geode/loader/Mod.hpp>
#include "Dropdown.hpp"

using namespace geode::prelude;

class DropdownSettingV3 : public SettingBaseValueV3<std::string> {
public:
    std::vector<std::string> options;

    static Result<std::shared_ptr<SettingV3>> parse(
        std::string const& key,
        std::string const& modID,
        matjson::Value const& json
    ) {
        auto res = std::make_shared<DropdownSettingV3>();
        auto root = checkJson(json, "DropdownSettingV3");
        res->parseBaseProperties(key, modID, root);
        if (json.contains("one-of")) {
            auto arr = json["one-of"];
            if (arr.isArray()) {
                for (auto const& v : arr.asArray().unwrap()) {
                    if (v.isString()) res->options.push_back(v.asString().unwrap());
                }
            }
        }
        if (json.contains("default")) {
            auto def = json["default"];
            if (def.isString()) res->setValue(def.asString().unwrap());
        } else if (!res->options.empty()) {
            res->setValue(res->options[0]);
        }
        root.checkUnknownKeys();
        return root.ok(std::static_pointer_cast<SettingV3>(res));
    }

    matjson::Value toJson() const {
        return matjson::Value(this->getValue());
    }
    Result<> fromJson(matjson::Value const& json) {
        auto str = json.asString();
        if (!str.ok()) return Err(str.unwrapErr());
        this->setValue(str.unwrap());
        return Ok();
    }

    SettingNodeV3* createNode(float width) override;
};


class DropdownSettingNodeV3 : public SettingValueNodeV3<DropdownSettingV3> {
public:
    Dropdown* m_dropdown = nullptr;
    bool init(std::shared_ptr<DropdownSettingV3> setting, float width) {
        if (!SettingValueNodeV3::init(setting, width)) return false;
        auto options = setting->options;
        log::info("Creating Dropdown with {} options", options.size());
        m_dropdown = Dropdown::create(width / 2, options, [this](std::string const& value, size_t) {
            log::info("Dropdown callback: value='{}'", value);
            this->setValue(value, m_dropdown);
        });
        m_dropdown->setSelected(this->getSetting()->getValue());
        m_dropdown->setScale(.7f);
        
        m_dropdown->setAnchorPoint({1, 0.5f});
        float rowHeight = this->getButtonMenu()->getContentSize().height;
        float y = rowHeight / 2.f;
        m_dropdown->setPosition(width / 2, y);
        log::info("Adding Dropdown to ButtonMenu at y={} (rowHeight={})", y, rowHeight);
        this->getButtonMenu()->addChild(m_dropdown);
        return true;
    }
    void updateState(CCNode* invoker) override {
        SettingValueNodeV3::updateState(invoker);
        if (m_dropdown) {
            m_dropdown->setSelected(this->getSetting()->getValue());
        }
    }
};

SettingNodeV3* DropdownSettingV3::createNode(float width) {
    auto node = new DropdownSettingNodeV3();
    if (node->init(std::static_pointer_cast<DropdownSettingV3>(shared_from_this()), width)) {
        node->autorelease();
        return node;
    }
    delete node;
    return nullptr;
}

$on_mod(Loaded) {
    (void)Mod::get()->registerCustomSettingType("dropdown", &DropdownSettingV3::parse);
}
