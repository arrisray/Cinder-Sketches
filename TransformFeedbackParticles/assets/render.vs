#version 150 core

uniform mat4	ciModelViewProjection;
uniform float   emitterCap;

in vec3   iPosition;
in vec3   iPPostion;
in vec3   iHome;
in vec4   iColor;
in float  iDamping;
in float  iGroupId;
in float  iSize;

out vec3  position;
out vec3  pposition;
out vec3  home;
out vec4  color;
out float damping;
out float groupId;
out float size;

void main( void )
{
    position =  iPosition;
    pposition = iPPostion;
    damping =   iDamping;
    home =      iHome;
    color =     iColor;
    groupId =   iGroupId;
    size =      iSize;
    
    gl_Position	= ciModelViewProjection * vec4( position, 1.0 );
    gl_PointSize = size;
}
