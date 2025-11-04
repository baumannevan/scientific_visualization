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
bool toggle_height = false;
bool toggle_contours;
int color_scheme = 0; // 0 = soild color, 1 = grayscale, 3 = rainbow

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

// shader programs
std::shared_ptr<Shader> surfaceShader = nullptr;
std::shared_ptr<Shader> soildColorShader = nullptr;
std::shared_ptr<Shader> grayscaleShader = nullptr;
std::shared_ptr<Shader> bicolorShader = nullptr;
std::shared_ptr<Shader> rainbowShader = nullptr;
std::shared_ptr<Shader> contourShader = nullptr;


// Helper functions
void set_scene();
void load_shaders();
void update_shaders();

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
    
    
    // std::unique_ptr<Shader> surfaceShader = 
    //     std::make_unique<Shader>("../shaders/solid_color.vert", 
    //                              "../shaders/solid_color.frag");
    load_shaders();

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

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        set_scene();
        
        
        // draw mesh surface
        if (mesh_surface) {
            // Enable shader and set uniform variables
            surfaceShader->use();
            surfaceShader->setMat4("projectionMatrix", projection);
            surfaceShader->setMat4("viewMatrix", view);
            surfaceShader->setMat4("modelMatrix", model);
            surfaceShader->setVec3("viewPos", cameraPos);
            
            glDepthMask(GL_TRUE);            
            mesh_surface->draw();
        }

        if (mesh_surface && toggle_contours) {
            glEnable(GL_POLYGON_OFFSET_FILL);   // these two lines make sure that we 
            glPolygonOffset(-1.0f, -1.0f);      // draw on top of anything already drawn
            contourShader->use();
            contourShader->setMat4("projectionMatrix", projection);
            contourShader->setMat4("viewMatrix", view);
            contourShader->setMat4("modelMatrix", model);
            contourShader->setVec3("viewPos", cameraPos);
            mesh_surface->draw();
            glDisable(GL_POLYGON_OFFSET_FILL);  // 
        }


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

void load_shaders(){
    soildColorShader = std::make_shared<Shader>("../shaders/solid_color.vert", "../shaders/solid_color.frag");
    grayscaleShader = std::make_shared<Shader>("../shaders/color_map.vert", "../shaders/grayscale.frag");
    bicolorShader = std::make_shared<Shader>("../shaders/color_map.vert", "../shaders/bicolor.frag");
    rainbowShader = std::make_shared<Shader>("../shaders/color_map.vert", "../shaders/rainbow.frag");
    contourShader = std::make_shared<Shader>("../shaders/contours.vert", "../shaders/contours.frag");

    // set the active shader to solid color by defualt
    surfaceShader = soildColorShader;
}

void update_shaders() {
    if (color_scheme == 0) {
        surfaceShader = soildColorShader;
        std::cout << "Using solid color shader" << std::endl;
        return;
    }
    else if (color_scheme == 1) {
        surfaceShader = grayscaleShader;
        std::cout << "Using grayscale shader" << std::endl;
        
    }
    else if (color_scheme == 2) {
        surfaceShader = bicolorShader;
        std::cout << "Using bi color shader" << std::endl;
        
    }
    else if (color_scheme == 3) {
        surfaceShader = rainbowShader;
        std::cout << "Using rainbow shader" << std::endl;
        
    }

    // get the min and max scalar values from the mesh
    double min_scalar = 0.0;
    double max_scalar = 1.0;
    if (mesh_data)
        mesh_data->get_min_max_scalar(min_scalar, max_scalar);
    surfaceShader->use();
    surfaceShader->setFloat("minScalar", static_cast<float>(min_scalar));
    surfaceShader->setFloat("maxScalar", static_cast<float>(max_scalar));
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
    if (action != GLFW_PRESS)
        return;
    
    switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, true);
            break;
        case GLFW_KEY_R:
            // reset view
            ZOOM = 1.0;
            TRANSLATION = glm::vec3(0.0f, 0.0f, 0.0f);
            ROTATION = glm::mat4(1.0f);
            break;
        case GLFW_KEY_T:
            toggle_contours = !toggle_contours;
            if (toggle_contours && mesh_data) {
                std::cout << "Enter a number of contours to draw e.g. 10: ";
                int num_contours;
                std::cin >> num_contours;
                
                double min_scalar, max_scalar;
                mesh_data->get_min_max_scalar(min_scalar, max_scalar);

                contourShader->use();
                    contourShader->setInt("numContours",num_contours);
                    contourShader->setFloat("minScalar",static_cast<float>(min_scalar));
                    contourShader->setFloat("maxScalar",static_cast<float>(max_scalar));
                }
                break;
        case GLFW_KEY_H:
            // set the mesh vertex heights based on their scalar values
            if (!mesh_data) // if there is no mesh data, do nothing
                break;
            toggle_height = !toggle_height;
            if (toggle_height)
            {
                // get a height factor from the user and use it to set the vertex heights for the mesh_data object
                std::cout << "Enter a height factor (e.g. 0.1 to 10.0): ";
                float height_factor;
                std::cin >> height_factor;
                mesh_data->set_height_from_scalar(height_factor);
            }
            else
            {
                // reset the vertex positions for the mesh_data object
                mesh_data->reset_vertex_positions();
            }
            // reconstruct the drawable surface
            mesh_surface = std::make_unique<DrawItem>(*mesh_data, DrawItem::DrawMode::Surface);
            break;
        case GLFW_KEY_C:
            // cycle through color schemes
            color_scheme = (color_scheme + 1) % 4;
            update_shaders();
            break;
    default:
        break;
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

    // update shaders with new scalar range
    color_scheme = 0; // reset to solid color
    update_shaders();
    
    // load in mesh file
    mesh_data = std::make_unique<QuadMesh>(paths[0]);

    // create a drawable surface from the mesh
    mesh_surface = std::make_unique<DrawItem>(*mesh_data, DrawItem::DrawMode::Surface);
    // reset transformations
    ZOOM = 1.0;
    ROTATION = glm::mat4(1.0f);
    translating = false;
    rotating = false;
    // un-toggle height feild
    toggle_height = false;
    toggle_contours = false;



    // update the window
    std::string title = "Scientific Visualization - ";
    title += paths[0];
    glfwSetWindowTitle(window, title.c_str());
    glfwRequestWindowAttention(window);

}
