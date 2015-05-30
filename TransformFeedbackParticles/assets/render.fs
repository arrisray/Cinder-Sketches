#version 150
#extension all : warn

uniform sampler2D ParticleTex;

in vec4  color;
 
out vec4 FragColor;

void main()
{
    if( color.a < 0.01 )
        discard;
    
    FragColor = texture( ParticleTex, gl_PointCoord );
    // FragColor *= color;
}