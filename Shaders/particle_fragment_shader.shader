#version 330

#define M_LN10 2.30258509299404568401799145468436421

uniform int kPointsPerParticle;

in GeometryShaderOut
{
	vec3 color[2];
	vec2 uv;
	flat int trajectory_index[2];
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
	float clipping_radius = 0.5f;
	
	if ((fs_in.trajectory_index[1] == kPointsPerParticle - 1) && (fs_in.uv.y > 0.5f) && (dist_squared > clipping_radius * clipping_radius))
	{
		color_out_of_fragment_shader = mix(vec4(fs_in.color[0], float(fs_in.trajectory_index[0]) / float(kPointsPerParticle - 1)), vec4(fs_in.color[1], float(fs_in.trajectory_index[1]) / float(kPointsPerParticle - 1)), fs_in.uv.y);
//		color_out_of_fragment_shader[3] = exp(- 3.0f * dist_squared);
		discard;
	}
	else if ((fs_in.trajectory_index[0] == 0) && (fs_in.uv.y < 0.5f) && (dist_squared > clipping_radius * clipping_radius))
	{
		discard;
	}
	else
	{
		float alpha_0 = float(fs_in.trajectory_index[0]) / float(kPointsPerParticle - 1);
		float alpha_1 = float(fs_in.trajectory_index[1]) / float(kPointsPerParticle - 1);
// 		float alpha_0 = sqrt(1.0f - (1.0f - float(fs_in.trajectory_index[0]) / float(kPointsPerParticle - 1)) * (1.0f - float(fs_in.trajectory_index[0]) / float(kPointsPerParticle - 1)));
// 		float alpha_1 = sqrt(1.0f - (1.0f - float(fs_in.trajectory_index[1]) / float(kPointsPerParticle - 1)) * (1.0f - float(fs_in.trajectory_index[1]) / float(kPointsPerParticle - 1)));
// 		float alpha_0 = 0.8f * log(float(fs_in.trajectory_index[0]) / float(kPointsPerParticle - 1)) / M_LN10 + 1.0f;
// 		float alpha_1 = 0.8f * log(float(fs_in.trajectory_index[1]) / float(kPointsPerParticle - 1)) / M_LN10 + 1.0f;
// 		float alpha_0 = pow(float(fs_in.trajectory_index[0]) / float(kPointsPerParticle - 1), 2.0f);
// 		float alpha_1 = pow(float(fs_in.trajectory_index[1]) / float(kPointsPerParticle - 1), 2.0f);
		
       color_out_of_fragment_shader = mix(vec4(fs_in.color[0], alpha_0), vec4(fs_in.color[1], alpha_1), fs_in.uv.y);
// 		color_out_of_fragment_shader = vec4(1.0f, 1.0f, 1.0f, 1.0f);
//        color_out_of_fragment_shader = mix(vec4(1.0f, 1.0f, 1.0f, alpha_0), vec4(1.0f, 1.0f, 1.0f, alpha_1), fs_in.uv.y);
	}
	
//    float radius = 0.5f;
//    vec2 pos = gl_PointCoord - radius;
//    float dist_squared = dot(pos, pos);
	
//	float dist_squared = dot(fs_in.uv, fs_in.uv);
//	dist_squared = exp(dist_squared * -1.2f);

//    if (dist_squared > radius * radius)
//    {
//        discard;
//    }
//    else
//    {
//		color_out_of_fragment_shader = mix(color_out_of_vertex_shader_into_fragment_shader, vec4(1.0f, 1.0f, 1.0f, 0.0f), smoothstep(0.2f*0.2f, 0.7f*0.7f, dist_squared));
//    }
}
