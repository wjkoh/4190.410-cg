#ifndef _RAY_TRACKER_H_
#define _RAY_TRACKER_H_

#include "common.h"
#include "object.h"

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
        float min_dist = numeric_limits<float>::max();
        bool flag = false;

        bool from_in_to_out = false;
        for (auto i = s.objs.begin(); i != s.objs.end(); ++i)
        {
            auto dist = (*i)->get_hit_dist(in_ray);

            bool tmp_in_out = false;
            float tmp = 0;
            if (dist.first < -EPS_F && dist.second > EPS_F) // -, +
            {
                tmp_in_out = true;
                tmp = dist.second;
            }
            else if (dist.first > EPS_F) // +, +
            {
                tmp = dist.first;
            }
            else if (dist.second > EPS_F) // 0, +
            {
                tmp_in_out = true;
                tmp = dist.second;
            }
            else // -, - && 0, 0
                continue;

            if (tmp < min_dist)
            {
                min_i = i;
                min_dist = tmp;
                flag = true;
                from_in_to_out = tmp_in_out;
            }
        }

        // shadow ray
        const point3 pt = in_ray*min_dist;
        for (auto i = s.lights.begin(); i != s.lights.end(); ++i)
        {
            ray ray1(pt, i->get_pos() - pt); // shadow ray
            auto result = find_if(s.objs.begin(), s.objs.end(), [ray1](shared_ptr<object> obj) { return obj->get_hit_dist(ray1).first > EPS_F; });
            if (result == s.objs.end())
                lights.push_back(*i);
        }

        ++depth;
        if (depth > MAX_DEPTH) return;

        if (flag)
        {
            specular = (*min_i)->mat.specular;
            transparency = (*min_i)->mat.transparency;

            local_illu = ((*min_i)->mat.ambient)*s.g_amb_light;
			vector3 tmp_illu = local_illu;
            for_each(lights.begin(), lights.end(), [=, &tmp_illu](const light& lit) { tmp_illu += (*min_i)->calc_local_illu(pt, lit, in_ray.dir); });
			local_illu = tmp_illu;

            //local_illu[0] = min(1.0f, local_illu[0]);
            //local_illu[1] = min(1.0f, local_illu[1]);
            //local_illu[2] = min(1.0f, local_illu[2]);

            //cout << "DEPTH " << depth <<  endl;
            //cout << "RAy " << in_ray << " " << min_dist <<  endl;

            auto result = (*min_i)->calc_intersect(in_ray, min_dist, from_in_to_out);
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
};

vector3 traverse_tree(shared_ptr<ray_tree_node> node, int depth = 0)
{
    if (node.get() == NULL) return vector3().zero();

    ++depth;
    vector3 int_t = traverse_tree(node->t, depth);
    vector3 int_r = traverse_tree(node->r, depth);

    //if (depth == 1) return node->specular*int_r + int_t; // 1-transparency ?
    //if (depth == 1 && node->t.get() == NULL && node->r.get() == NULL) return node->local_illu;
    return node->transparency*(node->local_illu) + node->specular*int_r + (1-node->transparency)*int_t; // 1-transparency ?
}
#include <ctime>
#include <cstdlib>

class ray_tracer
{
    static const int res = 100; // 0.01

    public:
        void run(int img_width, int img_height, const scene& s)
        {
            const int JITTER = 4;
            const float RES = 0.01;
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
                    
                    for (int k = 0; k < JITTER*JITTER; ++k)
                    {
                        int off_x = k % JITTER;
                        int off_y = k / JITTER;

                        float x_random = ((float)rand() / (float)RAND_MAX)*term;
                        float y_random = ((float)rand() / (float)RAND_MAX)*term;

                        const point3 ray_org((-img_width/2 + j)*RES + off_x*term + x_random, (img_height/2 - i)*RES - off_y*term - y_random, 0);

                        // calculate an intensity of light
                        shared_ptr<ray_tree_node> root(new ray_tree_node(s, ray(ray_org)));
                        root->process(s, ray(ray_org), 0);

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
