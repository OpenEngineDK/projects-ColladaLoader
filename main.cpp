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
#include <Scene/RenderStateNode.h>

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

#include <Renderers/TextureLoader.h>

using namespace OpenEngine::Logging;
using namespace OpenEngine::Core;
using namespace OpenEngine::Utils;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Display;
using namespace OpenEngine::Renderers;
using namespace OpenEngine::Renderers::OpenGL;

int main(int argc, char** argv) {
    
    ResourceManager<IModelResource>::AddPlugin(new ColladaPlugin());
    OpenEngine::Renderers::TextureLoader* tl;

    int width = 800;
    int height = 600;
    // Create Viewport and renderingview
    IEnvironment* env = new SDLEnvironment(width,height);
    Viewport* vp = new Viewport(env->GetFrame());
    IRenderingView* rv = new RenderingView(*vp);
    // Create simple setup
    SimpleSetup* setup = new SimpleSetup("ColladaLoader", vp, env, rv);
    
    setup->GetCamera()->SetPosition(Vector<3,float>(100,100,100));
    setup->GetCamera()->LookAt(Vector<3,float>(0,0,0));
    setup->GetRenderer().SetBackgroundColor(Vector<4,float>(.3,.3,.3,1.0));
    setup->AddDataDirectory("projects/ColladaLoader/data/");

    tl = new OpenEngine::Renderers::TextureLoader(setup->GetRenderer());
    setup->GetRenderer().InitializeEvent().Attach(*tl);

    RenderStateNode* root = new RenderStateNode();
    root->EnableOption(RenderStateNode::TEXTURE);
    root->EnableOption(RenderStateNode::BACKFACE);
    root->DisableOption(RenderStateNode::SHADER);
    root->DisableOption(RenderStateNode::LIGHTING);
    root->DisableOption(RenderStateNode::COLOR_MATERIAL);
    setup->GetScene()->AddNode(root);

    // string file = "missile/missile.dae";
    string file = "leopardshark/models/lepord shark.dae";
    if (argc > 1) file = string(argv[1]);
    IModelResourcePtr resource = ResourceManager<IModelResource>::Create(file);
    //IModelResourcePtr resource = ResourceManager<IModelResource>::Create("fish/models/clownfish.dae");
    //IModelResourcePtr resource = ResourceManager<IModelResource>::Create("Podium/Podium001.dae");
    //IModelResourcePtr resource = ResourceManager<IModelResource>::Create("missile/missile.dae");

    ISceneNode* node;
    resource->Load();
    node = resource->GetSceneNode();
    resource->Unload();
    root->AddNode(node);

    // IModelResourcePtr resource2 = ResourceManager<IModelResource>::Create("Dragon/DragonHead.obj");
    // resource2->Load();
    // node = resource2->GetSceneNode();
    // resource2->Unload();
    // root->AddNode(node);

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

    tl->Load(*root);

    // FPS
    setup->ShowFPS();

    // Start the engine.
    setup->GetEngine().Start();

    // Return when the engine stops.
    return EXIT_SUCCESS;
}
