#pragma once

#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <memory>

#include "shader.h"
#include "quadmesh.h"

class DrawItem
{
private:

    // std::vector<glm::vec3> m_vertex_data;
    std::vector<float> m_vertex_data;
    std::vector<unsigned int> m_face_data;

    unsigned int m_VAO;
    unsigned int m_VBO;
    unsigned int m_EBO;

public:

    enum class DrawMode { Points, Wireframe, Surface };

    // add the resolution and radius parameters
    DrawItem(const QuadMesh& mesh, DrawMode draw_mode = DrawMode::Surface, int resolution = 4, float radius = 0.1f); 
    ~DrawItem();

    void draw() const;

private:

    void initializeSurface(const QuadMesh& mesh);
    // add the tube_sides and tube_radius parameters
    void initializeTubes(const QuadMesh& mesh, int tube_sides, float tube_radius); 
    // add the sphere_divisions and sphere_radius parameters
    void initializeSpheres(const QuadMesh& mesh, int shpere_divisions, float sphere_radius); 
};
