# Assignment 2 Coding Walkthrough

This walkthrough contains code snippets that you will need to add to the specified files in the **SciVis_2025** project. Some of the functions and classes in these code snippets already exists in the project, and you are just making modifications or adding variables. In these cases, `...` is used to indicate existing code that is not shown here. Some of the code snippets are also incomplete and `???` is used to indicate where you need to add more details. It is assumed that you already have a working version of the functions needed for Assignment 1 (a version of the Assignment 1 solutions can be found on Canvas).

Instead of implementing the marching squares algorithm to extract 2D scalar field contours, we will use shaders to draw our contour lines. This will let us avoid using any new data structures to store the contours. We will create a new shader program that tests the interpolated scalar value at each fragment (pixel) of our mesh surface. If the scalar value is close to one of our desired contours, we will shade it with an opaque color. Otherwise, we will leave the original color of the mesh surface by setting the alpha channel close to zero.

1. First, create two new shader files in the **shaders** directory:

    - `contours.vert`
    - `contours.frag`
    
    From Assignment 1, you should have shader files for color mapping called something like

    - `color_map.vert`
    - `grayscale.frag`

    For now, copy the contents of `color_map.vert` into `contours.vert` and copy the contents of `grayscale.frag` into `contours.frag`. We don't actually need to make any changes to `contours.vert` compared to `color_map.vert` and we are including it here just for naming clarity. You can omit `contours.vert` and just re-use `color_map.vert` if you want to avoid redundant files.

2. In `contours.frag` we first need to add a uniform variable to store the number of contours that we want to draw:

    ```glsl
    #version 330 core

    // uniform variables are passed in from the application, and are the same for each fragment
    uniform int numContours;
    ...
    ```

3. Next, add a constant variable to `contours.frag` for your desired contour line color. Black and white work well in most cases, but you can use any color you'd like:

    ```glsl
    // const variables are local to the shader and cannot be changed by the application
    ...
    const vec3 contourColor = vec3(???);
    ...
    ```

4. At the top of the `main` function in `contours.frag`, we will first use the desired number of contours to figure out how far apart the contour lines should be spaced:

    ```glsl
    void main() 
    {
        // The scalars are normalized in the vertex shader and should be between 0.0 and 1.0 for every fragment.
        //Divide 1.0 by the desired number of contours to get the spacing between them
        float spacing = ???
        ...
    }
    ```

5. Next we want to figure out the contour that is just below the scalar value at the current fragment. We will do that in the `main` function of `contours.frag` by finding how many contours can fit between 0.0 and the scalar value at the fragment:

    ```glsl
    ...
    float spacing = ???;
    int num_contours_below = int(vScalar / spacing);
    float contour_below = ???;
    ...
    ```

6. We can now use the scalar value of the contour just below our fragment to determine how close it is to the nearest contour. In the `main` function of `contours.frag`, find

    ```glsl
    ...
    float contour_below = ???;
    float dist_to_below = vScalar - contour_below;
    float dist_to_above = ???;
    ...
    ```

7. We can use the distance of the fragment to the nearest contour in a couple of different ways. One option is to check to see if either distance is below a certain threshold and use the `discard;` operator otherwise. My prefered method is to set the alpha channel of the fragment based on its distance to the contour just above it. To do this, add the following the `main` function of `contours.cpp`:

    ```glsl
    ...
    float dist_to_below = vScalar - contour_below;
    float dist_to_above = ???;
    float alpha = (dist_to_below / spacing) - 0.5;
    // make sure alpha is between 0.0 and 1.0
    alpha = clamp(alpha, 0.0, 1.0);
    ```

8. The last thing we need to change in `contours.frag` is to use our alpha value in the output color **fColor**. At the bottom of the main function change the following lines:

    ```glsl
    ...
    // set the color of the fragment by the scalar value and lighting
    // vec3 color = vec3(vScalar, vScalar, vScalar); // contours will have the same color
    // fColor = vec4((ambient + diffuse + specular) * color, 1.0);
    fColor = vec4((ambient + diffuse + specular) * contourColor, alpha);
    }
    ```

9. Now that we have our new contour line shaders, we will set up some user controls in `main.cpp`. Add two new global variables to the top of `main.cpp`, one for toggling the contours on and off, and another for storing the contour line shader:

    ```c++
    // variables
    ...
    bool toggle_height = false;
    int color_scheme = 0; // 0 = solid color, 1 = grayscale, 2 = bicolor, 3 = rainbow
    bool toggle_contours = false; // ADD THIS ONE!
    ...
    // shader programs
    std::shared_ptr<Shader> surfaceShader = nullptr;
    std::shared_ptr<Shader> solidColorShader = nullptr;
    std::shared_ptr<Shader> grayscaleShader = nullptr;
    std::shared_ptr<Shader> bicolorShader = nullptr;
    std::shared_ptr<Shader> rainbowShader = nullptr;
    std::shared_ptr<Shader> contourShader = nullptr; // ADD THIS ONE!
    ...
    ```

10. Inside the `main` function of `main.cpp`, we will now add the logic to draw our scalar field mesh twice if contours are enabled: once for drawing the surface itself, and a second time to draw the contour lines over the surface. To do this, we first need to tell OpenGL that we want to blend the colors when we try to draw the same pixel multiple times. Then, we will separate out tw different blocks for drawing the mesh surface, and drawing the contours.

    ```c++
    int main(int argc, char* argv[])
    {
        ...
        while (!glfwWindowShouldClose(window)) 
        {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_DEPTH_TEST);
            // ADD THE FOLLOWING TWO LINES!
            glEnable(GL_BLEND); // tells OpenGL that we want to blend colors when drawing pixels multiple times
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); // tells OpenGL how to blend new colors with existing colors
            ///////////////////////////////
            set_scene();

            // CHANGE THE DRAW CALLS TO THE FOLLOWING TWO IF BLOCKS!
            
            // Draw the mesh surface if it exists
            if (mesh_surface)
            {
                surfaceShader->use();
                surfaceShader->setMat4("projectionMatrix", projection);
                surfaceShader->setMat4("viewMatrix", view);
                surfaceShader->setMat4("modelMatrix", model);
                surfaceShader->setVec3("viewPos", cameraPos);
                glDepthMask(GL_TRUE); // saves the "depth" of shaded pixel so we can draw over it later
                mesh_surface->draw();
            }

            // draw the contours if enabled
            if (mesh_surface && toggle_contours)
            {
                glEnable(GL_POLYGON_OFFSET_FILL);   // these two lines make sure that we 
                glPolygonOffset(-1.0f, -1.0f);      // draw on top of anything already drawn
                contourShader->use();
                contourShader->setMat4("projectionMatrix", projection);
                contourShader->setMat4("viewMatrix", view);
                contourShader->setMat4("modelMatrix", model);
                contourShader->setVec3("viewPos", cameraPos);
                mesh_surface->draw();
                glDisable(GL_POLYGON_OFFSET_FILL);  // disable ofsetting once we are done
            }
            ...
        }
        ...
    }
    ```

11. In the `load_shaders` function of `main.cpp` (added in the Assignment 1 walkthrough), we need to load our new contour line shader. **Note:** depending on your system and environment setup, the paths to your shader files may be different. If you get shader compilation errors when running the program, check here first to make sure the path strings are correct.

    ```c++
    void load_shaders()
    {
        ...
        contourShader = std::make_shared<Shader>("../shaders/contours.vert", "../shaders/contours.frag");
        ...
    }
    ```

12. In the `key_callback` function of `main.cpp`, we will use the `t` key as the contour line toggle. Add the follwoing case to the switch statement in `key_callback`:

    ```c++
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
    {
        if (action == GLFW_PRESS)
        {
            switch (key)
            {
                ...
                case GLFW_KEY_T:
                    // toggle the contour drawing
                    toggle_contours = !toggle_contours; // turns contours on or off
                    if (toggle_contours)
                    {
                        // get a number of contours to draw from the user
                        std::cout << "Enter a number of contours to draw (e.g. 10): ";
                        int num_contours;
                        std::cin >> num_contours;

                        // get the min and max scalar values from the mesh_data object
                        // (We defined a function in quadmesh.cpp to do this in Assignment 1)
                        ???

                        // set variables in the contour shader
                        contourShader->use();
                        contourShader->setInt("numContours",num_contours);
                        contourShader->setFloat("minScalar",static_cast<float>(min_scalar));
                        contourShader->setFloat("maxScalar",static_cast<float>(max_scalar));
                    }
                    break;
                ...
            }
        }
    }
    ```

13. Finally, when loading a new scalar field mesh in the `drop_callback` of `main.cpp`, make sure that contours are turned off initially:

    ```c++
    void drop_callback(GLFWwindow* window, int count, const char** paths) 
    {
        ...
        // turn off contours
        toggle_contours = false;
        ...
    }
    ```