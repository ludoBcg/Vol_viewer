// Fragment shader
#version 330

// Ouput data (G-buffer)
layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gColor;


uniform sampler3D u_volumeTexture;
uniform sampler2D u_backFaceTexture;
uniform sampler2D u_frontFaceTexture;
uniform sampler2D u_perlinTex;
uniform sampler1D u_lookupTexture;
uniform bool u_useGammaCorrec;
uniform bool u_useShadow;
uniform bool u_useJitter;
uniform int u_useTF;
uniform int u_maxSteps;
uniform float u_isoValue;
uniform vec3 u_lightDir;
uniform mat4 u_matM;
uniform mat4 u_matV;
uniform mat4 u_matP;
uniform vec2 u_screenDims;
uniform vec3 u_ambientColor;
uniform float u_transparency;


in vec2 v_texcoord;

out vec4 frag_color;


vec2 perlinNoiseScale = vec2(u_screenDims[0] * 0.01, u_screenDims[1] * 0.01);
const float PI = 3.14159265359;

// -------------------------------------------------------------------------------
// PBR functions
// see https://learnopengl.com/PBR/

// Normal distribution function (D)
float DistributionGGX(vec3 _N, vec3 _H, float _a)
{
	// Compute NDF using Trowbridge-Reitz GGX
	// (statistically approximate the general alignment of the microfacets given some roughness parameter)

	float a2 = _a * _a;
	float NdotH = max(dot(_N, _H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}

// Geometry function (G)
float GeometrySchlickGGX(float _NdotV, float _k)
{
	float nom = _NdotV;
	float denom = _NdotV * (1.0 - _k) + _k;

	return nom / denom;
}

float GeometrySmith(vec3 _N, vec3 _V, vec3 _L, float _k)
{
	// Compute geometry function using Schlick-GGX
	// (statistically approximates the relative surface area where its micro surface-details overshadow each other causing light rays to be occluded)

	float NdotV = max(dot(_N, _V), 0.0);
	float NdotL = max(dot(_N, _L), 0.0);
	float ggx1 = GeometrySchlickGGX(NdotV, _k);
	float ggx2 = GeometrySchlickGGX(NdotL, _k);

	return ggx1 * ggx2;
}

// Fresnel equation (F)
vec3 fresnelSchlick(float _cosTheta, vec3 _F0)
{
	// Fresnel-Schlick approximation for F
	// (describes the ratio of light that gets reflected over the light that gets refracted)

	return _F0 + (1.0 - _F0) * pow(1.0 - _cosTheta, 5.0);
}


// -------------------------------------------------------------------------------
// Interval bisection that can be used to improve the
// accuracy of iso-surface detection. Based on a CG example in the
// SIGGRAPH2009 course notes on Advanced Illumination Techniques for
// GPU-Based Volume Raycasting.
vec3 interval_bisection(vec3 ray_position, vec3 ray_direction, float _stepSize)
{
	// start = voxel pos - stepSize*raydir
	vec3 start = ray_position - _stepSize * ray_direction;
	// end = voxel pos
	vec3 end = ray_position;
	int num_bisections = 4;
	for (int b = 0; b < num_bisections; ++b)
	{
		// test pos = middle between start and end
		vec3 test_position = (start + end) / 2.0;

		if (texture(u_volumeTexture, test_position).r < u_isoValue)
		{
			// if test pos is outside isosurface, use it as new start position
			start = test_position;
		}
		else
		{
			// if test pos is inside isosurface, use it as new start position
			end = test_position;
		}
	}

	return (start + end) / 2.0;
}


// -------------------------------------------------------------------------------
// Shadow computation 
// Casts a secondary ray in the direction of light source
// and detects intersections with isosurface
int shadowRay(vec3 _pos, vec3 _lightDir, vec3 _rayDir, float _stepSize) 
{
	float maxStep = u_maxSteps * 0.5;

	_pos -= _stepSize * _rayDir; // offset to get out of surface 

	for (int i = 0; i < int(maxStep); i++)
	{
		_pos += _stepSize * _lightDir;

		if (texture(u_volumeTexture, _pos).r > u_isoValue)
			return 1;
	}

	return 0;
}


// -------------------------------------------------------------------------------
// Phong shading specular term
float specular_normalized(in vec3 _N, in vec3 _H, in float _specularPower)
{
	float normalization = (8.0 + _specularPower) / 8.0;
	float specular = normalization * pow(max(0.0, dot(_N, _H)), _specularPower); // add max to remove black artifacts
	specular = min(1.0f, specular); // make sure max specular value is 1
	return specular;
}

// -------------------------------------------------------------------------------
// 3D filters

// Find highest intensity value in 6-voxel neigborhood
float maxNbhVal(in sampler3D image, in vec3 pos)
{
	float maxVal = texture(image, pos).r;

	maxVal = max(maxVal, textureOffset(image, pos, ivec3(1, 0, 0)).r);
	maxVal = max(maxVal, textureOffset(image, pos, -ivec3(1, 0, 0)).r);
	maxVal = max(maxVal, textureOffset(image, pos, ivec3(0, 1, 0)).r);
	maxVal = max(maxVal, textureOffset(image, pos, -ivec3(0, 1, 0)).r);
	maxVal = max(maxVal, textureOffset(image, pos, ivec3(0, 0, 1)).r);
	maxVal = max(maxVal, textureOffset(image, pos, -ivec3(0, 0, 1)).r);

	return maxVal;
}

// 6-voxel neigborhood Sobel filter
vec3 imageGradient(in sampler3D image, in vec3 pos)
{
	vec3 grad = vec3(0.0);
	grad.x += textureOffset(image, pos, ivec3(1, 0, 0)).r;
	grad.x -= textureOffset(image, pos, -ivec3(1, 0, 0)).r;
	grad.y += textureOffset(image, pos, ivec3(0, 1, 0)).r;
	grad.y -= textureOffset(image, pos, -ivec3(0, 1, 0)).r;
	grad.z += textureOffset(image, pos, ivec3(0, 0, 1)).r;
	grad.z -= textureOffset(image, pos, -ivec3(0, 0, 1)).r;

	return grad;
}

// -------------------------------------------------------------------------------
// Gamma correction
vec3 gammaToLinear(in vec3 color)
{
	return pow(color, vec3(2.2));
}

vec3 linearToGamma(in vec3 color)
{
	return pow(color, vec3(1.0 / 2.2));
}


void main()
{
	// init B-buffer values to zero
	gPosition = vec4(0.0);
	gColor = vec4(0.0);
	gNormal = vec4(0.0);

    vec4 color = vec4(0.0);
	mat4 matMVP = u_matP * u_matV * u_matM;     // assemble model-viw-projection matrix
	float stepSize = 1.732/float(u_maxSteps);   //1.732 = sqrt(3) = diag length

    vec4 frontFace = texture(u_frontFaceTexture, v_texcoord);
    vec4 backFace = texture(u_backFaceTexture, v_texcoord);

	// sample random value from Perlin noise
	float randomVal = texture(u_perlinTex, v_texcoord * perlinNoiseScale).r;

    if (frontFace.a == 0.0 || backFace.a == 0) { discard; }

    vec3 rayStart = frontFace.xyz;
    vec3 rayStop = backFace.xyz;
    vec3 rayDir = normalize(rayStop - rayStart);
	int numSteps = int(length(rayStart - rayStop) / stepSize);

	vec3 L = vec3(0.0, 0.0, 1.0);            // light vector
	L = normalize(u_lightDir);
	L = normalize(mat3(inverse(u_matM)) * u_lightDir);

	vec3 pos = rayStart;
	if (u_useJitter)
	{
		// add random length in ray direction to start position
		pos += randomVal * stepSize * rayDir;
	}

    float intensity = 0.0;

	for (int i = 0; i < numSteps; ++i) 
	{
		intensity = texture(u_volumeTexture, pos).r;

		if (intensity >= u_isoValue)
		{
			break;
		}

		pos += stepSize * rayDir;
	}

	// shading of surface voxel
	if (intensity >= u_isoValue)
	{
		//improve accuracy of iso surface position
		pos = interval_bisection(pos, rayDir, stepSize);

		// normal vec in 3D texture space
		vec3 normal = normalize(-imageGradient(u_volumeTexture, pos));

		// normal in img space used for lighting
		vec3 N = normalize(normal);

		// normal in view space to write in B-buffer
		vec4 Nreturn = normalize(mat4(u_matV * u_matM) * vec4(normal.xyz, 1.0));
		
		// second ray casting after surface penetration
		int numSteps2 = int(float(numSteps) * 0.2);
		vec3 pos2 = pos; // start second ray casting from isosurface
		vec4 accumAB = vec4(0.0);
		for (int i = 0; i < numSteps2 && accumAB.a < 1.0; ++i)
		{
			float intensity2 = texture(u_volumeTexture, pos2).r;
			intensity2 = maxNbhVal(u_volumeTexture, pos2);

			// read color from TF
			vec3 material2 = texture(u_lookupTexture, intensity2).rgb * u_useTF
				+ vec3(intensity2, intensity2, intensity2) * (1 - u_useTF);

			vec4 tfColor = vec4(material2.rgb, intensity2);
			tfColor.a = clamp(1.0 * intensity2, 0.0, 1.0);
			tfColor.a *= stepSize / u_transparency; // reduce the alpha when you accumulate too many layers
			accumAB.rgb += (tfColor.rgb * tfColor.a) * (1.0 - accumAB.a); // accumulate color (ponderated by reduced alpha) with a decreasing weight
			accumAB.a += tfColor.a * (1.0 - accumAB.a); //accumulate alpha with a decreasing weight			

			pos2 += stepSize * rayDir;
		}

		// Blinn-Phong illumination
		vec3 diffuseColor = accumAB.rgb * max(0.0, dot(N, L));

		vec4 vecV = normalize(mat4(u_matV* inverse(u_matM)) * vec4(pos.xyz, 1.0));
		vec3 vecH = normalize(L + vecV.xyz);
		vec3 specularColor = vec3(1.0, 1.0, 1.0) * specular_normalized(N, vecH, 64.0);

		if (u_useShadow)
		{
			diffuseColor *= (1.0 - shadowRay(pos, L, rayDir, stepSize));
			specularColor *= (1.0 - shadowRay(pos, L, rayDir, stepSize));
		}

		color.rgb = diffuseColor + u_ambientColor + specularColor;
		color.a = 1.0;

		// write model space position coords into G-buffer
		vec4 Preturn = u_matM * vec4(pos.rgb, 1.0);
		gPosition = vec4(Preturn.rgb, 1.0);

		// write normal into G-buffer
		gNormal = vec4(Nreturn.xyz, 1.0);

	}


	if(u_useGammaCorrec)
		color.rgb = linearToGamma(color.rgb);
	
	// write final color into G-buffer
	gColor = vec4(color.rgb, 1.0);

    frag_color = color;
}
