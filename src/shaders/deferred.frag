// Fragment shader
#version 330


// UNIFORMS
uniform sampler2D u_colorTex;
uniform sampler2D u_normalTex;
uniform sampler2D u_positionTex;
uniform sampler2D u_noiseTex;
uniform vec3 u_samples[64];
uniform mat4 u_matM;
uniform mat4 u_matV;
uniform mat4 u_matP;
uniform bool u_useAO;
uniform vec2 u_screenDims;
	
// INPUT	
in vec3 vert_uv;

// OUTPUT
out vec4 frag_color;


float compAO()
{
	int kernelSize = 40;
	float bias = 0.1;
	vec2 noiseScale = vec2(u_screenDims[0] * 0.2, u_screenDims[1] * 0.2);
	float radius = 1.0;
	float occlusion = 0.0;

	// read fragment 3D pos from G-buffer position texture
	vec3 fragPos = texture(u_positionTex, vert_uv.xy).xyz;
	// read fragment 3D normal from G-buffer normal texture
	vec3 normal = texture(u_normalTex, vert_uv.xy).rgb;

	// ignore fragment if normal or position is empty
	if (normal == vec3(0.0f) || fragPos == vec3(0.0f))
	{
		occlusion = 1.0;
	}
	else
	{
		// build random direction vector from noise texture
		vec3 randomVec = texture(u_noiseTex, vert_uv.xy * noiseScale).xyz;

		// build TBN matrix
		vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
		vec3 bitangent = cross(normal, tangent);
		mat3 TBN = mat3(tangent, bitangent, normal);


		for (int i = 0; i < kernelSize; ++i)
		{
			// get sample position
			vec3 samplePos = TBN * u_samples[i]; // from tangent to view-space
			samplePos = fragPos + samplePos * radius;

			vec4 offset = vec4(samplePos, 1.0);
			offset = u_matP * offset;    	  		// from view to clip-space
			offset.xyz /= offset.w;               	// perspective divide
			offset.xyz = offset.xyz * 0.5 + 0.5; 	// transform to range 0.0 - 1.0  

			float sampleDepth = texture(u_positionTex, offset.xy).z;

			float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
			// closer samples increase factor
			occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;

		}

		occlusion /= kernelSize;
		occlusion = pow(occlusion, 0.5);

	}

	return occlusion;
}


// MAIN
void main()
{

	// final color
	vec4 color = vec4(1.0f);
	color = texture(u_colorTex, vert_uv.xy);

	if (color.a == 0.0) { discard; }
	
	if (u_useAO)
	{
		float AO = compAO();
		color.rgb *= AO;
	}
		
	frag_color = color;
}

