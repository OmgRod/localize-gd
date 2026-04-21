// Adapted from ChangeFont19 - credit that was previously here was making this file way more annoying to read so i decided to move it to `about.md`

#include <Geode/Geode.hpp>
#include <Geode/modify/CCLabelBMFont.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/CCSpriteBatchNode.hpp>
#include <Geode/modify/CCTextureCache.hpp>

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
		log::info("Entering createTextLayers");
		isInCreateTextLayers = true;
		GJBaseGameLayer::createTextLayers();
		isInCreateTextLayers = false;
		log::info("Exiting createTextLayers");
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

class $modify(CCLabelBMFont) {
	static CCLabelBMFont* createBatched(const char* str, const char* fntFile, CCArray* a, int a1) {
		if (static_cast<std::string>(fntFile) == "goldFont.fnt") fntFile = getFnt();
		return CCLabelBMFont::createBatched(str, fntFile, a, a1);
	}
	#ifndef GEODE_IS_IOS
	void setFntFile(const char* fntFile) {
		if (strcmp(fntFile, "goldFont.fnt") == 0) {
			return CCLabelBMFont::setFntFile(getFnt());
		}
		CCLabelBMFont::setFntFile(fntFile);
	}
	#endif
};

class $modify(CCTextureCache) {
	static void onModify(auto& self) {
        if (!self.setHookPriorityPre("cocos2d::CCTextureCache::addImage", Priority::Late)) {
            log::warn("Failed to set hook priority.");
        }
    }

	CCTexture2D* addImage(const char* fileImage, bool p1) {
		// log::info("addImage called with: {}", fileImage ? fileImage : "(null)");
		CCTexture2D* ret = nullptr;
		bool didChange = false;
		if (fileImage && static_cast<std::string>(fileImage) == "goldFont.png") {
			if (GJBaseGameLayer::get()) {
				didChange = true;
				const char* png = getPng();
				if (!CCFileUtils::get()->isFileExist(png)) {
					log::error("addImage: goldFont.png missing: {}", png);
				}
				ret = CCTextureCache::addImage(png, p1);
			}
		}
		if (!didChange) {
			// if (!fileImage || !CCFileUtils::get()->isFileExist(fileImage)) {
			// 	log::error("addImage: file missing: {}", fileImage ? fileImage : "(null)");
			// }
			ret = CCTextureCache::addImage(fileImage, p1);
		}
		if (!ret) {
			log::error("addImage: Failed to load texture: {}", fileImage ? fileImage : "(null)");
		}
		return ret;
	}
};
