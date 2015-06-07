#version 150
#extension all : warn

uniform sampler2D ParticleTex;

in vec4  color;
 
out vec4 FragColor;

void main()
{
    FragColor = texture( ParticleTex, gl_PointCoord ) * color;
    FragColor.a *= 0.7;
}