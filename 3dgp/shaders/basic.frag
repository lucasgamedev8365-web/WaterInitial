#version 330 

uniform vec3 waterColor;
uniform sampler2D texture0;

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in float fogFactor;

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

void main(void) 
{
	outColor = color;
	outColor *= texture(texture0, texCoord0);
	outColor = mix(vec4(waterColor, 1), outColor, fogFactor);
}
