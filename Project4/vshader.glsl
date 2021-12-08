#version 120

attribute vec4 vPosition;
attribute vec4 vColor;
attribute vec4 vNormal;

varying vec4 color;//color

uniform mat4 ctm;
uniform mat4 model_view;
uniform mat4 projection;

uniform vec4 ambient_product, diffuse_product, specular_product, light_position;
vec4 ambient, diffuse, specular;
uniform float shininess, attenuation_constant, attenuation_linear, attenuation_quadratic;


void main()
{
	ambient =  ambient_product;

	vec4 N = normalize(model_view * ctm * vNormal);//normal
	vec4 L_temp = model_view * (light_position - ctm * vPosition);
	vec4 L = normalize(L_temp);
	//diffuse
	diffuse = max(dot(L, N), 0.0) * diffuse_product;
	vec4 eye_position = vec4(0.0, 0.0, 0.0, 1.0);

	vec4 V = normalize(eye_position - (model_view * ctm * vPosition));
	vec4 H = normalize(L + V);//half vector
	//specular 
	specular = pow(max(dot(N, H), 0.0), shininess) * specular_product;

	//attenuation
	float distance = length(L_temp);
	float attenuation = 1/(attenuation_constant + (attenuation_linear * distance) 
            + (attenuation_quadratic * distance * distance));
	
	//color be sent to frag shader
	//color = ambient + (attenuation * (diffuse + specular));//be sent to frag shader
	color = vColor;

	//position
	gl_Position = projection * model_view * ctm * vPosition;
	

	
}
