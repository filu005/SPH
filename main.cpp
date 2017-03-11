#include <iostream>
#include <ctime>
#include <thread>
#include <memory>

// GLEW
//#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// My includes
#include "Application.hpp"

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void do_movement(float dt);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void render(GLFWwindow* window);
double calcFPS(GLFWwindow* window, double theTimeInterval = 1.0, std::string theWindowTitle = "NONE");
bool secElapse(double interval = 1.0);

// Camera
bool keys[1024];
GLfloat lastX = c::width * 0.5f, lastY = c::height * 0.5f;
bool firstMouse = true;

unique_ptr<Application> app;

// The MAIN function, from here we start our application and run our Game loop
int main(int argc, char* argv[])
{
	srand(static_cast<unsigned>(time(0)));

	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(c::width, c::height, "LearnOpenGL", nullptr, nullptr); // Windowed
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();
	glfwSetWindowPos(window, 100, 100);

	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointSize(5.0f);

	// Define the viewport dimensions
	glViewport(0, 0, c::width, c::height);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_MULTISAMPLE);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glCullFace(GL_FRONT_AND_BACK);

	app = make_unique<Application>();
	double t0 = glfwGetTime();
	double dt, fps;

	// Game loop
	while(!glfwWindowShouldClose(window))
	{
		dt = glfwGetTime() - t0;
		t0 = glfwGetTime();

		// Check and call events
		glfwPollEvents();

		fps = calcFPS(window, 1.0, "");
		do_movement(static_cast<GLfloat>(dt));
		
		app->tick(static_cast<GLfloat>(dt));

		render(window);

		//http://stackoverflow.com/questions/17138579/idle-thread-time-in-glfw-3-0-c
		std::this_thread::yield();
	}

	glfwTerminate();
	return 0;
}

// Moves/alters the camera positions based on user input
void do_movement(GLfloat dt)
{
	Camera& camera = app->camera;
	// Camera controls
	if(keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, dt);
	if(keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, dt);
	if(keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, dt);
	if(keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, dt);
	if(keys[GLFW_KEY_EQUAL]) // +
		app->box_editor.process_extrusion(c::extrusion_step);
	if(keys[GLFW_KEY_MINUS]) // -
		app->box_editor.process_extrusion(-c::extrusion_step);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if(action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		switch(key)
		{
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		}
	}

	if(action == GLFW_PRESS)
		keys[key] = true;
	else if(action == GLFW_RELEASE)
		keys[key] = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	Camera& camera = app->camera;
	BoxEditor& bedit = app->box_editor;
	GLfloat const xposf = static_cast<GLfloat>(xpos);
	GLfloat const yposf = static_cast<GLfloat>(ypos);

	if(firstMouse)
	{
		lastX = xposf;
		lastY = yposf;
		firstMouse = false;
	}

	GLfloat xoffset = xposf - lastX;
	GLfloat yoffset = lastY - yposf;  // Reversed since y-coordinates go from bottom to left

	lastX = xposf;
	lastY = yposf;

	if (bedit.editor_state == BoxEditor::FREE_MODE)
	{
		camera.ProcessMouseMovement(xoffset*2.5f, yoffset*2.5f);
	}
	bedit.process_mouse_movement(static_cast<float>(xpos), static_cast<float>(ypos), static_cast<float>(xoffset), static_cast<float>(yoffset));
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if(button == GLFW_MOUSE_BUTTON_1) // LMB
	{
		if(action == GLFW_PRESS)
		{
			auto xpos = 0.0, ypos = 0.0;
			glfwGetCursorPos(window, &xpos, &ypos);
			app->box_editor.process_mouse_click(static_cast<float>(xpos), static_cast<float>(ypos));
		}
	}
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Camera& camera = app->camera;
	camera.ProcessMouseScroll(static_cast<GLfloat>(yoffset) * 0.1f);
}

void render(GLFWwindow* window)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	app->paint();

	// Swap the buffers
	glfwSwapBuffers(window);
}

bool secElapse(double interval)
{
	static double t0 = glfwGetTime();

	double currentTime = glfwGetTime();

	if((currentTime - t0) > interval)
	{
		t0 = glfwGetTime();
		return true;
	}
	else
		return false;
}

// http://r3dux.org/2012/07/a-simple-glfw-fps-counter/
double calcFPS(GLFWwindow* window, double theTimeInterval, std::string theWindowTitle)
{
	// Static values which only get initialised the first time the function runs
	static double t0Value = glfwGetTime(); // Set the initial time to now
	static int    fpsFrameCount = 0;             // Set the initial FPS frame count to 0
	static double fps = 0.0;           // Set the initial FPS value to 0.0

	// Get the current time in seconds since the program started (non-static, so executed every time)
	double currentTime = glfwGetTime();

	// Ensure the time interval between FPS checks is sane (low cap = 0.1s, high-cap = 10.0s)
	// Negative numbers are invalid, 10 fps checks per second at most, 1 every 10 secs at least.
	if(theTimeInterval < 0.1)
	{
		theTimeInterval = 0.1;
	}
	if(theTimeInterval > 10.0)
	{
		theTimeInterval = 10.0;
	}

	// Calculate and display the FPS every specified time interval
	if((currentTime - t0Value) > theTimeInterval)
	{
		// Calculate the FPS as the number of frames divided by the interval in seconds
		fps = (double) fpsFrameCount / (currentTime - t0Value);

		// If the user specified a window title to append the FPS value to...
		if(theWindowTitle != "NONE")
		{
			// Convert the fps value into a string using an output stringstream
			std::ostringstream stream;
			stream << fps;
			std::string fpsString = stream.str();

			// Append the FPS value to the window title details
			theWindowTitle += "FPS: " + fpsString;

			// Convert the new window title to a c_str and set it
			const char* pszConstString = theWindowTitle.c_str();
			glfwSetWindowTitle(window, pszConstString);
		}
		else // If the user didn't specify a window to append the FPS to then output the FPS to the console
		{
			std::cout << "FPS: " << fps << std::endl;
		}

		// Reset the FPS frame counter and set the initial time to be now
		fpsFrameCount = 0;
		t0Value = glfwGetTime();
	}
	else // FPS calculation time interval hasn't elapsed yet? Simply increment the FPS frame counter
	{
		fpsFrameCount++;
	}

	// Return the current FPS - doesn't have to be used if you don't want it!
	return fps;
}