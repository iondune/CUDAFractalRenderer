#version 400

in vec2 vTexCoord;


uniform sampler2D uColorMap;

uniform double cX;
uniform double cY;

uniform double sX;
uniform double sY;

uniform vec3 uSetColor;

uniform int uScreenWidth;
uniform int uScreenHeight;

uniform int max_iteration;


out vec4 FragColor;

vec4 getFractalColor(vec2 pos)
{
    int iteration = 0;
    
    double x0 = pos.x;
    double y0 = pos.y;

    x0 -= 0.5;
    y0 -= 0.5;
    
    x0 *= sX;
    y0 *= sY;
    
    x0 += cX;
    y0 += cY;
    
    double x = 0.0, y = 0.0;
    while (x*x + y*y < 4.0 && iteration < max_iteration)
    {
        double xtemp = x*x - y*y + x0;
        y = 2.0*x*y + y0;
        
        x = xtemp;
        
        ++ iteration;
    }
    
    float a = float(iteration + 1) / float(max_iteration);
    a = pow(a, 0.333);
    vec4 Color = texture2D(uColorMap, vec2(a, 0));
    
    return iteration == max_iteration ? vec4(uSetColor, 1.0) : Color;
}

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    float xOff = 1.0 / float(uScreenWidth) / 2.0;
    float yOff = 1.0 / float(uScreenHeight) / 2.0;
    FragColor = getFractalColor(vTexCoord + vec2(xOff * rand(vTexCoord), yOff * rand(vTexCoord + vec2(uScreenWidth, uScreenHeight))));
}
