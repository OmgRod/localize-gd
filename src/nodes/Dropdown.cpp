#include <Geode/utils/ColorProvider.hpp>
#include <Geode/ui/Button.hpp>
#include "Dropdown.hpp"

using namespace geode::prelude;

class DropdownOverlay;

class Dropdown::Impl {
public:
    Dropdown* m_self;
    std::vector<std::string> m_options;
    Function<void(std::string const&, size_t)> m_callback;
    size_t m_selectedIndex = 0;
    float m_width;
    bool m_enabled = true;
    int m_savedZOrder = 0;

    CCLabelBMFont* m_label = nullptr;
    NineSlice* m_bg = nullptr;
    CCSprite* m_arrow = nullptr;
    CCMenuItemSpriteExtra* m_button = nullptr;
    CCMenu* m_menu = nullptr;
    DropdownOverlay* m_overlay = nullptr;

    bool init(
        float width, std::vector<std::string> options,
        Function<void(std::string const&, size_t)> callback
    );

    void updateLabel();
    void openOverlay();
    void closeOverlay();
    void selectOption(size_t index);
    void setEnabled(bool enabled);
};

class DropdownOverlay : public CCLayerColor {
    Dropdown::Impl* m_dropdown;
    ScrollLayer* m_scrollLayer = nullptr;
    CCRect m_panelRect;

    float m_itemHeight;
    float m_itemWidth;
    float m_panelPadding;
    float m_scrollY = 0.f;

    bool m_dragging = false;
    float m_startY = 0.f;
    float m_lastY = 0.f;
    bool m_movedEnough = false;
    Button* m_pressedButton = nullptr;



public:
    static DropdownOverlay* create(Dropdown::Impl* dropdown) {
        auto ret = new DropdownOverlay();

        if (ret->init(dropdown)) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }

    bool init(Dropdown::Impl* dropdown) {
        log::info("DropdownOverlay::init called");
        if (!CCLayerColor::initWithColor({0, 0, 0, 0})) return false;

        m_dropdown = dropdown;
        this->setTouchEnabled(true);
        this->setKeypadEnabled(true);

        Mod* geodeLoader = Loader::get()->getInstalledMod("geode.loader");
        std::string usedTheme = geodeLoader->getSettingValue<std::string>("used-theme");
        bool geodeTheme = (usedTheme == "Geode" || usedTheme == "Sapphire" || usedTheme == "Rainbow");

        auto winSize = CCDirector::get()->getWinSize();
        auto buttonWorldPos = m_dropdown->m_self->convertToWorldSpace(ccp(0, 0));
        auto buttonSize = m_dropdown->m_self->getContentSize();

        log::info("DropdownOverlay: winSize=({}, {}), buttonWorldPos=({}, {}), buttonSize=({}, {})", winSize.width, winSize.height, buttonWorldPos.x, buttonWorldPos.y, buttonSize.width, buttonSize.height);

        float dropdownWidth = m_dropdown->m_width;
        m_itemHeight = 28.f;
        float itemSpacing = 2.f;
        float totalListHeight = m_itemHeight * m_dropdown->m_options.size() +
            itemSpacing * (m_dropdown->m_options.size() - 1);
        m_panelPadding = 6.f;

        float maxPanelHeight = 200.f;
        float naturalPanelHeight = totalListHeight + m_panelPadding * 2;
        float panelHeight = std::min(naturalPanelHeight, maxPanelHeight);

        float screenMidY = winSize.height / 2.f;
        float panelY;
        if (buttonWorldPos.y + buttonSize.height / 2.f > screenMidY) {
            panelY = buttonWorldPos.y - panelHeight - 2.f;
            if (panelY < 5.f) panelY = 5.f;
        } else {
            panelY = buttonWorldPos.y + buttonSize.height + 2.f;
            if (panelY + panelHeight > winSize.height - 5.f) {
                panelY = winSize.height - panelHeight - 5.f;
            }
        }

        float panelX = buttonWorldPos.x + buttonSize.width - dropdownWidth;
        if (panelX + dropdownWidth > winSize.width - 5.f) {
            panelX = winSize.width - dropdownWidth - 5.f;
        }
        if (panelX < 5.f) panelX = 5.f;

        m_panelRect = CCRect(panelX, panelY, dropdownWidth, panelHeight);

        const char* panelBGPaths[] = {
            geodeTheme ? "geode.loader/GE_square02.png" : "GJ_square02.png",
            "GJ_square02.png",
            "geode.loader/GE_square02.png",
            "res/sprites/GE_square02.png",
            "res/sprites/GJ_square02.png"
        };
        NineSlice* panelBG = nullptr;
        for (const char* path : panelBGPaths) {
            panelBG = NineSlice::create(path);
            if (panelBG) {
                log::info("DropdownOverlay: Loaded panelBG from '{}'.", path);
                break;
            } else {
                log::warn("DropdownOverlay: Failed to load panelBG from '{}'.", path);
            }
        }
        if (!panelBG) {
            log::error("DropdownOverlay: Failed to create panelBG from all known paths!");
        } else {
            panelBG->setID("dropdown-panel-bg");
            panelBG->setContentSize({dropdownWidth, panelHeight});
            panelBG->setAnchorPoint({0, 0});
            panelBG->setPosition(panelX, panelY);
            this->addChild(panelBG);
        }

        m_itemWidth = dropdownWidth - m_panelPadding * 2;
        float scrollAreaWidth = m_itemWidth;
        float scrollAreaHeight = panelHeight - m_panelPadding * 2;

        m_scrollLayer = ScrollLayer::create({scrollAreaWidth, scrollAreaHeight}, true, true);
        if (m_scrollLayer) {
            m_scrollLayer->setAnchorPoint({0, 0});
            m_scrollLayer->setPosition(panelX + m_panelPadding, panelY + m_panelPadding);
            this->addChild(m_scrollLayer);
        }

        log::info("DropdownOverlay: Adding {} options", m_dropdown->m_options.size());
        for (size_t i = 0; i < m_dropdown->m_options.size(); i++) {
            float itemY = totalListHeight - (m_itemHeight + itemSpacing) * i - m_itemHeight;

            log::info("DropdownOverlay: Creating itemBG for option [{}]", i);
            NineSlice* itemBG = nullptr;
            const char* itemBGSpriteNames[] = {
                "geode.loader/tab-bg.png",
                "GJ_square02.png"
            };
            for (const char* spriteName : itemBGSpriteNames) {
                if (CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(spriteName)) {
                    itemBG = NineSlice::createWithSpriteFrameName(spriteName);
                    if (itemBG) {
                        log::info("DropdownOverlay: Loaded itemBG from sprite frame '{}'.", spriteName);
                        break;
                    } else {
                        log::warn("DropdownOverlay: Failed to create itemBG from sprite frame '{}'.", spriteName);
                    }
                }
            }
            if (!itemBG) {
                const char* itemBGPaths[] = {
                    "GJ_square02.png",
                    "res/sprites/GJ_square02.png"
                };
                for (const char* path : itemBGPaths) {
                    itemBG = NineSlice::create(path);
                    if (itemBG) {
                        log::info("DropdownOverlay: Loaded itemBG from '{}'.", path);
                        break;
                    } else {
                        log::warn("DropdownOverlay: Failed to load itemBG from '{}'.", path);
                    }
                }
            }
            if (!itemBG) {
                log::error("DropdownOverlay: Failed to create item background from all known paths!");
                continue;
            }
            itemBG->setScale(.5f);
            itemBG->setContentSize({m_itemWidth / .5f, m_itemHeight / .5f});

            if (i == m_dropdown->m_selectedIndex) {
                itemBG->setColor(to3B(ColorProvider::get()->color("geode.loader/mod-list-tab-selected-bg")));
            }
            else {
                itemBG->setColor(to3B(ColorProvider::get()->color("geode.loader/mod-list-tab-deselected-bg")));
            }

            log::info("DropdownOverlay: Creating label for option [{}] with bigFont.fnt", i);
            auto label = CCLabelBMFont::create(m_dropdown->m_options[i].c_str(), "bigFont.fnt");
            if (!label) {
                log::error("DropdownOverlay: Failed to create label with font 'bigFont.fnt'");
                label = CCLabelBMFont::create(m_dropdown->m_options[i].c_str(), "chatFont.fnt");
                if (label) log::info("DropdownOverlay: Fallback to chatFont.fnt succeeded");
            }
            if (!label) {
                log::error("DropdownOverlay: Failed to create label with fallback font 'chatFont.fnt'");
                continue;
            }
            label->setScale(0.35f / .5f);
            label->setAnchorPoint({0.f, 0.5f});
            label->limitLabelWidth((m_itemWidth - 14.f) / .5f, 0.35f / .5f, 0.1f / .5f);

            if (i == m_dropdown->m_selectedIndex) {
                label->setColor(ccWHITE);
            }
            else {
                label->setColor(ccc3(200, 200, 200));
            }

            itemBG->addChildAtPosition(label, Anchor::Left, ccp(8.f / .5f, 0));

            if (i == m_dropdown->m_selectedIndex) {
                auto check = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                if (check) {
                    check->setScale(0.4f / .5f);
                    itemBG->addChildAtPosition(check, Anchor::Right, ccp(-10.f / .5f, 0));
                } else {
                    log::warn("DropdownOverlay: Could not load checkmark sprite 'GJ_completesIcon_001.png'");
                }
            }

            size_t index = i;
            log::info("DropdownOverlay: Creating Button for option [{}]", i);
            auto btn = Button::createWithNode(itemBG, [this, index](Button*) {
                if (!m_movedEnough) {
                    log::info("DropdownOverlay: Option [{}] selected", index);
                    m_dropdown->selectOption(index);
                } else {
                    log::info("DropdownOverlay: Drag detected, not selecting option [{}]", index);
                }
            });
            btn->setContentSize({m_itemWidth, m_itemHeight});
            btn->setAnchorPoint({0, 0});
            btn->setPosition(0, itemY);
            btn->setTouchPriority(-501);
            btn->setAnimationType(Button::AnimationType::None);
            if (m_scrollLayer) m_scrollLayer->m_contentLayer->addChild(btn);
        }

        if (m_scrollLayer) {
            m_scrollLayer->m_contentLayer->setContentSize({scrollAreaWidth, totalListHeight});
            m_scrollLayer->scrollToTop();
        }

        log::info("DropdownOverlay: init finished");
        return true;
    }
};

Dropdown::Dropdown() : m_impl(std::make_unique<Impl>()) {
    m_impl->m_self = this;
}

Dropdown::~Dropdown() {
    if (m_impl->m_overlay) {
        m_impl->closeOverlay();
    }
}

void Dropdown::onOpen(CCObject*) {
    m_impl->openOverlay();
}

bool Dropdown::init(
    float width, std::vector<std::string> options, Function<void(std::string const&, size_t)> callback
) {
    if (!CCNode::init()) return false;
    return m_impl->init(width, std::move(options), std::move(callback));
}

Dropdown* Dropdown::create(
    float width, std::vector<std::string> options, Function<void(std::string const&, size_t)> callback
) {
    auto ret = new Dropdown();
    if (ret->init(width, std::move(options), std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void Dropdown::setSelected(size_t index) {
    if (index < m_impl->m_options.size()) {
        m_impl->m_selectedIndex = index;
        m_impl->updateLabel();
        if (m_impl->m_callback) {
            m_impl->m_callback(m_impl->m_options[m_impl->m_selectedIndex], m_impl->m_selectedIndex);
        }
    }
}

void Dropdown::setSelected(std::string_view value) {
    for (size_t i = 0; i < m_impl->m_options.size(); i++) {
        if (m_impl->m_options[i] == value) {
            m_impl->m_selectedIndex = i;
        }
    }
}

size_t Dropdown::getSelectedIndex() const {
    return m_impl->m_selectedIndex;
}

std::string Dropdown::getSelectedValue() const {
    if (m_impl->m_options.empty()) return "";
    return m_impl->m_options[m_impl->m_selectedIndex];
}

void Dropdown::setEnabled(bool enabled) {
    m_impl->setEnabled(enabled);
}

bool Dropdown::isEnabled() const {
    return m_impl->m_enabled;
}

void Dropdown::setItems(std::vector<std::string> options) {
    m_impl->m_options = std::move(options);
    m_impl->m_selectedIndex = 0;
    m_impl->updateLabel();
}

void Dropdown::Impl::updateLabel() {
    log::info("Dropdown::Impl::updateLabel called");
    if (m_options.empty()) {
        log::warn("Dropdown::Impl::updateLabel: options empty");
        if (m_label) m_label->setString("");
        return;
    }
    if (m_label) {
        log::info("Dropdown::Impl::updateLabel: setting label to '{}' (index {})", m_options[m_selectedIndex], m_selectedIndex);
        m_label->setString(m_options[m_selectedIndex].c_str());
        m_label->limitLabelWidth(m_width - 30.f, 0.35f, 0.1f);
    } else {
        log::error("Dropdown::Impl::updateLabel: m_label is null!");
    }
};

void Dropdown::Impl::openOverlay() {
    log::info("Dropdown::Impl::openOverlay called");
    if (!m_enabled) {
        log::warn("Dropdown::Impl::openOverlay: not enabled");
        return;
    }
    if (m_overlay) {
        log::warn("Dropdown::Impl::openOverlay: overlay already open");
        return;
    }
    auto scene = CCDirector::get()->getRunningScene();
    if (!scene) {
        log::error("Dropdown::Impl::openOverlay: no running scene");
        return;
    }
    m_savedZOrder = m_self->getZOrder();
    m_self->setZOrder(9998);
    m_overlay = DropdownOverlay::create(this);
    if (m_overlay) {
        log::info("Dropdown::Impl::openOverlay: overlay created, adding to scene");
        CCTouchDispatcher::get()->registerForcePrio(m_overlay, 2);
        scene->addChild(m_overlay, 9999);
    } else {
        log::error("Dropdown::Impl::openOverlay: failed to create overlay");
    }
}

void Dropdown::Impl::closeOverlay() {
    log::info("Dropdown::Impl::closeOverlay called");
    if (m_overlay) {
        CCTouchDispatcher::get()->unregisterForcePrio(m_overlay);
        m_overlay->removeFromParentAndCleanup(true);
        m_overlay = nullptr;
        m_self->setZOrder(m_savedZOrder);
    }
}

void Dropdown::Impl::selectOption(size_t index) {
    log::info("Dropdown::Impl::selectOption called with index={}", index);
    if (index >= m_options.size()) {
        log::warn("Dropdown::Impl::selectOption: index out of range");
        return;
    }
    m_selectedIndex = index;
    updateLabel();
    closeOverlay();
    if (m_callback) {
        log::info("Dropdown::Impl::selectOption: invoking callback with value='{}'", m_options[m_selectedIndex]);
        m_callback(m_options[m_selectedIndex], m_selectedIndex);
    }
}

void Dropdown::Impl::setEnabled(bool enabled) {
    log::info("Dropdown::Impl::setEnabled called with enabled={}", enabled);
    m_enabled = enabled;
    if (m_button) m_button->setEnabled(enabled);
    GLubyte opacity = enabled ? 255 : 155;
    auto color = enabled ? ccWHITE : ccGRAY;
    if (m_label) {
        m_label->setOpacity(opacity);
        m_label->setColor(color);
    }
    if (m_arrow) {
        m_arrow->setOpacity(opacity);
        m_arrow->setColor(color);
    }
    if (m_bg) m_bg->setOpacity(enabled ? 255 : 155);
}

bool Dropdown::Impl::init(
    float width, std::vector<std::string> options,
    Function<void(std::string const&, size_t)> callback
) {
    m_width = width;
    m_options = std::move(options);
    m_callback = std::move(callback);
    m_selectedIndex = 0;
    m_enabled = true;

    auto buttonSprite = CCSprite::create("GJ_square02.png");
    if (!buttonSprite) {
        log::warn("Dropdown: Failed to load GJ_square02.png, using fallback");
        buttonSprite = CCSprite::create("geode.loader/GE_square02.png");
    }
    if (!buttonSprite) {
        log::error("Dropdown: Failed to create any button background! Using blank node.");
        buttonSprite = CCSprite::create();
        buttonSprite->setTextureRect({0, 0, m_width, 36.f});
        buttonSprite->setColor(ccc3(60, 60, 60));
    }
    buttonSprite->setContentSize({m_width, 36.f});
    buttonSprite->setAnchorPoint({0, 0});

    m_label = CCLabelBMFont::create(m_options.empty() ? "" : m_options[m_selectedIndex].c_str(), "bigFont.fnt");
    if (!m_label) {
        log::warn("Dropdown: Failed to create label with bigFont.fnt, using chatFont.fnt");
        m_label = CCLabelBMFont::create(m_options.empty() ? "" : m_options[m_selectedIndex].c_str(), "chatFont.fnt");
    }
    if (m_label) {
        m_label->setAnchorPoint({0, 0.5f});
        m_label->setPosition({14.f, 18.f});
        m_label->limitLabelWidth(m_width - 30.f, 0.35f, 0.1f);
        buttonSprite->addChild(m_label);
    } else {
        log::error("Dropdown: Failed to create any label!");
    }

    m_arrow = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    if (!m_arrow) {
        m_arrow = CCSprite::create("GJ_arrow_03_001.png");
    }
    if (m_arrow) {
        m_arrow->setScale(0.5f);
        m_arrow->setAnchorPoint({1, 0.5f});
        m_arrow->setPosition({m_width - 10.f, 18.f});
        buttonSprite->addChild(m_arrow);
    } else {
        log::warn("Dropdown: Failed to create arrow sprite");
    }

    m_button = CCMenuItemSpriteExtra::create(buttonSprite, nullptr, nullptr);
    m_button->setContentSize({m_width, 36.f});
    m_button->setAnchorPoint({0, 0});
    m_button->setTarget(m_self, menu_selector(Dropdown::onOpen));

    m_menu = CCMenu::create();
    m_menu->setPosition({0, 0});
    m_menu->addChild(m_button);
    m_self->addChild(m_menu);

    updateLabel();
    setEnabled(true);
    return true;
}