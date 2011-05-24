#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "common.h"
#include "material.h"
#include <utility>
#include <iostream>

struct intersect_info
{
    intersect_info(const float dist, const point3& pt, const vector3& refl, const vector3& refr)
        : dist(dist), pt(pt), refl(refl), refr(refr)
    {}

    const float dist; // ray.org + dist*ray.dir
    const point3 pt;
    const vector3 refl;
    const vector3 refr;
};

class ray
{
    public:
        ray(const point3& org, const vector3& dir = -Z, float refr_idx = REFR_AIR)
            : org(org), dir(normalize(dir)), refr_idx(refr_idx)
        {
            flag = true;
        }

        point3 operator*(const float s) const { return org + s*dir; }

    vector3 org;
    vector3 dir;
    float refr_idx;
    bool flag;
};

inline std::ostream& operator<<(std::ostream& os, const ray& r)
{
    os << "ray " << r.org << "/" << r.dir;
    return os;
}

class light;
class object
{
    public:
        object(const vector3& pos = vector3(0, 0, -1))
            : pos(pos), refr_idx(REFR_GLASS)
        {
        }

        virtual ~object() {}

        virtual float get_hit_dist(const ray& ray) const = 0;
        virtual vector3 get_normal(const point3& pt) const  = 0; // Phong shading
        point3 get_pos() const { return pos; }

        vector3 calc_local_illu(const point3& pt, const light& light, const vector3& v) const;

        // intersect with ray
        std::pair<ray, ray> calc_intersect(const ray& in_ray, const float dist)
        {
            //const float s = get_hit_dist(ray);
            const point3 pt = in_ray*dist;
            const point3 pt2 = in_ray*(dist + 0.000001);

            vector3 n = get_normal(pt);
            const vector3& u = in_ray.dir;

            float out_refr_idx = refr_idx;
            /*
            if (dot(u, n) < -EPS_F)
            {
                out_refr_idx = REFR_AIR;
                n = -n;
            }
            */

            vector3 refl = get_reflect(n, u);
            const vector3 refr = get_refract(n, u, in_ray.refr_idx, out_refr_idx);

            ray refl_ray(pt, refl, in_ray.refr_idx);
            ray refr_ray(pt2, refr, out_refr_idx);

            if (dot(in_ray.dir, pt - pos) >= 0)
            {
                refl_ray.flag = false;
            }
            return std::make_pair(refl_ray, refr_ray);
        }

        material mat;

    protected:
        point3 pos;
        float refr_idx;
};

class sphere : public object
{
    public:
        sphere() : object(), r(1.0) {}
        sphere(const vector3& pos) : object(pos), r(1.0) {}

        float get_hit_dist(const ray& ray) const
        {
            //std::cout << ray.org << " " << ray.dir << std::endl;

            const vector3 d_p = pos - ray.org;
            const float det = pow(r, 2) - (d_p - dot(ray.dir, d_p)*ray.dir).length_squared();
            if (det < -EPS_F)
                return -1.0;

            float x = dot(ray.dir, d_p) - sqrt(det);
            return dot(ray.dir, d_p) - sqrt(det); // + 는 outgoing
        }

        vector3 get_normal(const point3& pt) const { return (pt - pos).normalize(); }
        
    private:
        float r; // radius
};

/*
class polygons : public object
{
    public:

    private:
        vector<triangle> polygons;
};
*/

class light : public object // point, directional, area
{
    public:
        light()
            : object(vector3(0, 3, -1)), intensity(1.0, 1.0, 1.0)
        {}
        light(const vector3& pos)
            : object(pos), intensity(1.0, 1.0, 1.0)
        {}

        float get_hit_dist(const ray& ray) const
        {
            const float s = length(pos - ray.org);
            if (s < -EPS_F) return -1.0;
            return s;
        }

        vector3 get_normal(const point3& pt) const { return (pt - pos).normalize(); }

        vector3 intensity;
    private:
        vector3 dir;
};
/*
*/

#endif //_OBJECT_H_
