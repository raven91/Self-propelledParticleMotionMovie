#version 330

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in VertexShaderOut
{
	vec3 color;
	int trajectory_index;
} gs_in[1];

out GeometryShaderOut
{
	vec3 color;
	vec2 uv;
	flat int trajectory_index;
} gs_out;

void main()
{
	/*mat4 translate = mat4(1.0);
	translate[3] = vec4(-1.0, -1.0, 0.0, 1.0);
	
	mat4 scale = mat4(1.0);
	scale[0][0] = 2.0;
	scale[1][1] = 2.0;*/
	
	float w = 0.03f*1;
	float v_0 = 0.03f*1;
	
	vec3 x_0 = gl_in[0].gl_Position.xyz;
	vec3 v = vec3(0.0f, 1.0f, 0.0f);
	vec3 v_orth = vec3(1.0f, 0.0f, 0.0f);

	//if (v.x < -0.5f || v.x > 0.5f || v.y < -0.5f || v.y > 0.5f)
	//{
	//
	//}
	//else
	//{
		vec3 A = x_0 - v_0 * v / 2.0f - v_orth * w / 2.0f;
		vec3 B = x_0 - v_0 * v / 2.0f + v_orth * w / 2.0f;
		vec3 C = x_0 + v_0 * v / 2.0f + v_orth * w / 2.0f;
		vec3 D = x_0 + v_0 * v / 2.0f - v_orth * w / 2.0f;
		
		gs_out.color = gs_in[0].color;
		gs_out.trajectory_index = gs_in[0].trajectory_index;
		
		gs_out.uv = vec2(1, 1); gl_Position = vec4(C, 1.0f); EmitVertex();
		gs_out.uv = vec2(0, 1); gl_Position = vec4(D, 1.0f); EmitVertex();
		gs_out.uv = vec2(1, 0); gl_Position = vec4(B, 1.0f); EmitVertex();
		gs_out.uv = vec2(0, 0); gl_Position = vec4(A, 1.0f); EmitVertex();
		
		EndPrimitive();
	//}
}
