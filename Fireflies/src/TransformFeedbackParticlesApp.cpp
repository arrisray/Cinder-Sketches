#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "IComponent.h"
#include "AudioComponent.h"
#include "CamComponent.h"
#include "SceneComponent.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// NOISE GENERATION, e.g. Perlin Noise, fBm, etc.
//
// Google Code, "fractalterraingeneration: Fractional Brownian Motion" - https://code.google.com/p/fractalterraingeneration/wiki/Fractional_Brownian_Motion - Investigation and implementation of various PGC techniques.
// Google Code, "fractalterraingeneration: Simplex Noise" - https://code.google.com/p/fractalterraingeneration/wiki/Simplex_Noise
// Cinder Docs, "Enter Perlin Noise" - http://libcinder.org/docs/v0.8.4/hello_cinder_chapter4.html
// StackOverflow, "GLSL Noise" - http://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
// Catlike Coding, "Simplex Noise, keeping it simple" - http://catlikecoding.com/unity/tutorials/simplex-noise/ - Unity tutorial.

// PARTICLES
//
// null program, "A GPU Approach to Particle Physics" - http://nullprogram.com/blog/2014/06/29/ - Technical breakdown of techniques.
// Roblox, "Explore the Endless Possibilities of Custom Particles" - http://blog.roblox.com/2015/04/explore-the-endless-possibilities-of-custom-particles/ - List of particle system variables.

// FOURIER TRANSFORMS
//
// Float AudioProgramming, "DFT Spectral Analysis with OpenGL" - https://christianfloisand.wordpress.com/2013/06/11/dft-spectral-analysis-with-opengl - Apply DFT analysis to particle system attributes.
// jackschaedler.github.io, "SEEING CIRCLES, SINES, AND SIGNALS: A COMPACT PRIMER ON DIGITAL SIGNAL PROCESSING" - "http://jackschaedler.github.io/circles-sines-signals/
// Cinder Docs, "MonitorNode and MonitorSpectralNode" - http://libcinder.org/docs/dev/guide_audio.html
// https://dadorran.wordpress.com/2014/02/20/plotting-frequency-spectrum-using-matlab/
// http://gribblelab.org/scicomp/09_Signals_and_sampling.html#sec-2 - Good illustrated breakdown of sampling theory.

// BEAT DETECTION
//
// http://hans.fugal.net/blog/2009/04/02/dft-magnitudepower-spectra/
// http://archive.gamedev.net/archive/reference/programming/features/beatdetection/
// http://www.badlogicgames.com/wordpress/?cat=18
// http://mziccard.me/2015/05/28/beats-detection-algorithms-1/

// SCANLINES
//
// Soulwire, "Plasmatic Isosurface: WebGL / GLSL plasma simulation running on the GPU" - http://soulwire.github.io/Plasmatic-Isosurface/

class TransformFeedbackParticlesApp : public App
{
public:
    void setup();
    void mouseDown( MouseEvent event );
    void mouseDrag( MouseEvent event );
    void keyDown( KeyEvent event );
    void update();
    void draw();
    void resize();
    
private:
    std::shared_ptr<AudioComponent> mAudio;
    std::shared_ptr<CamComponent> mCam;
    std::shared_ptr<SceneComponent> mScene;
    std::vector< std::shared_ptr<IComponent> > mComponents;
};

void TransformFeedbackParticlesApp::setup()
{
    this->mAudio.reset( new AudioComponent() );
    this->mCam.reset( new CamComponent( this ) );
    this->mScene.reset( new SceneComponent( this ) );
    
    this->mComponents.push_back( this->mAudio );
    this->mComponents.push_back( this->mCam );
    this->mComponents.push_back( this->mScene );
    
    for( auto c : this->mComponents )
    {
        c->setup();
    }
}

void TransformFeedbackParticlesApp::keyDown( KeyEvent event )
{
    for( auto c : this->mComponents )
    {
        c->keyDown( event );
    }
}

void TransformFeedbackParticlesApp::mouseDown( MouseEvent event )
{
    for( auto c : this->mComponents )
    {
        c->mouseDown( event );
    }
}

void TransformFeedbackParticlesApp::mouseDrag( MouseEvent event )
{
    for( auto c : this->mComponents )
    {
        c->mouseDrag( event );
    }
}

void TransformFeedbackParticlesApp::update()
{
    this->mScene->setMagSpectrum( this->mAudio->getMagSpectrum() );
    this->mScene->setBeats( this->mAudio->getBeats() );
    this->mScene->setVolume( this->mAudio->getVolume() );
    
    for( auto c : this->mComponents )
    {
        c->update();
    }
}

void TransformFeedbackParticlesApp::draw()
{
    for( auto c : this->mComponents )
    {
        c->draw();
    }
}

void TransformFeedbackParticlesApp::resize()
{
    for( auto c : this->mComponents )
    {
        c->resize();
    }
}


CINDER_APP( TransformFeedbackParticlesApp, RendererGl, [] ( App::Settings *settings ) {
    settings->setWindowSize( 1280, 720 );
    settings->setMultiTouchEnabled( false );
} )
