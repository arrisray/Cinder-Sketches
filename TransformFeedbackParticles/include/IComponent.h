//
//  IComponent.h
//  AudioVertexDisplacement
//
//  Created by Arris Ray on 5/25/15.
//
//

#ifndef AudioVertexDisplacement_IComponent_h
#define AudioVertexDisplacement_IComponent_h

#include "cinder/app/App.h"
#include "cinder/app/KeyEvent.h"
#include "cinder/app/MouseEvent.h"
#include "cinder/app/TouchEvent.h"

using namespace ci;
using namespace ci::app;

class IComponent
{
public:
    virtual ~IComponent() {}
    
    //! Override to perform any application setup after the Renderer has been initialized.
    virtual void	setup() {}
    //! Override to perform any application cleanup before exiting.
    virtual void	shutdown() {}
    
    //! Override to perform any once-per-loop computation.
    virtual void	update() {}
    //! Override to perform any rendering once-per-loop or in response to OS-prompted requests for refreshes.
    virtual void	draw() {}
    
    //! Override to receive mouse-down events.
    virtual void	mouseDown( MouseEvent event ) {}
    //! Override to receive mouse-up events.
    virtual void	mouseUp( MouseEvent event ) {}
    //! Override to receive mouse-wheel events.
    virtual void	mouseWheel( MouseEvent event ) {}
    //! Override to receive mouse-move events.
    virtual void	mouseMove( MouseEvent event ) {}
    //! Override to receive mouse-drag events.
    virtual void	mouseDrag( MouseEvent event ) {}
    
    //! Override to respond to the beginning of a multitouch sequence
    virtual void	touchesBegan( TouchEvent event ) {}
    //! Override to respond to movement (drags) during a multitouch sequence
    virtual void	touchesMoved( TouchEvent event ) {}
    //! Override to respond to the end of a multitouch sequence
    virtual void	touchesEnded( TouchEvent event ) {}
    
    //! Override to receive key-down events.
    virtual void	keyDown( KeyEvent event ) {}
    //! Override to receive key-up events.
    virtual void	keyUp( KeyEvent event ) {}
    //! Override to receive window resize events.
    virtual void	resize() {}
    //! Override to receive file-drop events.
    virtual void	fileDrop( FileDropEvent event ) {}
    
    //! Quits the application gracefully
    virtual void	quit() {}
};

#endif
