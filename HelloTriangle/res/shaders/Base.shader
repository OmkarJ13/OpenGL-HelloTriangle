#shader vertex
#version 330 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 colour;
out vec4 colour0;
uniform mat4 model;

void main()
{
	gl_Position = model * pos;
	colour0 = colour;
}

#shader fragment
#version 330 core

in vec4 colour0;
out vec4 colour;

void main()
{
	colour = colour0;
}