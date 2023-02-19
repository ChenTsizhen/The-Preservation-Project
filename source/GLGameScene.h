//
//  GLGameScene.h
//  Geometry Lab
//
//  This is the primary class file for running the game.  You should study this file
//  for ideas on how to structure your own root class. This time we are not going to
//  have any additional model classes
//
//  Author: Walker White
//  Version: 1/20/22
//
#ifndef __GL_GAME_SCENE_H__
#define __GL_GAME_SCENE_H__
#include <cugl/cugl.h>
#include <vector>
#include "GLInputController.h"

/**
 * This class is the primary gameplay constroller for the demo.
 *
 * A world has its own objects, assets, and input controller.  Thus this is
 * really a mini-GameEngine in its own right.  As in 3152, we separate it out
 * so that we can have a separate mode for the loading screen.
 */
class GameScene : public cugl::Scene2 {
protected:
    // CONTROLLERS are attached directly to the scene (no pointers)
    /** The controller to manage the ship */
    InputController _input;
    
    // MODELS should be shared pointers or a data structure of shared pointers
    /** The spline for the exterior circle */
    cugl::Spline2 _spline;
    /** The physics world to animate the falling */
    std::shared_ptr<cugl::physics2::ObstacleWorld> _world;
    /** The outside "circle" */
    std::shared_ptr<cugl::physics2::PolygonObstacle> _center;
    /** The falling star */
    std::shared_ptr<cugl::physics2::PolygonObstacle> _star;

    // PUT OTHER ATTRIBUTES HERE AS NECESSARY

    /**
     * Rebuilds the geometry.
     *
     * This method should recreate all the polygons for the spline, the handles
     * and the falling star. It should also recreate all physics objects.
     *
     * However, to cut down on performance overhead, this method should NOT add
     * those physics objects to the world inside this method (as this method is
     * called repeatedly while the user moves a handle). Instead, those objects
     * should not be activated until the state is "stable".
     */
    void buildGeometry();
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new game mode with the default values.
     *
     * This constructor does not allocate any objects or start the game.
     * This allows us to use the object without a heap pointer.
     */
    GameScene() : cugl::Scene2() {}
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~GameScene() { dispose(); }
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    void dispose() override;
    
    /**
     * Initializes the controller contents, and starts the game
     *
     * The constructor does not allocate any objects or memory.  This allows
     * us to have a non-pointer reference to this controller, reducing our
     * memory allocation.  Instead, allocation happens in this method.
     *
     * @param assets    The (loaded) assets for this game mode
     *
     * @return true if the controller is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);

    
#pragma mark -
#pragma mark Gameplay Handling
    /**
     * The method called to update the game mode.
     *
     * This method contains any gameplay code that is not an OpenGL call.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void update(float timestep) override;

    /**
     * Draws all this scene to the given SpriteBatch.
     *
     * The default implementation of this method simply draws the scene graph
     * to the sprite batch.  By overriding it, you can do custom drawing
     * in its place.
     *
     * @param batch     The SpriteBatch to draw with.
     */
    void render(const std::shared_ptr<cugl::SpriteBatch>& batch) override;
};

#endif /* __SG_GAME_SCENE_H__ */
