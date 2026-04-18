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
	const char* path = "goldFont.png"_spr;
	if (!CCFileUtils::get()->isFileExist(path)) {
		log::error("Font PNG file missing: {}", path);
	}
	return path;
}

const char* getFnt() {
	const char* path = "goldFont.fnt"_spr;
	if (!CCFileUtils::get()->isFileExist(path)) {
		log::error("Font FNT file missing: {}", path);
	}
	return path;
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
		log::info("[Localize] Entering createTextLayers");
		isInCreateTextLayers = true;
		GJBaseGameLayer::createTextLayers();
		isInCreateTextLayers = false;
		log::info("[Localize] Exiting createTextLayers");
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
			log::info("[Localize] createBatched called with fntFile: {}", fntFile);
			if (static_cast<std::string>(fntFile) == "goldFont.fnt") fntFile = getFnt();
			if (!fntFile || !CCFileUtils::get()->isFileExist(fntFile)) {
				log::error("[Localize] FNT file missing in createBatched: {}", fntFile ? fntFile : "(null)");
			}
			auto font = CCLabelBMFont::createBatched(str, fntFile, a, a1);
			if (!font) {
				log::error("[Localize] Failed to create CCLabelBMFont in createBatched");
				return nullptr;
			}
			font->setString(str);
			return font;
		}

	static CCLabelBMFont* create(char const* str, char const* fntFile, float width, CCTextAlignment alignment, CCPoint offset) {
		log::info("[Localize] create called with fntFile: {}", fntFile);
		const char* newStr = str;
		if (hasTranslationKey(str)) {
			std::string translated = getLanguageString(str);
			if (translated.empty()) {
				log::warn("[Localize] Translation for '{}' is empty", str);
			} else {
				newStr = translated.c_str();
				log::info("[Localize] Translated '{}' to '{}'", str, newStr);
			}
		}
		if (!fntFile || !CCFileUtils::get()->isFileExist(fntFile)) {
			log::error("[Localize] FNT file missing in create: {}", fntFile ? fntFile : "(null)");
		}
		auto label = CCLabelBMFont::create(newStr, fntFile, width, alignment, offset);
		if (!label) {
			log::error("[Localize] Failed to create CCLabelBMFont in create");
		}
		return label;
	}


	#ifndef GEODE_IS_IOS
		void setFntFile(const char* fntFile) {
			log::info("[Localize] setFntFile called with: {}", fntFile);
			if (!fntFile) {
				log::error("[Localize] setFntFile called with null pointer");
				return;
			}
			if (strcmp(fntFile, "goldFont.fnt") == 0) {
				return CCLabelBMFont::setFntFile(getFnt());
			}
			if (!CCFileUtils::get()->isFileExist(fntFile)) {
				log::error("[Localize] setFntFile: FNT file missing: {}", fntFile);
			}
			CCLabelBMFont::setFntFile(fntFile);
		}
	#endif

		void setString(const char* string, bool needUpdateLabel) {
			if (!(typeinfo_cast<MultilineBitmapFont*>(this->getParent()))) {
				if (hasTranslationKey(string)) {
					std::string newStr = getLanguageString(string);
					if (newStr.empty()) {
						log::warn("[Localize] setString: Translation for '{}' is empty", string);
						CCLabelBMFont::setString(string, needUpdateLabel);
					} else {
						log::info("[Localize] setString: Translated '{}' to '{}'", string, newStr);
						CCLabelBMFont::setString(newStr.c_str(), needUpdateLabel);
					}
				} else {
					CCLabelBMFont::setString(string, needUpdateLabel);
				}
			}
		}

		void setCString(const char* string) {
			if (hasTranslationKey(string)) {
				std::string newStr = getLanguageString(string);
				if (newStr.empty()) {
					log::warn("[Localize] setCString: Translation for '{}' is empty", string);
					CCLabelBMFont::setCString(string);
				} else {
					log::info("[Localize] setCString: Translated '{}' to '{}'", string, newStr);
					CCLabelBMFont::setCString(newStr.c_str());
				}
			} else {
				CCLabelBMFont::setCString(string);
			}
		}
};

class $modify(CCTextureCache) {
	CCTexture2D* addImage(const char* fileImage, bool p1) {
		log::info("[Localize] addImage called with: {}", fileImage ? fileImage : "(null)");
		CCTexture2D* ret = nullptr;
		bool didChange = false;
		if (fileImage && static_cast<std::string>(fileImage) == "goldFont.png") {
			if (GJBaseGameLayer::get()) {
				didChange = true;
				const char* png = getPng();
				if (!CCFileUtils::get()->isFileExist(png)) {
					log::error("[Localize] addImage: goldFont.png missing: {}", png);
				}
				ret = CCTextureCache::addImage(png, p1);
			}
		}
		if (!didChange) {
			if (!fileImage || !CCFileUtils::get()->isFileExist(fileImage)) {
				log::error("[Localize] addImage: file missing: {}", fileImage ? fileImage : "(null)");
			}
			ret = CCTextureCache::addImage(fileImage, p1);
		}
		if (!ret) {
			log::error("[Localize] addImage: Failed to load texture: {}", fileImage ? fileImage : "(null)");
		}
		return ret;
	}
};

#ifndef GEODE_IS_WINDOWS
class $modify(MyMultilineBitmapFont, MultilineBitmapFont) {
	static MultilineBitmapFont* createWithFont(char const* font, gd::string text, float scale, float width, CCPoint anchor, int height, bool disableColor) {
		log::info("[Localize] createWithFont called with font: {}", font ? font : "(null)");
		gd::string useText = text;
		if (hasTranslationKey(text)) {
			std::string newStr = getLanguageString(text);
			if (newStr.empty()) {
				log::warn("[Localize] createWithFont: Translation for '{}' is empty", text.c_str());
			} else {
				useText = newStr;
				log::info("[Localize] createWithFont: Translated '{}' to '{}'", text.c_str(), useText.c_str());
			}
		}
		if (!font || !CCFileUtils::get()->isFileExist(font)) {
			log::error("[Localize] createWithFont: Font file missing: {}", font ? font : "(null)");
		}
		auto result = MultilineBitmapFont::createWithFont(font, useText, scale, width, anchor, height, disableColor);
		if (!result) {
			log::error("[Localize] createWithFont: Failed to create MultilineBitmapFont");
		}
		return result;
	}
};
#endif
