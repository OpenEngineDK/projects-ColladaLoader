// Handler to manipulate transformation node
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------
#ifndef _TRANSFORMATION_HANDLER_
#define _TRANSFORMATION_HANDLER_

#include <Devices/IKeyboard.h>
#include <Scene/TransformationNode.h>
#include <Core/IGameEngine.h>

namespace OpenEngine {
namespace Utils {

using namespace OpenEngine::Devices;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Core;

class TransformationHandler : public IModule, public IListener<KeyboardEventArg> {
private:
    bool up, down, left, right, b, n, m, mod;
    float step;
    TransformationNode* tn;
public:
    TransformationHandler(TransformationNode* tn) 
        : up(false), down(false), left(false), right(false), 
          b(false), n(false), m(false), tn(tn) {}

    bool IsTypeOf(const std::type_info& inf) { return typeid(TransformationHandler) == inf; }
    void Initialize() {
        step = 0.0f;
    }
 
    void Deinitialize() {}
    
    void Process(const float deltaTime, const float percent) {
        float delta = deltaTime / 1000;
        
        int speed = 20;
        // Forward 
        if( up ){

            tn->Move(delta*speed,0.0,0.0);
        }
        if( down ){
            tn->Move(-delta*speed,0.0,0.0);
        }
        if( left ){
            tn->Move(0.0,0.0,delta*speed);
        }
        if( right ) {
            tn->Move(0.0,0.0,-delta*speed);
        }
        if( b ) {
            tn->Rotate(delta*speed,0.0,0.0);
        }
        if( n ) {
            tn->Rotate(0.0,delta*speed,0.0);
        }
        if( m ) {
            tn->Rotate(0.0,0.0,delta*speed);
        }
    }

    void KeyDown(KeyboardEventArg arg) {
        switch ( arg.sym ) {
        // Move the car forward
        case KEY_UP:    up    = true; break;
        case KEY_DOWN:  down  = true; break;
        case KEY_LEFT:  left  = true; break;
        case KEY_RIGHT: right = true; break;
        case 'b':       b     = true; break; 
        case 'n':       n     = true; break; 
        case 'm':       m     = true; break; 
        default: break;
        }
    }

    void KeyUp(KeyboardEventArg arg) {
        switch ( arg.sym ) {
        case KEY_UP:    up    = false; break;
        case KEY_DOWN:  down  = false; break;
        case KEY_LEFT:  left  = false; break;
        case KEY_RIGHT: right = false; break;
        case 'b':       b     = false; break;
        case 'n':       n     = false; break;
        case 'm':       m     = false; break;
        default: break;
        }
    }
    

    void Handle(KeyboardEventArg arg) {
        bool state = (arg.type == KeyboardEventArg::PRESS);
        switch ( arg.sym ) {
        // Move the car forward
        case KEY_UP:    up    = state; break;
        case KEY_DOWN:  down  = state; break;
        case KEY_LEFT:  left  = state; break;
        case KEY_RIGHT: right = state; break;
        case 'b':       b     = state; break; 
        case 'n':       n     = state; break; 
        case 'm':       m     = state; break; 
        default: break;
        }

    }
    
    void BindToEventSystem() {
        IKeyboard::keyEvent.Attach(*this);
    }

};

}
}
#endif
