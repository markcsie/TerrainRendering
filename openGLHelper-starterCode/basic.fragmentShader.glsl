#version 150

uniform vec4 fCol;
in vec4 col;

void main()
{
  // compute the final pixel color
  gl_FragColor = fCol * col;
}

