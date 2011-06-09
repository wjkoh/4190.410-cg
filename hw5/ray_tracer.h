#ifndef _RAY_TRACER_H_
#define _RAY_TRACER_H_

#include "common.h"
#include "object.h"
#include "scene.h"

class ray_tree_node
{
    public:
    ray_tree_node(const scene& s, const ray& in_ray, int depth = 0)
        : in_ray(in_ray), t(), r(), local_illu(BACKGND_COLOR), specular(0, 0, 0), transparency(1)
    {
    }

    void process(const scene& s, const ray& in_ray, float time, int depth = 0)
    {
        intersect_info min_info(in_ray);
        if (BSP_ENABLED)
        {
            min_info = s.tree.traverse(in_ray);
        }
        else
        {
            for (auto i = s.objs.begin(); i != s.objs.end(); ++i)
            {
                intersect_info tmp_info = (*i)->check(in_ray, time);
                if (tmp_info.intersect && tmp_info.dist < min_info.dist)
                {
                    min_info = tmp_info;
                }
            }
        }
        auto min_i = min_info.obj;

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
                // soft shadows
                int hit_count = 0;
                for (int j = 0; j < SHADOW_RAY*SHADOW_RAY; ++j)
                {
                    point3 ray_pt = i->get_jittered_pos(j, time);
                    vector3 ray_vec = ray_pt - pt;
                    float ray_dist = ray_vec.length();

                    ray s_ray(pt, ray_vec); // shadow ray

#if BSP_ENABLED
                    intersect_info&& min_info = s.tree.traverse(s_ray);
                    if (!min_info.intersect || min_info.dist > ray_dist)
                        ++hit_count;
#else
                    auto result = std::find_if(s.objs.begin(), s.objs.end(),
                                               [s_ray, ray_dist, time](std::shared_ptr<object> obj)
                                               -> bool
                                               {
                                               intersect_info info = obj->check(s_ray, time);
                                               return info.intersect && info.dist < ray_dist;
                                               });
                    if (result == s.objs.end())
                        ++hit_count;
#endif
                }

                if (hit_count > 0)
                {
                    lights.push_back(*i);
                    lights.back().intensity *= (float)hit_count/((float)SHADOW_RAY*SHADOW_RAY);
                }
            }
            DEBUG_MODE = false;

            specular = (min_i)->mat.specular;
            transparency = (min_i)->mat.transparency;
            reflection = (min_i)->mat.reflection;

            local_illu = ((min_i)->mat.ambient)*s.g_amb_light;
            {
                vector3 tmp_illu; // VS2010
                tmp_illu.zero();
                std::for_each(lights.begin(), lights.end(),
                              [=, &tmp_illu](const light& lit)
                              {
                              tmp_illu += (min_i)->calc_local_illu(pt, normal, lit, in_ray.dir);
                              }
                             );
                local_illu += tmp_illu;
            }

            auto result = (min_i)->calc_reflect_refract(min_info);
            if (result.first.flag)
            {
                r = std::shared_ptr<ray_tree_node>(new ray_tree_node(s, result.first, depth)); // reflection
                r->process(s, result.first, time, depth);
            }

            if (result.second.flag)
            {
                t = std::shared_ptr<ray_tree_node>(new ray_tree_node(s, result.second, depth)); // refraction
                t->process(s, result.second, time, depth);
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

class ray_tracer
{
    public:
        ray_tracer() : image(IMG_WIDTH, std::vector<vector3>(IMG_HEIGHT)) {}
        void run(int img_width, int img_height, scene& s);
        std::vector<std::vector<vector3>> image;
};

#endif
