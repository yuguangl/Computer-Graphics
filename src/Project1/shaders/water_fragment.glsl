#version 330 core
in vec4 clipSpace;
in vec2 textureCoords;
in vec3 toCameraVector;
in vec3 fromLightVector;
uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D waterDudv;
uniform sampler2D normalMap;
uniform float moveFactor;
uniform vec3 lightColor;
uniform vec3 cameraPosition;
out vec4 out_Color;

const float waveStrength = 0.02;
const float shineDamper = 100.0;
const float reflectivity = 0.8;
const float ambientStrength = 0.01;
const float waterLevel = 400.0;

void main(void) {
	vec2 ndc = (clipSpace.xy/clipSpace.w) / 2.0 + 0.5;
	
	bool underwater = cameraPosition.y < waterLevel;
	
	vec2 reflectTextCoords = vec2(ndc.x, -ndc.y);
	vec2 refractTexCoords = vec2(ndc.x, ndc.y);
	
	vec2 distortedTexCoords1 = fract(textureCoords + vec2(moveFactor, moveFactor));
	vec2 distortedTexCoords2 = fract(textureCoords * 1.3 - vec2(moveFactor * 0.7, moveFactor * 0.5));
	
	vec2 distortion1 = (texture(waterDudv, distortedTexCoords1).rg * 2.0 - 1.0);
	vec2 distortion2 = (texture(waterDudv, distortedTexCoords2).rg * 2.0 - 1.0);
	vec2 totalDistortion = (distortion1 + distortion2 * 0.5) * waveStrength;
	
	reflectTextCoords += totalDistortion;
	reflectTextCoords.x = clamp(reflectTextCoords.x, 0.001, 0.999);
	reflectTextCoords.y = clamp(reflectTextCoords.y, -0.999, -0.001);
	
	refractTexCoords += totalDistortion;
	refractTexCoords = clamp(refractTexCoords, 0.001, 0.999);
	
	vec4 reflectColor = texture(reflectionTexture, reflectTextCoords);
	vec4 refractColor = texture(refractionTexture, refractTexCoords);
	
	vec3 viewVector = normalize(toCameraVector);
	float refractiveFactor = dot(viewVector, vec3(0.0, 1.0, 0.0));
	refractiveFactor = pow(refractiveFactor, 0.8);
	
	vec4 normalMapColor1 = texture(normalMap, distortedTexCoords1);
	vec4 normalMapColor2 = texture(normalMap, distortedTexCoords2);
	vec4 normalMapColor = mix(normalMapColor1, normalMapColor2, 0.5);
	
	vec3 normal = vec3(normalMapColor.r * 2.0 - 1.0, normalMapColor.b * 3.0, normalMapColor.g * 2.0 - 1.0);
	normal = normalize(normal);
	
	vec3 lightDir = normalize(vec3(0.3, -1.0, 0.2));
	vec3 reflectedLight = reflect(lightDir, normal);
	
	float specular = max(dot(reflectedLight, viewVector), 0.0);
	specular = pow(specular, shineDamper);
	
	vec3 specularHighlights = vec3(1.0, 1.0, 1.0) * specular * reflectivity;
	
	if (underwater) {
		// When underwater: transparent surface showing skybox with subtle blue fog
		out_Color = reflectColor;
		out_Color = mix(out_Color, vec4(0.0, 0.05, 0.2, 1.0), 0.1);
		out_Color.a = 0.3;
	} else {
		// When above water: more translucent water
		vec3 waterBlue = vec3(0.0, 0.1, 0.4);  // Slightly lighter blue
		vec3 reflectionMix = mix(reflectColor.rgb, refractColor.rgb, refractiveFactor);
		
		// Show more of what's below the water (higher refraction weight)
		out_Color.rgb = waterBlue * 0.1 + reflectionMix * 0.7 + specularHighlights;  // Changed from 0.6/0.4 to 0.3/0.7
		out_Color.rgb = clamp(out_Color.rgb, 0.0, 1.0);
		out_Color.a = 0.7;  // Make it translucent (was 1.0)
	}
}
