#include <iostream>
#include <GL/glew.h>
#include <3dgl/3dgl.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

// 3D Models
C3dglTerrain terrain, water;

C3dglModel fish, statue;

C3dglProgram programBasic;
C3dglProgram programWater;
C3dglProgram programTerrain;

GLuint idTexGrass, idTexSand, idTexFish, idTexWater;

// Water specific variables
float waterLevel = 4.6f;
float fishSpeed = 0.0f;
bool wavesToggle = 1;

// The View Matrix
mat4 matrixView;

// Camera & navigation
float maxspeed = 4.f;	// camera max speed
float accel = 4.f;		// camera acceleration
vec3 _acc(0), _vel(0);	// camera acceleration and velocity vectors
float _fov = 60.f;		// field of view (zoom)

bool init()
{
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	// Initialise Shaders
	C3dglShader vertexShader;
	C3dglShader fragmentShader;

	if (!vertexShader.create(GL_VERTEX_SHADER)) return false;
	if (!vertexShader.loadFromFile("shaders/basic.vert")) return false;
	if (!vertexShader.compile()) return false;

	if (!fragmentShader.create(GL_FRAGMENT_SHADER)) return false;
	if (!fragmentShader.loadFromFile("shaders/basic.frag")) return false;
	if (!fragmentShader.compile()) return false;

	if (!programBasic.create()) return false;
	if (!programBasic.attach(vertexShader)) return false;
	if (!programBasic.attach(fragmentShader)) return false;
	if (!programBasic.link()) return false;
	if (!programBasic.use(true)) return false;

	if (!vertexShader.create(GL_VERTEX_SHADER)) return false;
	if (!vertexShader.loadFromFile("shaders/water.vert")) return false;
	if (!vertexShader.compile()) return false;

	if (!fragmentShader.create(GL_FRAGMENT_SHADER)) return false;
	if (!fragmentShader.loadFromFile("shaders/water.frag")) return false;
	if (!fragmentShader.compile()) return false;

	if (!programWater.create()) return false;
	if (!programWater.attach(vertexShader)) return false;
	if (!programWater.attach(fragmentShader)) return false;
	if (!programWater.link()) return false;
	if (!programWater.use(true)) return false;


	if (!vertexShader.create(GL_VERTEX_SHADER)) return false;
	if (!vertexShader.loadFromFile("shaders/terrain.vert")) return false;
	if (!vertexShader.compile()) return false;

	if (!fragmentShader.create(GL_FRAGMENT_SHADER)) return false;
	if (!fragmentShader.loadFromFile("shaders/terrain.frag")) return false;
	if (!fragmentShader.compile()) return false;

	if (!programTerrain.create()) return false;
	if (!programTerrain.attach(vertexShader)) return false;
	if (!programTerrain.attach(fragmentShader)) return false;
	if (!programTerrain.link()) return false;
	if (!programTerrain.use(true)) return false;

	// glut additional setup
	glutSetVertexAttribCoord3(programBasic.getAttribLocation("aVertex"));
	glutSetVertexAttribNormal(programBasic.getAttribLocation("aNormal"));
	glutSetVertexAttribCoord3(programTerrain.getAttribLocation("aVertex"));
	glutSetVertexAttribNormal(programTerrain.getAttribLocation("aNormal"));

	
	// load your 3D models here!
	programTerrain.use();
	if (!terrain.load("models\\heightmap1.png", 10)) return false;
	if (!water.load("models\\watermap.png", 10, &programWater)) return false;
	programBasic.use();
	if (!fish.load("models\\fish1.fbx")) return false;
	fish.loadAnimations();

	C3dglBitmap bm;

	bm.load("models\\FishTex.png", GL_RGBA);
	if (!bm.getBits()) return false;

	glActiveTexture(GL_TEXTURE0); //set current active texture unit
	glGenTextures(1, &idTexFish);
	glBindTexture(GL_TEXTURE_2D, idTexFish);//bind texture to current active texture unit
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), bm.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.getBits()); //bind current bitmap to texture unit

	bm.load("models/grass.png", GL_RGBA);
	if (!bm.getBits()) return false;

	glActiveTexture(GL_TEXTURE1); //set current active texture unit
	glGenTextures(1, &idTexGrass);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);//bind texture to current active texture unit
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), bm.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.getBits()); //bind current bitmap to texture unit

	programTerrain.sendUniform("textureShore", 1);

	bm.load("models/sand.png", GL_RGBA);
	if (!bm.getBits()) return false;

	glActiveTexture(GL_TEXTURE2); //set current active texture unit
	glGenTextures(1, &idTexSand);
	glBindTexture(GL_TEXTURE_2D, idTexSand);//bind texture to current active texture unit
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), bm.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.getBits()); //bind current bitmap to texture unit

	programTerrain.sendUniform("textureBed", 2);

	programWater.sendUniform("textureWater", 3);

	// setup lights (for basic and terrain programs only, water does not use these lights):
	programBasic.sendUniform("lightAmbient.color", vec3(0.3f, 0.3f, 0.3f));
	programBasic.sendUniform("lightDir.direction", vec3(1.0, 0.5, 1.0));
	programBasic.sendUniform("lightDir.diffuse", vec3(1.0, 1.0, 1.0));
	programTerrain.sendUniform("lightAmbient.color", vec3(0.3f, 0.3f, 0.3f));
	programTerrain.sendUniform("lightDir.direction", vec3(1.0, 0.5, 1.0));
	programTerrain.sendUniform("lightDir.diffuse", vec3(1.0, 1.0, 1.0));
	programWater.sendUniform("lightAmbient.color", vec3(1.0, 1.0, 1.0));

	// setup materials (for basic and terrain programs only, water does not use these materials):
	programBasic.sendUniform("materialAmbient", vec3(1.0, 1.0, 1.0));
	programTerrain.sendUniform("materialAmbient", vec3(1.0, 1.0, 1.0));
	programWater.sendUniform("materialAmbient", vec3(1.0, 1.0, 1.0));
	programBasic.sendUniform("materialDiffuse", vec3(1.0, 1.0, 1.0));
	programTerrain.sendUniform("materialDiffuse", vec3(1.0, 1.0, 1.0));

	// send the sky colours, water colours, fog density and water level values to their respective shader programs
	programWater.sendUniform("waterColor", vec3(0.2f, 0.22f, 0.02f));
	programWater.sendUniform("skyColor", vec3(0.2f, 0.6f, 1.f));
	programBasic.sendUniform("waterColor", vec3(0.2f, 0.22f, 0.02f));
	programTerrain.sendUniform("waterColor", vec3(0.2f, 0.22f, 0.02f));
	programTerrain.sendUniform("waterLevel", waterLevel);
	programBasic.sendUniform("waterLevel", waterLevel);
	programBasic.sendUniform("fogDensity", 0.4f);
	programTerrain.sendUniform("fogDensity", 0.4f);
	
	programWater.sendUniform("wavesOverReflections", wavesToggle);

	//allows texture colours (e.g. water) to be blended
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1), radians(12.f), vec3(1, 0, 0));
	matrixView *= lookAt(
		vec3(4.0, 0.4, 30.0),
		vec3(4.0, 0.4, 0.0),
		vec3(0.0, 1.0, 0.0));

	// setup the screen background colour
	glClearColor(0.2f, 0.6f, 1.f, 1.0f);   // blue sky colour

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;

	return true;
}

void renderWater(mat4& matrixView, float time, float deltaTime)
{
	mat4 m;

	// Render Water
	programWater.use();

	programWater.sendUniform("t", time);

	// Setup the Diffuse Material to: Watery Green
	programWater.sendUniform("materialAmbient", vec3(0.2f, 0.22f, 0.02f));

	// render the water
	m = matrixView;
	m = translate(m, vec3(0, waterLevel, 0));
	m = scale(m, vec3(0.5f, 1.0f, 0.5f));
	programWater.sendUniform("matrixModelView", m);
	water.render(m);
}

void renderScene(mat4& matrixView, float time, float deltaTime)
{
	mat4 m;

	programBasic.use();

	//animations
	std::vector<mat4> transforms;
	fish.getAnimData(0, time, transforms);
	programBasic.sendUniform("bones", &transforms[0], transforms.size());

	//advanced fish rendering
	//anti-clockwise moving fish
	m = matrixView;
	m = translate(m, vec3(0 + sin(radians(fishSpeed)), 4, 0 + cos(radians(fishSpeed))));
	m = scale(m, vec3(0.0001f, 0.0001f, 0.0001f));
	m = rotate(m, radians(90.f), vec3(1, 0, 0));
	m = rotate(m, radians(-fishSpeed), vec3(0, 0, 1));
	fish.render(m);
	m = matrixView;
	m = translate(m, vec3(-2 + sin(radians(fishSpeed + 10)), 3.5f, -2 + cos(radians(fishSpeed + 10))));
	m = scale(m, vec3(0.0001f, 0.0001f, 0.0001f));
	m = rotate(m, radians(90.f), vec3(1, 0, 0));
	m = rotate(m, radians(-(fishSpeed + 10)), vec3(0, 0, 1));
	fish.render(m);
	m = matrixView;
	m = translate(m, vec3(-3 + sin(radians(fishSpeed - 30)), 3, -3 + cos(radians(fishSpeed - 30))));
	m = scale(m, vec3(0.0001f, 0.0001f, 0.0001f));
	m = rotate(m, radians(90.f), vec3(1, 0, 0));
	m = rotate(m, radians(-(fishSpeed - 30)), vec3(0, 0, 1));
	fish.render(m);

	//clockwise moving fish
	m = matrixView;
	m = translate(m, vec3(-3 + cos(radians(fishSpeed - 30)), 2.5f, -3 + sin(radians(fishSpeed - 30))));
	m = scale(m, vec3(0.0001f, 0.0001f, 0.0001f));
	m = rotate(m, radians(90.f), vec3(1, 0, 0));
	m = rotate(m, radians(90 + (fishSpeed - 30)), vec3(0, 0, 1));
	fish.render(m);
	m = matrixView;
	m = translate(m, vec3(-30 + cos(radians(fishSpeed - 30)), 2.5f, -30 + sin(radians(fishSpeed - 30))));
	m = scale(m, vec3(0.0001f, 0.0001f, 0.0001f));
	m = rotate(m, radians(90.f), vec3(1, 0, 0));
	m = rotate(m, radians(90 + (fishSpeed - 30)), vec3(0, 0, 1));
	fish.render(m);

	// Render Terrain
	programTerrain.use();

	//send player position for underwater fog calculations
	programTerrain.sendUniform("playerPos", getPos(matrixView));

	// Setup the Diffuse Material to: White (aka textures remain untouched)
	programTerrain.sendUniform("materialDiffuse", vec3(1.0f, 1.0f, 1.0f));

	// render the terrain
	m = matrixView;
	terrain.render(m);

	renderWater(matrixView, time, deltaTime);
}

void onRender()
{
	//glEnable(GL_CLIP_PLANE0); //disable while testing stencil buffer for first time

	// these variables control time & animation
	static float prev = 0;
	float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;	// time since start in seconds
	float deltaTime = time - prev;						// time since last frame
	prev = time;										// framerate is 1/deltaTime

	// clear screen and buffers
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); //for mirror reflections
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //non-mirror reflection clear function

	// setup the View Matrix (camera)
	_vel = clamp(_vel + _acc * deltaTime, -vec3(maxspeed), vec3(maxspeed));
	float pitch = getPitch(matrixView);
	matrixView = rotate(translate(rotate(mat4(1),
		pitch, vec3(1, 0, 0)),	// switch the pitch off
		_vel * deltaTime),		// animate camera motion (controlled by WASD keys)
		-pitch, vec3(1, 0, 0))	// switch the pitch on
		* matrixView;

	// move the camera up following the profile of terrain (Y coordinate of the terrain)
	float terrainY = -std::max(terrain.getInterpolatedHeight(inverse(matrixView)[3][0], inverse(matrixView)[3][2]), waterLevel);
	matrixView = translate(matrixView, vec3(0, terrainY, 0));

	//fishSpeed is the Variable used to move fish around in a circlular path
	fishSpeed += deltaTime * 30;
	if (fishSpeed >= 360) fishSpeed -= 360;

	/*// mirror angles - all this crap is for mirror reflections
	float angleFrame = 0, angleMirror = -90;
	float ry = radians(angleFrame);
	float rx = -radians(angleMirror);

	// Find the reflection surface (point and normal)
	vec3 p(0, waterLevel, 0);
	vec3 n(sin(ry) * cos(rx), sin(rx), cos(ry) * cos(rx));

	// reflection matrix
	float a = n.x, b = n.y, c = n.z, d = -dot(p, n);
	// parameters of the reflection plane: Ax + By + Cz + d = 0
	mat4 matrixReflection = mat4(1 - 2 * a * a, -2 * a * b, -2 * a * c, 0,
		-2 * a * b, 1 - 2 * b * b, -2 * b * c, 0,
		-2 * a * c, -2 * b * c, 1 - 2 * c * c, 0,
		-2 * a * d, -2 * b * d, -2 * c * d, 1);

	programBasic.sendUniform("planeClip", vec4(a, b, c, d));
	programTerrain.sendUniform("planeClip", vec4(a, b, c, d));

	// check which side of the mirror is visible
	mat4 camView = inverse(matrixView);
	vec3 camPos = vec3(camView[3][0], camView[3][1], camView[3][2]);
	if (dot(p - camPos, n) > 0)  n = -n; // flip the normal if we are at the other side of the mirror... 

	// Prepare the stencil test
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 1);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	// Disable screen rendering
	glDisable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	// Use stencil test
	glStencilFunc(GL_EQUAL, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// Enable screen rendering
	glEnable(GL_DEPTH_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	
	// render the mirror with blocked color output
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	renderWater(matrixView, time, deltaTime);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// Enable clipping plane
	//glEnable(GL_CLIP_PLANE0);

	// Render the scene with the reflected camera
	matrixView *= matrixReflection;
	programBasic.sendUniform("matrixView", matrixView);
	programTerrain.sendUniform("matrixView", matrixView);
	programWater.sendUniform("matrixView", matrixView);
	renderScene(matrixView, time, deltaTime);

	// send the image to the water texture
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, idTexWater);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 0, 0, GLUT_WINDOW_WIDTH, GLUT_WINDOW_HEIGHT, 0);

	// Revert to the regular camera
	matrixView *= matrixReflection;

	// disable stencil test and clip plane
	//glDisable(GL_STENCIL_TEST);
	glDisable(GL_CLIP_PLANE0);*/

	// setup View Matrix
	programBasic.sendUniform("matrixView", matrixView);
	programTerrain.sendUniform("matrixView", matrixView);
	programWater.sendUniform("matrixView", matrixView);

	// render the scene objects
	renderScene(matrixView, time, deltaTime);

	// the camera must be moved down by terrainY to avoid unwanted effects
	matrixView = translate(matrixView, vec3(0, -terrainY, 0));

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

// called before window opened or resized - to setup the Projection Matrix
void onReshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	mat4 m = perspective(radians(_fov), ratio, 0.02f, 1000.f);
	programBasic.sendUniform("matrixProjection", m);
	programTerrain.sendUniform("matrixProjection", m);
	programWater.sendUniform("matrixProjection", m);
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': _acc.z = accel; break;
	case 's': _acc.z = -accel; break;
	case 'a': _acc.x = accel; break;
	case 'd': _acc.x = -accel; break;
	case 'e': _acc.y = accel; break;
	case 'q': _acc.y = -accel; break;
	case '1': wavesToggle = !wavesToggle;  programWater.sendUniform("wavesOverReflections", wavesToggle);
	}
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': _acc.z = _vel.z = 0; break;
	case 'a':
	case 'd': _acc.x = _vel.x = 0; break;
	case 'q':
	case 'e': _acc.y = _vel.y = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
void onMouse(int button, int state, int x, int y)
{
	glutSetCursor(state == GLUT_DOWN ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
	if (button == 1)
	{
		_fov = 60.0f;
		onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	}
}

// handle mouse move
void onMotion(int x, int y)
{
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

	// find delta (change to) pan & pitch
	float deltaYaw = 0.005f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
	float deltaPitch = 0.005f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

	if (abs(deltaYaw) > 0.3f || abs(deltaPitch) > 0.3f)
		return;	// avoid warping side-effects

	// View = Pitch * DeltaPitch * DeltaYaw * Pitch^-1 * View;
	constexpr float maxPitch = radians(80.f);
	float pitch = getPitch(matrixView);
	float newPitch = glm::clamp(pitch + deltaPitch, -maxPitch, maxPitch);
	matrixView = rotate(rotate(rotate(mat4(1.f),
		newPitch, vec3(1.f, 0.f, 0.f)),
		deltaYaw, vec3(0.f, 1.f, 0.f)),
		-pitch, vec3(1.f, 0.f, 0.f))
		* matrixView;
}

void onMouseWheel(int button, int dir, int x, int y)
{
	_fov = glm::clamp(_fov - dir * 5.f, 5.0f, 175.f);
	onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

int main(int argc, char** argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("3DGL Scene: Water Rendering (initial)");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		C3dglLogger::log("GLEW Error {}", (const char*)glewGetErrorString(err));
		return 0;
	}
	C3dglLogger::log("Using GLEW {}", (const char*)glewGetString(GLEW_VERSION));

	// register callbacks
	glutDisplayFunc(onRender);
	glutReshapeFunc(onReshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);
	glutMouseWheelFunc(onMouseWheel);

	C3dglLogger::log("Vendor: {}", (const char*)glGetString(GL_VENDOR));
	C3dglLogger::log("Renderer: {}", (const char*)glGetString(GL_RENDERER));
	C3dglLogger::log("Version: {}", (const char*)glGetString(GL_VERSION));
	C3dglLogger::log("");

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		C3dglLogger::log("Application failed to initialise\r\n");
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	return 1;
}

