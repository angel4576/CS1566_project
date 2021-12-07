#version 120

attribute vec4 vPosition;
attribute vec4 vColor;
attribute vec4 vNormal;

varying vec4 color;//color

uniform mat4 ctm;
uniform mat4 model_view;
uniform mat4 projection;

uniform vec4 ambient_product, diffuse_product, specular_product;
vec4 ambient, diffuse, specular;
uniform vec4 light_position;

void main()
{
	color = vColor;
	gl_Position = projection * model_view * ctm * vPosition;
	//caculate a, d, s
	//color = ambient + attenuation * (diffuse + specular);
	
}
