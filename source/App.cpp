//
//  TMApp.cpp
//  TileMap Lab
//
//  This is the header for the custom application. It is necessary so that
//  main.cpp can access your custom class. That is handled by a separate
//  subcontroller.
//
//  Authors: Walker White and Gonzalo Gonzalez
//  Version: 2/8/23
//
// Include the class header, which includes all of the CUGL classes
#include "App.h"
#include <Level/LevelConstants.h>
#include <Level/LevelModel.h>

// This keeps us from having to write cugl:: all the time
using namespace cugl;
using namespace std;
// The number of frames before moving the logo to a new position
#define TIME_STEP 60
// This is adjusted by screen aspect ratio to get the height
#define GAME_WIDTH 1024

/**
 * The method called after OpenGL is initialized, but before running the application.
 *
 * This is the method in which all user-defined program intialization should
 * take place.  You should not create a new init() method.
 *
 * When overriding this method, you should call the parent method as the
 * very last line.  This ensures that the state will transition to FOREGROUND,
 * causing the application to run.
 */
void App::onStartup() {
    // Init the asset manager
    _assets = AssetManager::alloc();
    
    // Start-up input
    Input::activate<Touchscreen>();
    Input::activate<CoreGesture>();
    
    // init the assetManager
    _assets->attach<Font>(FontLoader::alloc()->getHook());
     _assets->attach<Texture>(TextureLoader::alloc()->getHook());
    //_assets->attach<Sound>(SoundLoader::alloc()->getHook());
    _assets->attach<WidgetValue>(WidgetLoader::alloc()->getHook());
    _assets->attach<scene2::SceneNode>(Scene2Loader::alloc()->getHook());
    
    _assets->attach<LevelModel>(GenericLoader<LevelModel>::alloc()->getHook());

    // for now, just loading synchronous
    _assets->loadDirectoryAsync("json/assets.json", nullptr);
//    // load level files
    _assets->loadAsync<LevelModel>(LEVEL_ZERO_PAST_KEY, LEVEL_ZERO_PAST_FILE, nullptr);
    _assets->loadAsync<LevelModel>(LEVEL_ZERO_PRESENT_KEY, LEVEL_ZERO_PRESENT_FILE, nullptr);
//    _assets->loadAsync<LevelModel>(LEVEL_ONE_PAST_KEY, LEVEL_ONE_PAST_FILE, nullptr);
//    _assets->loadAsync<LevelModel>(LEVEL_ONE_PRESENT_KEY, LEVEL_ONE_PRESENT_FILE, nullptr);

    // Create a sprite batch (and background color) to render the scene
    _batch = SpriteBatch::alloc();
    auto cam = OrthographicCamera::alloc(getDisplaySize());
    setClearColor(Color4(229,229,229,255));
    
    // Activate the touchscreen for interacting with the tilemap

    Input::activate<Touchscreen>();
    
    active_scene = "GamePlay";
    
    // Initialize GameController, passing it the random number generator
    Size size = getDisplaySize();
    size *= GAME_WIDTH/size.width;
    
    
    _loadingController = make_unique<LoadingController>();
    
    _loaded = false;
    _loadingController->init(_assets);
    
    // parent call
    Application::onStartup();
}

/**
 * The method called when the application is ready to quit.
 *
 * This is the method to dispose of all resources allocated by this
 * application.  As a rule of thumb, everything created in onStartup()
 * should be deleted here.
 *
 * When overriding this method, you should call the parent method as the
 * very last line.  This ensures that the state will transition to NONE,
 * causing the application to be deleted.
 */
void App::onShutdown() {
    // Delete all smart pointers
    _batch = nullptr;
    
    // Deactivate input
    Input::deactivate<Touchscreen>();
    
    Application::onShutdown();
}

/**
 * The method called to update the application data.
 *
 * This is your core loop and should be replaced with your custom implementation.
 * This method should contain any code that is not an OpenGL call.
 *
 * When overriding this method, you do not need to call the parent method
 * at all. The default implmentation does nothing.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void App::update(float timestep) {
    if (!_loaded && _loadingController->_scene->isActive()) {
        _loadingController->update(timestep);
    } else if (!_loaded) {
        _loadingController->dispose(); // Disables the input listeners in this mode
        _loaded = true;
        Size size = getDisplaySize();
        size *= GAME_WIDTH/size.width;
        _gameplayController = make_unique<GamePlayController>(size, _assets);
    } else {
        
        _gameplayController->update(timestep);
    }
}

/**
 * The method called to draw the application to the screen.
 *
 * This is your core loop and should be replaced with your custom implementation.
 * This method should OpenGL and related drawing calls.
 *
 * When overriding this method, you do not need to call the parent method
 * at all. The default implmentation does nothing.
 */
void App::draw() {
    if (!_loaded) {
        _loadingController->render(_batch);
    } else {
        _gameplayController->render(_batch);
    }
   
}
