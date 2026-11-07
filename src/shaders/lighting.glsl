#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;     
uniform sampler2D lightBuffer;  

uniform float levels;   
uniform vec3 ambient;  
uniform float ditherAmount;  

out vec4 finalColor;

const float bayer[16] = float[16](
     0.0,  8.0,  2.0, 10.0,
    12.0,  4.0, 14.0,  6.0,
     3.0, 11.0,  1.0,  9.0,
    15.0,  7.0, 13.0,  5.0
);

void main()
{
    
    vec3 scene = texture(texture0, fragTexCoord).rgb;
    vec3 light = texture(lightBuffer, fragTexCoord).rgb;
 
    float b   = max(light.r,   max(light.g,   light.b));
    float amb = max(ambient.r, max(ambient.g, ambient.b));
 
    float t = clamp((b - amb) / (1.0 - amb), 0.0, 1.0);
 
    ivec2 p = ivec2(gl_FragCoord.xy) % 4;
    float threshold = (bayer[p.x + p.y * 4] + 0.5) / 16.0;
    threshold = mix(0.5, threshold, ditherAmount);
 
    float q = floor(t * (levels - 1.0) + threshold) / (levels - 1.0);
    q = clamp(q, 0.0, 1.0);
    float qb = amb + q * (1.0 - amb);
 
    vec3 lit = (b > 0.0001) ? light * (qb / b) : ambient;
 
    finalColor = vec4(scene * lit, 1.0);

}
