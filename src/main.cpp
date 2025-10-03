#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "shader.h"
#include "trackball.h"
#include "quadmesh.h"
#include "drawitem.h"



// variables
unsigned int WIN_WIDTH = 1000;
unsigned int WIN_HEIGHT = 1000;
float ZOOM = 1.0;
glm::mat4x4 ROTATION(1.0f);
glm::vec3 TRANSLATION(0.0f, 0.0f, 0.0f);
double last_mouse_x = 0.0;
double last_mouse_y = 0.0;
bool translating = false;
bool rotating = false;

glm::mat4 projection(1.0);
glm::mat4 view(1.0);
glm::mat4 model(1.0);

glm::vec3 cameraPos(0.0f, 0.0f, 10.0f);
glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

// objects
std::unique_ptr<Trackball> trackball = nullptr;
std::unique_ptr<QuadMesh> mesh_data = nullptr;
std::unique_ptr<DrawItem> mesh_surface = nullptr;

// Helper functions
void set_scene();

// GLFW Callback Declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void drop_callback(GLFWwindow* window, int count, const char** paths);


;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;// Main Program
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	// check command line arguments
    // const char* data_path = "";
    // if (argc > 1)
    //     data_path = argv[1];
    // else
    // {
	// 	std::cout << "Usage: " << argv[0] << " <path to PLY file>" << std::endl;
    //     return -1;
    // }

    // Initialize GLFW
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_SAMPLES, 4); // multi sample antialiasing

    // Create GLFW window
    GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Scientific Visualization", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // GLFW Callback Setup
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetDropCallback(window, drop_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // Create trackball object
    trackball = std::make_unique<Trackball>(0.8f);

    // load in mesh data
    // mesh_data = std::make_unique<QuadMesh>(data_path);

    // read in and compile shaders
    std::unique_ptr<Shader> surfaceShader = 
        std::make_unique<Shader>("../shaders/solid_color.vert", 
                                 "../shaders/solid_color.frag");

    // create drawable item from the mesh
    // commented out to allow for the drop files into window feature
    // std::unique_ptr<DrawItem> mesh_surface = 
    //     std::make_unique<DrawItem>(*mesh_data, DrawItem::DrawMode::Surface);

    // Display Loop
	glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
    while (!glfwWindowShouldClose(window)) 
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEPTH_TEST);
        set_scene();
        
        // Enable shader and set uniform variables
        surfaceShader->use();
        surfaceShader->setMat4("projectionMatrix", projection);
        surfaceShader->setMat4("viewMatrix", view);
        surfaceShader->setMat4("modelMatrix", model);
        surfaceShader->setVec3("viewPos", cameraPos);

        // draw mesh surface
        if (mesh_surface)
            mesh_surface->draw();

        // mesh_surface->draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}



;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;// Helper functions
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////

void set_scene()
{
    glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);

    // set projection matrix
    float aspect = static_cast<float>(WIN_WIDTH) / static_cast<float>(WIN_HEIGHT);
    projection = glm::ortho(-aspect * ZOOM, aspect * ZOOM, -1.0f * ZOOM, 1.0f * ZOOM, 0.1f, 1000.0f);
    
    // set view matrix
    view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

    // set model matrix
    model = glm::mat4(1.0f);
    model = glm::translate(model, TRANSLATION);
    model = model * ROTATION;
    if (mesh_data) {
        model = glm::scale(model, glm::vec3(0.9f / static_cast<float>(mesh_data->get_radius())));
        model = glm::translate(model, -glm::vec3(mesh_data->midpoint()));         
    }
}

;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;// GLFW Callback Definitions
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////

// to be called when the window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height) 
{
    WIN_WIDTH = width;
    WIN_HEIGHT = height;
    glViewport(0, 0, width, height);
}

// to be called when a key is pressed or released
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        // reset view
        ZOOM = 1.0;
        TRANSLATION = glm::vec3(0.0f, 0.0f, 0.0f);
        ROTATION = glm::mat4(1.0f);
	}
}

// to be called when a mouse button is pressed or released
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) 
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) 
    {
        if (action == GLFW_PRESS && (mods & GLFW_MOD_SHIFT)) 
        {
            translating = true;
            rotating = false;
            glfwGetCursorPos(window, &last_mouse_x, &last_mouse_y);
        } 
        else if (action == GLFW_PRESS && (mods & GLFW_MOD_CONTROL))
        {
            rotating = true;
            translating = false;
            glfwGetCursorPos(window, &last_mouse_x, &last_mouse_y);
        }
        else if (action == GLFW_RELEASE) 
        {
            translating = false;
            rotating = false;
        }
    }
}

// to be called when the mouse is moved
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) 
{
    if (!translating && !rotating) return;

    float s = (2.0f * static_cast<float>(xpos) - WIN_WIDTH) / WIN_WIDTH;
    float t = (WIN_HEIGHT - 2.0f * static_cast<float>(ypos)) / WIN_HEIGHT;
    float old_s = (2.0f * static_cast<float>(last_mouse_x) - WIN_WIDTH) / WIN_WIDTH;
    float old_t = (WIN_HEIGHT - 2.0f * static_cast<float>(last_mouse_y)) / WIN_HEIGHT;

    if (translating) 
    {
        TRANSLATION.x += (s - old_s) * ZOOM;
		TRANSLATION.y += (t - old_t) * ZOOM;
        last_mouse_x = xpos;
        last_mouse_y = ypos;
    }
    else if (rotating)
    {
        glm::quat old_rot = glm::quat_cast(ROTATION);
		glm::quat new_rot = trackball->getRotationQuat(old_s, old_t, s, t, old_rot);
		ROTATION = glm::mat4_cast(new_rot);
    }

    last_mouse_x = xpos;
    last_mouse_y = ypos;
};

// to be called when the mouse wheel is scrolled
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) 
{
    // Change zoom factor with scroll (yoffset)
    ZOOM -= static_cast<float>(yoffset) * 0.1f;
    if (ZOOM < 0.1f) ZOOM = 0.1f;
    if (ZOOM > 10.0f) ZOOM = 10.0f;
}

void drop_callback(GLFWwindow* window, int count, const char** paths){
    if (count < 1){
        return;
    }
    // load in mesh file
    mesh_data = std::make_unique<QuadMesh>(paths[0]);
    // create a drawable surface from the mesh
    mesh_surface = std::make_unique<DrawItem>(*mesh_data, DrawItem::DrawMode::Surface);
    // reset transformations
    ZOOM = 1.0;
    ROTATION = glm::mat4(1.0f);
    translating = false;
    rotating = false;
    
    // update the window
    std::string title = "Scientific Visualization - ";
    title += paths[0];
    glfwSetWindowTitle(window, title.c_str());
    glfwRequestWindowAttention(window);

}
