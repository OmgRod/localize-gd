#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <alphalaneous.alphas_geode_utils/include/ObjectModify.hpp>

using namespace geode::prelude;

// thanks alphalaneous for letting me use your code :)
class $nodeModify(ModsLayerExt, ModsLayer) {	
    void modify() {
		if (CCNode* installedTab = querySelector("installed-button")) {
            if (CCNode* sprite = installedTab->getChildByIndex(0)) {
                if (CCLabelBMFont* label = sprite->getChildByType<CCLabelBMFont>(0)) {
                    // label->setString("Test");
                }
            }
        }
	}
};
