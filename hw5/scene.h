#ifndef _SCENE_H_
#define _SCENE_H_

#include "common.h"
#include "octree.h"
#include "bsp_tree.h"

class scene
{
    public:
        scene(int scene_num);

        void move_scene(const vector3& delta = vector3(0, 0, 0))
        {
            for (auto i = objs.begin(); i != objs.end(); ++i)
                (*i)->set_pos((*i)->get_pos(0.0) + c_o_lens + delta);

            for (auto i = lights.begin(); i != lights.end(); ++i)
                i->set_pos(i->get_pos(0.0) + c_o_lens + delta);
        }

        //octree tree;
        bsp_tree tree;
        std::vector<std::shared_ptr<object> > objs;
        std::vector<light> lights;
        vector3 g_amb_light;
};

#endif // _SCENE_H_