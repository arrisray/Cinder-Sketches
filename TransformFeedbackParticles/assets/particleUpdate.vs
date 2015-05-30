#version 150 core

uniform float emitterCap;

in float  iIsActive;
in vec3   iPosition;
in vec3   iPPostion;
in vec3   iHome;
in float  iDamping;
in vec4   iColor;

out float isActive;
out vec3  position;
out vec3  pposition;
out vec3  home;
out float damping;
out vec4  color;

const float dt2 = 1.0 / (60.0 * 60.0);

void main()
{
    position =  iPosition;
    pposition = iPPostion;
    damping =   iDamping;
    home =      iHome;
    color =     iColor;
    
    if( gl_VertexID < emitterCap )
    {
        isActive = 1.0;
        color.a = 1.0;
    }
    
    if( gl_VertexID > emitterCap || iIsActive == 0.0f )
    {
        color.a = 0.0;
        return;
    }
    
    vec3 vel = (position - pposition) * damping;
    pposition = position;
    vec3 acc = (home - position) * 32.0f;
    position += vel + acc * dt2;
}