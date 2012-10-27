//==========================================================================================
// 
// SimplexNoise.fx by Elinor Townsend
// Based on Simplex Noise Demystified by Stefan Gustavson, Linköping University, 
// Sweden (stegu@itn.liu.se), 2005-03-22 and HLSL implementation of Simplex Noise from
// David Watt - http://www.gamedev.net/topic/548073-fixed-simplex-noise-hlsl-40/
//
//==========================================================================================

Texture2D PermTexture;
Texture1D SimplexTexture;

SamplerState ColourSampler 
{
    FILTER = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};  

cbuffer cbFixed
{
	// Texel offsets used in texture look up (currently set for resolution of 256x256)
	float texel		= 0.00390625;		// 1 / 256 
	float halfTexel	= 0.001953125;		// 1 / 512
};

// 2D Simplex Noise
float simplexNoise2D(float2 p)
{
	// Skew the input space to determine which simplex cell we're in
	float skewFactor = 0.366025403784;				// Skew factor -> 0.5 * (sqrt(3.0) - 1.0)
	float skew = (p.x + p.y) * skewFactor;
	float2 skewed = floor(p + skew);			// Point after skewing

	// Unskew the cell origin back to (x,y) space
	float unskewFactor = 0.211324865405;			// Unskew factor -> (0.5 - sqrt(3.0) / 6.0)	
	float unskew = (skewed.x + skewed.y) * unskewFactor;	
	float2 origin = skewed - unskew;				// Simplex p in (x,y) space
	float2 distance = p - origin;				// The x,y distance from the cell origin
	skewed = skewed * texel + halfTexel;			// Scale and offset to allow for texture lookup

	// For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
	float2 offset;
	if (distance.x > distance.y)
	{
		offset.x = 1.0;
		offset.y = 0.0;
	}
	else 
	{
		offset.x = 0.0;
		offset.y = 1.0;
	}	
		
	// Calculate the contribution from the three corners
	float noise, t;
	float2 grad, dist;
	grad = PermTexture.Sample(ColourSampler, skewed).rg * 4.0 - 1.0;
	t = 0.5 - dot(distance, distance);
	if (t > 0.0)
	{
		t *= t;
		noise = t * t * dot(grad, distance);
	}

	grad = PermTexture.Sample(ColourSampler, skewed + offset * texel).rg * 4.0 - 1.0;
	dist = distance - offset + unskewFactor;
	t = 0.5 - dot(dist, dist);
	if (t > 0.0)
	{
		t *= t;
		noise += t * t * dot(grad, dist);
	}

	grad = PermTexture.Sample(ColourSampler, skewed + float2(texel, texel)).rg * 4.0 - 1.0;
	dist = distance - (1.0 - 2.0 * unskewFactor);
	t = 0.5 - dot(dist, dist);
	if (t > 0.0)
	{
		t *= t;
		noise += t * t * dot(grad, dist);
	}
	
	return 70.0 * noise;

}

// 3D Simplex Noise
float simplexNoise3D(float3 p)
{
	 // Skew the input space to determine which simplex cell we're in
	float skewFactor = 0.333333333333;			// Skew factor -> 1/3
	float skew = (p.x + p.y + p.z) * skewFactor;
	float3 skewed = floor(p + skew);
	
	// Unskew the cell origin back to (x,y,z) space
	float unskewFactor = 0.166666666667;		// Unskew factor -> 1/6
	float unskew = (skewed.x + skewed.y + skewed.z) * unskewFactor;
	float3 origin = skewed - unskew;			// Simplex p in (x,y,z) space // line 100
	float3 distance = p - origin;			// The x,y,z distance from the cell origin
	skewed = skewed * texel + halfTexel;		// Scale and offset to allow for texture lookup
		
	// For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we are in.
	// To find out which of the six possible tetrahedra we're in, we need to
	// determine the magnitude ordering of x, y and z components of Pf0.
	// The method below uses a small 1D texture as a lookup table. The table 
	// is designed to work for both 3D and 4D noise, so only 8 (only 6, actually) 
	// of the 64 indices are used here.
	float c1 = (distance.x > distance.y) ? 0.5078125 : 0.0078125; // 1/2 + 1/128
	float c2 = (distance.x > distance.z) ? 0.25 : 0.0;
	float c3 = (distance.y > distance.z) ? 0.125 : 0.0;
	float sindex = c1 + c2 + c3;
	float3 offsets = SimplexTexture.Sample(ColourSampler, sindex).rgb;
	float3 offset1 = step(0.375, offsets);
	float3 offset2 = step(0.125, offsets);
		
	// Calculate the contribution from the four corners
	float noise, perm, t;
	float3 grad, dist;
	perm = PermTexture.Sample(ColourSampler, skewed.xy).a;
	grad = PermTexture.Sample(ColourSampler, float2(perm, skewed.z)).rgb * 4.0 - 1.0;
	t = 0.6 - dot(distance, distance);
	if (t > 0.0)
	{
		t *= t;
		noise = t * t * dot(grad, distance);
	}
	
	perm = PermTexture.Sample(ColourSampler, skewed.xy + offset1.xy * texel).a;
	grad = PermTexture.Sample(ColourSampler, float2(perm, skewed.z + offset1.z * texel)).rgb * 4.0 - 1.0;
	dist = distance - offset1 + unskewFactor;
	t = 0.6 - dot(dist, dist);
	if (t > 0.0)
	{
		t *= t;
		noise += t * t * dot(grad, dist);
	}
	
	perm = PermTexture.Sample(ColourSampler, skewed.xy + offset2.xy * texel).a;
	grad = PermTexture.Sample(ColourSampler, float2(perm, skewed.z + offset2.z * texel)).rgb * 4.0 - 1.0;
	dist = distance - offset2 + 2.0 * unskewFactor;
	t = 0.6 - dot(dist, dist);
	if (t > 0.0)
	{
		t *= t;
		noise += t * t * dot(grad, dist);
	}
	
	perm = PermTexture.Sample(ColourSampler, skewed.xy + float2(texel, texel)).a;
	grad = PermTexture.Sample(ColourSampler, float2(perm, skewed.z + texel)).rgb * 4.0 - 1.0;
	dist = distance - (1.0 - unskewFactor * 3.0);
	t = 0.6 - dot(dist, dist);
	if (t > 0.0)
	{
		t *= t;
		noise += t * t * dot(grad, dist);
	}
	
	return 32.0 * noise;
}

// Fractal Brownian motion
float fBm(float3 p, int octaves, float lacunarity = 2.0, float gain = 0.5)
{
	float frequency = 1.0;
	float amplitude = 0.5;
	float sum = 0;	
	for (int i = 0; i < octaves; i++) 
	{
		sum += simplexNoise3D(p * frequency) * amplitude;
		frequency *= lacunarity;
		amplitude *= gain;
	}
	return sum;
}

float turbulence(float3 p, int octaves, float lacunarity = 2.0, float gain = 0.5)
{
	float sum = 0;
	float frequency = 1.0;
	float amplitude = 1.0;
	for(int i = 0; i < octaves; i++) 
	{
		sum += abs(simplexNoise3D(p * frequency)) * amplitude;
		frequency *= lacunarity;
		amplitude *= gain;
	}
	return sum;
}

// Ridged multifractal
// See "Texturing & Modeling, A Procedural Approach", Chapter 12
float ridge(float h, float offset)
{
    h = abs(h);
    h = offset - h;
    h = h * h;
    return h;
}

float ridgedMultifractal(float3 p, int octaves, float lacunarity = 2.0, float gain = 0.5, float offset = 1.0)
{
	float sum = 0;
	float frequency = 1.0;
	float amplitude = 0.5;
	float previous = 1.0;
	for(int i = 0; i < octaves; i++) 
	{
		float n = ridge(simplexNoise3D(p * frequency), offset);
		sum += n * amplitude * previous;
		previous = n;
		frequency *= lacunarity;
		amplitude *= gain;
	}
	return sum;
}