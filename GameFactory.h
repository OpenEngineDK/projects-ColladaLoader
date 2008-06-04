// Game Factory for an example project.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

// header define to avoid including definitions several times
#ifndef _EXAMPLE_PROJECT_GAME_FACTORY_H_
#define _EXAMPLE_PROJECT_GAME_FACTORY_H_

// Game engine interface
#include <Core/IGameFactory.h>
#include <Scene/ISceneNode.h>

// forward declarations
namespace OpenEngine {
    namespace Scene {
        class TransformationNode;
    }

    namespace Display {
        class Camera;
    }
}

// Namespaces
using namespace OpenEngine::Core;
using namespace OpenEngine::Display;
using namespace OpenEngine::Renderers;
using namespace OpenEngine::Display;
using namespace OpenEngine::Scene;

/**
 * Game factory definition.
 *
 * The factory is implemented in GameFactory.cpp
 * Documentation on game factories can be found in Core/IGameFactory.h
 * 
 * @see IGameFactory
 */
class GameFactory : public IGameFactory {

private:
    IFrame*      frame;         // member variable for the engine frame
    IRenderer*   renderer;      // member variable for the engine renderer

    Camera* camera;
    TransformationNode* lt;
public:
    GameFactory();
    bool         SetupEngine(IGameEngine& engine);
    IFrame*      GetFrame();
    IRenderer*   GetRenderer();

};

#endif // _EXAMPLE_PROJECT_GAME_FACTORY_H_
