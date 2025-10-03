
// glm virtual trackball implementation adapted from Silcon Graphics 
// code written by Gavin Bell November 1988
#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Trackball
{
private:

    float m_size = 0.8f; // virtual trackball size
    // best if it is the distance from the center of rotation
    // to the point on the object underneath the mouse

	// make sure we normalize the quaternion every so often
	// to keep error from creeping in (works best if we just normalize every time)
    unsigned int m_add_count = 0;
	const unsigned int m_normalize_count = 1;

public:

    Trackball(float size = 0.8f);
    ~Trackball();

    /*
        Pass the x and y coordinates of the last and current positions of
        the mouse, scaled so they are from (-1.0 ... 1.0).
        
        if ox,oy is the window's center and sizex,sizey is its size, then
        the proper transformation from screen coordinates (sc) to world
        coordinates (wc) is:
        wcx = (2.0 * (scx-ox)) / (float)sizex - 1.0
        wcy = (2.0 * (scy-oy)) / (float)sizey - 1.0
        
        The resulting rotation is returned as a quaternion rotation.
    */
    glm::quat getRotationQuat(float p1x, float p1y, float p2x, float p2y, const glm::quat &old_rot);

private:

    // Project an x,y pair onto a sphere of radius r
    float project_to_sphere(float r, float x, float y);
};