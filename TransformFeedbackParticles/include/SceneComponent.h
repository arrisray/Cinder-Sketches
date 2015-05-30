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
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;

// How many particles to create. (600k default)
const int NUM_PARTICLES = 1e6;

/**
 Particle type holds information for rendering and simulation.
 Used to buffer initial simulation values.
 */
struct Particle
{
    float   isActive;
    vec3	pos;
    vec3	ppos;
    vec3	home;
    ColorA  color;
    float	damping;
};

class SceneComponent : public IComponent
{
public:
    SceneComponent() {}
    void setMagSpectrum( std::vector<float> const & spectrum );
    
    virtual void setup();
    virtual void mouseDown( MouseEvent event ) {}
    virtual void mouseDrag( MouseEvent event ) {}
    virtual void keyDown( KeyEvent event ) {}
    virtual void update();
    virtual void draw();
    virtual void resize() {}
    
private:
    std::vector<float> mMagSpectrum;
    
    float                           mEmitterCap;
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
    void drawGrid( float size, float st );
    void drawDiscs( float size, float step );
};


void SceneComponent::drawDiscs( float size, float step )
{
    gl::color( 1.0f, 0.0f, 0.0f );
    
    const float COLS = ( size / step ) * 2;
    const float GRID_OFFSET_X = size;
    const float GRID_OFFSET_Y = 0.0f;
    const float DISC_OFFSET = step * 0.5f;
    
    for( int i = 0; i < this->mMagSpectrum.size(); ++i )
    {
        int col = i % (int)COLS;
        int row = i / (int)COLS;
        
        float x = ( step * col ) + DISC_OFFSET - GRID_OFFSET_X;
        float y = ( step * row ) + DISC_OFFSET - GRID_OFFSET_Y;
        
        gl::drawSolidCircle( vec2( x, y ), log10( this->mMagSpectrum[ i ] * 1000.0f ) );
    }
}

void SceneComponent::drawGrid( float size, float step )
{
    gl::color( Color( 0.5f, 0.5f, 0.5f) );
    
    // draw grid
    glLineWidth( 0.5f );
    for( float i = -size; i <= size; i += step)
    {
        gl::drawLine( vec3( i, 0.f, -size), vec3( i, 0.f, size));
        gl::drawLine( vec3(-size, 0.f, i), vec3( size, 0.f, i));
    }
    
    // draw bold center lines
    glLineWidth( 1.f );
    gl::color( Color( 0.75f, 0.75f, 0.75f ) );
    
    gl::drawLine( vec3( 0.f, 0.f, -size), vec3( 0.f, 0.f, size ) );
    gl::drawLine( vec3(-size, 0.f, 0.f), vec3( size, 0.f, 0.f ) );
    glLineWidth( 1.f );
}

void SceneComponent::loadTexture()
{
    gl::Texture::Format mTextureFormat;
    mTextureFormat.magFilter( GL_LINEAR ).minFilter( GL_LINEAR ).mipmap().internalFormat( GL_RGBA );
    mSmokeTexture = gl::Texture::create( loadImage( loadAsset( "smoke_blur.png" ) ), mTextureFormat );
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
    // const float azimuth = 256.0f * M_PI / particles.size();
    // const float inclination = M_PI / particles.size();
    // const float radius = 180.0f;
    vec3 center = vec3( getWindowCenter() + vec2( 0.0f, 40.0f ), 0.0f );
    for( int i = 0; i < particles.size(); ++i )
    {	// assign starting values to particles.
        float x = 0; // radius * sin( inclination * i ) * cos( azimuth * i );
        float y = 0; // radius * cos( inclination * i );
        float z = 0; // radius * sin( inclination * i ) * sin( azimuth * i );
        
        auto &p = particles.at( i );
        p.isActive = 0.0f;
        p.pos = center + vec3( x, y, z );
        p.home = p.pos;
        p.ppos = p.home + Rand::randVec3() * 10.0f; // random initial velocity
        p.damping = Rand::randFloat( 0.965f, 0.985f );
        p.color = Color( CM_HSV, lmap<float>( i, 0.0f, particles.size(), 0.0f, 0.66f ), 1.0f, 1.0f );
        p.color.a = 0.0f;
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
        gl::vertexAttribPointer( 0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)offsetof(Particle, isActive) );
        gl::vertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)offsetof(Particle, pos) );
        gl::vertexAttribPointer( 2, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)offsetof(Particle, color) );
        gl::vertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)offsetof(Particle, ppos) );
        gl::vertexAttribPointer( 4, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)offsetof(Particle, home) );
        gl::vertexAttribPointer( 5, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)offsetof(Particle, damping) );
    }
    
    // Load our update program.
    // Match up our attribute locations with the description we gave.
    mUpdateProg = gl::GlslProg::create( gl::GlslProg::Format().vertex( loadAsset( "particleUpdate.vs" ) )
        .feedbackFormat( GL_INTERLEAVED_ATTRIBS )
        .feedbackVaryings( { "isActive", "position", "pposition", "home", "color", "damping" } )
        .attribLocation( "iIsActive", 0 )
        .attribLocation( "iPosition", 1 )
        .attribLocation( "iColor", 2 )
        .attribLocation( "iPPosition", 3 )
        .attribLocation( "iHome", 4 )
        .attribLocation( "iDamping", 5 )
        );
    
    // Create a default color shader.
    // mRenderProg = gl::getStockShader( gl::ShaderDef().color() );
    mRenderProg = gl::GlslProg::create( gl::GlslProg::Format().vertex( loadAsset( "render.vs" ) )
        .fragment( loadAsset( "render.fs" ) ) );
}

void SceneComponent::update()
{
    // Update particles on the GPU
    gl::ScopedGlslProg prog( mUpdateProg );
    gl::ScopedState rasterizer( GL_RASTERIZER_DISCARD, true );	// turn off fragment stage
    
    ++mEmitterCap;
    mUpdateProg->uniform( "emitterCap", mEmitterCap );
    
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
    // gl::enableDepthRead();
    // gl::enableDepthWrite();
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
