#version 460

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

layout (location = 0) out vec4 FragColor;

layout (location = 1) out vec3 PositionData;
layout (location = 2) out vec3 NormalData;
layout (location = 3) out vec3 ColorData;

uniform int DeferPass; // Pass number
layout(binding=1) uniform sampler2D PositionTex;
layout(binding=2) uniform sampler2D NormalTex;
layout(binding=3) uniform sampler2D ColorTex;

uniform struct LightInfo {
 vec4 Position; // Light position in eye coords.
 vec3 La; // Ambient light intensity
 vec3 L; // Diffuse and specular light intensity
}Light;


uniform struct MaterialInfo {
vec3 Ka; // Ambient reflectivity
vec3 Kd; // Diffuse reflectivity
vec3 Ks; // Specular reflectivity
float Shininess; // Specular shininess factor
}Material;


uniform mat4 ModelViewMatrix; 
uniform mat3 NormalMatrix; 
uniform mat4 MVP; 

vec3 blinnPhong(vec3 position, vec3 n, vec3 diffColor);

void pass1();
void pass2();


void main() {

if( DeferPass == 1 )
pass1();
 else if( DeferPass == 2)
pass2();
}

void pass1()
{
	// Store position, normal, and diffuse color in textures
	 PositionData = Position;
	 NormalData = normalize(Normal);
	 ColorData = Material.Kd;
}

void pass2()
{
 // Retrieve position and normal information from textures
	vec3 pos = vec3( texture( PositionTex, TexCoord ) );
	 vec3 norm = vec3( texture( NormalTex, TexCoord ) );
	 vec3 diffColor = vec3(texture(ColorTex, TexCoord) );
	FragColor = vec4( blinnPhong(pos,norm, diffColor), 1.0 );
}


vec3 blinnPhong(vec3 position, vec3 n, vec3 diffColor)
{
	//ambient material
	vec3 ambient = Material.Ka * Light.La;
	//ambient texture

	//calculate diffuse material
	vec3 s = vec3(normalize(Light.Position - vec4(Position, 1.0f))); //find out s vector
	float sDotN = max(dot(s,n), 0); //calculate dot product between s & n
	//vec3 diffuse = Material.Kd * sDotN;
 	vec3 diffuse = diffColor * sDotN;

	//calculate specular
	vec3 spec = vec3(0.0f);

	
	if(sDotN > 0.0f)
	{
		vec3 v = normalize(-position.xyz);
		vec3 h = normalize( v + s );
		spec = Material.Ks * pow(max(dot(h,n),0.0 ), Material.Shininess );
	}
	

	//return ambient + Light.L * (diffuse + spec);
	return diffuse;
	//return ambient;
	//return spec;
}