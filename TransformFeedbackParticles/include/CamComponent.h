//
//  CamComponent.h
//  AudioVertexDisplacement
//
//  Created by Arris Ray on 5/25/15.
//
//

#ifndef AudioVertexDisplacement_CamComponent_h
#define AudioVertexDisplacement_CamComponent_h

#include "IComponent.h"
#include "cinder/app/App.h"
#include "cinder/CameraUi.h"

using namespace ci;
using namespace ci::app;

class CamComponent : public IComponent
{
public:
    CamComponent( App * app ) : mApp( app ) { }
    virtual void setup();
    virtual void mouseDown( MouseEvent event );
    virtual void mouseDrag( MouseEvent event );
    virtual void keyDown( KeyEvent event ) {}
    virtual void update() {}
    virtual void draw();
    virtual void resize();
    
private:
    App * mApp;
    CameraPersp mCam;
    CameraUi mCamUi;
};

void CamComponent::setup()
{
    this->mCam.setPerspective( 45.0, this->mApp->getWindowAspectRatio(), 0.1f, 10000.0f );
    // this->mCam.setEyePoint( Vec3f( 0.0f, 0.0f, 240.0f ) );
    // this->mCam.setCenterOfInterestPoint( Vec3f( 0.0f, 0.0f, 0.0f ) );
    this->mCamUi = CameraUi( &mCam, this->mApp->getWindow() );
}

void CamComponent::mouseDown( MouseEvent event )
{
    this->mCamUi.mouseDown( event.getPos() );
}

void CamComponent::mouseDrag( MouseEvent event )
{
    this->mCamUi.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void CamComponent::draw()
{
    gl::setMatrices( this->mCam );
}

void CamComponent::resize()
{
    /*
    this->mCam = this->mMayaCam.getCamera();
    this->mCam.setAspectRatio( this->mApp->getWindowAspectRatio() );
    this->mMayaCam.setCurrentCam( this->mCam );
     */
}

#endif
