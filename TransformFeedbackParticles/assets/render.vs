#version 150 core

uniform mat4	ciModelViewProjection;
uniform float   emitterCap;

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

void main( void )
{
    position =  iPosition;
    pposition = iPPostion;
    damping =   iDamping;
    home =      iHome;
    color =     iColor;
    
    gl_Position	= ciModelViewProjection * vec4( iPosition, 1.0 );
    gl_PointSize = 5.0;
}
