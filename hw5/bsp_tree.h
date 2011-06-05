#ifndef _BSP_TREE_
#define _BSP_TREE_

#include "common.h"

class object;
class scene;
class intersect_info;
class ray;

class bsp_tree_node
{
    public:
        bsp_tree_node(const plane_t& p);
        ~bsp_tree_node();

        void add(std::shared_ptr<const object> obj, bool leaf = false, int depth = 0);
        intersect_info traverse(const ray& eye_ray) const;

    public:
        plane_t plane;
        bsp_tree_node* front;
        bsp_tree_node* back;

        std::vector<std::shared_ptr<const object>> intersect;
        std::vector<std::shared_ptr<const object>> overlap;
};

class bsp_tree
{
    public:
        void build(scene& s);
        intersect_info traverse(const ray& eye_ray) const;

        std::shared_ptr<bsp_tree_node> root;
};

#endif // _BSP_TREE_
