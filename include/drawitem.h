#pragma once

#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <memory>

#include "shader.h"
#include "quadmesh.h"

class DrawItem
{
private:

    std::vector<glm::vec3> m_vertex_data;
    std::vector<unsigned int> m_face_data;

    unsigned int m_VAO;
    unsigned int m_VBO;
    unsigned int m_EBO;

public:

    enum class DrawMode { Points, Wireframe, Surface };

    DrawItem(const QuadMesh& mesh, DrawMode draw_mode = DrawMode::Surface);
    ~DrawItem();

    void draw() const;

private:

    void initializeSurface(const QuadMesh& mesh);
    void initializeTubes(const QuadMesh& mesh);
    void initializeSpheres(const QuadMesh& mesh);
};
