#include "drawitem.h"

DrawItem::DrawItem(const QuadMesh& mesh, DrawMode draw_mode, int resolution, float radius)
    : m_VAO(0), m_VBO(0), m_EBO(0)
{
    switch (draw_mode)
    {
    case DrawMode::Surface:
        initializeSurface(mesh);
        break;
    case DrawMode::Wireframe:
        initializeTubes(mesh, resolution, radius); // tubes need a resolution and radius
        break;
    case DrawMode::Points:
        initializeSpheres(mesh, resolution, radius); // spheres need a resolution and radius 
        break;
    default:
        break;
    }
}


DrawItem::~DrawItem()
{
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

void DrawItem::draw() const
{
    if (m_vertex_data.empty() || m_face_data.empty()) return;

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m_face_data.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void DrawItem::initializeSurface(const QuadMesh& mesh)
{
    // get the vertices and faces from the mesh
    m_vertex_data.clear();
    m_face_data.clear();
    // m_vertex_data.reserve(mesh.num_vertices() * 2); // vertex position and normal
    m_vertex_data.reserve(mesh.num_vertices() * 10); // vertex position, normal, scalar values, and vector values
    m_face_data.reserve(mesh.num_faces() * 6); // each quad face will be drawn as two triangles
    for (const std::shared_ptr<Vertex>& v : mesh.vertices())
    {
        // m_vertex_data.push_back(glm::vec3(v->pos()));
        // m_vertex_data.push_back(glm::vec3(v->normal()));
        glm::vec3 pos = glm::vec3(v->pos());
        glm::vec3 norm = glm::vec3(v->normal());
        float scalar = static_cast<float>(v->scalar());
        // Make the scalar->vec3 conversion explicit to avoid narrowing warnings (and be clear)
        glm::vec3 vector = glm::vec3(static_cast<float>(v->scalar()));
        m_vertex_data.push_back(pos.x);
        m_vertex_data.push_back(pos.y);
        m_vertex_data.push_back(pos.z);
        m_vertex_data.push_back(norm.x);
        m_vertex_data.push_back(norm.y);
        m_vertex_data.push_back(norm.z);
        m_vertex_data.push_back(scalar);
        m_vertex_data.push_back(vector.x);
        m_vertex_data.push_back(vector.y);
        m_vertex_data.push_back(vector.z);
        
    }
    for (const std::shared_ptr<Face>& f : mesh.faces())
    {
        std::vector<std::shared_ptr<Vertex>> verts = f->vertices();
        m_face_data.push_back(verts[0]->id()); // lower right triangle
        m_face_data.push_back(verts[1]->id());
        m_face_data.push_back(verts[2]->id());
        m_face_data.push_back(verts[2]->id()); // upper left triangle
        m_face_data.push_back(verts[3]->id());
        m_face_data.push_back(verts[0]->id());
    }

    // set up buffers and arrays
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    // load vertex data into vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    // glBufferData(GL_ARRAY_BUFFER, m_vertex_data.size() * sizeof(glm::vec3), &m_vertex_data[0], GL_STATIC_DRAW); // fix mem issue
    glBufferData(GL_ARRAY_BUFFER, m_vertex_data.size() * sizeof(float), &m_vertex_data[0], GL_STATIC_DRAW);

    // load face data into element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_face_data.size() * sizeof(unsigned int), &m_face_data[0], GL_STATIC_DRAW);
    
    // Position: location 0, 3 floats, stride 10 floats
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);

    // Normal: location 1, 3 floats, stride 10 floats, offset 3 floats
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3 * sizeof(float)));

    // Scalar: location 2, 1 float, stride 10 floats, offset 6 floats
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6 * sizeof(float)));

    // Vertex Vector: location 3, 3 floats, stride 10 floats, offset 7 floats
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(7 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void DrawItem::initializeTubes(const QuadMesh& mesh, int tube_sides, float tube_radius)
{
    m_vertex_data.clear();
    m_face_data.clear();

    const float PI = 3.14159265358979323846f;
    float angle_increment = 2.0f * PI / static_cast<float>(tube_sides);

    unsigned int index_offset = 0;

    for (const std::shared_ptr<Edge>& edge : mesh.edges())
    {
        // Get edge endpoints
        auto v0 = edge->v1();
        auto v1 = edge->v2();
        glm::vec3 p0 = glm::vec3(v0->pos());
        glm::vec3 p1 = glm::vec3(v1->pos());

        // Compute edge direction and perpendicular vectors for square cross-section
        glm::vec3 edge_dir = glm::normalize(p1 - p0);
        glm::vec3 edge_norm = (glm::vec3(v0->normal()) + glm::vec3(v1->normal())) * 0.5f;

        glm::vec3 side_side = glm::normalize(glm::cross(edge_dir, edge_norm));
        glm::vec3 in_out = glm::normalize(glm::cross(side_side, edge_dir));

        // Generate vertices at each end of the tube
        std::vector<glm::vec3> corners0, corners1;
        corners0.reserve(tube_sides);
        corners1.reserve(tube_sides);
        for (int i = 0; i < tube_sides; ++i)
        {
            // get the vertex positions
            float angle = angle_increment * static_cast<float>(i);
            glm::vec3 offset = tube_radius * (glm::cos(angle) * side_side + glm::sin(angle) * in_out);
            corners0.push_back(p0 + offset);
            corners1.push_back(p1 + offset);
        }

        // Add vertices (position only for now)
        for (int i = 0; i < tube_sides; ++i)
        {
            glm::vec3 c0 = corners0[i];
            glm::vec3 c1 = corners1[i];
            // c0
            m_vertex_data.push_back(c0.x);
            m_vertex_data.push_back(c0.y);
            m_vertex_data.push_back(c0.z);
            // c1
            m_vertex_data.push_back(c1.x);
            m_vertex_data.push_back(c1.y);
            m_vertex_data.push_back(c1.z);
        }

        // Add faces (quads as two triangles per tube side)
        for (int i = 0; i < tube_sides; ++i)
        {
            int j = (i + 1) % tube_sides;
            unsigned int i0 = index_offset + 2 * i;
            unsigned int i1 = index_offset + 2 * j;
            unsigned int i2 = index_offset + 2 * j + 1;
            unsigned int i3 = index_offset + 2 * i + 1;

            // First triangle
            m_face_data.push_back(i0);
            m_face_data.push_back(i1);
            m_face_data.push_back(i2);
            // Second triangle
            m_face_data.push_back(i0);
            m_face_data.push_back(i2);
            m_face_data.push_back(i3);
        }

        index_offset += 2 * tube_sides;
    }

    // Set up buffers and arrays
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    // Vertex Position: location 0, 3 floats, stride 3 floats
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertex_data.size() * sizeof(float), &m_vertex_data[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_face_data.size() * sizeof(unsigned int), &m_face_data[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void DrawItem::initializeSpheres(const QuadMesh& mesh, int sphere_divisions, float sphere_radius)
{
    m_vertex_data.clear();
    m_face_data.clear();

    // Generate a unit sphere mesh (icosphere or UV sphere)
    std::vector<glm::vec3> sphere_vertices;
    std::vector<unsigned int> sphere_indices;

    const float PI = 3.14159265358979323846f;
    float angle_increment = PI / static_cast<float>(sphere_divisions);

    // Simple UV sphere generation
    // add the top vertex first
    sphere_vertices.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
    // then add the intermediate vertices
    for (int lat = 1; lat < sphere_divisions; ++lat) 
    {
        float theta = lat * angle_increment;
        float sin_theta = glm::sin(theta);
        float cos_theta = glm::cos(theta);

        for (int lon = 0; lon < sphere_divisions * 2; ++lon) 
        {
            float phi = lon * angle_increment;
            float sin_phi = glm::sin(phi);
            float cos_phi = glm::cos(phi);

            glm::vec3 v;
            v.x = sin_theta * cos_phi;
            v.y = sin_theta * sin_phi;
            v.z = cos_theta;
            sphere_vertices.push_back(v);
        }
    }
    // add the bottom vertex last
    sphere_vertices.push_back(glm::vec3(0.0f, 0.0f, -1.0f));

    // define the top cap triangle indices first
    for (int lon = 0; lon < sphere_divisions * 2; lon++) 
    {
        sphere_indices.push_back(0);
        sphere_indices.push_back(lon + 1);
        int next_lon = (lon + 1) % (sphere_divisions * 2);
        sphere_indices.push_back(next_lon + 1);
    }
    // define the intermediate triangle indices second
    for (int lat = 1; lat < sphere_divisions - 1; ++lat) 
    {
        for (int lon = 0; lon < sphere_divisions * 2; ++lon) 
        {
            int current = (lat - 1) * (sphere_divisions * 2) + lon + 1;
            int next_lon = (lon + 1) % (sphere_divisions * 2);
            int next = (lat - 1) * (sphere_divisions * 2) + next_lon + 1;
            int below_current = current + sphere_divisions * 2;
            int below_next = next + sphere_divisions * 2;
            
            sphere_indices.push_back(current);
            sphere_indices.push_back(below_current);
            sphere_indices.push_back(next);

            sphere_indices.push_back(next);
            sphere_indices.push_back(below_current);
            sphere_indices.push_back(below_next);
        }
    }
    // define the bottom cap triangle indices last
    int bottom_index = static_cast<int>(sphere_vertices.size() - 1);
    for (int lon = 0; lon < sphere_divisions * 2; lon++) 
    {
        sphere_indices.push_back(bottom_index);
        sphere_indices.push_back(bottom_index - lon - 1);
        int next_lon = (lon + 1) % (sphere_divisions * 2);
        sphere_indices.push_back(bottom_index - next_lon - 1);
    }

    // Now create a scaled sphere at each vertex position in the mesh
    unsigned int vertex_offset = 0;
    for (const std::shared_ptr<Vertex>& v : mesh.vertices())
    {
        glm::vec3 center = glm::vec3(v->pos());

        // Add sphere vertices, transformed to the vertex position and scaled
        for (const glm::vec3& sv : sphere_vertices) 
        {
            glm::vec3 pos = center + sphere_radius * sv;
            m_vertex_data.push_back(pos.x);
            m_vertex_data.push_back(pos.y);
            m_vertex_data.push_back(pos.z);
        }

        // Add sphere faces (indices)
        for (const unsigned int& idx : sphere_indices) 
        {
            m_face_data.push_back(vertex_offset + idx);
        }

        vertex_offset += static_cast<unsigned int>(sphere_vertices.size());
    }

    // Set up buffers and arrays
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    // Vertex Position: location 0, 3 floats, stride 3 floats
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertex_data.size() * sizeof(float), &m_vertex_data[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_face_data.size() * sizeof(unsigned int), &m_face_data[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}