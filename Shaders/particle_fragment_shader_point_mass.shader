#version 330

#define M_LN10 2.30258509299404568401799145468436421

uniform int kPointsPerParticle;

in GeometryShaderOut
{
	vec3 color;
	vec2 uv;
	flat int trajectory_index;
} fs_in;

out vec4 color_out_of_fragment_shader;

float linearstep(float lo, float hi, float x)
{
	return (clamp(x, lo, hi) - lo) / (hi - lo);
}

void main()
{
	vec2 dist_vec = fs_in.uv - vec2(0.5f, 0.5f);
	float dist_squared = dot(dist_vec, dist_vec);
	float clipping_radius = 0.1f;
	
	if (dist_squared > clipping_radius * clipping_radius)
	{
		discard;
	}
	else
	{
        color_out_of_fragment_shader = vec4(fs_in.color, 1.0f);
	}
}
