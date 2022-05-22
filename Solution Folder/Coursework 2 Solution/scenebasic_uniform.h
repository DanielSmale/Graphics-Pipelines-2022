#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"

#include "helper/plane.h"
#include "helper/objmesh.h"
#include "helper/cube.h"
#include "helper/teapot.h"
#include "helper/torus.h"
#include "helper/sphere.h"
#include <glm/glm.hpp>
#include"helper/random.h"

class SceneBasic_Uniform : public Scene
{
private:
	GLSLProgram prog, deferProg, HDRProg, waveProg;

	

	//HDR variables
	GLuint hdrFBO;
	GLuint HdrQuad;
	GLuint hdrTex, avgTex;
	

	//Deferred variables
	GLuint deferredFBO;
	GLuint deferredQuad;

 	Torus torus;
	Plane plane;
	Teapot teapot;
	//Sphere sphere;

	//Wave variables
	Random rand;

	Plane plane2;

	float angle, time, tPrev, rotSpeed;

	void RenderWave();

	void setMatrices(GLSLProgram& prog);

	void compile();
	//* HDR
	void setupHdrFBO();

	void HdrPass1();
	void HdrPass2();

	void computeLogAveLuminance();

	void drawScene();

	//* Defer
	void setupDeferFBO();

	void createGBufTex(GLenum, GLenum, GLuint&);

	void DeferPass1();
	void DeferPass2();

public:
    SceneBasic_Uniform();

    void initScene();
    void update(float t);
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H