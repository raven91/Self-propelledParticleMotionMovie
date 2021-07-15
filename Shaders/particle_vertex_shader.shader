#version 330

in vec2 position;
in vec3 color_into_vertex_shader;
in int trajectory_index;

uniform float x_size;
uniform float y_size;
uniform float x_shift;
uniform float y_shift;
uniform float z_scale;

out VertexShaderOut
{
	vec3 color;
	int trajectory_index;
} vs_out;

void main()
{
	mat4 translate = mat4(1.0);
	translate[3] = vec4(-1.0, -1.0, 0.0, 1.0);
	
	mat4 scale = mat4(1.0);
	scale[0][0] = 2.0 / x_size;
	scale[1][1] = 2.0 / y_size;

	mat4 external_translate = mat4(1.0);
	external_translate[3] = vec4(x_shift, y_shift, 0.0f, 1.0f);
	mat4 external_scale = mat4(1.0);
   	external_scale[0][0] = z_scale;
   	external_scale[1][1] = z_scale;
	
	//scaled down
	/*mat4 translate = mat4(1.0);
	translate[3] = vec4(-0.9, -0.9, 0.0, 1.0);
	
	mat4 scale = mat4(1.0);
	scale[0][0] = 1.8;
	scale[1][1] = 1.8;*/

	gl_Position = external_translate * external_scale * translate * scale * vec4(position, 0.0f, 1.0f);
	gl_PointSize = 1.0f;
	
	vs_out.color = color_into_vertex_shader;
	vs_out.trajectory_index = trajectory_index;
}