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
#include <Renderers2/OpenGL/ShadowMap.h>
#include <Resources2/OpenGL/FXAAShader.h>
#include <Resources2/ShaderResource.h>
#include <Display2/Canvas2D.h>
#include <Display2/Canvas3D.h>
#include <Display2/FadeCanvas.h>
#include <Display2/CompositeCanvas.h>
#include <Display2/SplitStereoCanvas.h>
#include <Display2/ColorStereoCanvas.h>

#include <Scene/AnimationNode.h>
#include <Scene/SearchTool.h>
#include <Animations/Animator.h>

#include <Math/Math.h>
#include <Utils/BetterMoveHandler.h>

#include <Logging/ColorStreamLogger.h>

#include <Utils/FPSSurface.h>


#include <Resources/Cubemap.h>

using OpenEngine::Renderers2::OpenGL::GLRenderer;
using OpenEngine::Renderers2::OpenGL::GLContext;
using OpenEngine::Resources2::OpenGL::FXAAShader;
using OpenEngine::Resources2::ShaderResource;
using OpenEngine::Resources2::ShaderResourcePtr;
using OpenEngine::Resources2::ShaderResourcePlugin;
using OpenEngine::Display2::Canvas3D;
using OpenEngine::Display2::Canvas2D;
using OpenEngine::Display2::FadeCanvas;
using OpenEngine::Display2::CompositeCanvas;
using OpenEngine::Display2::SplitStereoCanvas;
using OpenEngine::Display2::ColorStereoCanvas;
using OpenEngine::Display2::StereoCamera;

using namespace OpenEngine::Logging;
using namespace OpenEngine::Core;
using namespace OpenEngine::Utils;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Display;
using namespace OpenEngine::Resources;
using namespace OpenEngine::Renderers2;
using namespace OpenEngine::Renderers2::OpenGL;
using namespace OpenEngine::Animations;

class Rotator: public IListener<OpenEngine::Core::ProcessEventArg> {
private:
    TransformationNode* node;
public:
    bool active;
    Rotator(TransformationNode* node): node(node), active(false) {}
    virtual ~Rotator() {};

    void Handle(OpenEngine::Core::ProcessEventArg arg) {
        if (!active || !node) return;
        float dt = float(arg.approx) * 1e-6;
        node->Rotate(0.0, dt * .5, 0.0);
    }
};

class CustomHandler : public IListener<KeyboardEventArg> {
private:
    FXAAShader* fxaa;
    GLContext* ctx;
    IFrame& frame;
    GLRenderer* r;
    OpenEngine::Display2::ICanvas *c1, *c2, *c3;
    StereoCamera* cam;
    vector<Animator*> animators;

    Rotator& rotator;

    void Play(unsigned int i) {
        if (i < animators.size()) {
            if (animators[i]->IsPlaying()) {
                logger.info << "Pausing animation " << i << logger.end;
                animators[i]->Pause();
            }
            else {
                logger.info << "Playing animation " << i << logger.end;
                animators[i]->Play();
            }
        }
        else logger.info << "No animation at place " << i << logger.end;

    }
public:
    CustomHandler(FXAAShader* fxaa, 
                  GLContext* ctx, 
                  IFrame& frame, 
                  GLRenderer* r, 
                  OpenEngine::Display2::ICanvas* c1, 
                  OpenEngine::Display2::ICanvas* c2, 
                  OpenEngine::Display2::ICanvas* c3,
                  StereoCamera* cam,
                  vector<Animator*> animators,
                  Rotator& rotator) 
  : fxaa(fxaa)
  , ctx(ctx)
  , frame(frame)
  , r(r)
  , c1(c1)
  , c2(c2)
  , c3(c3)
  , cam(cam)
  , animators(animators)
  , rotator(rotator) { }
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
                break;
            case KEY_F1:
                Play(0);
                break;
            case KEY_F2:
                Play(1);
                break;
            case KEY_F3:
                Play(2);
                break;
            case KEY_F10:
                r->SetCanvas(c1);
                logger.info << "No stereo." << logger.end; 
               break;
            case KEY_F11:
                r->SetCanvas(c2);
                logger.info << "Split screen stereo." << logger.end; 
                break;
            case KEY_F12:
                r->SetCanvas(c3);
                logger.info << "Red/Blue color stereo." << logger.end; 
                break;
            case KEY_KP_PLUS:
                cam->SetEyeDistance(cam->GetEyeDistance() + 0.1);
                logger.info << "Eye distance " << cam->GetEyeDistance() << "." <<logger.end; 
                break;
            case KEY_KP_MINUS:
                cam->SetEyeDistance(cam->GetEyeDistance() - 0.1);
                logger.info << "Eye distance " << cam->GetEyeDistance() << "." <<logger.end; 
                break;
            case KEY_r:
                rotator.active = !rotator.active;
                break;
          default:break;
            } 
        }
    }

};


int main(int argc, char** argv) {
    int width = 800;
    int height = 600;

    bool fullscreen = false;
    bool docubemap = true;
    vector<string> files;

    for (int i=1;i<argc;i++) {
        if (strcmp(argv[i],"-res") == 0) {
            if (i + 2 < argc) {
                width = strtol(argv[i+1], NULL, 10);
                height = strtol(argv[i+2], NULL, 10);
                i += 2;
            }
        }
        else if (strcmp(argv[i],"-fullscreen") == 0) {
            fullscreen = true;
        }
        else if (strcmp(argv[i],"-nocubemap") == 0) {
            docubemap = false;
        }
        else {
            files.push_back(string(argv[i]));
        }
    }

    Logger::AddLogger(new ColorStreamLogger(&std::cout));
    
    DirectoryManager::AppendPath("projects/ColladaLoader/data/");
    DirectoryManager::AppendPath("resources/");

    ResourceManager<IModelResource>::AddPlugin(new AssimpPlugin()); 
    ResourceManager<ITextureResource>::AddPlugin(new FreeImagePlugin());

    Engine* engine = new Engine();
    IEnvironment* env = new SDLEnvironment(width,height);
    engine->InitializeEvent().Attach(*env);
    engine->ProcessEvent().Attach(*env);
    engine->DeinitializeEvent().Attach(*env);    

    ShaderResourcePlugin* shaderPlugin = new ShaderResourcePlugin();
    ResourceManager<ShaderResource>::AddPlugin(shaderPlugin);
    engine->ProcessEvent().Attach(*shaderPlugin);

    IFrame& frame  = env->CreateFrame();
    IMouse* mouse  = env->GetMouse();
    IKeyboard* keyboard = env->GetKeyboard();

    if (fullscreen) frame.ToggleOption(FRAME_FULLSCREEN);

    StereoCamera* stereoCam = new StereoCamera();
    Camera* cam = new Camera(*stereoCam);

    cam->SetPosition(Vector<3,float>(10,10,10));
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

    ShadowMap* shadowmap = new ShadowMap(width, height);
    r->InitializeEvent().Attach(*shadowmap);
    r->PostProcessEvent().Attach(*shadowmap);
    IViewingVolume* shadowView = new PerspectiveViewingVolume(1,300);
    Camera* shadowCam = new Camera(*(shadowView));
    shadowCam->SetPosition(Vector<3,float>(10,10,10));
    shadowCam->LookAt(Vector<3,float>(0,0,0));
    shadowmap->SetViewingVolume(shadowCam);

    FXAAShader* fxaa = new FXAAShader();
    r->InitializeEvent().Attach(*fxaa);
    r->PostProcessEvent().Attach(*fxaa);
    
    RenderStateNode* root = new RenderStateNode();
    SimpleRenderStateHandler* rsh = new SimpleRenderStateHandler(root);
    keyboard->KeyEvent().Attach(*rsh);
    
    FPSSurfacePtr fps = FPSSurface::Create();
    engine->ProcessEvent().Attach(*fps);

    RGBAColor bgc(0.5f, 0.5f, 0.5f, 1.0f);

    Canvas3D* canvas3D = new Canvas3D(width, height);
    canvas3D->SetScene(root);
    canvas3D->SetViewingVolume(cam);
    canvas3D->SetBackgroundColor(bgc);

    SplitStereoCanvas* sStereoCanvas = new SplitStereoCanvas(width, height, stereoCam, root);
    sStereoCanvas->SetBackgroundColor(bgc);

    ColorStereoCanvas* cStereoCanvas = new ColorStereoCanvas(width, height, stereoCam, root);
    cStereoCanvas->SetBackgroundColor(bgc);

    FadeCanvas* fadeCanvas = new FadeCanvas(width, height);
    engine->ProcessEvent().Attach(*fadeCanvas);

    CompositeCanvas* canvas = new CompositeCanvas(width, height);
    canvas->AddCanvas(canvas3D, 0, 0);
    // canvas->AddCanvas(cStereoCanvas, 0, 0);
    CompositeCanvas::Container& fpsc = canvas->AddCanvas(new Canvas2D(fps), 20, 20);
    fpsc.color = RGBColor(0.0, 0.20, 0.5);
    fpsc.opacity = 0.5;
    r->SetCanvas(fadeCanvas);
    fadeCanvas->FadeIn(canvas, 1.0f);
    //r->SetCanvas(canvas3D);
    //r->SetCanvas(stereoCanvas);

    root->EnableOption(RenderStateNode::TEXTURE);
    //root->EnableOption(RenderStateNode::WIREFRAME);
    root->DisableOption(RenderStateNode::BACKFACE);
    //root->DisableOption(RenderStateNode::SHADER);
    root->EnableOption(RenderStateNode::LIGHTING);
    //root->DisableOption(RenderStateNode::LIGHTING);
    // root->DisableOption(RenderStateNode::COLOR_MATERIAL);


    TransformationNode* lt = new TransformationNode();
    lt->Move(10, 10, 10);
    //lt->Rotate(-45, 0, 45);
    PointLightNode* l = new PointLightNode();
    l->constAtt = 1.0;
    //DirectionalLightNode* l = new DirectionalLightNode();
    //l->ambient = Vector<4,float>(0.5);//(0.2, 0.2, 0.3, 1.0) * 2;
    lt->AddNode(l);
    root->AddNode(lt);
    TransformationNode* scale = new TransformationNode();
    //scale->SetScale(Vector<3,float>(200,200,200));
    root->AddNode(scale);

    if (files.empty()) files.push_back("leopardshark/models/lepord shark.dae");

    Rotator rotator(scale);
    engine->ProcessEvent().Attach(rotator);
        

    SearchTool st;
    vector<Animator*> animators;

    // cubemap setup BEGIN

    ICubemapPtr cubemap;

    if (docubemap) {
        cubemap = Cubemap::Create(2048, RGBA, true);

        ITexture2DPtr negx = ResourceManager<ITexture2D>::Create("SaintLazarusChurch/negx.jpg");
        negx->Load();
        cubemap->SetPixels(negx, ICubemap::NEGATIVE_X);
        ITexture2DPtr posx = ResourceManager<ITexture2D>::Create("SaintLazarusChurch/posx.jpg");
        posx->Load();
        cubemap->SetPixels(posx, ICubemap::POSITIVE_X);

        ITexture2DPtr negy = ResourceManager<ITexture2D>::Create("SaintLazarusChurch/posy.jpg");
        negy->Load();
        cubemap->SetPixels(negy, ICubemap::NEGATIVE_Y);
        ITexture2DPtr posy = ResourceManager<ITexture2D>::Create("SaintLazarusChurch/negy.jpg");
        posy->Load();
        cubemap->SetPixels(posy, ICubemap::POSITIVE_Y);

        ITexture2DPtr negz = ResourceManager<ITexture2D>::Create("SaintLazarusChurch/negz.jpg");
        negz->Load();
        cubemap->SetPixels(negz, ICubemap::NEGATIVE_Z);
        ITexture2DPtr posz = ResourceManager<ITexture2D>::Create("SaintLazarusChurch/posz.jpg");
        posz->Load();
        cubemap->SetPixels(posz, ICubemap::POSITIVE_Z);

        ICubemap::GenerateMipmaps(cubemap);

        canvas3D->SetSkybox(cubemap);
        sStereoCanvas->SetSkybox(cubemap);
        cStereoCanvas->SetSkybox(cubemap);
    }
    // cubemap setup END



    for (unsigned int i = 0; i < files.size(); ++i) {
        try {
            IModelResourcePtr resource = ResourceManager<IModelResource>::Create(files[i]);
            
            ISceneNode* node;
            resource->Load();
            node = resource->GetSceneNode();
            resource->Unload();
            if (node) {
                list<MeshNode*> meshes = st.DescendantMeshNodes(node);
                list<MeshNode*>::iterator it = meshes.begin();
                for (; it != meshes.end(); ++it) {
                    MaterialPtr mat = (*it)->GetMesh()->GetMaterial();
                    if (docubemap)
                        mat->AddTexture(cubemap, "cubemap");
                }

                AnimationNode* anim = st.DescendantAnimationNode(node);
                if (anim)  {
                    Animator* animator = new Animator(anim);
                    scale->AddNode(animator->GetSceneNode());
                    animators.push_back(animator);
                    engine->ProcessEvent().Attach(*animator);
                    animator->SetActiveAnimation(0);
                }
                else scale->AddNode(node);

            }
            else logger.warning << "File: " << files[i] << " not loaded." << logger.end;
        }
        catch (ResourceException e) {
            logger.warning << "File: " << files[i] << ". " << e.what() << logger.end;
        }
    }

    
    CustomHandler* ch = new CustomHandler(fxaa, ctx, frame, r, canvas, sStereoCanvas, cStereoCanvas, stereoCam, animators, rotator);
    keyboard->KeyEvent().Attach(*ch);


    
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
