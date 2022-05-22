#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"

#include <glm/gtc/matrix_transform.hpp>
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

#include <sstream>
#include "helper/texture.h"


#include <GLFW/glfw3.h>
#include "helper/scenerunner.h"

SceneBasic_Uniform::SceneBasic_Uniform() : angle(0.0f), tPrev(0.0f), rotSpeed(glm::pi<float>() / 8.0f), plane(50.0f,
	50.0f, 1, 1), teapot(14, mat4(1.0f)), torus(0.7f * 1.5f, 0.3f * 1.5f, 50, 50),
	time(0), plane2(13.0f, 10.0f, 200, 2)
{

}
void SceneBasic_Uniform::initScene()
{

	compile();
	deferProg.use();

	glEnable(GL_DEPTH_TEST);

	//Deferred rendering
	float c = 1.5f;
	angle = glm::pi<float>() / 2.0f;

	// Array for quad
	GLfloat verts[] = {
	-1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f
	};

	GLfloat tc[] = {
	0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
	};

	// Set up the buffers
	unsigned int handle[2];
	glGenBuffers(2, handle);
	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts,
		GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);

	// Set up the vertex array object
	glGenVertexArrays(1, &deferredQuad);
	glBindVertexArray(deferredQuad);
	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(0); // Vertex position
	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(2); // Texture coordinates
	glBindVertexArray(0);
	setupDeferFBO();
	deferProg.setUniform("Light.L", vec3(1.0f));

	// *** Deferred Rendering END ***



	// *** HDR SetUp ***
//	compile();
	HDRProg.use();

	//projection = mat4(1.0f);

	setupHdrFBO();

	vec3 intense = vec3(5.0f);
	HDRProg.setUniform("Lights[0].L", intense);
	HDRProg.setUniform("Lights[1].L", intense);
	HDRProg.setUniform("Lights[2].L", intense);
	intense = vec3(0.2f);
	HDRProg.setUniform("Lights[0].La", intense);
	HDRProg.setUniform("Lights[1].La", intense);
	HDRProg.setUniform("Lights[2].La", intense);

	// Array for full-screen quad
	GLfloat Hdrverts[] = {
	-1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f
	};

	GLfloat HdrTc[] = {
	0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
	};

	// Set up the buffers
	unsigned int handle2[2];
	glGenBuffers(2, handle2);
	glBindBuffer(GL_ARRAY_BUFFER, handle2[0]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), Hdrverts, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, handle2[1]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), HdrTc, GL_STATIC_DRAW);

	// Set up the vertex array object
	glGenVertexArrays(1, &HdrQuad);
	glBindVertexArray(HdrQuad);
	glBindBuffer(GL_ARRAY_BUFFER, handle2[0]);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(0); // Vertex position
	glBindBuffer(GL_ARRAY_BUFFER, handle2[1]);
	glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(2); // Texture coordinates
	glBindVertexArray(0);

	HDRProg.setUniform("EdgeThreshold", 0.05f);
	HDRProg.setUniform("Light.L", vec3(1.0f));
	HDRProg.setUniform("Light.La", vec3(0.2f));

	// *** HDR setup END ***


	// *** Wave Setup Start ***
	waveProg.use();
	glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
	glEnable(GL_DEPTH_TEST);

	waveProg.setUniform("Light.L", vec3(1.0f, 1.0f, 1.0f));
	

	int bugger = 1760;
}

void SceneBasic_Uniform::compile()
{
	try {

		deferProg.compileShader("shader/basic_uniform.vert");
		deferProg.compileShader("shader/DeferShader.frag");
		deferProg.link();
		//deferProg.use();

		HDRProg.compileShader("shader/basic_uniform.vert");
		HDRProg.compileShader("shader/HDRShader.frag");
		HDRProg.link();
	//	HDRProg.use();

		waveProg.compileShader("shader/waveShader.vert");
		waveProg.compileShader("shader/waveShader.frag");
		waveProg.link();
		//waveProg.use();

	}
	catch (GLSLProgramException& e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::setupHdrFBO()
{
	HDRProg.use();

	GLuint depthBuf;

	// Create and bind the FBO
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

	// The depth buffer
	glGenRenderbuffers(1, &depthBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

	// The HDR color buffer
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &hdrTex);
	glBindTexture(GL_TEXTURE_2D, hdrTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Attach the images to the framebuffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER, depthBuf);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		hdrTex, 0);
	GLenum drawBuffers[] = { GL_NONE, GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(2, drawBuffers);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneBasic_Uniform::setupDeferFBO()
{
	deferProg.use();

	GLuint depthBuf, posTex, normTex, colorTex;

	// Create and bind the FBO
	glGenFramebuffers(1, &deferredFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, deferredFBO);

	// The depth buffer
	glGenRenderbuffers(1, &depthBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

	// Create the textures for position, normal and color
	createGBufTex(GL_TEXTURE1, GL_RGB32F, posTex); // Position
	createGBufTex(GL_TEXTURE2, GL_RGB32F, normTex); // Normal
	createGBufTex(GL_TEXTURE3, GL_RGB8, colorTex); // Color

	// Attach the textures to the framebuffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER, depthBuf);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		posTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
		normTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
		colorTex, 0);
	GLenum drawBuffers[] = { GL_NONE, GL_COLOR_ATTACHMENT0,
   GL_COLOR_ATTACHMENT1,
	GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(4, drawBuffers);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneBasic_Uniform::createGBufTex(GLenum texUnit, GLenum format, GLuint& texid)
{
	glActiveTexture(texUnit);
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);
	glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
}


void SceneBasic_Uniform::DeferPass1()
{
	deferProg.use();

	deferProg.setUniform("DeferPass", 1);
	glBindFramebuffer(GL_FRAMEBUFFER, deferredFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	view = glm::lookAt(vec3(7.0f * cos(angle), 4.0f, 7.0f * sin(angle)),
		vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	projection = glm::perspective(glm::radians(60.0f), (float)width / height,
		0.3f, 100.0f);

	deferProg.setUniform("Light.Position", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	deferProg.setUniform("Material.Kd", 0.9f, 0.9f, 0.9f);

	model = mat4(1.0f);
	model = glm::translate(model, vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));

	setMatrices(deferProg);

	teapot.render();

	deferProg.setUniform("Material.Kd", 0.4f, 0.4f, 0.4f);

	model = mat4(1.0f);
	model = glm::translate(model, vec3(0.0f, -0.75f, 0.0f));

	setMatrices(deferProg);
	plane.render();

	deferProg.setUniform("Light.Position", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	deferProg.setUniform("Material.Kd", 0.8f, 0.3f, 0.3f);

	model = mat4(1.0f);
	model = glm::translate(model, vec3(1.0f, 1.0f, 3.0f));
	model = glm::rotate(model, glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));

	setMatrices(deferProg);
	torus.render();
	glFinish();
}

void SceneBasic_Uniform::DeferPass2()
{
	deferProg.use();

	deferProg.setUniform("DeferPass", 2);
	// Revert to default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	view = mat4(1.0);
	model = mat4(1.0);
	projection = mat4(1.0);

	setMatrices(deferProg);

	// Render the quad
	glBindVertexArray(deferredQuad);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void SceneBasic_Uniform::HdrPass1()
{
	HDRProg.use();

	HDRProg.setUniform("HdrPass", 1);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	view = glm::lookAt(vec3(2.0f, 0.0f, 14.0f), vec3(0.0f, 0.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f));
	projection = glm::perspective(glm::radians(60.0f), (float)width / height,
		0.3f, 100.0f);

	drawScene();
}

void SceneBasic_Uniform::HdrPass2()
{
	HDRProg.use();

	HDRProg.setUniform("HdrPass", 2);
	// Revert to default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	view = mat4(1.0);
	model = mat4(1.0);
	projection = mat4(1.0);
	setMatrices(HDRProg);
	// Render the quad
	glBindVertexArray(HdrQuad);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void SceneBasic_Uniform::computeLogAveLuminance()
{
	HDRProg.use();

	int size = width * height;
	std::vector<GLfloat> texData(size * 3);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTex);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, texData.data());
	float sum = 0.0f;
	for (int i = 0; i < size; i++) {
		float lum = glm::dot(vec3(texData[i * 3 + 0], texData[i * 3 + 1],
			texData[i * 3 + 2]),
			vec3(0.2126f, 0.7152f, 0.0722f));
		sum += logf(lum + 0.00001f);
	}
	HDRProg.setUniform("AveLum", expf(sum / size));
}

void SceneBasic_Uniform::drawScene()
{
	HDRProg.use();

	vec3 intense = vec3(1.0f);
	HDRProg.setUniform("Lights[0].L", intense);
	HDRProg.setUniform("Lights[1].L", intense);
	HDRProg.setUniform("Lights[2].L", intense);
	vec4 lightPos = vec4(0.0f, 4.0f, 2.5f, 1.0f);
	lightPos.x = -7.0f;
	HDRProg.setUniform("Lights[0].Position", view * lightPos);
	lightPos.x = 0.0f;
	HDRProg.setUniform("Lights[1].Position", view * lightPos);
	lightPos.x = 7.0f;
	HDRProg.setUniform("Lights[2].Position", view * lightPos);
	HDRProg.setUniform("Material.Kd", 0.9f, 0.3f, 0.2f);
	HDRProg.setUniform("Material.Ks", 1.0f, 1.0f, 1.0f);
	HDRProg.setUniform("Material.Ka", 0.2f, 0.2f, 0.2f);
	HDRProg.setUniform("Material.Shininess", 100.0f);
	// The backdrop plane
	model = glm::rotate(mat4(1.0f), glm::radians(90.0f), vec3(1.0f, 0.0f,
		0.0f));
	setMatrices(HDRProg);
	plane.render();
	// The bottom plane
	model = glm::translate(mat4(1.0f), vec3(0.0f, -5.0f, 0.0f));
	setMatrices(HDRProg);
	plane.render();
	// Top plane
	model = glm::translate(mat4(1.0f), vec3(0.0f, 5.0f, 0.0f));
	model = glm::rotate(model, glm::radians(180.0f), vec3(1.0f, 0.0f, 0.0f));
	setMatrices(HDRProg);
	plane.render();
	HDRProg.setUniform("Material.Kd", vec3(0.4f, 0.9f, 0.4f));
	model = glm::translate(mat4(1.0f), vec3(-3.0f, -3.0f, 2.0f));
	setMatrices(HDRProg);
	//sphere.render();
	HDRProg.setUniform("Material.Kd", vec3(0.4f, 0.4f, 0.9f));
	model = glm::translate(mat4(1.0f), vec3(3.0f, -5.0f, 1.5f));
	model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
	setMatrices(HDRProg);
	teapot.render();

}

void SceneBasic_Uniform::update(float t)
{
	time = t;


	float deltaT = t - tPrev;
	if (tPrev == 0.0f)
		deltaT = 0.0f;
	tPrev = t;
	angle += rotSpeed * deltaT;
	if (angle > glm::two_pi<float>())
		angle -= glm::two_pi<float>();
}

void SceneBasic_Uniform::render()
{
	/*
	DeferPass1();
	DeferPass2();
	*/

	/*
	HdrPass1();
	computeLogAveLuminance();
	HdrPass2();
	*/
	
	
	std::cout << shaderSelection << endl;

	if (shaderSelection == 1)
	{
		std::cout << "Called defer render" << endl;
		std::cout << shaderSelection << endl;

		DeferPass1();
		DeferPass2();
	}


	if (shaderSelection == 2)
	{
		std::cout << "Called HDR render" << endl;
		std::cout << shaderSelection << endl;

		HdrPass1();
		computeLogAveLuminance();
		HdrPass2();
	}
	
	if (shaderSelection == 3) 
	{
		RenderWave();
	}
	
	
}

void SceneBasic_Uniform::RenderWave()
{
	waveProg.use();
	waveProg.setUniform("Time", time);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	view = glm::lookAt(vec3(2.0f, 10.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f));
	projection = glm::perspective(glm::radians(60.0f), (float)width / height, 0.3f, 100.0f);

	waveProg.setUniform("Material.Kd", 0.2f, 0.5f, 0.9f);
	waveProg.setUniform("Materal.Ks", 0.8f, 0.8f, 0.8f);
	waveProg.setUniform("Material.Ka", 0.2f, 0.5f, 0.9f);
	waveProg.setUniform("Material.Shininess", 100.0f);

	model = mat4(1.0f);
	model = glm::rotate(model, glm::radians(45.0f), vec3(0.0f, 0.0f, 1.0f));
	setMatrices(waveProg);
	plane2.render();
}

void SceneBasic_Uniform::setMatrices(GLSLProgram& prog) {

	mat4 mv = view * model;
	prog.setUniform("ModelViewMatrix", mv);

	prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));

	prog.setUniform("MVP", projection * mv);

}

void SceneBasic_Uniform::resize(int w, int h)
{
	glViewport(0, 0, w, h);
	width = w;
	height = h;
	projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);

}
