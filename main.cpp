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
//#include <Resources/ColladaResource.h>
#include <Resources/AssimpResource.h>
// camera tool
#include <Utils/MouseSelection.h>
#include <Utils/CameraTool.h>

#include <Renderers/TextureLoader.h>
#include <Scene/TransformationNode.h>
#include <Scene/PointLightNode.h>
#include <Scene/DirectionalLightNode.h>
#include <Renderers/DataBlockBinder.h>

#include <Renderers2/OpenGL/GLRenderer.h>
#include <Renderers2/OpenGL/GLContext.h>
#include <Display2/Canvas3D.h>

#include <Math/Math.h>
#include <Utils/BetterMoveHandler.h>

using OpenEngine::Renderers2::OpenGL::GLRenderer;
using OpenEngine::Renderers2::OpenGL::GLContext;
using OpenEngine::Display2::Canvas3D;

using namespace OpenEngine::Logging;
using namespace OpenEngine::Core;
using namespace OpenEngine::Utils;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Display;
using namespace OpenEngine::Renderers;
using namespace OpenEngine::Renderers::OpenGL;

int main(int argc, char** argv) {
    
    ResourceManager<IModelResource>::AddPlugin(new AssimpPlugin());
    // OpenEngine::Renderers::TextureLoader* tl;

    const int width = 800;
    const int height = 600;
    IEnvironment* env = new SDLEnvironment(width,height);
    SimpleSetup* setup = new SimpleSetup("ColladaLoader", env);
    
    setup->GetCamera()->SetPosition(Vector<3,float>(100,100,100));
    setup->GetCamera()->LookAt(Vector<3,float>(0,0,0));
    setup->GetRenderer().SetBackgroundColor(Vector<4,float>(.3,.3,.3,1.0));
    setup->AddDataDirectory("projects/ColladaLoader/data/");


    BetterMoveHandler* mh = new BetterMoveHandler(*setup->GetCamera(), setup->GetMouse());
    setup->GetEngine().InitializeEvent().Attach(*mh);
    setup->GetEngine().ProcessEvent().Attach(*mh);
    setup->GetEngine().DeinitializeEvent().Attach(*mh);
    setup->GetMouse().MouseMovedEvent().Attach(*mh);
    setup->GetMouse().MouseButtonEvent().Attach(*mh);
    setup->GetKeyboard().KeyEvent().Attach(*mh);

    GLContext* ctx = new GLContext();
    GLRenderer* r = new GLRenderer(ctx);
    (*((SDLFrame*)&setup->GetFrame())).SetRenderModule(r);
    setup->GetFrame().SetCanvas(NULL);

    // tl = new OpenEngine::Renderers::TextureLoader(setup->GetRenderer());
    // setup->GetRenderer().InitializeEvent().Attach(*tl);

    RenderStateNode* root = new RenderStateNode();

    Canvas3D* canvas = new Canvas3D(width, height);
    canvas->SetScene(root);
    canvas->SetViewingVolume(setup->GetCamera());
    r->SetCanvas(canvas);

    root->EnableOption(RenderStateNode::TEXTURE);
    // root->EnableOption(RenderStateNode::WIREFRAME);
    root->DisableOption(RenderStateNode::BACKFACE);
    // root->DisableOption(RenderStateNode::SHADER);
    root->EnableOption(RenderStateNode::LIGHTING);
    // root->DisableOption(RenderStateNode::COLOR_MATERIAL);

    // delete setup->GetScene();
    // setup->SetScene(*root);

    TransformationNode* lt = new TransformationNode();
    lt->Move(0, 10000, 0);
    77lt->Rotate(OpenEngine::Math::PI * 1.25, 0, 0);
    PointLightNode* l = new PointLightNode();
    //DirectionalLightNode* l = new DirectionalLightNode();
    l->ambient = Vector<4,float>(1.0);
    lt->AddNode(l);
    root->AddNode(lt);
    // string file = "missile/missile.dae";
    string file = "leopardshark/models/lepord shark.dae";
    if (argc > 1) file = string(argv[1]);
    IModelResourcePtr resource = ResourceManager<IModelResource>::Create(file);
    TransformationNode* scale = new TransformationNode();
    //scale->SetScale(Vector<3,float>(200,200,200));
    root->AddNode(scale);
    ISceneNode* node;
    resource->Load();
    node = resource->GetSceneNode();
    resource->Unload();
    if (node)
        scale->AddNode(node);
    else logger.warning << "No scene Loaded." << logger.end;

    //root->AddNode(an);

    // camera tool setup
    // MouseSelection* ms =
    //     new MouseSelection(env->GetFrame(),
    //                        setup->GetMouse(),
    //                        NULL);
    // CameraTool* ct = new CameraTool(false);
    // ms->BindTool(vp, ct);

    // attach the mouse tool!
    // setup->GetKeyboard().KeyEvent().Attach(*ms);
    // setup->GetMouse().MouseMovedEvent().Attach(*ms);
    // setup->GetMouse().MouseButtonEvent().Attach(*ms);
    // setup->GetRenderer().PostProcessEvent().Attach(*ms);

    // tl->Load(*root);
    // DataBlockBinder* bob = new DataBlockBinder(setup->GetRenderer());
    // bob->Bind(*root);

    // FPS
    setup->ShowFPS();

    // Start the engine.
    setup->GetEngine().Start();

    // Return when the engine stops.
    return EXIT_SUCCESS;
}
