//
//  SavedGameModel.cpp
//  The Preservation Project
//
//  Created by Ann Zhou on 4/27/23.
//

#include "SavedGameModel.h"

#include <cugl/assets/CUJsonLoader.h>
#include <string>

#pragma mark -
#pragma mark Static Constructors

/**
* Creates a new, empty level.
*/
SavedGameModel::SavedGameModel(void) : Asset()
{
}

/**
* Destroys this level, releasing all resources.
*/
SavedGameModel::~SavedGameModel(void) {
    unload();
    clearRootNode();
}


#pragma mark -
#pragma mark Drawing Methods
/**
* Clears the root scene graph node for this level
*/
void SavedGameModel::clearRootNode() {
}


#pragma mark -
#pragma mark Asset Loading
/**
 * Loads this game level from the source file
 *
 * This load method should NEVER access the AssetManager.  Assets are loaded in
 * parallel, not in sequence.  If an asset (like a game level) has references to
 * other assets, then these should be connected later, during scene initialization.
 *
 * @return true if successfully loaded the asset from a file
 */
bool SavedGameModel::preload(const std::string& file) {
    std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(file);
    CULog("SavedGameModel...preload...");
    return preload(reader->readJson());
}

/**
 * Loads this game level from the source file
 *
 * This load method should NEVER access the AssetManager.  Assets are loaded in
 * parallel, not in sequence.  If an asset (like a game level) has references to
 * other assets, then these should be connected later, during scene initialization.
 *
 * @return true if successfully loaded the asset from a file
 */
bool SavedGameModel:: preload(const std::shared_ptr<cugl::JsonValue>& json) {
    if (json == nullptr) {
        CUAssertLog(false, "Failed to load level file");
        return false;
    }
    
    // Get each object in each layer
//    for (int i = 0; i < json->get("layers")->size(); i++) {
//        // Get the objects per layer
//        auto objects = json->get("layers")->get(i)->get("objects");
//        std::string type = json->get("layers")->get(i)->get("name")->asString();
//        for (int j = 0; j < objects->size(); j++) {
//            // For each object, determine what it is and load it
//            loadObject(type, totalHeight, objects->get(j));
//        }
//    }

    return true;
}

/**
* Unloads this game level, releasing all sources
*
* This load method should NEVER access the AssetManager.  Assets are loaded and
* unloaded in parallel, not in sequence.  If an asset (like a game level) has
* references to other assets, then these should be disconnected earlier.
*/
void SavedGameModel::unload() {
}

#pragma mark -
#pragma mark Individual Loaders

/**
* Loads character initial position
*/
bool SavedGameModel::loadSavedGame(const std::shared_ptr<JsonValue>& json) {
    bool success = true;
    _currentLevel = json->get("currentLevel")->asInt();
    return success;
}
