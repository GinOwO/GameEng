#version 460 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

out vec2 texCoord0;

uniform mat4 MVP; // Model View Projection

void main()
{
	gl_Position = MVP * vec4(position, 1.0);
	texCoord0 = texCoord;
}
