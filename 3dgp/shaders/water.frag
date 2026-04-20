#version 330

uniform vec3 waterColor;
uniform vec3 skyColor;
uniform sampler2D textureWater;
uniform bool wavesOverReflections;

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in float reflFactor; // reflection coefficient

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

void main(void) 
{
	outColor = color;

	//how much should the water reflect, or be its own colour?
	outColor = mix(vec4(waterColor, 0.2), vec4(skyColor, 1), reflFactor);
	outColor = mix(texture(textureWater, texCoord0), outColor, reflFactor);
}
