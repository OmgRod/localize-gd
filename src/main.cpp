/*
This file is adapted from ChangeFont19.

ChangeFont19 is free software: you can redistribute it
and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

ChangeFont19 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ChangeFont19. If not, see <https://www.gnu.org/licenses/>.
*/

/*
Source code based on and adapted from Alpahalaneous' code
for the "Pusab Font Fix" feature in Happy Textures.
Reused with permission granted by GPLv3.

Bindings found by https://github.com/hiimjustin000,
and verified from building HappyTextures manually
and enabling "Pusab Fix" in-game.
*/

#include <Geode/Geode.hpp>
#include <Geode/modify/CCLabelBMFont.hpp>
#include <Geode/modify/MultilineBitmapFont.hpp>
#include <Geode/modify/MultilineBitmapFont.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/CCSpriteBatchNode.hpp>
#include <Geode/modify/CCTextureCache.hpp>
#include "Utils.hpp"

using namespace geode::prelude;

static bool isInCreateTextLayers = false;

const char* getPng() {
	return "goldFont.png"_spr;
}

const char* getFnt() {
	return "goldFont.fnt"_spr;
}

$on_mod(Loaded) {
	const std::string& resourcesDir = geode::utils::string::pathToString(Mod::get()->getResourcesDir());
	log::info("MAKING TEXTURE PACK USING DIRECTORY: {}", resourcesDir);
	auto directoryVector = std::vector<std::string>{ resourcesDir };
	const auto texturePack = CCTexturePack {
		.m_id = Mod::get()->getID(), // they're the same ID so it doesnt matter
		.m_paths = directoryVector
	};
	CCFileUtils::get()->addTexturePack(texturePack);
}

class $modify(GJBaseGameLayer) {
	void createTextLayers() {
		isInCreateTextLayers = true;
		GJBaseGameLayer::createTextLayers();
		isInCreateTextLayers = false;
	}
};

class $modify(CCSpriteBatchNode) {
	bool initWithTexture(CCTexture2D* texture, unsigned int capacity) {
		if (isInCreateTextLayers && texture == CCTextureCache::sharedTextureCache()->addImage("goldFont.png", false)) {
			return CCSpriteBatchNode::initWithTexture(CCTextureCache::sharedTextureCache()->addImage(getPng(), false), capacity);
		}
		return CCSpriteBatchNode::initWithTexture(texture, capacity);
	}
};

class $modify(MyCCLabelBMFont, CCLabelBMFont) {
	static CCLabelBMFont* createBatched(const char* str, const char* fntFile, CCArray* a, int a1) {
		if (static_cast<std::string>(fntFile) == "goldFont.fnt") fntFile = getFnt();
        auto font = CCLabelBMFont::createBatched(str, fntFile, a, a1);
        font->setString(str);
		return font;
	}

    static CCLabelBMFont* create(char const* str, char const* fntFile, float width, CCTextAlignment alignment, CCPoint offset) {
        auto newStr = str;
        if (hasTranslationKey(str)) {
            std::string newStr = getLanguageString(str);
		}
        auto label = CCLabelBMFont::create(newStr, fntFile, width, alignment, offset);
        return label;
    }

    #ifndef GEODE_IS_IOS
	void setFntFile(const char* fntFile) {
		if (strcmp(fntFile, "goldFont.fnt") == 0) {
			return CCLabelBMFont::setFntFile(getFnt());
		}
		CCLabelBMFont::setFntFile(fntFile);
	}
    #endif

	void setString(const char* string, bool needUpdateLabel) {
        if (!(typeinfo_cast<MultilineBitmapFont*>(this->getParent()))) {
            if (hasTranslationKey(string)) {
                std::string newStr = getLanguageString(string);
                CCLabelBMFont::setString(newStr.c_str(), needUpdateLabel);
            } else {
                CCLabelBMFont::setString(string, needUpdateLabel);
            }
        }
	}

	void setCString(const char* string) {
		if (hasTranslationKey(string)) {
			std::string newStr = getLanguageString(string);
			CCLabelBMFont::setCString(newStr.c_str());
		} else {
			CCLabelBMFont::setCString(string);
		}
	}
};

class $modify(CCTextureCache) {
	CCTexture2D* addImage(const char* fileImage, bool p1) {
		CCTexture2D* ret = nullptr;
		bool didChange = false;
		if (static_cast<std::string>(fileImage) == "goldFont.png") {
			if (GJBaseGameLayer::get()) {
				didChange = true;
				ret = CCTextureCache::addImage(getPng(), p1);
			}
		}
		if (!didChange) ret = CCTextureCache::addImage(fileImage, p1);
		return ret;
	}
};

#ifndef GEODE_IS_WINDOWS
class $modify(MyMultilineBitmapFont, MultilineBitmapFont) {
    static MultilineBitmapFont* createWithFont(char const* font, gd::string text, float scale, float width, CCPoint anchor, int height, bool disableColor) {
        if (hasTranslationKey(text)) {
			std::string newStr = getLanguageString(text);
			MultilineBitmapFont::createWithFont(font, newStr, scale, width, anchor, height, disableColor);
		} else {
			MultilineBitmapFont::createWithFont(font, text, scale, width, anchor, height, disableColor);
		}
    }
};
#endif
