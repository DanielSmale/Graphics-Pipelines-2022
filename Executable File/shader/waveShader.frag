#version 460

in vec4 Position;
in vec3 Normal;
layout (location = 0) out vec4 FragColor;

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
 

vec3 blinnPhong(vec4 position, vec3 n);


void main() {
	
	vec3 color = blinnPhong(Position, Normal);
	
	FragColor = vec4(color, 1.0f);
}


vec3 blinnPhong(vec4 position, vec3 n)
{
	//ambient material
	vec3 ambient = Material.Ka * Light.La;


	//calculate diffuse material
	// vec3 s = vec3(normalize(Light.Position - vec4(position, 1.0f))); //find out s vector
		vec3 s = vec3(normalize(Light.Position - position)); //find out s vector

 	float sDotN = max(dot(s,n), 0); //calculate dot product between s & n
	vec3 diffuse = Material.Kd * sDotN;
 
	//calculate specular
	vec3 spec = vec3(0.0f);

	
	if(sDotN > 0.0f)
	{
		vec3 v = normalize(-position.xyz);
		vec3 h = normalize( v + s );
		spec = Material.Ks * pow(max(dot(h,n),0.0 ), Material.Shininess );
	}
	

	return ambient + Light.L * (diffuse + spec);
}
