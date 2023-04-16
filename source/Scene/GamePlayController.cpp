#include <chrono>
#include "GamePlayController.h"
#include <chrono>
#include <thread>
#include "Level/LevelConstants.h"
#include <common.h>

// This is NOT in the same directory
using namespace std;
using namespace cugl;
#define PHYSICS_SCALE 50
/** This is adjusted by screen aspect ratio to get the height */
#define SCENE_WIDTH 1024
#define DURATION 0.8f
#define ACT_KEY  "current"

GamePlayController::GamePlayController(const Size displaySize, std::shared_ptr<cugl::AssetManager>& assets ):
_scene(cugl::Scene2::alloc(displaySize)), _other_scene(cugl::Scene2::alloc(displaySize)) {
    // Initialize the assetManager
    _assets = assets;
    
    // load the level info
    
    _assets->load<LevelModel>(LEVEL_ZERO_PAST_KEY, LEVEL_ZERO_PAST_FILE);
    _assets->load<LevelModel>(LEVEL_ZERO_PRESENT_KEY, LEVEL_ZERO_PRESENT_FILE);
    
    
    // Initialize the scene to a locked width
    Size dimen = Application::get()->getDisplaySize();
    dimen *= SCENE_WIDTH/dimen.width; // Lock the game to a reasonable resolution
    _input->init(dimen);
    
    _cam = _scene->getCamera();
    _other_cam = _other_scene->getCamera();
    
    // Allocate the manager and the actions
    _actions = cugl::scene2::ActionManager::alloc();
    
    // Allocate the camera manager
    _camManager = CameraManager::alloc();
    
    _scene->setSize(displaySize * 3);
    _other_scene->setSize(displaySize * 3);
//    _scene->setSize(displaySize);
//    _other_scene->setSize(displaySize);
    
    _path = make_unique<PathController>();
    // initialize character, two maps, path
    
    // Draw past world
    _pastWorldLevel = _assets->get<LevelModel>(LEVEL_ZERO_PAST_KEY);
    if (_pastWorldLevel == nullptr) {
        CULog("Failed to import level!");
    }
    _pastWorldLevel->setAssets(_assets);
    _pastWorldLevel->setTilemapTexture();
    _pastWorld = _pastWorldLevel->getWorld();
    _artifactSet = _pastWorldLevel->getItem();
    _wallSetPast = _pastWorldLevel->getWall();

    // Draw present world
    _presentWorldLevel = _assets->get<LevelModel>(LEVEL_ZERO_PRESENT_KEY);
    if (_presentWorldLevel == nullptr) {
        CULog("Failed to import level!");
    }
    _presentWorldLevel->setAssets(_assets);
    _presentWorldLevel->setTilemapTexture();
    _presentWorld = _presentWorldLevel->getWorld();
    _wallSetPresent = _presentWorldLevel->getWall();
    _presentWorld->updateColor(Color4::CLEAR);
    _pastWorld->updateColor(Color4::CLEAR);
    
    auto pastEdges = _pastWorld->getEdges(_scene);
    generatePastMat(_pastWorld->getVertices());
    for (int i = 0; i < pastEdges.size(); i++){
        addPastEdge(pastEdges[i].first, pastEdges[i].second);
    }
    
    auto presentEdges = _presentWorld->getEdges(_other_scene);
    generatePresentMat(_presentWorld->getVertices());
    for (int i = 0; i < presentEdges.size(); i++){
        addPresentEdge(presentEdges[i].first, presentEdges[i].second);
    }
    
    _guardSetPast = std::make_unique<GuardSetController>(_assets, _actions, _pastWorld, pastMatrix, _pastWorld->getNodes());
    _guardSetPresent = std::make_unique<GuardSetController>(_assets, _actions, _presentWorld, presentMatrix, _presentWorld->getNodes());
    generatePastGuards();
    generatePresentGuards();
    
//    Vec2 start = Vec2(_scene->getSize().width *.85, _scene->getSize().height *.15);
    Vec2 start = Vec2(0,0);

    _character = make_unique<CharacterController>(start, _actions, _assets);



    // init the button
    _button_layer = _assets->get<scene2::SceneNode>("button");
    _button_layer->setContentSize(dimen);
    _button_layer->doLayout(); // This rearranges the children to fit the screen
    
    _reset_button = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("button_reset"));
    _reset_button->addListener([this](const std::string& name, bool down) {
        if (!down) {

            // cout<<"reset"<<endl;
            this->init();
        }
    });
    _reset_button->activate();
    
    // load label for n_res and n_art
    _res_label  = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("button_resources"));
    
    _art_label  = std::dynamic_pointer_cast<scene2::Label>(assets->get<scene2::SceneNode>("button_progress"));
    
    // add Win/lose panel
    _complete_layer = _assets->get<scene2::SceneNode>("complete");
    _complete_again_button = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("complete_again"));
    
    _complete_again_button->addListener([this](const std::string& name, bool down) {
        if (!down) {
            // cout<<"complete_again"<<endl;
            this->init();
        }
    });
    
    _complete_back_button = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("complete_back"));
    
    _complete_back_button->addListener([this](const std::string& name, bool down) {
        if (!down) {
            // cout<<"complete_back"<<endl;
            nextScene = MENU;
        }
    });

    // fail panel
    _fail_layer = _assets->get<scene2::SceneNode>("fail");
    _fail_again_button = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("fail_again"));
    
    _fail_again_button->addListener([this](const std::string& name, bool down) {
        if (!down) {
            // cout<<"fail_again"<<endl;
            this->init();
        }
    });
    
    _fail_back_button = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("fail_back"));
    
    _fail_back_button->addListener([this](const std::string& name, bool down) {
        if (!down) {
            // cout<<"fail_back"<<endl;
            nextScene = MENU;
        }
    });

    // add switch indicator
    _switchNode = _assets->get<scene2::SceneNode>("button_switch");
    
    _moveTo = cugl::scene2::MoveTo::alloc();
    _moveCam = CameraMoveTo::alloc();
    _moveCam->setDuration(DURATION);
    _moveTo->setDuration(DURATION);
    
    init();
    
}



// init some assets when restart
void GamePlayController::init(){
    
    // dispose all active actions
    _actions->dispose();
    _camManager->dispose();
    
    // remove everything first
    _scene->removeAllChildren();

    _reset_button->activate();

    _other_scene->removeAllChildren();

    
    _pastWorld->addChildTo(_scene);
    _pastWorld->setVisibility(true);
    
    auto edges = _pastWorld->getEdges(_scene);
    generatePastMat(_pastWorld->getVertices());
    for (int i = 0; i < edges.size(); i++){
        addPastEdge(edges[i].first, edges[i].second);
    }
    
//    _artifactSet->clearSet();
//    _artifactSet = _pastWorldLevel->getItem();
    _artifactSet->addChildTo(_scene);
    _artifactSet->setVisibility(true);
    _wallSetPast->addChildTo(_scene);
    _wallSetPast->setVisibility(true);
    
    _presentWorld->addChildTo(_other_scene);
    _presentWorld->setActive(false);
    _wallSetPresent->addChildTo(_other_scene);
    _wallSetPresent->setVisibility(false);
    
    auto presentEdges = _presentWorld->getEdges(_other_scene);
    generatePresentMat(_presentWorld->getVertices());
    for (int i = 0; i < presentEdges.size(); i++){
        addPresentEdge(presentEdges[i].first, presentEdges[i].second);
    }
    
    
    _activeMap = "pastWorld";
    _template = 0;
    
    Vec2 start = Vec2(0,0);
    _character = make_unique<CharacterController>(start, _actions, _assets);
    _character->addChildTo(_scene);


//    _guardSet2->removeChildFrom(_scene);
    
    _guardSetPast = std::make_unique<GuardSetController>(_assets, _actions, _pastWorld, pastMatrix, _pastWorld->getNodes());
    _guardSetPresent = std::make_unique<GuardSetController>(_assets, _actions, _presentWorld, presentMatrix, _presentWorld->getNodes());
    
    _guardSetPast->clearSet();
    _guardSetPresent->clearSet();
    
    generatePastGuards();
    generatePresentGuards();
    _guardSetPast->setVisbility(true);
    _guardSetPresent->setVisbility(false);
    
    _path = make_unique<PathController>();
    path_trace = {};
    
    _scene->addChild(_button_layer);
    
    Vec2 cPos = _character->getPosition();
    _cam->setPosition(Vec3(cPos.x,cPos.y,0));
    _other_cam->setPosition(Vec3(cPos.x,cPos.y,0));
    
    _cam->update();
    _other_cam->update();
    
    // to make the button pos fixed relative to screen
    _button_layer->setPosition(_cam->getPosition());
    
    // reload initial label for n_res and n_art
    _res_label->setText("0");
    _art_label->setText("0/3");
    
    //_pastWorld->addPoints(_scene->getSize(), _scene);
    
}

void GamePlayController::update(float dt){
    
    if(_fail_layer->getScene()!=nullptr || _complete_layer->getScene()!=nullptr){
        return;
    }
    
#pragma mark Switch World Methods
    static auto last_time = std::chrono::steady_clock::now();
    // Calculate the time elapsed since the last call to pinch
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_time);

    // codes to determine if buttons should be activated
    if(_fail_layer->getScene() == nullptr){
        _fail_back_button->deactivate();
        _fail_again_button->deactivate();
    }
    if(_complete_layer->getScene() == nullptr){
        _complete_back_button->deactivate();
        _complete_again_button->deactivate();
    }
    

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
        
        if (_activeMap == "pastWorld") {
            _activeMap = "presentWorld";
            
            _presentWorld->setVisibility(true);
            _guardSetPresent->setVisbility(true);
            _wallSetPresent->setVisibility(true);

            _pastWorld->setVisibility(false);
            _guardSetPast->setVisbility(false);
            _artifactSet->setVisibility(false);
            _wallSetPast->setVisibility(false);
            
            _character->removeChildFrom(_scene);
            _character->addChildTo(_other_scene);
            
            // when move to the second world, minus 1 visually
            _res_label->setText(cugl::strtool::to_string(_character->getNumRes()-1));
        }
        else {
            _pastWorld->setVisibility(true);
            _guardSetPast->setVisbility(true);
            _artifactSet->setVisibility(true);
            _wallSetPast->setVisibility(true);
            
            _presentWorld->setActive(false);
            _wallSetPresent->setVisibility(false);

            _activeMap = "pastWorld";
            
            _character->removeChildFrom(_other_scene);
            _character->addChildTo(_scene);
            
            // when move to the second world, minus 1 in model
            _character->useRes();
        }
        
        // stop previous movement after switch world
        _path->clearPath();
    }
#pragma mark Pan Methods

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
    
#pragma mark Character Movement Methods
    else if(_input->didPress()){        // if press, determine if press on character
        _isPreviewing = true;
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
    
    else if(_input->isDown() and _isPreviewing){
        Vec2 input_posi = _input->getPosition();
        input_posi = _scene->screenToWorldCoords(input_posi);
        _pastWorld->makePreview(input_posi);
    }
    
    else if(_input->didRelease()){
        _isPreviewing = false;
        _pastWorld->removePreview();
        Vec2 input_posi = _input->getPosition();
        input_posi = _scene->screenToWorldCoords(input_posi);
        _path->setIsDrawing(false);
        // path_trace = _path->getPath();
        _path->removeFrom(_scene);
    }
    
    if (_path->getPath().size() != 0 && !_actions->isActive("moving")  && !_actions->isActive("character_animation")){
        _moveTo->setTarget(_path->getPath()[0]);
        _moveCam->setTarget(_path->getPath()[0]);
        _character->moveTo(_moveTo);
        _character->updateAnimation(_path->getPath()[0]);
        _camManager->activate("movingCam", _moveCam, _cam);
        _camManager->activate("movingOtherCam", _moveCam, _other_cam);
        // path_trace.erase(path_trace.begin());
        _path->removeFirst(_scene);
    }

#pragma mark Resource Collection Methods
    // if collect a resource
    if(_activeMap == "pastWorld"){
        for(int i=0; i<_artifactSet->_itemSet.size(); i++){
            // detect collision
            if(_character->contains(_artifactSet->_itemSet[i]->getNodePosition())){
                // if close, should collect it
                
                // if resource
                if(_artifactSet->_itemSet[i]->isResource()){
                    _character->addRes();
                    // update panel
                    _res_label->setText(cugl::strtool::to_string(_character->getNumRes()));
                    CULog("resource");
                }
                // if artifact
                else{
                    _character->addArt();
                    _art_label->setText(cugl::strtool::to_string(_character->getNumArt()) + "/3");
                }
                // make the artifact disappear and remove from set
                _artifactSet->remove_this(i, _scene);
                if(_character->getNumArt() == 10){
                    completeTerminate();
                }
                break;
            }
            
        }
    }

#pragma mark Guard Methods
    _guardSetPast->patrol(_character->getNodePosition(), _scene, _character->getAngle());
    _guardSetPresent->patrol(_character->getNodePosition(), _other_scene, _character->getAngle());
    // if collide with guard
    if(_activeMap == "pastWorld"){
        for(int i=0; i<_guardSetPast->_guardSet.size(); i++){
            if(_character->contains(_guardSetPast->_guardSet[i]->getNodePosition())){
                failTerminate();
                break;
            }
        }
    }
    
    else{
        for(int i=0; i<_guardSetPresent->_guardSet.size(); i++){
            if(_character->contains(_guardSetPresent->_guardSet[i]->getNodePosition())){
                failTerminate();
                break;
            }
        }
    }
    
    // Animate
    _actions->update(dt);
    _camManager->update(dt);
    
    // the camera is moving smoothly, but the UI only set its movement per frame
    _button_layer->setPosition(_cam->getPosition() - Vec2(900, 70));
}
    
    
#pragma mark Main Methods

    
#pragma mark -
#pragma mark Generation Helpers

    void GamePlayController::generatePastGuards() {
        vector<Vec2> patrol_stops = { Vec2(100, 500), Vec2(190, 500), Vec2(190, 400) }; //must be at least two stops
        addMovingGuard1(100, 500, patrol_stops);
//        vector<Vec2> patrol_stops = { Vec2(0, 600), Vec2(500, 600), Vec2(1000, 600) };
//        addMovingGuard1(0, 600, patrol_stops);
        
        vector<Vec2> patrol_stops1 = { Vec2(1200, 200), Vec2(1000, 200)};
        addMovingGuard1(1200, 200, patrol_stops1);
        //addGuard1(100, 500);
        addGuard1(400, 600);
    }

    void GamePlayController::generatePresentGuards() {
        addGuard2(720, 320);
    }

    
#pragma mark -
#pragma mark Helpers

    
    void GamePlayController::render(std::shared_ptr<SpriteBatch>& batch){
        _other_scene->render(batch);
        _scene->render(batch);
    }
    
