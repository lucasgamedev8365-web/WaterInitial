#version 330

uniform vec3 waterColor;
uniform sampler2D textureBed;
uniform sampler2D textureShore; 

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in float waterDepth;
in float fogFactor;


// Output Variable (sent down through the Pipeline)
out vec4 outColor;

void main(void)
{
	outColor = color;
	//shoreline multitexturing
	float isAboveWater = clamp(-waterDepth, 0, 1);
	outColor *= mix(texture(textureBed, texCoord0), texture(textureShore, texCoord0), isAboveWater);
	outColor = mix(vec4(waterColor, 1), outColor, fogFactor);
}
