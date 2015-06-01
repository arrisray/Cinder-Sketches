#version 150 core

uniform mat4	ciModelViewProjection;
uniform float   emitterCap;

in vec3   iPosition;
in vec3   iPPostion;
in vec3   iHome;
in vec4   iColor;
in float  iDamping;

out vec3  position;
out vec3  pposition;
out vec3  home;
out vec4  color;
out float damping;

void main( void )
{
    position =  iPosition;
    pposition = iPPostion;
    damping =   iDamping;
    home =      iHome;
    color =     iColor;
    
    gl_Position	= ciModelViewProjection * vec4( position, 1.0 );
    gl_PointSize = 10.0;
}
