#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/audio/Context.h"
#include "cinder/audio/NodeEffects.h"
#include "cinder/audio/Source.h"
#include "cinder/audio/MonitorNode.h"
#include "cinder/audio/Utilities.h"
#include "cinder/qtime/QuickTime.h"
#include "cinder/ip/Resize.h"

using namespace ci;
using namespace ci::app;
using namespace ci::audio;
using namespace ci::qtime;
using namespace std;

//------------------------------------------------------------------------------
//! @brief Sample Cinder app demonstrating how to find, load and sample a SoundFlower device to drive some neat visual effects.
//! @note I run pretty slow at higher resolution; be gentle. I need some shader love, for sure.
class SoundflowerApp : public App
{
public:
    //! @brief The movie we want to parametrically destroy using our audio input.
    static const char *SAMPLE_MOVIE;
    
    //! @brief The audio device we expect to use for evil intent!
    static const char *SOUNDFLOWER_DEVICE_NAME;
    
    //! @brief Load up an audio device and a sample movie.
    void setup();
    
    //! @brief Sample audio and video for the current frame.
    void update();
    
    //! @brief This is where we wield some audio to beat the shit out of our movie for awesome!
    void draw();
    
private:
    MonitorSpectralNodeRef mSpectralMonitor;
    InputDeviceNodeRef mInputDeviceNode;
    qtime::MovieSurfaceRef m_movie;
    Surface8uRef m_surface;
    
    //! @brief Create audio nodes for audio stream processing.
    void setupAudio();
    
    //! @brief Load a sample movie to freak out.
    void setupVideo( const fs::path &path );

    //! @brief Draw stereo waveform in the center of our screen.
    void drawWaveForm();
};


//------------------------------------------------------------------------------
// const char* SoundflowerApp::SAMPLE_MOVIE = "skydiverr.mov";
const char* SoundflowerApp::SAMPLE_MOVIE = "sample.m4v";


//------------------------------------------------------------------------------
const char *SoundflowerApp::SOUNDFLOWER_DEVICE_NAME = "Soundflower (2ch)";


//------------------------------------------------------------------------------
void SoundflowerApp::setup()
{
    this->setupVideo( SoundflowerApp::SAMPLE_MOVIE );
    this->setupAudio();
}


//------------------------------------------------------------------------------
void SoundflowerApp::setupAudio()
{
    // Create our master audio Context
    auto ctx = audio::Context::master();
    
    std::cout << "=== INPUT DEVICES ===" << std::endl;
    std::vector<DeviceRef> audioDevices = Device::getInputDevices();
    for( size_t i = 0; i < audioDevices.size(); ++i )
    {
        std::cout << audioDevices[ i ]->getName() << std::endl;
    }
    std::cout << std::endl;
    
    std::cout << "=== OUTPUT DEVICES ===" << std::endl;
    audioDevices = Device::getOutputDevices();
    for( size_t i = 0; i < audioDevices.size(); ++i )
    {
        std::cout << audioDevices[ i ]->getName() << std::endl;
    }
    std::cout << std::endl;
    
    mSpectralMonitor = ctx->makeNode( new MonitorSpectralNode( MonitorSpectralNode::Format()
                                                              .fftSize( 2048 )
                                                              .windowSize( 1024 ) ) );
    
    // The InputDeviceNode is platform-specific, so you create it using a special method on the Context
    auto inputDevice = Device::findDeviceByName( SoundflowerApp::SOUNDFLOWER_DEVICE_NAME );
    mInputDeviceNode = ctx->createInputDeviceNode( inputDevice );
    
    // connect and enable the Context
    mInputDeviceNode >> mSpectralMonitor;
    
    ctx->enable();
    mInputDeviceNode->enable();
}


//------------------------------------------------------------------------------
void SoundflowerApp::setupVideo( const fs::path &moviePath )
{
    try
    {
        this->m_movie = qtime::MovieSurface::create( loadResource( moviePath )->getFilePath() );
        
        console() << "Dimensions:" << this->m_movie->getWidth() << " x " << this->m_movie->getHeight() << std::endl;
        console() << "Duration:  " << this->m_movie->getDuration() << " seconds" << std::endl;
        console() << "Frames:    " << this->m_movie->getNumFrames() << std::endl;
        console() << "Framerate: " << this->m_movie->getFramerate() << std::endl;
        // console() << "Alpha channel: " << this->m_movie->hasAlpha() << std::endl;
        console() << "Has audio: " << this->m_movie->hasAudio() << " Has visuals: " << this->m_movie->hasVisuals() << std::endl;
        
        this->m_movie->setLoop( true, true );
        this->m_movie->seekToStart();
        
        this->setWindowSize( this->m_movie->getWidth(), this->m_movie->getHeight() );
    }
    catch( ... )
    {
        console() << "Unable to load the movie." << std::endl;
    }
}


//------------------------------------------------------------------------------
void SoundflowerApp::update()
{
    // Sample video for the current frame
    if( this->m_movie )
    {
        this->m_movie->stepForward();
        if( !this->m_movie->checkNewFrame() )
        {
            this->m_movie->seekToStart();
        }
        
        this->m_surface = this->m_movie->getSurface();
        if( this->m_surface )
        {
            Surface8uRef resizedSurface( new Surface8u( cinder::app::getWindowWidth(), cinder::app::getWindowHeight(), true ) );
            cinder::ip::resize( *this->m_surface.get(), resizedSurface.get() );
            this->m_surface = resizedSurface;
        }
    }
}


//------------------------------------------------------------------------------
void SoundflowerApp::draw()
{
    // Clear to black!
    gl::clear( Color( 0, 0, 0 ) );
    gl::enableAlphaBlending( false );
    
    // Vertically displace columns of pixels in the video as a function of the current frame's audio waveform!
    if( this->m_surface )
    {
        // We are using OpenGL to draw the frames here,
        // so we'll make a texture out of the surface!
        int col = 0;
        Surface8u cloneSurface = this->m_surface->clone();
        Surface8u::Iter cloneIter = cloneSurface.getIter();
        Surface8u::Iter iter = this->m_surface->getIter();
    
        // Foreach row...
        std::vector<float> const & magSpectrum = this->mSpectralMonitor->getMagSpectrum();
        while( iter.line() && cloneIter.line() )
        {
            // Foreach column...
            while( iter.pixel() && cloneIter.pixel() )
            {
                uint32_t bufferIndex = (uint32_t)lmap<float>( col, 0, iter.getWidth(), 0, magSpectrum.size() );
                float magnitude = ci::audio::linearToDecibel( magSpectrum[ bufferIndex ] );
                
                // Modulate the alpha of the current column by the magnitude of its associated frequency band
                unsigned char v = (unsigned char)( lmap<float>(magnitude, 0.0f, 50.0f, 0.0f, 255.0f ) );
                cloneIter.a() = v;
                
                ++col;
            }
            col = 0;
        }
        
        // Show our work!
        gl::TextureRef movieTexture = gl::Texture2d::create( cloneSurface );
        gl::draw( movieTexture );
    }
    
    // Draw the audio waveform used for the video freakening we did above!
    this->drawWaveForm();
}


//------------------------------------------------------------------------------
void SoundflowerApp::drawWaveForm()
{
    uint32_t bufferLength = this->mSpectralMonitor->getMagSpectrum().size();
    
    int displaySize = getWindowWidth();
    float scale = displaySize / (float)bufferLength;
    
    const float VERTICAL_CENTER = cinder::app::getWindowHeight() / 2.0f;
    
    PolyLineT<vec2>	bufferLine;
    for( int i = 0; i < bufferLength; i++ )
    {
        float x = ( i * scale );
        
        //get the PCM value from the left channel buffer
        float decibels = -1.0f * ci::audio::linearToDecibel( this->mSpectralMonitor->getMagSpectrum()[ i ] );
        // std::cout << decibels << " ";
        float y = ( decibels + VERTICAL_CENTER );
        vec2 coords = vec2( x, y );
        bufferLine.push_back( coords );
        // std::cout << "{ " << coords.x << ", " << coords.y << " } ";
    }
    // std::cout << std::endl;
    gl::draw( bufferLine );
}


//------------------------------------------------------------------------------
//! @brief App entry point.
CINDER_APP( SoundflowerApp, RendererGl )
