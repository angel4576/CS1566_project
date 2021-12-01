#version 120

attribute vec4 vPosition;
attribute vec4 vColor;
varying vec4 color;

uniform mat4 ctm;

uniform mat4 model_view;
uniform mat4 projection;

void main()
{
	color = vColor;
	gl_Position = projection * model_view * ctm * vPosition;
}
