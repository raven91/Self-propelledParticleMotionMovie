#version 330

in vec2 tex_coord;
out vec4 text_color;

uniform sampler2D tex;

void main()
{
	text_color = vec4(1.0f, 1.0f, 1.0f, texture(tex, tex_coord).r) * vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
