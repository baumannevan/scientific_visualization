#include "quadmesh.h"
#include <iostream>

#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>

;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;// Vertex Class Methods
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////

Vertex::Vertex(unsigned int id, const glm::dvec3& position, const glm::dvec3& normal,
        const double scalar, const glm::dvec3& vector, const glm::dmat2x2& tensor)
{
    m_id = id;
    m_position = position;
    m_normal = normal;
    m_offset = glm::dvec3(0.0f);
    m_scalar = scalar;
    m_vector = vector;
    m_tensor = tensor;
    
}
Vertex::~Vertex() {}

unsigned int Vertex::id() const { return m_id; }
glm::dvec3 Vertex::pos() const { return m_position; }
glm::dvec3 Vertex::normal() const { return m_normal; }
double Vertex::scalar() const { return m_scalar; }
glm::dvec3 Vertex::vector() const { return m_vector; }
glm::dmat2x2 Vertex::tensor() const { return m_tensor; }

void Vertex::set_pos(const glm::dvec3& p) { m_position = p; }
void Vertex::set_normal(const glm::dvec3& n) { m_normal = n; }
void Vertex::set_scalar(double s) { m_scalar = s; }
void Vertex::set_vector(const glm::dvec3& v) { m_vector = v; }
void Vertex::set_tensor(const glm::dmat2x2& t) { m_tensor = t; }

void Vertex::add_edge(const std::shared_ptr<Edge> e) { m_edges.push_back(e); }
void Vertex::add_face(const std::shared_ptr<Face> f) { m_faces.push_back(f); }
const std::vector<std::shared_ptr<Edge>>& Vertex::edges() const { return m_edges; }
const std::vector<std::shared_ptr<Face>>& Vertex::faces() const { return m_faces; }
std::vector<std::shared_ptr<Edge>>& Vertex::edges() { return m_edges; }
std::vector<std::shared_ptr<Face>>& Vertex::faces() { return m_faces; }
size_t Vertex::num_edges() const { return m_edges.size(); }
size_t Vertex::num_faces() const { return m_faces.size(); }

void Vertex::compute_average_normal() 
{
    glm::dvec3 sum_normals(0.0, 0.0, 0.0);
    for (const auto& face : m_faces) 
    {
        sum_normals += face->normal();
    }
    if (!m_faces.empty()) 
    {
        m_normal = glm::normalize(sum_normals / static_cast<double>(m_faces.size()));
    }
}

void Vertex::reorder_pointers()
{
    std::vector<std::shared_ptr<Face>> forward_faces;
    std::vector<std::shared_ptr<Face>> backward_faces;
    std::vector<std::shared_ptr<Edge>> forward_edges;
    std::vector<std::shared_ptr<Edge>> backward_edges;

    std::shared_ptr<Face> start_face = m_faces[0];
    std::shared_ptr<Face> face = m_faces[0];
    std::shared_ptr<Edge> edge = nullptr;
	forward_faces.push_back(start_face);

    // march forward around the vertex (clockwise around the faces)
    while (face)
    {
        auto it = std::find(face->vertices().begin(), face->vertices().end(), shared_from_this());
        int idx = static_cast<int>(std::distance(face->vertices().begin(), it));
        edge = face->edges()[(idx + 3) % 4]; // edge before this vertex in face
        forward_edges.push_back(edge);
        face = edge->other_face(face);
        if (face == start_face) break; 
        if (face) forward_faces.push_back(face);
    }
    
    if (face == start_face) // already a closed loop, just copy forward lists to main lists
    {
        m_faces = forward_faces;
        m_edges = forward_edges;
		return;
    }

	// march backward around the vertex (counter-clockwise around the faces)
    face = start_face;
    edge = nullptr;
    while (face)
    {
        auto it = std::find(face->vertices().begin(), face->vertices().end(), shared_from_this());
        int idx = static_cast<int>(std::distance(face->vertices().begin(), it));
        edge = face->edges()[idx]; // edge after this vertex in face
        backward_edges.push_back(edge);
        face = edge->other_face(face);
		if (face == start_face) break;
        if (face) backward_faces.push_back(face);
    }

    // combine forward and backward lists
    std::reverse(backward_faces.begin(), backward_faces.end());
    std::reverse(backward_edges.begin(), backward_edges.end());
    m_faces = backward_faces;
    m_faces.insert(m_faces.end(), forward_faces.begin(), forward_faces.end());
    m_edges = backward_edges;
    m_edges.insert(m_edges.end(), forward_edges.begin(), forward_edges.end());
}

void Vertex::offset_position(const glm::dvec3& offset){
    m_offset += offset;
    m_position += offset;

}

void Vertex::reset_position(){
    m_position -= m_offset;
    m_offset -= glm::dvec3(0.0);
}


;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;// Edge Class Methods
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////

Edge::Edge(unsigned int id, const std::shared_ptr<Vertex> v1, const std::shared_ptr<Vertex> v2)
{
    m_id = id;
    m_v1 = v1;
    m_v2 = v2;
}
Edge::~Edge() {}

unsigned int Edge::id() const { return m_id; }
const std::shared_ptr<Vertex> Edge::v1() const { return m_v1; }
const std::shared_ptr<Vertex> Edge::v2() const { return m_v2; }
std::shared_ptr<Vertex> Edge::v1() { return m_v1; }
std::shared_ptr<Vertex> Edge::v2() { return m_v2; }
const std::vector<std::shared_ptr<Face>>& Edge::faces() const { return m_faces; }
std::vector<std::shared_ptr<Face>>& Edge::faces() { return m_faces; }
size_t Edge::num_faces() const { return m_faces.size(); }
void Edge::add_face(const std::shared_ptr<Face> f) { m_faces.push_back(f);}

const std::shared_ptr<Vertex> Edge::other_vertex(const std::shared_ptr<Vertex> v) const
{
    if (v == m_v1) 
        return m_v2;
    else 
        return m_v1;
}

std::shared_ptr<Vertex> Edge::other_vertex(const std::shared_ptr<Vertex> v) 
{
    if (v == m_v1) 
        return m_v2;
    else 
        return m_v1;
}

const std::shared_ptr<Face> Edge::other_face(const std::shared_ptr<Face> f) const
{
    std::shared_ptr<Face> result = nullptr;
    if (m_faces.size() == 2) 
    {
        if (f == m_faces[0]) 
            return m_faces[1];
        else 
            return m_faces[0];
    }
    return result;
}

std::shared_ptr<Face> Edge::other_face(const std::shared_ptr<Face> f)
{
    std::shared_ptr<Face> result = nullptr;
    if (m_faces.size() == 2) 
    {
        if (f == m_faces[0]) 
            return m_faces[1];
        else 
            return m_faces[0];
    }
    return result;
}

double Edge::length() const 
{
    if (m_v1 && m_v2) 
    {
        return glm::length(m_v2->pos() - m_v1->pos());
    }
    else
        return 0.0;
}



;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;// Face Class Methods
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////

Face::Face(unsigned int id, const std::vector<std::shared_ptr<Vertex>>& vertices)
{
    m_id = id;
    m_vertices = vertices;
    m_normal = glm::dvec3(0.0, 0.0, 0.0);
    m_color = glm::dvec4(1.0, 1.0, 1.0, 1.0);

    // Edges should be set separately after construction
}
Face::~Face() {}

unsigned int Face::id() const { return m_id; }
glm::dvec3 Face::normal() const { return m_normal; }
glm::dvec4 Face::color() const { return m_color; }
const std::vector<std::shared_ptr<Vertex>>& Face::vertices() const { return m_vertices; }
const std::vector<std::shared_ptr<Edge>>& Face::edges() const { return m_edges; }
std::vector<std::shared_ptr<Vertex>>& Face::vertices() { return m_vertices; }
std::vector<std::shared_ptr<Edge>>& Face::edges() { return m_edges; }
size_t Face::num_vertices() const { return m_vertices.size(); }
size_t Face::num_edges() const { return m_edges.size(); }

void Face::set_color(const glm::dvec4& c) { m_color = c; }

void Face::compute_normal()
{
    // Assumes quad vertices are ordered and co-planar
    if (m_vertices.size() >= 3) 
    {
        glm::dvec3 v0 = m_vertices[0]->pos();
        glm::dvec3 v1 = m_vertices[1]->pos();
        glm::dvec3 v2 = m_vertices[2]->pos();
        m_normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
    }
}

const glm::dvec3 Face::centroid() const 
{
    glm::dvec3 sum(0.0, 0.0, 0.0);
    for (const auto& v : m_vertices) 
        sum += v->pos();
    return sum / static_cast<double>(m_vertices.size());
}



;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;// QuadMesh Class Methods
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////
;///////////////////////////////////////////////////////////////////////////////


QuadMesh::QuadMesh()
{
    construct_simple_quad_mesh();
}

QuadMesh::QuadMesh(const char* filename)
{
    m_vertices.clear();
    m_edges.clear();
    m_faces.clear();

    // try to open the file
    std::ifstream file(filename);
    if (!file)
    {
        std::cout << "Could not open .ply file: " << filename << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line); // read first line
    if (line != "ply")
    {
        std::cout << "Not a valid .ply file: " << filename << std::endl;
        return;
    }

    std::getline(file, line); // read format line
    if (line != "format ascii 1.0")
    {
        std::cout << "Only ASCII .ply files are supported: " << filename << std::endl;
        return;
    }

    // File is open and seems valid, start parsing
    size_t num_vertices = 0, num_faces = 0;
    
    // Parse header
    bool header_done = false;
    bool reading_vertex_props = false;
    std::vector<std::string> vertex_props;
    while (std::getline(file, line)) 
    {
        if (line.find("element vertex") == 0)
        {
            std::istringstream iss(line);
            std::string dummy;
            iss >> dummy >> dummy >> num_vertices;
            reading_vertex_props = true;
        }
        else if (line.find("property") == 0 && reading_vertex_props)
        {
            std::istringstream iss(line);
            std::string dummy, type, name;
            iss >> dummy >> type >> name;
            vertex_props.push_back(name);
        }
        else if (line.find("element face") == 0) 
        {
            std::istringstream iss(line);
            std::string dummy;
            iss >> dummy >> dummy >> num_faces;
            reading_vertex_props = false;
        } 
        else if (line == "end_header") 
        {
            header_done = true;
            break;
        }
    }
    if (!header_done) 
    {
        std::cout << "Invalid .ply header." << std::endl;
        return;
    }

    // Parse the vertices
    double x,y,z,nx,ny,nz,s,vx,vy,vz,t00,t01,t10,t11;
    x = y = z = nx = ny = nz = s = vx = vy = vz = t00 = t01 = t10 = t11 = 0.0;
    for (int i = 0; i < num_vertices; ++i) 
    {
        if (!std::getline(file, line))
        {
            std::cout << "Unexpected end of file while reading vertices." << std::endl;
            return;
        }

        std::istringstream iss(line);
        for (const std::string& prop : vertex_props) 
        {
            if (prop == "x") iss >> x;
            else if (prop == "y") iss >> y;
            else if (prop == "z") iss >> z;
            else if (prop == "nx") iss >> nx;
            else if (prop == "ny") iss >> ny;
			else if (prop == "nz") iss >> nz;
            else if (prop == "s") iss >> s;
            else if (prop == "vx") iss >> vx;
            else if (prop == "vy") iss >> vy;
            else if (prop == "vz") iss >> vz;
            else if (prop == "t00") iss >> t00;
            else if (prop == "t01") iss >> t01;
            else if (prop == "t10") iss >> t10;
            else if (prop == "t11") iss >> t11;
            // ignore unknown properties
        }

        std::shared_ptr<Vertex> v = std::make_shared<Vertex>(
			static_cast<unsigned int>(i), glm::dvec3(x, y, z), glm::dvec3(nx, ny, nz),
            s, glm::dvec3(vx, vy, vz), glm::dmat2x2(t00, t01, t10, t11));
        m_vertices.push_back(v);
    }

    // Parse faces (should all be quads)
    for (int i = 0; i < num_faces; i++) 
    {
        if (!std::getline(file, line))
        {
            std::cout << "Unexpected end of file while reading faces." << std::endl;
            return;
        }

        std::istringstream iss(line);
        int n;
        iss >> n;
        if (n != 4)
        {
            std::cout << "Skipping non-quad face" << i << " with " << n << " vertices." << std::endl;
            continue;
        }

        std::vector<std::shared_ptr<Vertex>> verts;
        bool valid = true;
        for (int j = 0; j < n; j++)
        {
            int vid;
            iss >> vid;
            if (vid < 0 || vid >= m_vertices.size())
            {
                std::cout << "Invalid vertex index in face" << i << "." << std::endl;
                valid = false;
                break;
            }
            else
                verts.push_back(m_vertices[vid]);
        }

        if (valid)
            m_faces.push_back(std::make_shared<Face>(i, verts));
    }

    // done with the file
    file.close();

    // set up the rest of the mesh data structures
    vertex_to_face_pointers();
    set_up_edges();
    reorder_vertex_pointers();
    compute_face_normals();
    average_vertex_normals();
    compute_midpoint_and_radius();

    std::cout << "Opened quad mesh from " << filename << std::endl;
    print_info();
}

QuadMesh::~QuadMesh() {}

const std::vector<std::shared_ptr<Vertex>>& QuadMesh::vertices() const { return m_vertices; }
const std::vector<std::shared_ptr<Edge>>& QuadMesh::edges() const { return m_edges; }
const std::vector<std::shared_ptr<Face>>& QuadMesh::faces() const { return m_faces; }
size_t QuadMesh::num_vertices() const { return m_vertices.size(); }
size_t QuadMesh::num_edges() const { return m_edges.size(); }
size_t QuadMesh::num_faces() const { return m_faces.size(); }
glm::dvec3 QuadMesh::midpoint() const { return m_midpoint; }
double QuadMesh::get_radius() const { return radius; }

void QuadMesh::compute_midpoint_and_radius()
{
    if (m_vertices.empty()) 
    {
        m_midpoint = glm::dvec3(0.0, 0.0, 0.0);
        radius = 0.0;
        return;
    }

    glm::dvec3 min_pt = m_vertices[0]->pos();
    glm::dvec3 max_pt = m_vertices[0]->pos();

    for (const auto& v : m_vertices) 
    {
        glm::dvec3 pos = v->pos();
        min_pt = glm::min(min_pt, pos);
        max_pt = glm::max(max_pt, pos);
    }

    m_midpoint = 0.5 * (min_pt + max_pt);
    radius = 0.5 * glm::length(max_pt - min_pt);
}

void QuadMesh::construct_simple_quad_mesh()
{
    m_vertices.clear();
    std::shared_ptr<Vertex> v0 = std::make_shared<Vertex>(0, glm::dvec3(-1.0, -1.0, 0.0));
    std::shared_ptr<Vertex> v1 = std::make_shared<Vertex>(1, glm::dvec3( 1.0, -1.0, 0.0));
    std::shared_ptr<Vertex> v2 = std::make_shared<Vertex>(2, glm::dvec3( 1.0,  1.0, 0.0));
    std::shared_ptr<Vertex> v3 = std::make_shared<Vertex>(3, glm::dvec3(-1.0,  1.0, 0.0));
    m_vertices = {v0, v1, v2, v3};

    m_faces.clear();
    m_faces.push_back(std::make_shared<Face>(0, m_vertices));

    m_edges.clear();
    vertex_to_face_pointers();
    set_up_edges();
    reorder_vertex_pointers();
    compute_face_normals();
    average_vertex_normals();
    compute_midpoint_and_radius();
}

void QuadMesh::vertex_to_face_pointers()
{
    // clear all vertex to face pointers
    for (std::shared_ptr<Vertex>& v : m_vertices) 
        v->faces().clear();

    // set up vertex to face pointers
    for (std::shared_ptr<Face>& f : m_faces) 
    {
        for (std::shared_ptr<Vertex>& v : f->vertices()) 
            v->add_face(f);
    }
}

void QuadMesh::set_up_edges()
{
    // set null edge pointers for all faces
    m_edges.clear();
    for(std::shared_ptr<Face>& f : m_faces)
        f->edges() = std::vector<std::shared_ptr<Edge>>(4, nullptr);

    // clear all vertex to edge pointers
    for (std::shared_ptr<Vertex>& v : m_vertices) 
        v->edges().clear();

    // loop through all quads
    for (std::shared_ptr<Face>& face : m_faces)
    {
        // loop through consecutive vertex pairs in the quad
        std::vector<std::shared_ptr<Vertex>>& verts = face->vertices();
        for (int i = 0; i < 4; i++)
        {
            // check if the edge already exists
            if (face->edges()[i] != nullptr)
                continue;

            // create the edge for this vertex pair
            std::shared_ptr<Vertex> v1 = verts[i];
            std::shared_ptr<Vertex> v2 = verts[(i + 1) % 4];
            std::shared_ptr<Edge> edge = std::make_shared<Edge>(static_cast<unsigned int>(m_edges.size()), v1, v2);
            v1->add_edge(edge);
            v2->add_edge(edge);
			m_edges.push_back(edge);
            face->edges()[i] = edge;
            edge->add_face(face);

            // find any other quads sharing this edge by searching around v1
            // this prevents duplicate edges from being created
            for (std::shared_ptr<Face>& other_face : v1->faces())
            {
                if (face == other_face)
                        continue;
                std::vector<std::shared_ptr<Vertex>>& other_verts = other_face->vertices();
                for (int k = 0; k < 4; k++)
                {
                    if ((other_verts[k] == v1 && other_verts[(k + 1) % 4] == v2) ||
                        (other_verts[k] == v2 && other_verts[(k + 1) % 4] == v1))
                    {
                        other_face->edges()[k] = edge;
                        edge->add_face(other_face);
                    }
                }
            }
        }
    }
}

void QuadMesh::reorder_vertex_pointers()
{
    // reorder the faces and edges around each vertex so they are in a consistent order
    for (std::shared_ptr<Vertex>& vertex : m_vertices)
        vertex->reorder_pointers();
}

void QuadMesh::compute_face_normals()
{
    for (auto& face : m_faces) 
        face->compute_normal();
}

void QuadMesh::average_vertex_normals()
{
    for (auto& vertex : m_vertices) 
        vertex->compute_average_normal();
}

void QuadMesh::print_info() const
{
    std::cout << "Number of vertices: " << num_vertices() << std::endl;
    std::cout << "Number of edges: " << num_edges() << std::endl;
    std::cout << "Number of faces: " << num_faces() << std::endl;
}

void QuadMesh::get_min_max_scalar(double& min_scalar, double& max_scalar) const{
    // check to make sure we have vertices
    if (m_vertices.empty()){
        min_scalar = 0;
        max_scalar = 0;
        return;
    }    
    
    // initialize min and max values
    min_scalar = m_vertices[0]->scalar(); 
    max_scalar = m_vertices[0]->scalar();

    // loop through all vertices
    for (const auto& vert : m_vertices){
        double s = vert->scalar();
        if (s < min_scalar) min_scalar = s;
        if (s > max_scalar) max_scalar = s;
    }
}

void QuadMesh::set_height_from_scalar(double factor){
    double min_scalar, max_scalar;
    get_min_max_scalar(min_scalar, max_scalar);

    // nothing interesting to show
    if (min_scalar == max_scalar) {
        return;
    }

    for (auto& vert : m_vertices) {
        // get the normalized scalar value
        double scalar = vert->scalar();
        double normalized_scalar = (scalar - min_scalar) / (max_scalar - min_scalar);

        // offset the vertex postion in the direction of the vertex normal
        glm::dvec3 offset = factor * normalized_scalar * vert->normal();
        vert->offset_position(offset);
    }

    // recompute normals
    compute_face_normals();
    average_vertex_normals();
    compute_midpoint_and_radius();
    
}

void QuadMesh::reset_vertex_positions(){
        
    // remove the offsets from al all vertices
    for (auto& vert : m_vertices){
        vert->reset_position();
    }

    // recompute normals and bounding sphere for the modified mesh
    compute_face_normals();
    average_vertex_normals();
    compute_midpoint_and_radius();
}

void QuadMesh::get_min_max_coords(double& min_x, double& max_x, double& min_y,
     double& max_y, double& min_z, double& max_z) const
{ 
    // check to make sure we have vertices
    if (m_vertices.empty()){
        min_x, min_y, min_z = 0;
        max_x, max_y, max_z = 0;
        return;
    }    
    
    // initialize min and max values
    min_x = m_vertices[0]->pos().x; 
    max_x = m_vertices[0]->pos().x;
    max_y = m_vertices[0]->pos().y; 
    max_y = m_vertices[0]->pos().y;
    max_z = m_vertices[0]->pos().z; 
    max_z = m_vertices[0]->pos().z;
    
    
    // loop through all vertices
    for (const auto& vert : m_vertices){
     glm::dvec3 pos = vert->pos();
     if (pos.x < min_x) min_x = pos.x;
     if (pos.x > max_x) max_x = pos.x;
     if (pos.y < min_y) min_y = pos.y;
     if (pos.y > max_y) max_y = pos.y;
     if (pos.z < min_z) min_z = pos.z;
     if (pos.z > max_z) max_z = pos.z;
    }

}