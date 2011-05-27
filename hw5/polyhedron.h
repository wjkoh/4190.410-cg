#ifndef _POLYHEDRON_H_
#define _POLYHEDRON_H_

#include "common.h"
#include "object.h"
#include "triangle.h"

template<int NUM_OF_FACES> // 6, 20
class polyhedron : public sphere
{
    public:
        polyhedron(const vector3& pos, float r = 1.5);

        virtual intersect_info check(const ray& in_ray, std::pair<float, float>)
        {
            intersect_info result(in_ray);

            // check a bounding sphere first
            if (!sphere::check(in_ray, sphere::get_hit_dist(in_ray)).intersect)
            {
                result.intersect = false;
                return result;
            }

            // check triangles
            result.dist = std::numeric_limits<float>::max();
            for (auto i = triangles.begin(); i != triangles.end(); ++i)
            {
                intersect_info tmp_info = i->check(in_ray);
                if (tmp_info.intersect && tmp_info.dist < result.dist)
                {
                    result = tmp_info;
                    if (dot(tmp_info.normal, in_ray.dir) > FABS_EPS_F)
                    {
                        //result.from_in_to_out = true;
                        //result.normal = -result.normal;
                    }
                }
            }

            /*
            if (result.from_in_to_out)
                in_ray.refr_idx = refr_idx;
                */

            return result;
        }

        //vector3 get_normal(const point3& pt) const { assert(false); return (pt - pos).normalize(); }

        void polygon(int i, int j, int k, int l = -1)
        {
            triangle t(vertices[i], vertices[j], vertices[k]);
            //t.mat = mat;
            std::cout << mat.transparency << std::endl;

            t[0] += pos;
            t[1] += pos;
            t[2] += pos;
            triangles.push_back(t);

            if (l >= 0)
            {
                triangle t2(vertices[k], vertices[l], vertices[i]);
                //t2.mat = mat;

                t2[0] += pos;
                t2[1] += pos;
                t2[2] += pos;
                triangles.push_back(t2);
            }
        }

    private:
        std::vector<triangle> triangles;
        vector3 vertices[12]; /* 12 vertices with x, y, z coordinates */
        float r; // radius
};

// typpedefs
typedef polyhedron<6> cube;
typedef polyhedron<20> icosahedron;

#endif // _POLYHEDRON_H_
