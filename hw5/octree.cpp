#include "octree.h"
using namespace std;

intersect_info octree_node::check(const ray& eye_ray, float time) const
{
    // Out of a cube
    assert(is_ray_in(eye_ray));

    // Point is in this cube
    // 1. Leaf node
    if (child[0] == NULL)
    {
        // test objects
        intersect_info&& info = check_objs(eye_ray, time);
        if (info.intersect) return info;

        // find exit point
        float s_out = std::numeric_limits<float>::max();
        for (int i = 0; i < 3; ++i)
        {
            float dot_n_k_dir = dot_n_k(i, eye_ray.dir);
            if (dot_n_k_dir > FABS_EPS_F)
            {
                float tmp_s = (-get_d_k(i) - dot_n_k(i, eye_ray.org))/dot_n_k_dir;
                if (tmp_s < s_out)
                    s_out = tmp_s;
            }
        }
        return intersect_info(ray(eye_ray*s_out, eye_ray.dir));
    }

    // 2. node
    ray new_ray(eye_ray);
    while (true)
    {
        auto it = find_if(child, child + 6, [new_ray](const octree_node* node) { return node->is_ray_in(new_ray); });
        if (it == child + 6) break;

        intersect_info&& info = (*it)->check(new_ray, time);
        if (info.intersect) return info;

        new_ray = info.in_ray;
    }
    return intersect_info(new_ray);
}

intersect_info octree_node::check_objs(const ray& eye_ray, float time) const
{
    intersect_info min_info(eye_ray);
    for (auto i = objs.begin(); i != objs.end(); ++i)
    {
        intersect_info tmp_info = (*i)->check(eye_ray, time);
        if (tmp_info.intersect && tmp_info.dist < min_info.dist)
        {
            min_info = tmp_info;
        }
    }
    return min_info;
}

intersect_info octree::start(ray eye_ray, float time) const
{
    // Out of a root cube
    if (!root.is_pt_in(eye_ray.org))
    {
        bool flag = false;
        point3 pt_in = eye_ray.org;
        float s_in = std::numeric_limits<float>::max();
        for (int i = 0; i < 3; ++i)
        {
            float dot_n_k_dir = root.dot_n_k(i, eye_ray.dir);
            if (dot_n_k_dir < -FABS_EPS_F)
            {
                float tmp_s = (-root.get_d_k(i) - root.dot_n_k(i, eye_ray.org))/dot_n_k_dir;
                if (tmp_s < s_in)
                {
                    point3 tmp_pt = eye_ray*tmp_s;
                    if (root.is_pt_in(tmp_pt))
                    {
                        s_in = tmp_s;
                        pt_in = tmp_pt;
                        flag = true;
                    }
                }
            }
        }

        if (flag)
        {
            eye_ray.org = pt_in;
        }
        else
            return intersect_info(eye_ray);
    }
    if (root.is_ray_in(eye_ray)) return root.check(eye_ray, time); // intersect!
    return intersect_info(eye_ray);
}
