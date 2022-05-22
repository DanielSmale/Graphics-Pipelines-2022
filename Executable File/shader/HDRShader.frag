#version 460

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec3 HdrColor;


layout( binding=0 ) uniform sampler2D RenderTex;

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

uniform float EdgeThreshold;
uniform int HdrPass; // pass number

uniform float AveLum;


const vec3 lum = vec3(0.2126, 0.7152, 0.0722);
layout(binding=0) uniform sampler2D HdrTex;


// XYZ/RGB conversion matrices from:
// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html

uniform mat3 rgb2xyz = mat3(
 0.4124564, 0.2126729, 0.0193339,
 0.3575761, 0.7151522, 0.1191920,
 0.1804375, 0.0721750, 0.9503041 );

uniform mat3 xyz2rgb = mat3(
 3.2404542, -0.9692660, 0.0556434,
 -1.5371385, 1.8760108, -0.2040259,
 -0.4985314, 0.0415560, 1.0572252 );

uniform float Exposure = 0.35;

uniform float White = 0.928;

uniform bool DoToneMap = true;


uniform mat4 ModelViewMatrix; 
uniform mat3 NormalMatrix; 
uniform mat4 MVP; 

vec3 blinnPhong(vec3 position, vec3 n);

void pass1();
void pass2();

float luminance( vec3 color );

void main() {

	if( HdrPass == 1 )
		pass1();
	else if( HdrPass == 2)
		pass2();
}

void pass1()
{
 vec3 n = normalize(Normal);
 // Compute shading and store result in high-res framebuffer
 HdrColor = vec3(0.0);
 for( int i = 0; i < 3; i++)
 HdrColor += blinnPhong(Position, n);  // blinnPhong(pos,n, i)
}


// This pass computes the sum of the luminance of all pixels
void pass2()
{
 // Retrieve high-res color from texture
 vec4 color = texture( HdrTex, TexCoord );

 // Convert to XYZ
 vec3 xyzCol = rgb2xyz * vec3(color);

 // Convert to xyY
 float xyzSum = xyzCol.x + xyzCol.y + xyzCol.z;
 vec3 xyYCol = vec3( xyzCol.x / xyzSum, xyzCol.y / xyzSum, xyzCol.y);

 // Apply the tone mapping operation to the luminance (xyYCol.z or xyzCol.y)
 float L = (Exposure * xyYCol.z) / AveLum;
 L = (L * ( 1 + L / (White * White) )) / ( 1 + L );

 // Using the new luminance, convert back to XYZ
 xyzCol.x = (L * xyYCol.x) / (xyYCol.y);
 xyzCol.y = L;
 xyzCol.z = (L * (1 - xyYCol.x - xyYCol.y))/xyYCol.y;

 float gamma = 2.2f;

 // Convert back to RGB and send to output buffer
 if( DoToneMap )
{ 
	FragColor = vec4( xyz2rgb * xyzCol, 1.0);
}
 else
 {
// 	FragColor = vec4(pow(color,vec3(1.0/gamma)),1.0);
	FragColor = color;
}
}

float luminance( vec3 color )
{
	return dot(lum,color);
}

vec3 blinnPhong(vec3 position, vec3 n)
{
	//ambient material
	vec3 ambient = Material.Ka * Light.La;
	//ambient texture

	//calculate diffuse material
	vec3 s = vec3(normalize(Light.Position - vec4(Position, 1.0f))); //find out s vector
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
