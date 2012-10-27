//==========================================================================================
// 
// Light.fx by Elinor Townsend
// Functions for calculating parallel, point and spot lighting
//
//==========================================================================================

struct Light
{
	float4 ambientColour;
	float4 diffuseColour;
	float4 specularColour;
	float3 diffuseDirection;
	float ambientIntensity;
	float diffuseIntensity;	
	float specularIntensity;
	float shininess;	
};

float4 ParallelLight(Light light, float3 normal, float3 cameraView)
{
    float3 lightDirection = normalize(light.diffuseDirection);
    float3 halfAngle = normalize(lightDirection + cameraView);

	float4 ambient = light.ambientColour;
    float4 diffuse = light.diffuseColour * light.diffuseIntensity * max(dot(lightDirection, normal), 0.0f);
	float4 specular = light.specularColour * light.specularIntensity * pow(max(dot(normal, halfAngle), 0.0f), light.shininess);
 
    return saturate(ambient + diffuse + specular);
}