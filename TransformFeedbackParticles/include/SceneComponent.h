//
//  SceneComponent.h
//  AudioVertexDisplacement
//
//  Created by Arris Ray on 5/25/15.
//
//

#ifndef AudioVertexDisplacement_VizComponent_h
#define AudioVertexDisplacement_VizComponent_h

#include "IComponent.h"
#include "cinder/app/App.h"
#include "cinder/Rand.h"
#include "cinder/CinderMath.h"

using namespace ci;
using namespace ci::app;

// How many particles to create.
const int NUM_PARTICLES = 1024;

/**
 Particle type holds information for rendering and simulation.
 Used to buffer initial simulation values.
 */
struct Particle
{
    vec3	pos;
    vec3	ppos;
    vec3	home;
    ColorA  color;
    float	damping;
    float   groupId;
    float   size;
};

class SceneComponent : public IComponent
{
public:
    SceneComponent( App * app );
    void setMagSpectrum( std::vector<float> const & spectrum );
    void setBeats( std::vector<float> const & beats );
    void setVolume( float volume );
    
    virtual void setup();
    virtual void mouseDown( MouseEvent event ) {}
    virtual void mouseDrag( MouseEvent event ) {}
    virtual void keyDown( KeyEvent event );
    virtual void update();
    virtual void draw();
    virtual void resize();
    
private:
    bool mIsFullscreen;
    int mNumGroups;
    float mVolume;
    std::vector<float> mMagSpectrum;
    std::vector<float> mBeats;
    App * mApp;
    
    gl::TextureRef					mSmokeTexture;
    
    // Transform Feedback
    
    gl::GlslProgRef mRenderProg;
    gl::GlslProgRef mUpdateProg;
    // Descriptions of particle data layout.
    gl::VaoRef		mAttributes[2];
    // Buffers holding raw particle data on GPU.
    gl::VboRef		mParticleBuffer[2];
    
    // Current source and destination buffers for transform feedback.
    // Source and destination are swapped each frame after update.
    std::uint32_t	mSourceIndex		= 0;
    std::uint32_t	mDestinationIndex	= 1;

    // ~Transform Feedback
    
    void loadTexture();
};

SceneComponent::SceneComponent( App * app ) :
    mIsFullscreen( false ),
    mApp( app ),
    mNumGroups( 4 )
{
}

void SceneComponent::loadTexture()
{
    gl::Texture::Format mTextureFormat;
    mTextureFormat.magFilter( GL_LINEAR ).minFilter( GL_LINEAR ).mipmap().internalFormat( GL_RGBA );
    mSmokeTexture = gl::Texture::create( loadImage( loadAsset( "smoke_blur.png" ) ), mTextureFormat );
}

void SceneComponent::setVolume( float volume )
{
    this->mVolume = volume;
}

void SceneComponent::setBeats( const std::vector<float> & beats )
{
    this->mBeats.assign( beats.begin(), beats.end() );
}

void SceneComponent::setMagSpectrum( const std::vector<float> &spectrum )
{
    this->mMagSpectrum.assign( spectrum.begin(), spectrum.end() );
}

void SceneComponent::setup()
{
    loadTexture();
    
    // Create initial particle layout.
    std::vector<Particle> particles;
    particles.assign( NUM_PARTICLES, Particle() );
    vec3 center = vec3( 0, 0, 0 );
    
    int groupSize = particles.size() / this->mNumGroups;
    for( int i = 0, j = 0; i < particles.size(); ++i )
    {
        if( i > 0 && i % groupSize == 0 ) { ++j; }
        
        // assign starting values to particles.
        float x = Rand::randFloat() * this->mApp->getWindowWidth(); //
        float y = Rand::randFloat() * this->mApp->getWindowHeight(); //
        float z = 0;
        
        auto &p = particles.at( i );
        p.groupId = j;
        p.pos = center + vec3( x, y, z );
        p.home = p.pos;
        p.ppos = p.home + ( Rand::randVec3() ); // random initial velocity
        p.damping = Rand::randFloat( 0.7f, 0.95f ); // 0.965f, 0.985f );
        p.size = Rand::randFloat( 2.0f, 64.0f );
        float hue = lmap<float>( ( (float)j / (float)this->mNumGroups ), 0.0f, (float)this->mNumGroups, 0.14f, 0.4f );
        p.color = Color( CM_HSV, hue, 1.0f, math<float>::clamp( 32.0f / p.size ) );
    }
    
    // Create particle buffers on GPU and copy data into the first buffer.
    // Mark as static since we only write from the CPU once.
    mParticleBuffer[mSourceIndex] = gl::Vbo::create( GL_ARRAY_BUFFER, particles.size() * sizeof(Particle), particles.data(), GL_STATIC_DRAW );
    mParticleBuffer[mDestinationIndex] = gl::Vbo::create( GL_ARRAY_BUFFER, particles.size() * sizeof(Particle), nullptr, GL_STATIC_DRAW );
        
    for( int i = 0; i < 2; ++i )
    {
        // Describe the particle layout for OpenGL.
        mAttributes[i] = gl::Vao::create();
        gl::ScopedVao vao( mAttributes[i] );
        
        // Define attributes as offsets into the bound particle buffer
        gl::ScopedBuffer buffer( mParticleBuffer[i] );
        gl::enableVertexAttribArray( 0 );
        gl::enableVertexAttribArray( 1 );
        gl::enableVertexAttribArray( 2 );
        gl::enableVertexAttribArray( 3 );
        gl::enableVertexAttribArray( 4 );
        gl::enableVertexAttribArray( 5 );
        gl::enableVertexAttribArray( 6 );
        gl::vertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)offsetof(Particle, pos) );
        gl::vertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)offsetof(Particle, ppos) );
        gl::vertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)offsetof(Particle, home) );
        gl::vertexAttribPointer( 3, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)offsetof(Particle, color) );
        gl::vertexAttribPointer( 4, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)offsetof(Particle, damping) );
        gl::vertexAttribPointer( 5, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)offsetof(Particle, groupId) );
        gl::vertexAttribPointer( 6, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)offsetof(Particle, size) );
    }
    
    // Load our update program.
    // Match up our attribute locations with the description we gave.
    mUpdateProg = gl::GlslProg::create( gl::GlslProg::Format().vertex( loadAsset( "particleUpdate.vs" ) )
        .feedbackFormat( GL_INTERLEAVED_ATTRIBS )
        .feedbackVaryings( { "position", "pposition", "home", "color", "damping", "groupId", "size" } )
                                       .attribLocation( "iPosition", 0 )
                                       .attribLocation( "iPPosition", 1 )
                                       .attribLocation( "iHome", 2 )
                                       .attribLocation( "iColor", 3 )
                                       .attribLocation( "iDamping", 4 )
                                       .attribLocation( "iGroupId", 5 )
                                       .attribLocation( "iSize", 6 )
        );
    
    // Load our render program.
    // mRenderProg = gl::getStockShader( gl::ShaderDef().color() );
    mRenderProg = gl::GlslProg::create( gl::GlslProg::Format()
                                       .vertex( loadAsset( "render.vs" ) )
                                       .feedbackFormat( GL_INTERLEAVED_ATTRIBS )
                                       .feedbackVaryings( { "position", "pposition", "home", "color", "damping", "groupId", "size" } )
                                       .attribLocation( "iPosition", 0 )
                                       .attribLocation( "iPPosition", 1 )
                                       .attribLocation( "iHome", 2 )
                                       .attribLocation( "iColor", 3 )
                                       .attribLocation( "iDamping", 4 )
                                       .attribLocation( "iGroupId", 5 )
                                       .attribLocation( "iSize", 6 )
                                       .fragment( loadAsset( "render.fs" ) ) );
}

void SceneComponent::resize()
{
    this->setup();
}

void SceneComponent::keyDown( KeyEvent event )
{
    if( event.getCode() == KeyEvent::KEY_f )
    {
        this->mIsFullscreen = !this->mIsFullscreen;
        setFullScreen( this->mIsFullscreen );
    }
}

void SceneComponent::update()
{
    // Update particles on the GPU
    gl::ScopedGlslProg prog( mUpdateProg );
    gl::ScopedState rasterizer( GL_RASTERIZER_DISCARD, true );	// turn off fragment stage
    
    static float uTime = 0.0f;
    uTime += ( 1.0f / 60.0f ) * 0.001f;
    mUpdateProg->uniform( "uTime", uTime );
    
    for( int i = 0; i < this->mBeats.size(); ++i )
    {
        this->mBeats[ i ] = this->mBeats[ i ] + 0.1f;
    }
    mUpdateProg->uniform( "beats", this->mBeats.data(), this->mBeats.size() );
    
    float activity = powf( lmap<float>( this->mVolume, 0.0f, 1.0f, 0.1f, 10.0f ), 2.0f );
    mUpdateProg->uniform( "activity", activity );
    
    // Bind the source data (Attributes refer to specific buffers).
    gl::ScopedVao source( mAttributes[mSourceIndex] );
    // Bind destination as buffer base.
    gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, mParticleBuffer[mDestinationIndex] );
    gl::beginTransformFeedback( GL_POINTS );
    
    // Draw source into destination, performing our vertex transformations.
    gl::drawArrays( GL_POINTS, 0, NUM_PARTICLES );
    
    gl::endTransformFeedback();
    
    // Swap source and destination for next loop
    std::swap( mSourceIndex, mDestinationIndex );
}

void SceneComponent::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    gl::setMatricesWindowPersp( getWindowSize() );
    gl::enableAlphaBlending();
    
    gl::ScopedVao           vao( mAttributes[mSourceIndex] );
    gl::ScopedGlslProg      render( mRenderProg );
    gl::ScopedTextureBind	texScope( mSmokeTexture );
    gl::ScopedBlend			blendScope( GL_SRC_ALPHA, GL_ONE );
    gl::ScopedState         stateScope( GL_PROGRAM_POINT_SIZE, true );
    
    gl::context()->setDefaultShaderVars();
    gl::drawArrays( GL_POINTS, 0, NUM_PARTICLES );
}

#endif
