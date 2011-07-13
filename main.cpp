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
#include <Utils/SimpleRenderStateHandler.h>
#include <Core/Engine.h>
#include <Display/SDLEnvironment.h>
#include <Display/Viewport.h>
#include <Display/PerspectiveViewingVolume.h>
#include <Display/Camera.h>

// Resources
#include <Resources/ResourceManager.h>
#include <Resources/AssimpResource.h>
#include <Resources/FreeImage.h>

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
#include <Resources2/OpenGL/FXAAShader.h>
#include <Display2/Canvas3D.h>

#include <Math/Math.h>
#include <Utils/BetterMoveHandler.h>

#include <Logging/ColorStreamLogger.h>

using OpenEngine::Renderers2::OpenGL::GLRenderer;
using OpenEngine::Renderers2::OpenGL::GLContext;
using OpenEngine::Resources2::OpenGL::FXAAShader;
using OpenEngine::Display2::Canvas3D;

using namespace OpenEngine::Logging;
using namespace OpenEngine::Core;
using namespace OpenEngine::Utils;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Display;
using namespace OpenEngine::Resources;
using namespace OpenEngine::Renderers2;
using namespace OpenEngine::Renderers2::OpenGL;



class CustomHandler : public IListener<KeyboardEventArg> {
private:
    FXAAShader* fxaa;
    GLContext* ctx;
    IFrame& frame;
public:
    CustomHandler(FXAAShader* fxaa, GLContext* ctx, IFrame& frame) : fxaa(fxaa), ctx(ctx), frame(frame)  {
    }
    virtual ~CustomHandler() {}

    void Handle(KeyboardEventArg arg) {
        if (arg.type == EVENT_PRESS) {
            switch(arg.sym) {
            case KEY_0: fxaa->SetActive(!fxaa->GetActive()); break;
            case KEY_9: 
                ctx->ReleaseTextures(); 
                ctx->ReleaseVBOs(); 
                ctx->ReleaseShaders(); 
                logger.info << "Release textures, VBOs, and shaders." << logger.end; break;
            case KEY_ESCAPE: exit(0);
            case KEY_f:
                ctx->ReleaseTextures(); 
                ctx->ReleaseVBOs(); 
                ctx->ReleaseShaders();                 
                frame.ToggleOption(FRAME_FULLSCREEN);                
           default:break;
            } 
        }
    }

};


int main(int argc, char** argv) {

    Logger::AddLogger(new ColorStreamLogger(&std::cout));
    
    DirectoryManager::AppendPath("projects/ColladaLoader/data/");
    ResourceManager<IModelResource>::AddPlugin(new AssimpPlugin()); 
    ResourceManager<ITextureResource>::AddPlugin(new FreeImagePlugin());

    Engine* engine = new Engine();
    const int width = 800;
    const int height = 600;
    IEnvironment* env = new SDLEnvironment(width,height);
    engine->InitializeEvent().Attach(*env);
    engine->ProcessEvent().Attach(*env);
    engine->DeinitializeEvent().Attach(*env);    
    
    IFrame& frame  = env->CreateFrame();
    IMouse* mouse  = env->GetMouse();
    IKeyboard* keyboard = env->GetKeyboard();
    
    Camera* cam = new Camera(*(new PerspectiveViewingVolume()));

    cam->SetPosition(Vector<3,float>(100,100,100));
    cam->LookAt(Vector<3,float>(0,0,0));

    BetterMoveHandler* mh = new BetterMoveHandler(*cam, *mouse);
    engine->InitializeEvent().Attach(*mh);
    engine->ProcessEvent().Attach(*mh);
    engine->DeinitializeEvent().Attach(*mh);
    mouse->MouseMovedEvent().Attach(*mh);
    mouse->MouseButtonEvent().Attach(*mh);
    keyboard->KeyEvent().Attach(*mh);

    GLContext* ctx = new GLContext();
    GLRenderer* r = new GLRenderer(ctx);
    ((SDLFrame*)(&frame))->SetRenderModule(r);

    FXAAShader* fxaa = new FXAAShader();
    r->PostProcessEvent().Attach(*fxaa);
    CustomHandler* ch = new CustomHandler(fxaa, ctx, frame);
    keyboard->KeyEvent().Attach(*ch);
    
    RenderStateNode* root = new RenderStateNode();
    SimpleRenderStateHandler* rsh = new SimpleRenderStateHandler(root);
    keyboard->KeyEvent().Attach(*rsh);
    
    Canvas3D* canvas = new Canvas3D(width, height);
    canvas->SetScene(root);
    canvas->SetViewingVolume(cam);
    r->SetCanvas(canvas);

    root->EnableOption(RenderStateNode::TEXTURE);
    //root->EnableOption(RenderStateNode::WIREFRAME);
    root->DisableOption(RenderStateNode::BACKFACE);
    //root->DisableOption(RenderStateNode::SHADER);
    root->EnableOption(RenderStateNode::LIGHTING);
    //root->DisableOption(RenderStateNode::LIGHTING);
    // root->DisableOption(RenderStateNode::COLOR_MATERIAL);


    r->SetBackgroundColor(RGBAColor(0.5f, 0.5f, 0.5f, 1.0f));
    TransformationNode* lt = new TransformationNode();
    lt->Move(0, 1000, 1000);
    //lt->Rotate(-45, 0, 45);
    PointLightNode* l = new PointLightNode();
    l->constAtt = 1.0;
    //DirectionalLightNode* l = new DirectionalLightNode();
    //l->ambient = Vector<4,float>(0.5);//(0.2, 0.2, 0.3, 1.0) * 2;
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

    // Start the engine.
    engine->Start();

    // Return when the engine stops.
    return EXIT_SUCCESS;
}
