#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;
uniform mat4 u_MVP;


void main()
{
	gl_Position = u_MVP * vec4(a_Position, 1.0);
	v_TexCoord = a_TexCoord;	
};

#shader fragment
#version 330 core

out vec4 FragColor;
in vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main()
{
	FragColor = texture(u_Texture, v_TexCoord);
};