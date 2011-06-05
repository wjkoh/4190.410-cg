#ifndef _TRIANGLE_H_
#define _TRIANGLE_H_

#include "common.h"
#include "object.h"


class triangle : public object
{
    public:
        triangle(const vector3& v0, const vector3& v1, const vector3& v2)
            : object((v0 + v1 + v2)/3.0)
        {
            set_vertex(v0, v1, v2);

            vector3 n = get_normal(v[0], 0.0);
            set_normal(n, n, n);
        }

        triangle(const vector3& v0, const vector3& v1, const vector3& v2,
                   const vector3& n0, const vector3& n1, const vector3& n2)
            : object((v0 + v1 + v2)/3.0)
        {
            set_vertex(v0, v1, v2);
            set_normal(n0, n1, n2);
        }

        void set_vertex(const vector3& v0, const vector3& v1, const vector3& v2)
        {
            v[0] = v0;
            v[1] = v1;
            v[2] = v2;
            pos = (v0 + v1 + v2)/3.0;
        }

        void set_normal(const vector3& n0, const vector3& n1, const vector3& n2)
        {
            n[0] = n0;
            n[1] = n1;
            n[2] = n2;
        }

        vector3 get_normal(const point3&, const float time) const
        {
            vector3 ba = v[0] - v[1];
            vector3 bc = v[2] - v[1];
            return unit_cross(bc, ba);
        }

        virtual plane_t get_plane(const float time) const
        {
            vector3 normal = get_normal(v[0], time);
            return plane_t(normal, -dot(normal, v[1])); // 0, 1, 2 아무거나 상관없음
        }

        virtual int front_or_back(const plane_t& p) const
        {
            int f_or_b[3] = {0};

            for (int i = 0; i < 3; ++i)
            {
                float det = dot(p.imaginary(), (*this)[i]) + p.real();

                f_or_b[i] = BSP_OVERLAP;
                if (det > 2*std::numeric_limits<float>::epsilon())
                    f_or_b[i] = BSP_FRONT;
                else if (det < -2*std::numeric_limits<float>::epsilon())
                    f_or_b[i] = BSP_BACK;
            }

            // FFF or BBB or OOO
            if (f_or_b[0] == f_or_b[1] && f_or_b[1] == f_or_b[2])
                return f_or_b[0];

            int tmp = BSP_OVERLAP;
            for (int i = 0; i < 3; ++i)
            {
                if (f_or_b[i] == BSP_OVERLAP) continue;

                if (tmp == BSP_OVERLAP)
                    tmp = f_or_b[i];
                else if (tmp != f_or_b[i]) // F,B or B,F
                    return BSP_INTERSECT;
            }
            return tmp;
            //return BSP_OVERLAP;
        }

        const vector3& operator[](int idx) const    { return v[idx]; }
        vector3& operator[](int idx)                { return v[idx]; }

        virtual void set_pos(const point3& new_pos)
        {
            vector3 delta = new_pos - pos;
            v[0] += delta;
            v[1] += delta;
            v[2] += delta;
            pos = new_pos;
        }

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

        std::pair<float, float> get_hit_dist(const ray& ray, const float time) const
        {
            const plane_t plane = get_plane(time);
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
