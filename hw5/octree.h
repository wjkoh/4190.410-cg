#ifndef _OCTREE_H_
#define _OCTREE_H_

#include "common.h"
#include "object.h"

enum
{
    RIGHT = 0,
    TOP,
    FRONT,
    LEFT,
    BOTTOM,
    BACK
};

// Axis-aligned Bounding Box
class octree_node
{
    public:
        octree_node()
        {
        }

        point3 get_mid() const { return (rt + lb)/2.0; }

        bool is_pt_in(const point3& pt) const { return (pt <= rt && pt >= lb); }
        bool is_ray_in(const ray& eye_ray) const
        {
            if (is_pt_in(eye_ray.org))
            {
                vector3 v = normalize(get_mid() - eye_ray.org);
                return (fabs(dot(v, eye_ray.dir)) >= -FABS_EPS_F); // v.dir >= 0
            }
            return false;
        }

        float get_d_k(int face_num) const
        {
            // Dk
            // top -y
            // bottom y
            // front -z
            // back z 
            // left x
            // right -x

            switch (face_num)
            {
                case TOP: return -rt[face_num % 3];
                case BOTTOM: return lb[face_num % 3];
                case FRONT: return -rt[face_num % 3];
                case BACK: return lb[face_num % 3];
                case RIGHT: return -rt[face_num % 3];
                case LEFT: return lb[face_num % 3];
            }
        }

        float dot_n_k(int face_num, const vector3& v) const { return (face_num > 2 ? -1 : 1) * v[face_num % 3]; }

        intersect_info check(const ray& eye_ray, float time) const;
        intersect_info check_objs(const ray& eye_ray, float time) const;

        point3 lb; // Left-bottom
        point3 rt; // Right-top
        std::vector<std::shared_ptr<object>> objs; // objects in this cube

        octree_node* child[8];
};

class octree
{
    public:
        intersect_info start(ray eye_ray, float time) const;

        octree_node root;
};

#endif // _OCTREE_H_
