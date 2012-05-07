#version 400

varying vec2 vTexCoord;
uniform sampler2D uColorMap;

uniform double cX;
uniform double cY;

uniform double sX;
uniform double sY;

uniform int max_iteration;

void main()
{
    int iteration = 0;
    
    double x0 = vTexCoord.x;
    double y0 = vTexCoord.y;

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
    
    vec4 Color = texture2D(uColorMap, vec2(float(iteration) / float(max_iteration), 0));
    
    gl_FragColor = iteration == max_iteration ? vec4(0,0,0,1) : Color;
}
