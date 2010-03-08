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
#include <Resources/AssimpResource.h>
// camera tool
#include <Utils/MouseSelection.h>
#include <Utils/CameraTool.h>

#include <Renderers/TextureLoader.h>

#include <Renderers/OpenGL/AnimationRenderingView.h>
#include <Scene/TransformationNode.h>
#include <Scene/AnimationNode.h>
#include <Animations/Skeleton.h>
#include <Animations/AnimationController.h>
#include <Animations/Animation.h>
#include <Animations/KeyFrame.h>

#include <Scene/PointLightNode.h>
#include <Scene/DirectionalLightNode.h>

#include <Renderers/BufferObjectBinder.h>

using namespace OpenEngine::Logging;
using namespace OpenEngine::Core;
using namespace OpenEngine::Utils;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Display;
using namespace OpenEngine::Renderers;
using namespace OpenEngine::Renderers::OpenGL;
using namespace OpenEngine::Animations;

class ExtRenderingView
    : public RenderingView
    , public AnimationRenderingView {
public:
    ExtRenderingView(Viewport& viewport)
        : IRenderingView(viewport)
        , RenderingView(viewport)
        , AnimationRenderingView(viewport) {}
};

int main(int argc, char** argv) {
    
    ResourceManager<IModelResource>::AddPlugin(new ColladaPlugin());
    ResourceManager<IModelResource>::AddPlugin(new AssimpPlugin());
    OpenEngine::Renderers::TextureLoader* tl;

    int width = 800;
    int height = 600;
    // Create Viewport and renderingview
    IEnvironment* env = new SDLEnvironment(width,height);
    Viewport* vp = new Viewport(env->GetFrame());
    IRenderingView* rv = new ExtRenderingView(*vp);
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
    root->EnableOption(RenderStateNode::LIGHTING);
    // root->DisableOption(RenderStateNode::COLOR_MATERIAL);
    delete setup->GetScene();
    setup->SetScene(*root);

    TransformationNode* lt = new TransformationNode();
    lt->Move(0,500,0);
    PointLightNode* l = new PointLightNode();
    lt->AddNode(l);
    root->AddNode(lt);
    // string file = "missile/missile.dae";
    string file = "leopardshark/models/lepord shark.dae";
    if (argc > 1) {
        file = string(argv[1]);
        IModelResourcePtr resource = ResourceManager<IModelResource>::Create(file);

    //IModelResourcePtr resource = ResourceManager<IModelResource>::Create("fish/models/clownfish.dae");
    //IModelResourcePtr resource = ResourceManager<IModelResource>::Create("Podium/Podium001.dae");
    //IModelResourcePtr resource = ResourceManager<IModelResource>::Create("missile/missile.dae");

        
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
    // resource->Load();
    // node = resource->GetSceneNode();
    // resource->Unload();
    // if (node)
    //     scale->AddNode(node);
    }
    // IModelResourcePtr resource2 = ResourceManager<IModelResource>::Create("Dragon/DragonHead.obj");
    // resource2->Load();
    // node = resource2->GetSceneNode();
    // resource2->Unload();
    // root->AddNode(node);


    Skeleton* skel = new Skeleton();
    TransformationNode* joint0 = new TransformationNode();
    TransformationNode* joint1 = new TransformationNode();
    TransformationNode* joint2 = new TransformationNode();
    
    joint1->SetPosition(Vector<3,float>(0,20,0));
    joint0->SetPosition(Vector<3,float>(0,40,0));
    
    joint0->AddNode(joint1);
    joint1->AddNode(joint2);
    skel->AddJoints(joint0);

    AnimationController* ac = new AnimationController();
    setup->GetEngine().ProcessEvent().Attach(*ac);

    Animation* anim = new Animation();
    KeyFrame<TransformationNode> kf(Time(0,0));
    kf.AddTransformation(joint0, *joint0);
    kf.AddTransformation(joint1, *joint1);
    kf.AddTransformation(joint2, *joint2);
    anim->AddKeyFrame(kf);

    TransformationNode t(*joint0);
    t.Move(-30,0,-30);
    kf = KeyFrame<TransformationNode>(Time(1,0));
    kf.AddTransformation(joint0, t);
    kf.AddTransformation(joint1, *joint1);
    kf.AddTransformation(joint2, *joint2);
    anim->AddKeyFrame(kf);

    t = TransformationNode(*joint1);
    t.Move(0,30,30);
    kf = KeyFrame<TransformationNode>(Time(2,0));
    kf.AddTransformation(joint0, *joint0);
    kf.AddTransformation(joint1, t);
    kf.AddTransformation(joint2, *joint2);
    anim->AddKeyFrame(kf);

    kf = KeyFrame<TransformationNode>(Time(3,0));
    kf.AddTransformation(joint0, *joint0);
    kf.AddTransformation(joint1, *joint1);
    kf.AddTransformation(joint2, *joint2);
    anim->AddKeyFrame(kf);

    ac->AnimationEvent().Attach(*anim);

    AnimationNode* an = new AnimationNode(skel);
    //root->AddNode(an);

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
    BufferObjectBinder* bob = new BufferObjectBinder(setup->GetRenderer());
    setup->GetRenderer().InitializeEvent().Attach(*bob);

    // FPS
    setup->ShowFPS();

    // Start the engine.
    setup->GetEngine().Start();

    // Return when the engine stops.
    return EXIT_SUCCESS;
}
