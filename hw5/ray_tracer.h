#ifndef _RAY_TRACKER_H_
#define _RAY_TRACKER_H_

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

        //octree tree;
        vector<shared_ptr<object> > objs;
        vector<light> lights;
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

                auto result = find_if(s.objs.begin(), s.objs.end(), [ray1](shared_ptr<object> obj) { return obj->check(ray1).intersect; });
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
                r = shared_ptr<ray_tree_node>(new ray_tree_node(s, result.first, depth)); // reflection
                r->process(s, result.first, depth);
            }

            if (result.second.flag)
            {
                t = shared_ptr<ray_tree_node>(new ray_tree_node(s, result.second, depth)); // refraction
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
    shared_ptr<ray_tree_node> t;
    shared_ptr<ray_tree_node> r;
    vector<light> lights; // shadow rays (light vector)

    vector3 local_illu;

    vector3 specular;
    float transparency;
    float reflection;
};

vector3 traverse_tree(shared_ptr<ray_tree_node> node, int depth = 0)
{
    if (node.get() == NULL) return BACKGND_COLOR; //vector3().zero();

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
        void run(int img_width, int img_height, const scene& s)
        {
            const float term = RES / (float)JITTER;

            /* initialize random seed: */
            srand(time(NULL));
            
            cout << (float)rand() / (float)RAND_MAX << endl;
            for (int i = 0; i < img_height; ++i)
            {
                cout << "(" << i << ")" << endl;
                for (int j = 0; j < img_width; ++j)
                {
                    vector3 intensity;
                    intensity.zero();
                    
                    // Randomly shuffle jitter codes
                    vector<int> codes;
                    {
                    int idx = 0;
                    std::generate_n(back_inserter(codes), JITTER*JITTER, [&idx]() { return idx++; });
                    }
                    std::random_shuffle(codes.begin(), codes.end());

                    for (int k = 0; k < JITTER*JITTER; ++k)
                    {
                        int off_x = k % JITTER;
                        int off_y = k / JITTER;

                        // 아래 코드 하나로 합치지 말 것!
                        float x_random = ((float)rand()/(float)RAND_MAX)*term;
                        float y_random = ((float)rand()/(float)RAND_MAX)*term;

                        const point3 ray_org((-img_width/2 + j)*RES + off_x*term + x_random, (img_height/2 - i)*RES - off_y*term - y_random, 0);

                        //ray ray1(ray_org, normalize(ray_org - vector3(0, 0, 6))); // perspective
                        ray ray1(ray_org);
                        ray1.code = codes[k];

                        // calculate an intensity of light
                        shared_ptr<ray_tree_node> root(new ray_tree_node(s, ray1));
                        root->process(s, ray1, 0);

                        // traverse a tree
                        intensity += traverse_tree(root);
                    }
                    intensity /= JITTER*JITTER;
                    
                    // calculate an intensity of light
                    image[j][i] = intensity;
                }
            }
        }

        vector3 image[IMG_WIDTH][IMG_HEIGHT];
};

#endif
