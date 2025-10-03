#include "trackball.h"

Trackball::Trackball(float size)
{
    m_size = size;
}

Trackball::~Trackball()
{
}

/*
    Simulate a track-ball. Project the points onto the virtual
    trackball, then figure out the axis of rotation, which is the cross
    product of P1 P2 and O P1 (O is the center of the ball, 0,0,0)
    Note: This is a deformed trackball-- is a trackball in the center,
    but is deformed into a hyperbolic sheet of rotation away from the
    center. This particular function was chosen after trying out
    several variations.

    It is assumed that the arguments to this routine are in the range
    (-1.0 ... 1.0)
*/
glm::quat Trackball::getRotationQuat(float p1x, float p1y, float p2x, float p2y, const glm::quat &old_rot)
{
    glm::quat q; // quaternion for mouse-input rotation
    glm::vec3 a; // Axis of rotation
    float phi;   // how much to rotate about axis
    glm::vec3 p1, p2, d;
    float t;

    if (p1x == p2x && p1y == p2y)
    {
        return old_rot; // no new rotation
    }

    // First, figure out z-coordinates for projection of P1 and P2 to deformed sphere
    p1 = glm::vec3(p1x, p1y, project_to_sphere(m_size, p1x, p1y));
    p2 = glm::vec3(p2x, p2y, project_to_sphere(m_size, p2x, p2y));

    // now we want the cross product of P1 and P2
    a = glm::cross(p2,p1);

    // Figure out how much to rotate around that axis.
    d = p1 - p2;
    t = glm::length(d) / (2.0f * m_size);
    // Avoid problems with out-of-control values...
    if (t > 1.0f) t = 1.0f;
    if (t < -1.0f) t = -1.0f;
	phi = -2.0f * glm::asin(t); // needs to be negative for some reason to work with glfw window

    // get the quaternion from the axis and angle
    q = glm::angleAxis(phi, glm::normalize(a));

    // add q to the old rotation to get new rotation
    glm::quat new_rot = q * old_rot;

    // normalize every so often to prevent scaling errors from building up
    if (m_add_count++ > m_normalize_count)
    {
        new_rot = glm::normalize(new_rot);
        m_add_count = 0;
    }
         
    return new_rot;
}

/*
    Project an x,y pair onto a sphere of radius r OR a hyperbolic sheet
    if we are away from the center of the sphere.
*/
float Trackball::project_to_sphere(float r, float x, float y)
{
	float d, t, z;
	d = glm::sqrt(x*x + y*y);
	if (d < r*0.707)  	            // inside sphere
	    z = glm::sqrt(r*r - d*d);
	else                            // on hyperbola
	{ 			
		t = r / glm::sqrt(2.0f);
		z = t*t / d;
	}
	return z;
}