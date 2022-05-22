#version 460

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal; 
layout(location = 2) in vec2 VertexTexCoord;

out vec4 Position;
out vec3 Normal;
out vec2 TexCoord;

uniform float Time;

//wave parameters
uniform float Freq = 2.5;
uniform float Velocity = 2.5;
uniform float Amp = 0.6;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;


void main() { 
vec4 pos = vec4(VertexPosition,1.0);

	//translate vectices on y coordinate
	float u = Freq * pos.x - Velocity * Time;
	pos.y = Amp * sin(u);

	//compute the normal vector
	vec3 n = vec3(0.0);
	n.xy = normalize(vec2(cos(u),1.0));

	//pass values to the fragment shader
	Position = ModelViewMatrix * pos;

	Normal = NormalMatrix * n;

	TexCoord = VertexTexCoord;
	
	gl_Position = MVP * pos;

}
