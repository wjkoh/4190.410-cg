#ifndef _POLYHEDRON_H_
#define _POLYHEDRON_H_

#include "common.h"
#include "object.h"
#include "triangle.h"

template<int NUM_OF_FACES> // 6, 20
class polyhedron : public sphere
{
    public:
        polyhedron(const vector3& pos, float r);

        virtual int get_size() const { return triangles.size(); }
        virtual std::shared_ptr<const object> get_item(int idx) const { return triangles[idx]; }
        virtual std::shared_ptr<object> get_item(int idx) { return triangles[idx]; }

        virtual intersect_info check(const ray& in_ray, float time, std::pair<float, float>) const
        {
            intersect_info result(in_ray);

            // check a bounding sphere first
            if (!sphere::check(in_ray, time, sphere::get_hit_dist(in_ray, time)).intersect)
            {
                result.intersect = false;
                return result;
            }

            // check triangles
            result.dist = std::numeric_limits<float>::max();
            for (auto i = triangles.begin(); i != triangles.end(); ++i)
            {
                intersect_info tmp_info = (*i)->check(in_ray, time);
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

        //vector3 get_normal(const point3& pt) const { assert(false); return (pt - get_pos(time)).normalize(); }
        virtual void set_pos(const point3& new_pos) { pos = new_pos; make_polygon(); }

        void make_polygon();

        void polygon(int i, int j, int k, int l = -1)
        {
            std::shared_ptr<triangle> t(new triangle(vertices[i] + pos, vertices[j] + pos, vertices[k] + pos));
            //std::cout << "vert " << vertices[i] << " " << vertices[j] << " " << vertices[k] << std::endl;
            //std::cout << pos << std::endl;
            t->mat = mat;
            t->refr_idx = refr_idx;
            t->texture = texture;
            t->bump_map = bump_map;
            //std::cout << "vert2 " << (*t)[0] << " " << (*t)[1] << " " << (*t)[2] << std::endl;

            //(*t).set_vertex((*t)[0] + pos, (*t)[1] + pos, (*t)[2] + pos);
            triangles.push_back(t);

            if (l >= 0)
            {
                std::shared_ptr<triangle> t(new triangle(vertices[k], vertices[l], vertices[i]));
                t->mat = mat;
                t->refr_idx = refr_idx;
                t->texture = texture;
                t->bump_map = bump_map;
                t->inverted_xy = true;

                (*t).set_vertex((*t)[0] + pos, (*t)[1] + pos, (*t)[2] + pos);
                triangles.push_back(t);
            }
        }

    private:
        std::vector<std::shared_ptr<triangle>> triangles;
        vector3 vertices[12]; /* 12 vertices with x, y, z coordinates */
        float r; // radius
};

template<>
inline void polyhedron<20>::make_polygon()
{
    triangles.clear();
    polygon(0,1,2);
    polygon(0,2,3);
    polygon(0,3,4);
    polygon(0,4,5);
    polygon(0,5,1);
    polygon(7,6,11);
    polygon(8,7,11);
    polygon(9,8,11);
    polygon(10,9,11);
    polygon(6,10,11);
    polygon(6,2,1);
    polygon(7,3,2);
    polygon(8,4,3);
    polygon(9,5,4);
    polygon(10,1,5);
    polygon(6,7,2);
    polygon(7,8,3);
    polygon(8,9,4);
    polygon(9,10,5);
    polygon(10,6,1);
}

template<>
inline void polyhedron<6>::make_polygon()
{
    triangles.clear();
    polygon(0,1,2,3);
    polygon(7,6,5,4);
    polygon(4,5,1,0);
    polygon(5,6,2,1);
    polygon(6,7,3,2);
    polygon(7,4,0,3);
}

// typpedefs
typedef polyhedron<6> cube;
typedef polyhedron<20> icosahedron;

#endif // _POLYHEDRON_H_
