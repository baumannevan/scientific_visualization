#include "drawitem.h"

DrawItem::DrawItem(const QuadMesh& mesh, DrawMode draw_mode)
	: m_VAO(0), m_VBO(0), m_EBO(0)
{
    switch (draw_mode)
    {
    case DrawMode::Surface:
        initializeSurface(mesh);
        break;
    case DrawMode::Wireframe:
        initializeTubes(mesh);
        break;
    case DrawMode::Points:
        initializeSpheres(mesh);
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
        glm::vec3 vector = glm::vec3(v->scalar());
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

void DrawItem::initializeTubes(const QuadMesh& mesh)
{
    // TODO: implement wireframe initialization
}

void DrawItem::initializeSpheres(const QuadMesh& mesh)
{
    // TODO: implement point/sphere initialization
}