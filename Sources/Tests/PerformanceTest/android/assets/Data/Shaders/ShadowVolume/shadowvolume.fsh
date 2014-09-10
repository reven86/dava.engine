varying vec3 clr;

void main()
{
gl_FragColor = vec4((clr+1.0)/2.0, 1.0);
}
