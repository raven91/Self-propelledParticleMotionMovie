#version 330

in vec4 generic_coord;
out vec2 tex_coord;

void main()
{	
	gl_Position = vec4(generic_coord.xy, 0.0f, 1.0f);
	tex_coord = generic_coord.zw;
}