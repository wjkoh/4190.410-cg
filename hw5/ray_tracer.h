#ifndef _RAY_TRACER_H_
#define _RAY_TRACER_H_

#include "common.h"
#include "object.h"
#include "triangle.h"
#include "polyhedron.h"

class scene
{
    public:
        scene()
            : g_amb_light(0.4, 0.4, 0.4)
        {}

        void move_scene(const vector3& delta = vector3(0, 0, 0))
        {
            for (auto i = objs.begin(); i != objs.end(); ++i)
                (*i)->set_pos((*i)->get_pos() + c_o_lens + delta);

            for (auto i = lights.begin(); i != lights.end(); ++i)
                i->set_pos(i->get_pos() + c_o_lens + delta);
        }

        //octree tree;
        std::vector<std::shared_ptr<object> > objs;
        std::vector<light> lights;
        vector3 g_amb_light;
};

class ray_tree_node
{
    public:
    ray_tree_node(const scene& s, const ray& in_ray, int depth = 0)
        : in_ray(in_ray), t(), r(), local_illu(BACKGND_COLOR), specular(0, 0, 0), transparency(1)
    {
    }

    void process(const scene& s, const ray& in_ray, int depth)
    {
        auto min_i = s.objs.end();
        intersect_info min_info(in_ray);

        for (auto i = s.objs.begin(); i != s.objs.end(); ++i)
        {
            intersect_info tmp_info = (*i)->check(in_ray);
            if (tmp_info.intersect && tmp_info.dist < min_info.dist)
            {
                min_i = i;
                min_info = tmp_info;
            }
        }

        ++depth;
        if (depth > MAX_DEPTH) return;

        if (min_info.intersect)
        {
            // shadow ray
            const point3&& pt = min_info.get_pt();
            const vector3& normal = min_info.normal;
            
            lights.clear();
            DEBUG_MODE = true;
            for (auto i = s.lights.begin(); i != s.lights.end(); ++i)
            {
                ray ray1(pt, i->get_pos() - pt); // shadow ray

                auto result = find_if(s.objs.begin(), s.objs.end(), [ray1](std::shared_ptr<object> obj) { return obj->check(ray1).intersect; });
                if (result == s.objs.end())
                    lights.push_back(*i);
            }
            DEBUG_MODE = false;

            specular = (*min_i)->mat.specular;
            transparency = (*min_i)->mat.transparency;
            reflection = (*min_i)->mat.reflection;

            local_illu = ((*min_i)->mat.ambient)*s.g_amb_light;
            for_each(lights.begin(), lights.end(), [=, &local_illu](const light& lit) { local_illu += (*min_i)->calc_local_illu(pt, normal, lit, in_ray.dir); });

            auto result = (*min_i)->calc_reflect_refract(min_info);
            if (result.first.flag)
            {
                r = std::shared_ptr<ray_tree_node>(new ray_tree_node(s, result.first, depth)); // reflection
                r->process(s, result.first, depth);
            }

            if (result.second.flag)
            {
                t = std::shared_ptr<ray_tree_node>(new ray_tree_node(s, result.second, depth)); // refraction
                t->process(s, result.second, depth);
            }
        }
    }

    ~ray_tree_node()
    {
        //delete t;
        //t = NULL;
        //delete r;
        //r = NULL;
    }

    ray in_ray;
    std::shared_ptr<ray_tree_node> t;
    std::shared_ptr<ray_tree_node> r;
    std::vector<light> lights; // shadow rays (light std::vector)

    vector3 local_illu;

    vector3 specular;
    float transparency;
    float reflection;
};

inline vector3 traverse_tree(std::shared_ptr<ray_tree_node> node, int depth = 0)
{
    if (node.get() == NULL) return BACKGND_COLOR;

    ++depth;
    vector3 int_t = traverse_tree(node->t, depth);
    vector3 int_r = traverse_tree(node->r, depth);

    if (node->r.get() == NULL) return int_t;

    return (
            node->transparency*(node->local_illu)
            + node->reflection*(node->specular*int_r)
            + (1-node->transparency)*(int_t)
            );
}

#include <ctime>
#include <cstdlib>

class ray_tracer
{
    public:
        void run(int img_width, int img_height, scene& s);
        vector3 image[IMG_WIDTH][IMG_HEIGHT];
};

#endif
