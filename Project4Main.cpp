
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif


#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#include<sstream>
#include<algorithm>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include <SOIL/SOIL.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include<glm/vec3.hpp>
#include<glm/vec4.hpp>
#include<glm/mat4x4.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

// Other includes
#include "HeaderFiles/Shader2.h"
#include "HeaderFiles/OBJloader.h"
#include "HeaderFiles/Vertex.h"
#include "HeaderFiles/Texture.h"
#include "HeaderFiles/MyModelClass.h"
#include "HeaderFiles/Camera.h"
#include "HeaderFiles/Material.h"
#include "HeaderFiles/Light.h"
#include "HeaderFiles/ShadowPassClass.h"

using namespace std;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Window dimensions
const GLuint WIDTH = 1400, HEIGHT = 800;
GLFWwindow* window;

Camera* camera;

float dt = 0.0f;
float curTime = 0.0f;
float lastTime = 0.0f;

double lastMouseX = 0.0;
double lastMouseY = 0.0;
double mouseX = 0.0;
double mouseY = 0.0;
double mouseOffsetX = 0.0;
double mouseOffsetY = 0.0;
bool firstMouse = true;

glm::mat4 ViewMatrix;

ShadowPassClass* shadowPass;


//-----------------------------------------------------------------------------------------------------------------------------------------------
// Function to init Window
//-----------------------------------------------------------------------------------------------------------------------------------------------


void initWindow()
{
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    window = glfwCreateWindow(WIDTH, HEIGHT, "More 3D example", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    glewInit();

    // Define the viewport dimensions
    glViewport(0, 0, WIDTH, HEIGHT);

    // Setup OpenGL options
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
// Function to init Camera (view matrix)
//-----------------------------------------------------------------------------------------------------------------------------------------------


void initCamera()
{
	//camPosition = glm::vec3(0.f, 6.f, 1.0f);
	//worldUp = glm::vec3(0.f, 1.f, 0.f);
	//camFront = glm::vec3(0.f, -1.f, -1.f);
	
	camera = new Camera(glm::vec3(0.f, 0.f, 10.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f));
	
	ViewMatrix = glm::mat4(1.f);
	ViewMatrix = camera->getViewMatrix();
}


//-----------------------------------------------------------------------------------------------------------------------------------------------
// Keyboard inputs
//-----------------------------------------------------------------------------------------------------------------------------------------------


void updateKeyboardInput()
{
	//Camera
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera->move(dt, FORWARD);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera->move(dt, BACKWARD);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera->move(dt, LEFT);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera->move(dt, RIGHT);
	}
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
	{
		camera->move(dt,DOWN);
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		camera->move(dt,UP);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
// Time management
//-----------------------------------------------------------------------------------------------------------------------------------------------




void updateDt()
{
	curTime = static_cast<float>(glfwGetTime());
	dt = curTime - lastTime;
	lastTime = curTime;
}


//-----------------------------------------------------------------------------------------------------------------------------------------------
// Mouse Input
//-----------------------------------------------------------------------------------------------------------------------------------------------



void updateMouseInput()
{
	glfwGetCursorPos(window, &mouseX, &mouseY);

	if (firstMouse)
	{
		lastMouseX = mouseX;
		lastMouseY = mouseY;
		firstMouse = false;
	}

	//Calc offset
	mouseOffsetX = mouseX - lastMouseX;
	mouseOffsetY = lastMouseY - mouseY;

	//Set last X and Y
	lastMouseX = mouseX;
	lastMouseY = mouseY;

	//Move light
	/*if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
	{
		//pointLights[0]->setPosition(camera.getPosition());
	}*/
}


//-----------------------------------------------------------------------------------------------------------------------------------------------
// Update all inputs
//-----------------------------------------------------------------------------------------------------------------------------------------------


void updateInput()
{
	glfwPollEvents();

	updateKeyboardInput();
	//updateMouseInput();
	//camera->updateInput(dt, -1, mouseOffsetX, mouseOffsetY);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
// Update ViewMatrix, pointlight
//-----------------------------------------------------------------------------------------------------------------------------------------------



void updateUniforms(MyModelClass* model, vector<PointLight*> &pointLights, Shader2 &ourShader)
{

	shadowPass->setShaderUnifroms(ourShader);
	//View Matrix
	ViewMatrix = camera->getViewMatrix();
	ourShader.setVec3f(camera->getPosition(), "cameraPos");

	model->setViewMatrix(ViewMatrix);

	
	
	//Point Lights
	for (unsigned int i = 0; i < pointLights.size(); i++)
	{
		pointLights[i]->sendToShader(ourShader);
		ourShader.setVec3f(pointLights[i]->getPosition(), "lightPos0");
	}

}


//-----------------------------------------------------------------------------------------------------------------------------------------------
// Main Function
//-----------------------------------------------------------------------------------------------------------------------------------------------


int main()
{
    
    
    //Shaders
    vector<Shader2*> shaders;
    
    //Point Lights
    vector<PointLight*> pointLights;
    
    //Materials
    vector<Material*> materials; 

    //Textures
    vector<Texture*> textures; 
    
    //Models
    vector<MyModelClass*> myModels;
    
    //initwindow
    initWindow();

	cout << "hello" << endl;
    // Build and compile our shader program
    shaders.push_back(new Shader2("Shaders/MoreCubes.vs", "Shaders/MoreCubes.frag"));
    shaders.push_back(new Shader2("Shaders/MoreCubes.vs", "Shaders/Wood.frag"));
	shaders.push_back(new Shader2("Shaders/debug_quad.vs", "Shaders/debug_quad_depth.frag"));
    
	cout << "hello 2" << endl;

    initCamera();
    
	
    textures.push_back(new Texture("Textures/Cup1ColorFinal.png", GL_TEXTURE_2D));
    //textures.push_back(new Texture("Textures/Cup1AO.png", GL_TEXTURE_2D));
    textures.push_back(new Texture("Textures/Cup2BaseColor.png", GL_TEXTURE_2D));
	textures.push_back(new Texture("Textures/MakeupColor.png", GL_TEXTURE_2D));
	
    
	cout << "hello 3" << endl;

    materials.push_back(new Material(glm::vec3(0.9f), glm::vec3(1.f), glm::vec3(.8f), 0, 7)); 
    materials.push_back(new Material(glm::vec3(0.9f), glm::vec3(1.f), glm::vec3(.8f), 1, 2)); 
    materials.push_back(new Material(glm::vec3(0.9f), glm::vec3(1.f), glm::vec3(1.f), 3, 4)); 
	materials.push_back(new Material(glm::vec3(0.9f), glm::vec3(1.f), glm::vec3(1.f), 8, 9)); 
 
    pointLights.push_back(new PointLight(glm::vec3(15.f,8.0f,15.0f), 2.0f, glm::vec3(1.0f,0.0f,0.0f)));
    
    cout << "hello 4" << endl;

    myModels.push_back(new MyModelClass(materials[0], "Textures/Cup1ColorFinal.png", "Models/Cup1Model.txt", ViewMatrix));
    myModels.push_back(new MyModelClass(materials[1], "Textures/Cup2BaseColor.png",  "Models/Cup1Model.txt", ViewMatrix));
    myModels.push_back(new MyModelClass(materials[2], "Textures/Cup2BaseColor.png",  "Models/table top.txt", ViewMatrix));
	myModels.push_back(new MyModelClass(materials[3], "Textures/MakeupColor.png",  "Models/makeupremover2.txt", ViewMatrix));

	cout << "hello 5" << endl;
    
    myModels[0]->setOrigin(glm::vec3(-2.0f, 1.0f, -2.0f));
    myModels[0]->setRotation(glm::vec3(0.0f, 45.0f, 0.0f));
    myModels[1]->setOrigin(glm::vec3(4.0f, 1.0f, 0.8f));
    myModels[2]->setOrigin(glm::vec3(0.0f, 0.0f, -4.0f));
	myModels[3]->setOrigin(glm::vec3(-1.0f, 0.0f, -4.8f));


	//Shadow Pass
	shadowPass = new ShadowPassClass(pointLights[0], myModels);
	
	//for each model pass a refernece to the shadowPass depthmap
	for (int i = 0; i < myModels.size(); i++)
	{
		myModels[i]->setShadowTex(shadowPass->getDepthMap());
	}

	
    // Game loop
    while (!glfwWindowShouldClose(window))
    {
	
        //update functions
        updateDt();
       	updateInput();

        //Color buffer
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
		//glEnable(GL_CULL_FACE);

		//render the depth texture from the camera to the depthMap
		shadowPass->render();

		//glDisable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
		updateUniforms(myModels[1],pointLights, *shaders[0]);
		//Render the model using x shader. 
		myModels[1]->render(*shaders[0]);

		updateUniforms(myModels[0],pointLights, *shaders[0]);
		myModels[0]->render(*shaders[0]);
		
		updateUniforms(myModels[2],pointLights, *shaders[1]);
		myModels[2]->render(*shaders[1]);

		updateUniforms(myModels[3],pointLights, *shaders[0]);
		myModels[3]->render(*shaders[0]);
		
		// Swap the screen buffers
		glfwSwapBuffers(window);

		
		//glFlush();
			
		glBindVertexArray(0);
		glUseProgram(0);
		glActiveTexture(0);
		glBindTexture(GL_TEXTURE_2D, 0);
        
    }
    
    
    // Properly de-allocate all resources once they've outlived their purpose
    glfwTerminate();
    
    for (size_t i = 0; i < shaders.size(); i++)
		delete shaders[i];
	
	for (size_t i = 0; i < textures.size(); i++)
		delete textures[i];

	for (size_t i = 0; i < materials.size(); i++)
		delete materials[i];

	for (size_t i = 0; i < myModels.size(); i++)
		delete myModels[i];

	for (size_t i = 0; i < pointLights.size(); i++)
		delete pointLights[i];
    
    
    return 0;
}



// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

