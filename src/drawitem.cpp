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
    m_vertex_data.reserve(mesh.num_vertices() * 2); // vertex position and normal
    m_face_data.reserve(mesh.num_faces() * 6); // each quad face will be drawn as two triangles
    for (const std::shared_ptr<Vertex>& v : mesh.vertices())
    {
        m_vertex_data.push_back(glm::vec3(v->pos()));
        m_vertex_data.push_back(glm::vec3(v->normal()));
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
    glBufferData(GL_ARRAY_BUFFER, m_vertex_data.size() * 2 * sizeof(glm::vec3), &m_vertex_data[0], GL_STATIC_DRAW);

    // load face data into element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_face_data.size() * sizeof(unsigned int), &m_face_data[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void*)sizeof(glm::vec3));
    
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