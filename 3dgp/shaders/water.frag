#version 330

uniform vec3 waterColor;
uniform vec3 skyColor;
uniform sampler2D textureWater;
uniform bool wavesOverReflections;
uniform vec2 screenRes;

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in float reflFactor; // reflection coefficient
in vec2 distortion;

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

void main(void) 
{
	outColor = color;

	//how much should the water reflect, or be its own colour?
	outColor = mix(vec4(waterColor, 0.2), texture(textureWater, gl_FragCoord.xy / screenRes + 0.2 * distortion), reflFactor);
	//outColor = mix(outColor, vec4(skyColor, 1) , reflFactor); //use for when textures are being sent from the first pass render
}
