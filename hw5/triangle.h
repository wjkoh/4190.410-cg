#ifndef _TRIANGLE_H_
#define _TRIANGLE_H_

#include "common.h"
#include "object.h"

typedef quaterniond_p plane_t; // ax+by+cz+d=0 -> (a, b, c, d)

class triangle : public object
{
    public:
        triangle(const vector3& v0, const vector3& v1, const vector3& v2)
        {
            v[0] = v0;
            v[1] = v1;
            v[2] = v2;

            n[0] = n[1] = n[2] = get_normal(v[0]);
        }

        triangle(const vector3& v0, const vector3& v1, const vector3& v2,
                   const vector3& n0, const vector3& n1, const vector3& n2)
        {
            v[0] = v0;
            v[1] = v1;
            v[2] = v2;

            n[0] = n0;
            n[1] = n1;
            n[2] = n2;
        }

        void set_vertex(const vector3& v0, const vector3& v1, const vector3& v2)
        {
            v[0] = v0;
            v[1] = v1;
            v[2] = v2;
        }

        void set_normal(const vector3& n0, const vector3& n1, const vector3& n2)
        {
            n[0] = n0;
            n[1] = n1;
            n[2] = n2;
        }

        vector3 get_normal(const point3&) const
        {
            vector3 ba = v[0] - v[1];
            vector3 bc = v[2] - v[1];
            return unit_cross(bc, ba);
        }

        plane_t get_plane() const
        {
            vector3 normal = get_normal(v[0]);
            return plane_t(normal, -dot(normal, v[1])); // 0, 1, 2 아무거나 상관없음
        }

        const vector3& operator[](int idx) const    { return v[idx]; }
        vector3& operator[](int idx)                { return v[idx]; }

        /*
        bool operator<(const triangle& rhs) const
        { 
            if (!this->cube && rhs.cube) return true;
            else if (this->cube && !rhs.cube) return false;
            return this->area() < rhs.area();
            //return (!this->cube && rhs.cube) || ( || (this->area() < rhs.area());
        }
        */

        float area() const
        {
            vector3 ba = v[0] - v[1];
            vector3 bc = v[2] - v[1];
            return cross(ba, bc).length() / 2;
        }

        std::list<triangle> split() const;

        std::pair<float, float> get_barycentric_coord(const vector3& p) const
        {
            // Compute vectors        
            const vector3 v0 = v[0] - v[1];
            const vector3 v1 = v[2] - v[1];
            const vector3 v2 = p - v[1];

            // Compute dot products
            float dot00 = dot(v0, v0);
            float dot01 = dot(v0, v1);
            float dot02 = dot(v0, v2);
            float dot11 = dot(v1, v1);
            float dot12 = dot(v1, v2);

            // Compute barycentric coordinates
            float invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
            float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
            float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

            return std::make_pair(u, v);
        }

        std::pair<float, float> get_hit_dist(const ray& ray) const
        {
            const plane_t plane = get_plane();
            double s = -(plane.real() + dot(plane.imaginary(), ray.org))/dot(plane.imaginary(), ray.dir);
            if (fabs(s) <= FABS_EPS_F) s = 0.0;

            const point3 pt = ray*s;
            std::pair<float, float> u_v = get_barycentric_coord(pt);
            const bool is_in_triangle = (u_v.first >= -FABS_EPS_F) && (u_v.second >= -FABS_EPS_F) && (u_v.first + u_v.second <= 1 + FABS_EPS_F);

            if (!is_in_triangle)
                return std::make_pair(-1.0, -1.0);

            return std::make_pair(s, s);
        }

        vector3 v[3];   // 3 points of a triangle
        vector3 n[3];   // normal vector
};

inline std::ostream& operator<<(std::ostream& os, const triangle& t)
{
    os << "TRIANGLE ";
    os << t[0] << " | ";
    os << t[1] << " | ";
    os << t[2] << std::endl;
    return os;
}

#endif
