
varying vec2 vTexCoord;
uniform sampler2D uColorMap;

void main()
{
    gl_FragColor = texture2D(uColorMap, vTexCoord);
}
