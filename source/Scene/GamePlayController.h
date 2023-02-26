#ifndef __GAME_CONTROLLER_H__
#define __GAME_CONTROLLER_H__
#include <random>
#include <climits>

#include <cugl/cugl.h>
using namespace cugl;
using namespace std;
// Uncomment to activate (but comment out MVC)
#include <Path/PathController.h>
#include <Character/CharacterController.h>
#include <Tilemap/TilemapController.h>
#include <Input/InputController.h>

class GamePlayController {
#pragma mark Internal References
private:
    /** The Game scene */
    std::shared_ptr<cugl::Scene2> _scene;

    /** The current tile map template (for regeneration) */
    int _template;
    
#pragma mark External References
public:
    /** The tilemap to procedurally generate */

    std::unique_ptr<CharacterController> _character;
    std::unique_ptr<TilemapController> _tilemap1;
    std::unique_ptr<TilemapController> _tilemap2;

    // std::unique_ptr<PathController> _path;
    std::shared_ptr<InputController> _input = InputController::getInstance();
    vector<float> path_trace;
    
#pragma mark Main Methods
public:
    /**
     * Creates the game controller.
     *
     * This constructor will procedurally generate a tilemap immediately
     * on creation.
     *
     * @param displaySize   The display size of the game window
     * @param randoms        Reference to the random number generator
     */
    // GamePlayController(const Size displaySize, const std::shared_ptr<std::mt19937>& randoms);
    GamePlayController(const Size displaySize);
    /**
     * Responds to the keyboard commands.
     *
     * This method allows us to regenerate the procedurally generated tilemap
     * upon command.
     *
     * @param dt  The amount of time (in seconds) since the last frame
     */
    void update(float dt);
    
    /**
     * Renders the game elements using the`batch.
     *
     * @param batch The SpriteBatch used to render this scene
     */

    void render(std::shared_ptr<SpriteBatch> batch) { _scene->render(batch); }

#pragma mark Generation Helpers
private:
    /** Generates primary world with guards. */
    void generatePrimaryWorld(std::unique_ptr<TilemapController>& tilemap);

    /** Generates secondary world without guards. */
    void generateSecondaryWorld(std::unique_ptr<TilemapController>& tilemap);
    

#pragma mark Helpers
private:

    /**
     * Creates a new tile map with the given template number
     *
     * @param choice    The template number
     */
    void generateTemplate(int choice);
    
    /**
     * Executes a function with debugging information.
     *
     * This function runs function `name` wrapped in `wrapper` and will call
     * CULog twice. The information from CLog will indicate
     *
     * - when the function starts
     * - how long it took to execute
     *
     * @param name      The name of the wrapped function
     * @param wrapper   The function wrapper to execute
     */
    void printExecution(std::string name, std::function<void()> wrapper);
};

#endif /* __GAME_CONTROLLER_H__ */
