#version 150

in vec4 position;

void main()
{
	mat4 translate = mat4(1.0);
	translate[3] = vec4(-1.0, -1.0, 0.0, 1.0);
	
	mat4 scale = mat4(1.0);
	scale[0][0] = 2.0;
	scale[1][1] = 2.0;
	
	//scaled down
	/*mat4 translate = mat4(1.0);
	translate[3] = vec4(-0.9, -0.9, 0.0, 1.0);
	
	mat4 scale = mat4(1.0);
	scale[0][0] = 1.8;
	scale[1][1] = 1.8;*/

	gl_Position = translate * scale * position;
}