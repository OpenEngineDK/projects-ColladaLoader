// main
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

// OpenEngine stuff
#include <Meta/Config.h>
#include <Logging/Logger.h>
#include <Logging/StreamLogger.h>
#include <Core/Engine.h>
#include <Renderers/OpenGL/RenderingView.h>
#include <Display/Camera.h>
#include <Devices/IKeyboard.h>
#include <Devices/IMouse.h>

// SimpleSetup
#include <Utils/SimpleSetup.h>
#include <Display/SDLEnvironment.h>
#include <Display/Viewport.h>
#include <Display/ViewingVolume.h>

// Resources
#include <Resources/ResourceManager.h>
#include <Resources/ColladaResource.h>

// camera tool
#include <Utils/MouseSelection.h>
#include <Utils/CameraTool.h>

using namespace OpenEngine::Logging;
using namespace OpenEngine::Core;
using namespace OpenEngine::Utils;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Display;
using namespace OpenEngine::Renderers;
using namespace OpenEngine::Renderers::OpenGL;

int main(int argc, char** argv) {
    int width = 800;
    int height = 600;
    // Create Viewport and renderingview
    IEnvironment* env = new SDLEnvironment(width,height);
    Viewport* vp = new Viewport(env->GetFrame());
    IRenderingView* rv = new RenderingView(*vp);
    // Create simple setup
    SimpleSetup* setup = new SimpleSetup("Collada Loader", vp, env, rv);

    string resourcedir = "projects/ColladaLoader/data/";
    DirectoryManager::AppendPath(resourcedir);
    ResourceManager<IModelResource>::AddPlugin(new ColladaPlugin());

    ISceneNode* root = setup->GetScene();

    IModelResourcePtr resource = ResourceManager<IModelResource>::Create("rock.dae");
    resource->Load();
    ISceneNode* node = resource->GetSceneNode();
    resource->Unload();

    root->AddNode(node);


    // camera tool setup
    MouseSelection* ms =
        new MouseSelection(env->GetFrame(),
                           setup->GetMouse(),
                           NULL);
    CameraTool* ct = new CameraTool(false);
    ms->BindTool(vp, ct);

    // attach the mouse tool!
    setup->GetKeyboard().KeyEvent().Attach(*ms);
    setup->GetMouse().MouseMovedEvent().Attach(*ms);
    setup->GetMouse().MouseButtonEvent().Attach(*ms);
    setup->GetRenderer().PostProcessEvent().Attach(*ms);

    // FPS
    setup->ShowFPS();

    // Start the engine.
    setup->GetEngine().Start();

    // Return when the engine stops.
    return EXIT_SUCCESS;
}
