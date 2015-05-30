//
//  AudioComponent.h
//  AudioVertexDisplacement
//
//  Created by Arris Ray on 5/25/15.
//
//

#ifndef AudioVertexDisplacement_AudioComponent_h
#define AudioVertexDisplacement_AudioComponent_h

#include "IComponent.h"
#include "cinder/audio/Context.h"
#include "cinder/audio/NodeEffects.h"
#include "cinder/audio/Source.h"
#include "cinder/audio/MonitorNode.h"
#include "cinder/audio/SamplePlayerNode.h"

using namespace ci;
using namespace ci::app;
using namespace ci::audio;

class AudioComponent : public IComponent
{
public:
    AudioComponent() {}
    std::vector<float> const & getMagSpectrum() const;
    
    virtual void setup();
    virtual void mouseDown( MouseEvent event ) {}
    virtual void mouseDrag( MouseEvent event ) {}
    virtual void keyDown( KeyEvent event );
    virtual void update() {}
    virtual void draw() {}
    virtual void resize() {}
    
private:
    GainNodeRef	mGain;
    BufferPlayerNodeRef mBufferPlayerNode;
    MonitorSpectralNodeRef mSpectralMonitor;
    InputDeviceNodeRef mInputDeviceNode;
};

std::vector<float> const & AudioComponent::getMagSpectrum() const
{
    return this->mSpectralMonitor->getMagSpectrum();
}

void AudioComponent::setup()
{
    // Audio
    auto ctx = audio::Context::master();
    
    // create a SourceFile and set its output samplerate to match the Context.
    audio::SourceFileRef sourceFile = audio::load( loadResource( "sample.mp3" ), ctx->getSampleRate() );
    
    // load the entire sound file into a BufferRef, and construct a BufferPlayerNode with this.
    audio::BufferRef buffer = sourceFile->loadBuffer();
    mBufferPlayerNode = ctx->makeNode( new audio::BufferPlayerNode( buffer ) );
    mSpectralMonitor = ctx->makeNode( new MonitorSpectralNode );
    
    // add a Gain to reduce the volume
    mGain = ctx->makeNode( new audio::GainNode( 0.5f ) );
    
    /*
     std::vector<DeviceRef> audioDevices = ci::audio::Device::getOutputDevices();
     for( size_t i = 0; i < audioDevices.size(); ++i )
     {
     std::cout << audioDevices[ i ]->getName() << std::endl;
     }
     */
    
    // The InputDeviceNode is platform-specific, so you create it using a special method on the Context:
    mInputDeviceNode = ctx->createInputDeviceNode();
    
    // Create headphones output
    auto device = ci::audio::Device::findDeviceByName( "Built-in Output" );
    ci::audio::OutputDeviceNodeRef output = ctx->createOutputDeviceNode( device );
    ctx->setOutput( output );
    
    // connect and enable the Context
    mBufferPlayerNode >> mGain >> output;
    mGain >> mSpectralMonitor;
    ctx->enable();
    
    mBufferPlayerNode->setLoopEnabled( true );
    mBufferPlayerNode->start();
}

void AudioComponent::keyDown( KeyEvent event )
{
    static size_t lastBufferReadPos = 0;
    
    if( event.getCode() == KeyEvent::KEY_SPACE )
    {
        if( mBufferPlayerNode->isEnabled() )
        {
            lastBufferReadPos = mBufferPlayerNode->getReadPosition();
            mBufferPlayerNode->stop();
        }
        else
        {
            mBufferPlayerNode->start();
            mBufferPlayerNode->seek( lastBufferReadPos );
        }
    }
}

#endif
