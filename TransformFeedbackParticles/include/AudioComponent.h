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
#include "cinder/audio/Utilities.h"
#include "cinder/CinderMath.h"

using namespace ci;
using namespace ci::app;
using namespace ci::audio;

class AudioComponent : public IComponent
{
public:
    AudioComponent();
    virtual ~AudioComponent() {}
    
    float getVolume();
    std::vector<float> getBeats();
    std::vector<float> const & getMagSpectrum() const;
    
    virtual void setup();
    virtual void mouseDown( MouseEvent event ) {}
    virtual void mouseDrag( MouseEvent event ) {}
    virtual void keyDown( KeyEvent event );
    virtual void update();
    virtual void draw() {}
    virtual void resize() {}
    
private:
    GainNodeRef	mGain;
    BufferPlayerNodeRef mBufferPlayerNode;
    MonitorSpectralNodeRef mSpectralMonitor;
    InputDeviceNodeRef mInputDeviceNode;
    
    int mHistorySize;
    int mNumGroups;
    std::map<int, std::vector<float> > mEnergyHistory;
    std::vector<float> mEnergyAverages;
};

AudioComponent::AudioComponent() :
    mHistorySize( 43 ),
    mNumGroups( 4 ),
    mEnergyAverages( mNumGroups, 0.0f )
{}

float AudioComponent::getVolume()
{
    return this->mSpectralMonitor->getVolume();
}

std::vector<float> AudioComponent::getBeats()
{
    std::vector<float> beats( this->mNumGroups, 0.0f );
    for( int i = 0; i < this->mNumGroups; ++i )
    {
        std::vector<float> & energyHistory = this->mEnergyHistory[ i ];
        float instantEnergy = energyHistory[ this->mHistorySize - 1 ];
        float averageEnergy = this->mEnergyAverages[ i ];
        
        float energyRatio = ci::math<float>::clamp( ( instantEnergy / averageEnergy ) - 1.0f, 0.0f, 0.35f );
        beats[ i ] = energyRatio;
        
        // std::cout << instantEnergy << "/" << averageEnergy << "=" << beats[ i ] << " * ";
    }
    std::cout << std::endl;
    return beats;
}

std::vector<float> const & AudioComponent::getMagSpectrum() const
{
    return this->mSpectralMonitor->getMagSpectrum();
}

void AudioComponent::setup()
{
    for( int i = 0; i < this->mNumGroups; ++i )
    {
        this->mEnergyHistory[ i ] = std::vector<float>( this->mHistorySize, 0.0f );
    }
    
    // Audio
    auto ctx = audio::Context::master();
    
    // create a SourceFile and set its output samplerate to match the Context.
    audio::SourceFileRef sourceFile = audio::load( loadResource( "sample.mp3" ), ctx->getSampleRate() );
    
    // load the entire sound file into a BufferRef, and construct a BufferPlayerNode with this.
    audio::BufferRef buffer = sourceFile->loadBuffer();
    mBufferPlayerNode = ctx->makeNode( new audio::BufferPlayerNode( buffer ) );
    mSpectralMonitor = ctx->makeNode( new MonitorSpectralNode( MonitorSpectralNode::Format()
                                                              .fftSize( 2048 )
                                                              .windowSize( 1024 ) ) );
    
    // add a Gain to reduce the volume
    mGain = ctx->makeNode( new audio::GainNode( 0.5f ) );
    
    //*
    std::vector<DeviceRef> audioDevices = ci::audio::Device::getOutputDevices();
    for( size_t i = 0; i < audioDevices.size(); ++i )
    {
        std::cout << audioDevices[ i ]->getName() << std::endl;
    }
    //*/
    
    // The InputDeviceNode is platform-specific, so you create it using a special method on the Context:
    mInputDeviceNode = ctx->createInputDeviceNode();
    
    // Create headphones output
    auto device = ci::audio::Device::findDeviceByName( "Built-in Output" );
    // auto device = ci::audio::Device::findDeviceByName( "Soundflower (2ch)" );
    ci::audio::OutputDeviceNodeRef output = ctx->createOutputDeviceNode( device );
    ctx->setOutput( output );
    
    // connect and enable the Context
    mBufferPlayerNode >> mGain >> output;
    mGain >> mSpectralMonitor;
    ctx->enable();
    
    mBufferPlayerNode->setLoopEnabled( true );
    mBufferPlayerNode->start();
    
    std::cout << "FFT Size: " << this->mSpectralMonitor->getFftSize() << "\n"
        << "Frames per block: " << this->mSpectralMonitor->getFramesPerBlock() << "\n"
        << "Num bins: " << this->mSpectralMonitor->getNumBins() << "\n"
        << "Num channels: " << this->mSpectralMonitor->getNumChannels() << "\n"
        << "Num connected inputs: " << this->mSpectralMonitor->getNumConnectedInputs() << "\n"
        << "Num connected outputs: " << this->mSpectralMonitor->getNumConnectedOutputs() << "\n"
        << "Sample rate: " << this->mSpectralMonitor->getSampleRate() << "\n"
        << "Window size: " << this->mSpectralMonitor->getWindowSize() << "\n"
        << std::endl;
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
            // mBufferPlayerNode->seek( lastBufferReadPos );
        }
    }
}

void AudioComponent::update()
{
    std::vector<float> magSpectrum( this->mSpectralMonitor->getMagSpectrum() );
    audio::linearToDecibel( &magSpectrum[0], magSpectrum.size() );
    
    // Calculate instant energies
    std::vector<float> instantEnergies( this->mNumGroups, 0.0f );
    int binsPerGroup = magSpectrum.size() / this->mNumGroups;
    float energy = 0.0;
    for( int i = 0, j = 0; i < magSpectrum.size(); ++i )
    {
        if( i > 0 && i % binsPerGroup == 0 )
        {
            instantEnergies[ j ] = powf( energy, 2.0f );
            ++j;
            energy = 0.0f;
        }
        
        energy += magSpectrum[ i ];
    }
    
    // Add instant energies to energy history
    for( int i = 0; i < instantEnergies.size(); ++i )
    {
        float instantEnergy = instantEnergies[ i ];
        std::vector<float> & energyHistory = this->mEnergyHistory[ i ];
        if( energyHistory.size() + 1 > this->mHistorySize ) { energyHistory.erase( energyHistory.begin() ); }
        energyHistory.push_back( instantEnergy );
        
        // std::cout << i << ":" << instantEnergy << " ";
        
        // Update energy averages
        float energyAverage = 0.0f;
        for( int j = 0; j < energyHistory.size(); ++j )
        {
            energyAverage += energyHistory[ j ];
        }
        energyAverage /= energyHistory.size();
        this->mEnergyAverages[ i ] = energyAverage;
        // std::cout << "avg=" << energyAverage << " | ";
    }

    std::cout << "\n" << std::endl;
}

#endif
