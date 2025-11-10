#pragma once
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include <memory>
#include <string>

// forward declarations
class Vertex;
class Edge;
class Face;

class Vertex : public std::enable_shared_from_this<Vertex>
{
private:

    unsigned int m_id;
    glm::dvec3 m_position;
    glm::dvec3 m_normal;
    glm::dvec3 m_offset = glm::dvec3(0.0);

    double m_scalar;
    glm::dvec3 m_vector;
    glm::dmat2x2 m_tensor;

    std::vector<std::shared_ptr<Edge>> m_edges;
    std::vector<std::shared_ptr<Face>> m_faces;

public:

    Vertex(unsigned int id, 
        const glm::dvec3& position = glm::dvec3(0.0, 0.0, 0.0),
		const glm::dvec3& normal = glm::dvec3(0.0, 0.0, 0.0),
        const double scalar = 0.0,
        const glm::dvec3& vector = glm::dvec3(0.0f,0.0f,0.0f),
        const glm::dmat2x2& tensor = glm::dmat2x2(0.0));
    ~Vertex();

    unsigned int id() const;
    glm::dvec3 pos() const;
    glm::dvec3 normal() const;
    glm::dvec4 color() const;
    double scalar() const;
    glm::dvec3 vector() const;
    glm::dmat2x2 tensor() const;

    void set_pos(const glm::dvec3& p);
    void set_normal(const glm::dvec3& n);
    void set_color(const glm::dvec4& c);
    void set_scalar(double s);
    void set_vector(const glm::dvec3& v);
    void set_tensor(const glm::dmat2x2& t);

    void add_edge(const std::shared_ptr<Edge> e);
    void add_face(const std::shared_ptr<Face> f);
    const std::vector<std::shared_ptr<Edge>>& edges() const;
    const std::vector<std::shared_ptr<Face>>& faces() const;
    std::vector<std::shared_ptr<Edge>>& edges();
    std::vector<std::shared_ptr<Face>>& faces();
    size_t num_edges() const;
    size_t num_faces() const;

    void compute_average_normal();
    void reorder_pointers();
    
    void offset_position(const glm::dvec3& offset);
    void reset_position();
};



class Edge
{
private:

    unsigned int m_id;
    std::shared_ptr<Vertex> m_v1;
    std::shared_ptr<Vertex> m_v2;
    std::vector<std::shared_ptr<Face>> m_faces; 
    // should contain 1 or 2 faces in a manifold mesh surface

public:

    Edge(unsigned int id, const std::shared_ptr<Vertex> v1, const std::shared_ptr<Vertex> v2);
    ~Edge();

    unsigned int id() const;
    const std::shared_ptr<Vertex> v1() const;
    const std::shared_ptr<Vertex> v2() const;
    std::shared_ptr<Vertex> v1();
    std::shared_ptr<Vertex> v2();
    const std::shared_ptr<Vertex> other_vertex(const std::shared_ptr<Vertex> v) const;
    std::shared_ptr<Vertex> other_vertex(const std::shared_ptr<Vertex> v);

    const std::vector<std::shared_ptr<Face>>& faces() const;
    std::vector<std::shared_ptr<Face>>& faces();
    size_t num_faces() const;
    const std::shared_ptr<Face> other_face(const std::shared_ptr<Face> f) const;
    std::shared_ptr<Face> other_face(const std::shared_ptr<Face> f);
    void add_face(const std::shared_ptr<Face> f);

    double length() const;
};

class Face
{
private:

    unsigned int m_id;
    glm::dvec3 m_normal;
    glm::dvec4 m_color;

    std::vector<std::shared_ptr<Vertex>> m_vertices; // should always contain 4 vertices
    std::vector<std::shared_ptr<Edge>> m_edges;      // should always contain 4 edges

public:

    Face(unsigned int id, const std::vector<std::shared_ptr<Vertex>>& vertices);
    ~Face();

    unsigned int id() const;
    glm::dvec3 normal() const;
    glm::dvec4 color() const;

    void compute_normal();
    void set_color(const glm::dvec4& c);

    const std::vector<std::shared_ptr<Vertex>>& vertices() const;
    std::vector<std::shared_ptr<Vertex>>& vertices();
    const std::vector<std::shared_ptr<Edge>>& edges() const;
    std::vector<std::shared_ptr<Edge>>& edges();
    size_t num_vertices() const;
    size_t num_edges() const;
    
    const glm::dvec3 centroid() const;

    bool contains_xy_point(const glm::dvec3& point) const;
    glm::dvec3 bilinear_interpolate_xy_vector(const glm::dvec3& point) const;
};



class QuadMesh
{
private:

    std::vector<std::shared_ptr<Vertex>> m_vertices;
    std::vector<std::shared_ptr<Edge>> m_edges;
    std::vector<std::shared_ptr<Face>> m_faces;

	glm::dvec3 m_midpoint = glm::dvec3(0.0, 0.0, 0.0);
	double radius = 0.0;

public:

    QuadMesh();
    QuadMesh(const char* filename); // load from PLY file
    QuadMesh(const QuadMesh& base_mesh, double step_size, int num_steps);
    ~QuadMesh();

    const std::vector<std::shared_ptr<Vertex>>& vertices() const;
    const std::vector<std::shared_ptr<Edge>>& edges() const;
    const std::vector<std::shared_ptr<Face>>& faces() const;

    size_t num_vertices() const;
    size_t num_edges() const;
    size_t num_faces() const;

    glm::dvec3 midpoint() const;
    double get_radius() const;

    void compute_face_normals();
    void average_vertex_normals();
    void compute_midpoint_and_radius();

    void print_info() const;

    void get_min_max_scalar(double& min_scalar, double& max_scalar) const;
    void set_height_from_scalar(double factor);
    void reset_vertex_positions();
    void get_min_max_coords(double& min_x, double& max_x, 
                            double& min_y, double& max_y,
                            double& min_z, double& max_z) const;

    double get_grid_spacing() const;

    const std::shared_ptr<Face> get_face_containing_xy_point(const glm::dvec3& point) const;

    glm::dvec3 take_xy_streamline_step(const glm::dvec3& current_pos,
        const std::shared_ptr<Face>& current_face, std::shared_ptr<Face>& next_face,
        double step_size, int direction) const;

    void compute_xy_streamline(std::vector<glm::dvec3>& streamline,
        const glm::dvec3& start_pos, const std::shared_ptr<Face>& start_face,
        double step_size, int num_steps) const;


private:

    void construct_simple_quad_mesh();
    void vertex_to_face_pointers();
    void set_up_edges();
    void reorder_vertex_pointers();
};