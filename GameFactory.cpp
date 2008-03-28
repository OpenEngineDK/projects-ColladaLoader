// Game Factory for an example project.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

// Class header
#include "GameFactory.h"

// OpenEngine library
#include <Display/Viewport.h>
#include <Display/ViewingVolume.h>
#include <Display/SDLFrame.h>
#include <Devices/SDLInput.h>
#include <Renderers/OpenGL/Renderer.h>
#include <Renderers/OpenGL/RenderingView.h>
#include <Scene/SceneNode.h>
#include <Utils/Statistics.h>
#include <Resources/ResourceManager.h>
#include <Geometry/FaceSet.h>
#include <Scene/GeometryNode.h>
#include <Scene/TransformationNode.h>
#include <Resources/OBJResource.h>
#include <Resources/TGAResource.h>
#include <Display/Camera.h>

// From extensions
#include <Resources/ColladaResource.h>
#include <Utils/MoveHandler.h>
#include <Utils/QuitHandler.h>

// Additional namespaces (others are in the header).
using namespace OpenEngine::Devices;
using namespace OpenEngine::Renderers::OpenGL;
using namespace OpenEngine::Resources;
using namespace OpenEngine::Utils;
using namespace OpenEngine::Display;
/**
 * Game quit handler.
 *
 * This is an example of a keyboard listener.
 * Every time the a key is released a key (UP) event is sent to the
 * handle method. If the key symbol is escape it quits the game.
 *
 * Note that this will not work before you have implemented the
 * keyboard and mouse interfaces in SDLInput.
 *
 * The code to register the handler is found in the
 * GameFactory::SetupEngine method.
 */
class QuitEventHandler {
public:
    void HandleQuit(KeyboardEventArg arg) {
        if (arg.sym == KEY_ESCAPE) {
            IGameEngine::Instance().Stop();
        }
    }
};

/**
 * Factory constructor.
 *
 * Initializes the different components so they are accessible when
 * the setup method is executed.
 */
GameFactory::GameFactory() {

    // Create a frame and viewport.
    this->frame = new SDLFrame(800, 600, 32);

    // Main viewport
    Viewport* viewport = new Viewport(*frame);

    // Bind the camera to the viewport
    camera = new Camera(*(new ViewingVolume()));
    viewport->SetViewingVolume(camera);

    // Create a renderer.
    this->renderer = new Renderer();

    // Add a rendering view to the renderer
    this->renderer->AddRenderingView(new RenderingView(*viewport));

    
    // Create scene root
    SceneNode* root = new SceneNode();
    this->renderer->SetSceneRoot(root);

    // Add models from models.txt to the scene
    // First we set the resources directory
    string resourcedir = "projects/ColladaLoader/data/";
    ResourceManager::AppendPath(resourcedir);
    logger.info << "Resource directory: " << resourcedir << logger.end;

    // load the resource plug-ins
	ResourceManager::AddModelPlugin(new ColladaPlugin());
    ResourceManager::AddModelPlugin(new OBJPlugin());
    ResourceManager::AddTexturePlugin(new TGAPlugin());

    IModelResourcePtr resource = ResourceManager::CreateModel("Tank.dae");
    //IModelResourcePtr resource = ResourceManager::CreateModel("model.obj");
    resource->Load();
    FaceSet* fs = resource->GetFaceSet();
    resource->Unload();

    GeometryNode* gn = new GeometryNode(fs);

    TransformationNode* tn = new TransformationNode();
    
    tn->AddNode(gn);
    root->AddNode(tn);
}

/**
 * Setup handler.
 *
 * This is the main setup method of the game factory. Here you can add
 * any non-standard modules and perform other setup tasks prior to the
 * engine start up.
 *
 * @param engine The game engine instance.
 */
bool GameFactory::SetupEngine(IGameEngine& engine) {

    // Add your mouse and keyboard module here
    SDLInput* input = new SDLInput();
    engine.AddModule(*input);

    // Register the handler as a listener on up and down keyboard events.
    MoveHandler* move_h = new MoveHandler(*camera);
	move_h->RegisterWithEngine(engine);
    QuitHandler* quit_h = new QuitHandler();
	quit_h->BindToEventSystem();

    // Add some module
    engine.AddModule(*(new OpenEngine::Utils::Statistics(1000)));

    // Return true to signal success.
    return true;

}

// Other factory methods. The returned objects are all created in the
// factory constructor.
IFrame*      GameFactory::GetFrame()    { return this->frame;    }
IRenderer*   GameFactory::GetRenderer() { return this->renderer; }