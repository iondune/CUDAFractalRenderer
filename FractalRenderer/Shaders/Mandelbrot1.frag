#version 400

varying vec2 vTexCoord;


uniform sampler2D uColorMap;

uniform double cX;
uniform double cY;

uniform double sX;
uniform double sY;

uniform vec3 uSetColor;

uniform int max_iteration;

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

void main()
{
    gl_FragColor = getFractalColor(vTexCoord);
}
