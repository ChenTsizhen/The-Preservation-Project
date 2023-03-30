#include <chrono>
#include "GamePlayController.h"
#include <chrono>
#include <thread>
#include "Level/LevelConstants.h"
// This is NOT in the same directory
using namespace std;
using namespace cugl;
#define PHYSICS_SCALE 50
/** This is adjusted by screen aspect ratio to get the height */
#define SCENE_WIDTH 1024
#define DURATION 1.0f
#define ACT_KEY  "current"

GamePlayController::GamePlayController(const Size displaySize, std::shared_ptr<cugl::AssetManager>& assets ):_scene(cugl::Scene2::alloc(displaySize)) {
    // Initialize the assetManager
    _assets = assets;
    // Initialize the scene to a locked width
    Size dimen = Application::get()->getDisplaySize();
    dimen *= SCENE_WIDTH/dimen.width; // Lock the game to a reasonable resolution
    _input->init(dimen);
    _cam = _scene->getCamera();
    // Allocate the manager and the actions
    _actions = cugl::scene2::ActionManager::alloc();
    
    // Allocate the camera manager
    _camManager = CameraManager::alloc();
    
    _scene->setSize(displaySize/1);
    
    _path = make_unique<PathController>();
    // initialize character, two maps, path
    
    // Draw past world
    _pastWorldLevel = _assets->get<LevelModel>(LEVEL_ZERO_PAST_KEY);
//    _pastWorldLevel = _assets->get<LevelModel>(LEVEL_ONE_PAST_KEY);
    if (_pastWorldLevel == nullptr) {
        CULog("Failed to import level!");
    }
    _pastWorldLevel->setAssets(_assets);
    _pastWorldLevel->setTilemapTexture();
    _pastWorld = _pastWorldLevel->getWorld();
    _artifactSet = _pastWorldLevel->getItem();
    _resourceSet = _pastWorldLevel->getResource();
    _artifactSet->addChildTo(_scene);
    _resourceSet->addChildTo(_scene);

    // Draw present world
    _presentWorldLevel = _assets->get<LevelModel>(LEVEL_ZERO_PRESENT_KEY);
//    _presentWorldLevel = _assets->get<LevelModel>(LEVEL_ONE_PRESENT_KEY);
    if (_presentWorldLevel == nullptr) {
        CULog("Failed to import level!");
    }
    _presentWorldLevel->setAssets(_assets);
    _presentWorldLevel->setTilemapTexture();
    _presentWorld = _presentWorldLevel->getWorld();

    _guardSet1 = std::make_shared<GuardSetController>(_assets, _actions);
    _guardSet2 = std::make_unique<GuardSetController>(_assets, _actions);
    generateGuard();
    secondaryGuard();
    
//    Vec2 start = Vec2(_scene->getSize().width * 0.85, _scene->getSize().height * 0.15);
    Vec2 start = Vec2(1,1);

    _character = make_unique<CharacterController>(start, _actions, _assets);
    // Forward character movement
    const int span = 8;
    std::vector<int> forward;
    for(int ii = 1; ii < span; ii++) {
        forward.push_back(ii);
    }
    // Loop back to beginning
    forward.push_back(0);
    _characterRight = cugl::scene2::Animate::alloc(forward, DURATION);

    // Reverse charater movement
    std::vector<int> reverse;
    for(int ii = 1; ii <= span; ii++) {
        reverse.push_back(span-ii);
    }

    _characterLeft = cugl::scene2::Animate::alloc(forward, DURATION);

    // init the button
    _button_layer = _assets->get<scene2::SceneNode>("button");
    _button_layer->setContentSize(dimen);
    _button_layer->doLayout(); // This rearranges the children to fit the screen
    
    _button = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("button_reset"));
    _button->addListener([this](const std::string& name, bool down) {
        if (down) {
            this->init();
        }
    });
    _button->activate();
    
    // load label for n_res and n_art
    _res_label  = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("button_resources"));
    
    _art_label  = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("button_progress"));
    
    // add Win/lose panel
    _complete_layer = _assets->get<scene2::SceneNode>("complete");
    auto complete_again_button = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("complete_again"));
    
    complete_again_button->addListener([this](const std::string& name, bool down) {
        if (down) {
            this->init();
        }
    });
    
    complete_again_button->activate();
    
    _fail_layer = _assets->get<scene2::SceneNode>("fail");
    auto fail_again_button = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("fail_again"));
    
    fail_again_button->addListener([this](const std::string& name, bool down) {
        if (down) {
            this->init();
        }
    });
    
    fail_again_button->activate();
    
    
    
    // add switch indicator
    _switchNode = _assets->get<scene2::SceneNode>("button_switch");

    
    _moveTo = cugl::scene2::MoveTo::alloc();
    _moveCam = CameraMoveTo::alloc();
    _moveCam->setDuration(.08);
    _moveTo->setDuration(.08);
    
    init();
    
    
//    _scene->addChild(_complete_layer);
//    _complete_layer->setPosition(_cam->getPosition());
//    _scene->addChild(_fail_layer);
//    _fail_layer->setPosition(_cam->getPosition());
}


// init some assets when restart
void GamePlayController::init(){
    
    // dispose all active actions
    _actions->dispose();
    _camManager->dispose();
    
    // remove everything first
    _scene->removeAllChildren();
    
    
    
    _pastWorld->addChildTo(_scene);
    _activeMap = "pastWorld";
    _template = 0;
    
    Vec2 start = Vec2(0,0);
    _character = make_unique<CharacterController>(start, _actions, _assets);
    _character->addChildTo(_scene);
    // Forward character movement
    const int span = 8;
    std::vector<int> forward;
    for(int ii = 1; ii < span; ii++) {
        forward.push_back(ii);
    }
    // Loop back to beginning
    forward.push_back(0);
    _characterRight = cugl::scene2::Animate::alloc(forward, DURATION);

    // Reverse charater movement
    std::vector<int> reverse;
    for(int ii = 1; ii <= span; ii++) {
        reverse.push_back(span-ii);
    }

    _characterLeft = cugl::scene2::Animate::alloc(forward, DURATION);
    
    _artifactSet->addChildTo(_scene);
    _resourceSet->addChildTo(_scene);
    
    _guardSet1 = make_unique<GuardSetController>(_assets, _actions);
    _guardSet2 = make_unique<GuardSetController>(_assets, _actions);
    _guardSet1->clearSet();
    _guardSet2->clearSet();
    generateGuard();
    secondaryGuard();
    _guardSet2->removeChildFrom(_scene);
    
    _path = make_unique<PathController>();
    path_trace = {};
    
    _scene->addChild(_button_layer);
    
    Vec2 cPos = _character->getPosition();
    _cam->setPosition(Vec3(cPos.x,cPos.y,0));

    _cam->update();
    
    // to make the button pos fixed relative to screen
    _button_layer->setPosition(_cam->getPosition());

    
    // reload initial label for n_res and n_art
    _res_label->setText("0");
    _art_label->setText("0/4");
}


void GamePlayController::update(float dt){
    _guardSet1->patrol();
    if(_fail_layer->getScene()!=nullptr || _complete_layer->getScene()!=nullptr){
        return;
    }
    static auto last_time = std::chrono::steady_clock::now();
    // Calculate the time elapsed since the last call to pinch
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_time);
       
    
    _input->update(dt);
    // if pinch, switch world
    bool cant_switch = ((_activeMap == "pastWorld" && _presentWorld->inObstacle(_character->getPosition())) || (_activeMap == "presentWorld" && _pastWorld->inObstacle(_character->getPosition())));
    

    cant_switch = cant_switch || (_character->getNumRes() == 0);
    
    if(cant_switch){
        _switchNode->setColor(Color4::RED);
    }
    else{
        _switchNode->setColor(Color4::GREEN);
    }
    if(elapsed.count() >= 0.5 && _input->getPinchDelta() != 0 && !cant_switch){
        // if the character's position on the other world is obstacle, disable the switch
        last_time = now;
        // remove and add the child back so that the child is always on the top layer
        
        _character->removeChildFrom(_scene);
        if (_activeMap == "pastWorld") {
            _pastWorld->removeChildFrom(_scene);
            _presentWorld->addChildTo(_scene);
            _guardSet1->removeChildFrom(_scene);
            _guardSet2->addChildTo(_scene);
            _artifactSet->removeChildFrom(_scene);
            _resourceSet->removeChildFrom(_scene);
            _activeMap = "presentWorld";
            
            // when move to the second world, minus 1 visually
            _res_label->setText(cugl::strtool::to_string(_character->getNumRes()-1));
        }
        else {
            _presentWorld->removeChildFrom(_scene);
            _pastWorld->addChildTo(_scene);
            _guardSet2->removeChildFrom(_scene);
            _guardSet1->addChildTo(_scene);
            _artifactSet->addChildTo(_scene);
            _resourceSet->addChildTo(_scene);
            _activeMap = "pastWorld";
            
            // when move to the second world, minus 1 in model
            _character->useRes();
        }
        _character->addChildTo(_scene);
        _scene->removeChild(_button_layer);
        _scene->addChild(_button_layer);
        // stop previous movement after switch world
        _path->clearPath();
    }
    
    else if (!_input->getPanDelta().isZero() && _path->getPath().size() == 0) {
        Vec2 delta = _input->getPanDelta();

        // init camera action
        _moveCam = CameraMoveTo::alloc();
        
        // pan move with the center of the camera view
        Vec2 pos = _cam->getPosition() - delta;
        if (pos.distance(_character->getNodePosition()) < 150){
            _moveCam->setTarget(_cam->getPosition() - delta);
            _camManager->activate("movingCam", _moveCam, _cam);
        }
    }
    
    else if (_input->didPan() && _path->getPath().size() == 0){
        _moveCam = CameraMoveTo::alloc();
        _moveCam->setDuration(1.25);
        // pan move with the center of the camera view
        _moveCam->setTarget(_character->getNodePosition());
        auto fcn = EasingFunction::alloc(EasingFunction::Type::BACK_OUT);
        _camManager->activate("movingCam", _moveCam, _cam, fcn);
    }
        
    else if(_input->didPress()){        // if press, determine if press on character
        Vec2 input_posi = _input->getPosition();
        input_posi = _scene->screenToWorldCoords(input_posi);
        
        if(_character->contains(input_posi)){
            // create path
            _path->setIsDrawing(true);
            _path->setIsInitiating(true);
            _path->updateLastPos(_character->getPosition()); //change to a fixed location on the character
            _path->clearPath();
        }
    }
    
    else if (_input->isDown() && _path->isDrawing){
        Vec2 input_posi = _input->getPosition();
        input_posi = _scene->screenToWorldCoords(input_posi);
        // if input still within the character
        if(_path->isInitiating){
            // if input leaves out of the character's radius, draw the initial segments
            if (!_character->contains(input_posi)){
                _path->setIsInitiating(false);
            }
        }
        // an updated version to iteratively add segments until the input_posi is too close to last point in the model
        if(_path->isInitiating == false){
            while(_path->farEnough(input_posi)){
                Vec2 checkpoint = _path->getLastPos() + (input_posi - _path->getLastPos()) / _path->getLastPos().distance(input_posi) * _path->getSize();
                if((_activeMap == "pastWorld" && _pastWorld->inObstacle(checkpoint)) || (_activeMap == "presentWorld" && _presentWorld->inObstacle(checkpoint))){
                    _path->setIsDrawing(false);
                    // path_trace.clear();
                    return;
                }
                else{
                    _path->addSegment(checkpoint, _scene);
                }
            }
        }
    }
    
    else if(_input->didRelease()){
        Vec2 input_posi = _input->getPosition();
        input_posi = _scene->screenToWorldCoords(input_posi);
        _path->setIsDrawing(false);
        // path_trace = _path->getPath();
        
        _path->removeFrom(_scene);
        
    }
    
    if (_path->getPath().size() != 0 && _actions->isActive("moving") == false){
        _moveTo->setTarget(_path->getPath()[0]);
        _moveCam->setTarget(_path->getPath()[0]);
        
        _character->moveTo(_moveTo);
        _camManager->activate("movingCam", _moveCam, _cam);
        // path_trace.erase(path_trace.begin());
        _path->removeFirst(_scene);
    }

    if (_actions->isActive("moving") && !_actions->isActive("character_animation")) {
            _character->updateAnimation(_characterRight);
        }
    
    // if collect a resource
    if(_activeMap == "pastWorld"){
        for(int i=0; i<_artifactSet->_artifactSet.size(); i++){
            // detect collision
            if(_character->contains(_artifactSet->_artifactSet[i]->getNodePosition())){
                // if close, should collect it
                
                // if resource
                if(_artifactSet->_artifactSet[i]->isResource()){
                    _character->addRes();
                    // update panel
                    _res_label->setText(cugl::strtool::to_string(_character->getNumRes()));
                    
                }
                // if artifact
                else{
                    _character->addArt();
                    _art_label->setText(cugl::strtool::to_string(_character->getNumArt()) + "/4");
                    
                }

                // make the artifact disappear and remove from set
                _artifactSet->remove_this(i, _scene);
                if(_character->getNumArt() == 4){
                    _scene->addChild(_complete_layer);
                    
                    _complete_layer->setPosition(_cam->getPosition());
                }
                
                break;
            }
            
        }
    }
    
    
    // if collide with guard
    if(_activeMap == "pastWorld"){
        for(int i=0; i<_guardSet1->_guardSet.size(); i++){
            if(_character->contains(_guardSet1->_guardSet[i]->getNodePosition())){
                _scene->addChild(_fail_layer);
                _fail_layer->setPosition(_cam->getPosition());
                break;
            }
        }
    }
    else{
        for(int i=0; i<_guardSet2->_guardSet.size(); i++){
            if(_character->contains(_guardSet2->_guardSet[i]->getNodePosition())){
                _scene->addChild(_fail_layer);
                _fail_layer->setPosition(_cam->getPosition());
                break;
            }
        }
    }
    
    // if guard cone collide with wall
//    if(_activeMap == "pastWorld"){
//        for(int i=0; i<_guardSet1->_guardSet.size(); i++){
//            if(_pastWorld->inObstacle(_guardSet1->_guardSet[i]->getNodePosition())){
//                break;
//            }
//        }
//    }
    
    // Animate
    
    _actions->update(dt);
    _camManager->update(dt);
    
    // the camera is moving smoothly, but the UI only set its movement per frame
    _button_layer->setPosition(_cam->getPosition() - Vec2(900, 70));
}
    
    
#pragma mark Main Methods

    
#pragma mark -
#pragma mark Generation Helpers

//    void GamePlayController::generateResource() {
        // switching hourglass
//        bool isResource = true;
//        addArtifact(0, 175, isResource);
//        addArtifact(280, 0, isResource);
//        addArtifact(550, 0, isResource);
//        addArtifact(850, 530, isResource);
//    }

    void GamePlayController::generateGuard() {
        vector<Vec2> patrol_stops = { Vec2(90, 500), Vec2(190, 500), Vec2(190, 400) }; //must be at least two stops
        addMovingGuard1(90, 500, patrol_stops);
        addGuard1(450, 250);
        addGuard1(500, 100);
        addGuard1(630, 500);
        addGuard1(850, 380);
        addGuard1(970, 75);
    }
    void GamePlayController::secondaryGuard() {
//        bool cone = false;
        addGuard2(350, 350);
        addGuard2(720, 320);
//        cone = true;
//        addGuard2(350, 350, cone);
//        addGuard2(720, 320, cone);
    }

    
        
    
#pragma mark -
#pragma mark Helpers

    
    void GamePlayController::render(std::shared_ptr<SpriteBatch>& batch){
        _scene->render(batch);
    }
    
