/*
 * Example two meshes and two shaders (could also be used for Program 2)
 * includes modifications to shape and initGeom in preparation to load
 * multi shape objects 
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "Model.h"
#include "Object.h"
#include "Scene.h"
#include "Spline.h"
#include "MatrixStack.h"
#include "WindowManager.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

constexpr auto PI = 3.1415926535;
constexpr auto SQRT_2_OVER_2 = 0.707106781;

using namespace std;
using namespace glm;

class Application : public EventCallbacks {

public:

	WindowManager * windowManager = nullptr;

	std::shared_ptr<Program> normalProg;
	std::shared_ptr<Program> blinnPhongProg;
	std::shared_ptr<Program> texCoordProg;
	std::shared_ptr<Program> texBlinnPhongProg;

	std::shared_ptr<Material> s_normal;
	std::shared_ptr<Material> s_texCoord;
	std::shared_ptr<Material> s_white;
	std::shared_ptr<Material> s_black;
	std::shared_ptr<Material> s_blueWater;
	std::shared_ptr<Material> s_redWater;
	std::shared_ptr<Material> s_metal;
	std::shared_ptr<Material> s_marble;
	std::shared_ptr<Material> s_skybox;
	std::shared_ptr<Material> s_rock;

	shared_ptr<Texture> t_skybox;
	shared_ptr<Texture> t_rock;

	shared_ptr<Model> m_uvSphere;
	shared_ptr<Model> m_uvSphereHires;
	shared_ptr<Model> m_icosphere;
	shared_ptr<Model> m_icosphereHires;
	shared_ptr<Model> m_island;
	shared_ptr<Model> m_chain;
	shared_ptr<Model> m_pillar;
	shared_ptr<Model> m_clawBase;
	shared_ptr<Model> m_clawPart;
	shared_ptr<Model> m_rock1;
	shared_ptr<Model> m_rock2;
	shared_ptr<Model> m_rock3;

	shared_ptr<BlackHoleMap> blackHole;

	shared_ptr<Scene> scene;

	shared_ptr<Object> planetParent;
	shared_ptr<Object> light1Parent;
	shared_ptr<Object> light2Parent;
	shared_ptr<Object> player;
	shared_ptr<MeshObject> claw1;
	shared_ptr<MeshObject> claw2;
	shared_ptr<MeshObject> claw3;
	shared_ptr<MeshObject> claw4;
	shared_ptr<MeshObject> claw5;
	shared_ptr<MeshObject> claw6;
	shared_ptr<MeshObject> claw7;
	shared_ptr<MeshObject> claw8;
	shared_ptr<CameraObject> fpsCamera;

	int windowWidth;
	int windowHeight;

	double lastMouseX = 0.0;
	double lastMouseY = 0.0;
	double mouseSensitivity = 0.01;
	bool controllingFpsCamera = true;
	bool playerCollisions = true;
	bool freeCam = false;
	bool blackHoleActive = false;

	bool leftPressed, rightPressed, upPressed, downPressed, risePressed, fallPressed;
	double inputX = 0.0;
	double inputY = 0.0;
	double inputZ = 0.0;

	double timeSinceStart = 0.0;
	double deltaTime = 0.0;


	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
		{
			toggleFpsCameraControl(window, false);
		}

		if (key == GLFW_KEY_A || key == GLFW_KEY_LEFT)
		{
			leftPressed = action != GLFW_RELEASE;
		}
		if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT)
		{
			rightPressed = action != GLFW_RELEASE;
		}
		if (key == GLFW_KEY_W || key == GLFW_KEY_UP)
		{
			upPressed = action != GLFW_RELEASE;
		}
		if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN)
		{
			downPressed = action != GLFW_RELEASE;
		}
		if (key == GLFW_KEY_E)
		{
			risePressed = action != GLFW_RELEASE;
		}
		if (key == GLFW_KEY_Q)
		{
			fallPressed = action != GLFW_RELEASE;
		}
		inputX = (leftPressed ? -1.0 : 0.0) + (rightPressed ? 1.0 : 0.0);
		inputY = (downPressed ? -1.0 : 0.0) + (upPressed ? 1.0 : 0.0);
		inputZ = (fallPressed ? -1.0 : 0.0) + (risePressed ? 1.0 : 0.0);

		if (key == GLFW_KEY_F && action == GLFW_PRESS)
		{
			freeCam = !freeCam;
			playerCollisions = !freeCam;
		}
		if (key == GLFW_KEY_B && action == GLFW_PRESS)
		{
			blackHoleActive = !blackHoleActive;
			m_uvSphere->useBlackHole = blackHoleActive;
			m_uvSphereHires->useBlackHole = blackHoleActive;
			m_icosphere->useBlackHole = blackHoleActive;
			m_icosphereHires->useBlackHole = blackHoleActive;
			m_island->useBlackHole = blackHoleActive;
			m_chain->useBlackHole = blackHoleActive;
			m_pillar->useBlackHole = blackHoleActive;
			m_clawBase->useBlackHole = blackHoleActive;
			m_clawPart->useBlackHole = blackHoleActive;
		}

		if (key == GLFW_KEY_Z && action == GLFW_PRESS)
		{
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE)
		{
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			toggleFpsCameraControl(window, true);
		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
	{
	}

	void toggleFpsCameraControl(GLFWwindow* window, bool inControl)
	{
		if (inControl)
		{
			controllingFpsCamera = true;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else
		{
			controllingFpsCamera = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	void mousePosCallback(GLFWwindow* window, double newX, double newY)
	{
		float deltaX = (float)(newX - lastMouseX);
		float deltaY = (float)(newY - lastMouseY);

		if (controllingFpsCamera)
		{
			float newXRot = fpsCamera->rotation.x - deltaY * mouseSensitivity;
			if (newXRot > PI / 2 - 0.1)
			{
				newXRot = PI / 2 - 0.1;
			}
			if (newXRot < -PI / 2 + 0.1)
			{
				newXRot = -PI / 2 + 0.1;
			}
			float newYRot = fpsCamera->rotation.y - deltaX * mouseSensitivity;
			fpsCamera->rotation = vec3(newXRot, newYRot, 0);
		}

		lastMouseX = newX;
		lastMouseY = newY;
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		windowWidth = width;
		windowHeight = height;
		glViewport(0, 0, width, height);
	}

	void init()
	{
		glfwGetFramebufferSize(windowManager->getHandle(), &windowWidth, &windowHeight);
		toggleFpsCameraControl(windowManager->getHandle(), true);
	}

	void initBasicShader(shared_ptr<Program> program)
	{
		program->setVerbose(true);
		program->init();
		program->addUniform("P");
		program->addUniform("V");
		program->addUniform("M");
		program->addUniform("blackHoleMesh");
		program->addUniform("blackHolePosition");
		program->addUniform("blackHoleSize");
		program->addUniform("blackHoleVertexMin");
		program->addUniform("blackHoleVertexMax");
		program->addUniform("blackHoleObserverMin");
		program->addUniform("blackHoleObserverMax");
		program->addUniform("useBlackHole");
		program->addUniform("blackHoleSecondary");
		program->addUniform("freeCam");
		program->addUniform("flipNormals");
		program->addUniform("dirLightDirections");
		program->addUniform("dirLightIntensities");
		program->addUniform("pointLightPositions");
		program->addUniform("pointLightIntensities");
		program->addAttribute("vertPos");
		program->addAttribute("vertNor");
		program->addAttribute("vertTex");
	}

	void initShaders(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.0f, .0f, .0f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		normalProg = make_shared<Program>();
		normalProg->setShaderNames(resourceDirectory + "/shaders/simple_vert.glsl", resourceDirectory + "/shaders/normal_frag.glsl");
		initBasicShader(normalProg);

		texCoordProg = make_shared<Program>();
		texCoordProg->setShaderNames(resourceDirectory + "/shaders/simple_vert.glsl", resourceDirectory + "/shaders/tex_coord_frag.glsl");
		initBasicShader(texCoordProg);

		blinnPhongProg = make_shared<Program>();
		blinnPhongProg->setShaderNames(resourceDirectory + "/shaders/simple_vert.glsl", resourceDirectory + "/shaders/blinn_phong_frag.glsl");
		initBasicShader(blinnPhongProg);
		blinnPhongProg->addUniform("matAmb");
		blinnPhongProg->addUniform("matDif");
		blinnPhongProg->addUniform("matSpec");
		blinnPhongProg->addUniform("specIntensity");

		texBlinnPhongProg = make_shared<Program>();
		texBlinnPhongProg->setShaderNames(resourceDirectory + "/shaders/simple_vert.glsl", resourceDirectory + "/shaders/tex_blinn_phong_frag.glsl");
		initBasicShader(texBlinnPhongProg);
		texBlinnPhongProg->addUniform("Texture0");
		texBlinnPhongProg->addUniform("amb");
		texBlinnPhongProg->addUniform("dif");
		texBlinnPhongProg->addUniform("spec");
		texBlinnPhongProg->addUniform("specIntensity");

		t_skybox = make_shared<Texture>();
		t_skybox->setFilename(resourceDirectory + "/textures/skybox.jpeg");
		t_skybox->init();
		t_skybox->setUnit(0);
		t_skybox->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		t_rock = make_shared<Texture>();
		t_rock->setFilename(resourceDirectory + "/textures/rock.jpg");
		t_rock->init();
		t_rock->setUnit(0);
		t_rock->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		s_normal = std::make_shared<Material>(0);
		s_texCoord = std::make_shared<Material>(1);
		s_white = std::make_shared<BlinnPhongMaterial>(
			2,
			vec3(1.0, 1.0, 1.0),
			vec3(0.0, 0.0, 0.0),
			vec3(0.0, 0.0, 0.0),
			30.0
		);
		s_black = std::make_shared<BlinnPhongMaterial>(
			2,
			vec3(0.0, 0.0, 0.0),
			vec3(0.0, 0.0, 0.0),
			vec3(0.0, 0.0, 0.0),
			30.0
		);
		s_blueWater = std::make_shared<BlinnPhongMaterial>(
			2,
			vec3(0.1, 0.1, 0.2),
			vec3(0.3, 0.4, 0.7),
			vec3(1.0, 1.0, 1.0),
			40.0
		);
		s_redWater = std::make_shared<BlinnPhongMaterial>(
			2,
			vec3(0.2, 0.1, 0.1),
			vec3(0.3, 0.0, 0.0),
			vec3(1.0, 0.5, 0.5),
			100.0
		);
		s_metal = std::make_shared<BlinnPhongMaterial>(
			2,
			vec3(0.05, 0.05, 0.07),
			vec3(0.4, 0.4, 0.5),
			vec3(1.0, 1.0, 1.0),
			50.0
		);
		s_marble = std::make_shared<BlinnPhongMaterial>(
			2,
			vec3(.4),
			vec3(1.0),
			vec3(1.0),
			20.0
		);
		s_skybox = std::make_shared<TexBlinnPhongMaterial>(
			3,
			t_skybox,
			1.0,
			0.0,
			0.0,
			5.0
		);
		s_rock = std::make_shared<TexBlinnPhongMaterial>(
			3,
			t_rock,
			0.1,
			0.7,
			0.3,
			5.0
		);
	}

	void initGeom(const std::string& resourceDirectory)
	{
		m_uvSphere = make_shared<Model>(resourceDirectory + "/meshes/UVSphere.obj");
		m_uvSphereHires = make_shared<Model>(resourceDirectory + "/meshes/UVSphereHires.obj");
		m_icosphere = make_shared<Model>(resourceDirectory + "/meshes/Icosphere.obj");
		m_icosphereHires = make_shared<Model>(resourceDirectory + "/meshes/IcosphereHires.obj");
		m_island = make_shared<Model>(resourceDirectory + "/meshes/Island.obj");
		m_chain = make_shared<Model>(resourceDirectory + "/meshes/Chain.obj");
		m_pillar = make_shared<Model>(resourceDirectory + "/meshes/Pillar.obj");
		m_clawBase = make_shared<Model>(resourceDirectory + "/meshes/ClawBase.obj");
		m_clawPart = make_shared<Model>(resourceDirectory + "/meshes/ClawPart.obj");
		m_rock1 = make_shared<Model>(resourceDirectory + "/meshes/Rock1.obj");
		m_rock2 = make_shared<Model>(resourceDirectory + "/meshes/Rock2.obj");
		m_rock3 = make_shared<Model>(resourceDirectory + "/meshes/Rock3.obj");
		m_uvSphere->useBlackHole = blackHoleActive;
		m_uvSphereHires->useBlackHole = blackHoleActive;
		m_icosphere->useBlackHole = blackHoleActive;
		m_icosphereHires->useBlackHole = blackHoleActive;
		m_island->useBlackHole = blackHoleActive;
		m_chain->useBlackHole = blackHoleActive;
		m_pillar->useBlackHole = blackHoleActive;
		m_clawBase->useBlackHole = blackHoleActive;
		m_clawPart->useBlackHole = blackHoleActive;
	}

	void initBlackHole(const std::string& resourceDirectory)
	{
		blackHole = make_shared<BlackHoleMap>();
		blackHole->size = 0.4f;
		blackHole->position = vec3(0, 2.5, 0);
		blackHole->loadFromFile(resourceDirectory + "/blackhole/blackhole_128_32_32_64_32.txt");
		cout << "Black hole resolution: " << blackHole->vrResolution << ", " << blackHole->vPhiResolution << ", " << blackHole->orResolution << endl;
		blackHole->sendToGPU();
	}

	void initScene()
	{
		scene = make_shared<Scene>();

		scene->addShaderProgram(normalProg);
		scene->addShaderProgram(texCoordProg);
		scene->addShaderProgram(blinnPhongProg);
		scene->addShaderProgram(texBlinnPhongProg);

		scene->blackHole = blackHole;
		blackHole->sendToGPU();
		blackHole->textureUnit = 1;

		// planets
		planetParent = make_shared<Object>(scene);
		planetParent->translation = vec3(0, 2.5, 0);
		scene->addObject(planetParent);

		auto planet1Object = make_shared<MeshObject>(scene, m_icosphere, s_blueWater);
		planet1Object->translation = vec3(2, 0, 0);
		planet1Object->scale = vec3(0.2);
		planet1Object->setParent(planetParent);
		planetParent->addChild(planet1Object);
		scene->addObject(planet1Object);

		auto planet2Object = make_shared<MeshObject>(scene, m_icosphere, s_redWater);
		planet2Object->translation = vec3(-2, 0, 0);
		planet2Object->scale = vec3(0.2);
		planet2Object->setParent(planetParent);
		planetParent->addChild(planet2Object);
		scene->addObject(planet2Object);

		// first light
		auto light1Angler = make_shared<Object>(scene);
		light1Angler->translation = vec3(0.0, 2.5, 0.0);
		light1Angler->rotation = vec3(0.5, 0, 0);
		scene->addObject(light1Angler);

		light1Parent = make_shared<Object>(scene);
		light1Parent->setParent(light1Angler);
		light1Angler->addChild(light1Parent);
		scene->addObject(light1Parent);

		auto light1Object = make_shared<PointLightObject>(scene, 5.0);
		light1Object->translation = vec3(2.5, 0, 0);
		light1Object->setParent(light1Parent);
		light1Parent->addChild(light1Object);
		scene->addObject(light1Object);

		auto light1Marker = make_shared<MeshObject>(scene, m_icosphere, s_white);
		light1Marker->scale = vec3(0.2f);
		light1Marker->setParent(light1Object);
		light1Object->addChild(light1Marker);
		scene->addObject(light1Marker);

		// second light
		auto light2Angler = make_shared<Object>(scene);
		light2Angler->translation = vec3(0.0, 2.5, 0.0);
		light2Angler->rotation = vec3(-0.5, 0, 0);
		scene->addObject(light2Angler);

		light2Parent = make_shared<Object>(scene);
		light2Parent->setParent(light2Angler);
		light2Angler->addChild(light2Parent);
		scene->addObject(light2Parent);

		auto light2Object = make_shared<PointLightObject>(scene, 5.0);
		light2Object->translation = vec3(-2.5, 0, 0);
		light2Object->setParent(light2Parent);
		light2Parent->addChild(light2Object);
		scene->addObject(light2Object);

		auto light2Marker = make_shared<MeshObject>(scene, m_icosphere, s_white);
		light2Marker->scale = vec3(0.2f);
		light2Marker->setParent(light2Object);
		light2Object->addChild(light2Marker);
		scene->addObject(light2Marker);

		// player related
		player = make_shared<Object>(scene);
		player->translation = vec3(0, 0.05, 5);
		scene->addObject(player);

		fpsCamera = make_shared<CameraObject>(scene, 45.0f, 1.0f, 0.01f, 400.0f);
		fpsCamera->translation = vec3(0, 1.5, 0);
		player->addChild(fpsCamera);
		fpsCamera->setParent(player);
		scene->addObject(fpsCamera);
		scene->activeCamera = fpsCamera;

		// claw
		auto clawBase = make_shared<MeshObject>(scene, m_clawBase, s_metal);
		clawBase->scale = vec3(0.5);
		scene->addObject(clawBase);

		claw1 = make_shared<MeshObject>(scene, m_clawPart, s_metal);
		claw1->translation = vec3(0.5, 0, 0);
		claw1->rotation = vec3(0, 0, 0);
		claw1->setParent(clawBase);
		clawBase->addChild(claw1);
		scene->addObject(claw1);

		claw2 = make_shared<MeshObject>(scene, m_clawPart, s_metal);
		claw2->translation = vec3(1, 1, 0);
		claw2->rotation = vec3(0, 0, PI / 2);
		claw2->setParent(claw1);
		claw1->addChild(claw2);
		scene->addObject(claw2);

		claw3 = make_shared<MeshObject>(scene, m_clawPart, s_metal);
		claw3->translation = vec3(0, 0, -0.5);
		claw3->rotation = vec3(0, PI / 2, 0);
		claw3->setParent(clawBase);
		clawBase->addChild(claw3);
		scene->addObject(claw3);

		claw4 = make_shared<MeshObject>(scene, m_clawPart, s_metal);
		claw4->translation = vec3(1, 1, 0);
		claw4->rotation = vec3(0, 0, PI / 2);
		claw4->setParent(claw3);
		claw3->addChild(claw4);
		scene->addObject(claw4);

		claw5 = make_shared<MeshObject>(scene, m_clawPart, s_metal);
		claw5->translation = vec3(-0.5, 0, 0);
		claw5->rotation = vec3(0, PI, 0);
		claw5->setParent(clawBase);
		clawBase->addChild(claw5);
		scene->addObject(claw5);

		claw6 = make_shared<MeshObject>(scene, m_clawPart, s_metal);
		claw6->translation = vec3(1, 1, 0);
		claw6->rotation = vec3(0, 0, PI / 2);
		claw6->setParent(claw5);
		claw5->addChild(claw6);
		scene->addObject(claw6);

		claw7 = make_shared<MeshObject>(scene, m_clawPart, s_metal);
		claw7->translation = vec3(0, 0, 0.5);
		claw7->rotation = vec3(0, 3 * PI / 2, 0);
		claw7->setParent(clawBase);
		clawBase->addChild(claw7);
		scene->addObject(claw7);

		claw8 = make_shared<MeshObject>(scene, m_clawPart, s_metal);
		claw8->translation = vec3(1, 1, 0);
		claw8->rotation = vec3(0, 0, PI / 2);
		claw8->setParent(claw7);
		claw7->addChild(claw8);
		scene->addObject(claw8);

		// scene
		auto island = make_shared<MeshObject>(scene, m_island, s_rock);
		island->scale = vec3(10);
		scene->addObject(island);


		auto pillar1 = make_shared<MeshObject>(scene, m_pillar, s_marble);
		pillar1->translation = vec3(0, 0, 8);
		pillar1->scale = vec3(2);
		scene->addObject(pillar1);

		auto pillar2 = make_shared<MeshObject>(scene, m_pillar, s_marble);
		pillar2->translation = vec3(8 * SQRT_2_OVER_2, 0, 8 * SQRT_2_OVER_2);
		pillar2->rotation = vec3(0, PI / 4, 0);
		pillar2->scale = vec3(2);
		scene->addObject(pillar2);

		auto pillar3 = make_shared<MeshObject>(scene, m_pillar, s_marble);
		pillar3->translation = vec3(8, 0, 0);
		pillar3->scale = vec3(2);
		scene->addObject(pillar3);

		auto pillar4 = make_shared<MeshObject>(scene, m_pillar, s_marble);
		pillar4->translation = vec3(8 * SQRT_2_OVER_2, 0, -8 * SQRT_2_OVER_2);
		pillar4->rotation = vec3(0, PI / 4, 0);
		pillar4->scale = vec3(2);
		scene->addObject(pillar4);

		auto pillar5 = make_shared<MeshObject>(scene, m_pillar, s_marble);
		pillar5->translation = vec3(0, 0, -8);
		pillar5->scale = vec3(2);
		scene->addObject(pillar5);

		auto pillar6 = make_shared<MeshObject>(scene, m_pillar, s_marble);
		pillar6->translation = vec3(-8 * SQRT_2_OVER_2, 0, -8 * SQRT_2_OVER_2);
		pillar6->rotation = vec3(0, PI / 4, 0);
		pillar6->scale = vec3(2);
		scene->addObject(pillar6);

		auto pillar7 = make_shared<MeshObject>(scene, m_pillar, s_marble);
		pillar7->translation = vec3(-8, 0, 0);
		pillar7->scale = vec3(2);
		scene->addObject(pillar7);

		auto pillar8 = make_shared<MeshObject>(scene, m_pillar, s_marble);
		pillar8->translation = vec3(-8 * SQRT_2_OVER_2, 0, 8 * SQRT_2_OVER_2);
		pillar8->rotation = vec3(0, PI / 4, 0);
		pillar8->scale = vec3(2);
		scene->addObject(pillar8);

		auto chain1 = make_shared<MeshObject>(scene, m_chain, s_metal);
		chain1->translation = vec3(1.35, -0.6, 0);
		chain1->rotation = vec3(0, PI, 0);
		chain1->setParent(pillar5);
		pillar5->addChild(chain1);
		scene->addObject(chain1);

		auto chain2 = make_shared<MeshObject>(scene, m_chain, s_metal);
		chain2->translation = vec3(-1.35, -0.6, 0);
		chain2->rotation = vec3(0, 0, 0);
		chain2->setParent(pillar6);
		pillar6->addChild(chain2);
		scene->addObject(chain2);

		auto skybox = make_shared<MeshObject>(scene, m_uvSphereHires, s_skybox);
		skybox->scale = vec3(blackHole->size * blackHole->vrMax);
		skybox->rotation = vec3(0, 0, 0.8);
		scene->addObject(skybox);
	}

	void update() {
		scene->evaluateAllGlobalTransforms();
		
		planetParent->rotation = vec3(0, timeSinceStart * 0.3, 0);
		light1Parent->rotation = vec3(0, -timeSinceStart * 0.5, 0);
		light2Parent->rotation = vec3(0, timeSinceStart * 0.5, 0);

		float primaryClawRotation = sin(timeSinceStart) * 0.1 - 0.2;
		float secondaryClawRotation = sin(timeSinceStart - PI / 2) * 0.2 + PI / 2;
		claw1->rotation = vec3(0, 0, primaryClawRotation);
		claw2->rotation = vec3(0, 0, secondaryClawRotation);
		claw3->rotation = vec3(0, PI / 2, primaryClawRotation);
		claw4->rotation = vec3(0, 0, secondaryClawRotation);
		claw5->rotation = vec3(0, PI, primaryClawRotation);
		claw6->rotation = vec3(0, 0, secondaryClawRotation);
		claw7->rotation = vec3(0, 3 * PI / 2, primaryClawRotation);
		claw8->rotation = vec3(0, 0, secondaryClawRotation);

		vec3 up = vec3(0, 1, 0);
		player->translation += fpsCamera->getFacing() * (float)(inputY * deltaTime * 3.0);
		player->translation += fpsCamera->getStrafe(up) * (float)(inputX * deltaTime * 3.0);
		player->translation += up * (float)(inputZ * deltaTime * 3.0);

		if (playerCollisions)
		{
			if (player->translation.y < 0)
			{
				player->translation.y = 0;
			}
			vec3 toBlackHole = blackHole->position - player->translation;
			if (length(toBlackHole) < 3.5)
			{
				player->translation = blackHole->position - normalize(toBlackHole) * 3.5f;
			}
			vec3 fromCenter = player->translation;
			if (length(fromCenter) > 10.0)
			{
				player->translation = normalize(fromCenter) * 10.0f;
			}

			vec2 pillarLocations[8] = {
				vec2(0, 8),
				vec2(8 * SQRT_2_OVER_2, 8 * SQRT_2_OVER_2),
				vec2(8, 0),
				vec2(8 * SQRT_2_OVER_2, -8 * SQRT_2_OVER_2),
				vec2(0, -8),
				vec2(-8 * SQRT_2_OVER_2, -8 * SQRT_2_OVER_2),
				vec2(-8, 0),
				vec2(-8 * SQRT_2_OVER_2, 8 * SQRT_2_OVER_2)
			};

			for (auto pillar : pillarLocations)
			{
				vec2 playerPos2d = vec2(player->translation.x, player->translation.z);
				vec2 toPillar = pillar - playerPos2d;
				if (length(toPillar) < 1)
				{
					playerPos2d = pillar - normalize(toPillar);
				}
				player->translation.x = playerPos2d.x;
				player->translation.z = playerPos2d.y;
			}
		}
	}

	void render() {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float aspect = width/(float)height;
		fpsCamera->aspect = aspect;

		scene->drawAll(freeCam);
	}
};

mat4 rotationMatrix(vec3 axis, float angle)
{
	axis = normalize(axis);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;

	return mat4(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 0.0,
		oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 0.0,
		oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c, 0.0,
		0.0, 0.0, 0.0, 1.0);
}

float valueMap(float value, float min1, float max1, float min2, float max2)
{
	return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

void testBlackHole(shared_ptr<BlackHoleMap> blackHole, double time)
{
	vec4 viewPositionV4 = vec4(3, 0, -5, 1);
	vec4 viewNormalV4 = vec4(0, 0, 1, 0);

	vec3 bhHole = vec3(0, 0, -5);
	vec3 bhVertex = vec3(3 * cos(time * 0.2), 0, -5 - 3 * sin(time * 0.2));
	vec3 bhObserver = vec3(0);

	vec3 bhXAxis = normalize(bhObserver - bhHole);
	vec3 bhNormal = normalize(cross(bhXAxis, bhVertex - bhHole));
	vec3 bhYAxis = cross(bhNormal, bhXAxis);

	cout << "X Axis: " << bhXAxis.x << ", " << bhXAxis.y << ", " << bhXAxis.z << endl;
	cout << "Y Axis: " << bhYAxis.x << ", " << bhYAxis.y << ", " << bhYAxis.z << endl;

	vec3 bhVertexRelative = bhVertex - bhHole;
	float bhVertex2dX = dot(bhVertexRelative, bhXAxis);
	float bhVertex2dY = dot(bhVertexRelative, bhYAxis);
	vec2 bhVertex2dCart = vec2(bhVertex2dX, bhVertex2dY);

	cout << "Vertex2D: " << bhVertex2dX << ", " << bhVertex2dY << endl;

	vec3 bhObserverRelative = bhObserver - bhHole;
	float bhObserver2dX = dot(bhObserverRelative, bhXAxis);
	float bhObserver2dY = dot(bhObserverRelative, bhYAxis);
	vec2 bhObserver2dCart = vec2(bhObserver2dX, bhObserver2dY);

	cout << "Observer2D: " << bhObserver2dX << ", " << bhObserver2dY << endl;

	float vertexR = length(bhVertex2dCart);
	float vertexPhi = atan(bhVertex2dY, bhVertex2dX);
	if (vertexPhi < 0.0f)
	{
		vertexPhi += 2 * PI;
	}
	float observerR = length(bhObserver2dCart);

	cout << "VertexR: " << vertexR << ", VertexPhi: " << vertexPhi << ", ObserverR: " << observerR << endl;

	float vertexRMapped = valueMap(vertexR, blackHole->vrMin, blackHole->vrMax, 0.0, 1.0);
	float vertexPhiMapped = valueMap(vertexPhi, 0, 2 * PI, 0.0, 1.0);
	float observerRMapped = valueMap(observerR, blackHole->orMin, blackHole->orMax, 0.0, 1.0);

	vec3 coord = vec3(vertexRMapped, vertexPhiMapped, observerRMapped);
	cout << "Coord: " << vertexRMapped << ", " << vertexPhiMapped << ", " << observerRMapped << endl;
	vec3 bh = blackHole->getValue(coord.x, coord.y, coord.z);
	float va = bh.x;
	float oa = bh.y;
	float d = bh.z;

	cout << "VA: " << va << ", OA: " << oa << ", D: " << d << endl;

	vec3 directionFromObserver = cos(oa) * bhXAxis + sin(oa) * bhYAxis;
	vec3 displacementFromObserver = directionFromObserver * d;
	vec3 normalRotationAxis = cross(bhXAxis, bhYAxis);
	float normalRotationAmount = (oa + PI) - va; // probably right?
	mat4 normalRotationMatrix = rotationMatrix(normalRotationAxis, normalRotationAmount);

	cout << "displacmentFromObserver: " << displacementFromObserver.x << ", " << displacementFromObserver.y << ", " << displacementFromObserver.z << endl;
	cout << "normalRotationAxis: " << normalRotationAxis.x << ", " << normalRotationAxis.y << ", " << normalRotationAxis.z << endl;
	cout << "normalRotationAmount: " << normalRotationAmount << endl;

	viewPositionV4 = vec4(displacementFromObserver, 1.0);
	viewNormalV4 = normalRotationMatrix * viewNormalV4;
}

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init();
	application->initShaders(resourceDir);
	application->initGeom(resourceDir);
	application->initBlackHole(resourceDir);
	application->initScene();

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		auto newTime = glfwGetTime();
		application->deltaTime = newTime - application->timeSinceStart;
		application->timeSinceStart = newTime;

		// testBlackHole(application->blackHole, application->timeSinceStart);

		// Update state.
		application->update();
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
