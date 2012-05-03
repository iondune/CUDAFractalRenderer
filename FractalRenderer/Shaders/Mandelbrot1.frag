#version 400

varying vec2 vTexCoord;

uniform double cX;
uniform double cY;

uniform double sX;
uniform double sY;

void main()
{
    int iteration = 0;
    int max_iteration = 1000;
    
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
    
    int fifth = max_iteration / 7;
    float seventh = max_iteration / 7.0;
    float r = 0, g = 0, b = 0;
    switch (iteration / fifth)
    {
    default:
    case 0:
        
        r = iteration / seventh;
        break;
        
    case 1:
        
        r =  1.0;
        g = (iteration % fifth) / seventh;
        break;
        
    case 2:
        
        r = 1.0 - (iteration % fifth) / seventh;
        g = 1.0;
        break;
        
    case 3:
        
        g = 1.0;
        b = (iteration % fifth) / seventh;
        break;
        
    case 4:
        
        g = 1.0 - (iteration % fifth) / seventh;
        b = 1.0;
        break;
        
    case 5:
        
        b = 1.0;
        r = (iteration % fifth) / seventh;
        break;
        
    case 6:
        
        b = 1.0 - (iteration % fifth) / seventh;
        r = 1.0 - (iteration % fifth) / seventh;
        
    case 7:
        
        r = g = 
        b = 0.0;
        break;
        
    }
    vec4 Color = vec4(r, g, b, 1.0);
    
    gl_FragColor = Color;
}
