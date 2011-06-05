#include "bsp_tree.h"
#include "object.h"
#include "ray_tracer.h"
using namespace std;

bsp_tree_node::bsp_tree_node(const plane_t& p)
: plane(p), front(NULL), back(NULL)
{
}

bsp_tree_node::~bsp_tree_node()
{
    delete front;
    front = NULL;
    delete back;
    back = NULL;
}

void bsp_tree_node::add(shared_ptr<const object> obj, bool leaf, int depth)
{
    static int max_depth = 0;

    if (depth > max_depth)
    {
        max_depth = depth;
        cout << "max depth " << max_depth << endl;
    }
    ++depth;

    int result = obj->front_or_back(plane);
    switch (result)
    {
        case BSP_FRONT:
            if (front == NULL)
            {
                if (leaf)
                {
                    intersect.push_back(obj);
                    return;
                }
                front = new bsp_tree_node(obj->get_plane(0.0));
            }
            front->add(obj, leaf, depth);
            break;
        case BSP_INTERSECT:
            intersect.push_back(obj);
            break;
        case BSP_OVERLAP:
            overlap.push_back(obj);
            break;
        case BSP_BACK:
            if (back == NULL)
            {
                if (leaf)
                {
                    intersect.push_back(obj);
                    return;
                }
                back = new bsp_tree_node(obj->get_plane(0.0));
            }
            back->add(obj, leaf, depth);
            break;
        default:
            assert(false);
            break;
    }
}

template<class T>
inline intersect_info find_min_intersect(const T& lst, const ray& eye_ray)
{
    intersect_info min_info(eye_ray);
    for (auto i = lst.begin(); i != lst.end(); ++i)
    {
        intersect_info&& tmp_info = (*i)->check(eye_ray, 0.0);
        if (tmp_info.intersect && tmp_info.dist < min_info.dist)
        {
            min_info = tmp_info;
        }
    }
    return min_info;
}

intersect_info bsp_tree_node::traverse(const ray& eye_ray) const
{
    bool eye_front = true;
    // find an eye location
    {
        const plane_t& p = plane;
        const vector3& eye_pos = eye_ray.org;

        float det = dot(p.imaginary(), eye_pos) + p.real();
        if (det > numeric_limits<float>::epsilon())
            eye_front = true;
        else if (det < -numeric_limits<float>::epsilon())
            eye_front = false;
        else
        {
            if (dot(eye_ray.dir, p.imaginary()) > 0)
                eye_front = true;
            else
                eye_front = false;
        }
    }

    float dist_to_plane = -1.0;
    if (!intersect.empty())
        dist_to_plane = -(plane.real() + dot(plane.imaginary(), eye_ray.org))/dot(plane.imaginary(), eye_ray.dir);

    // (front or back) and intersect 
    bsp_tree_node* next = front;
    if (!eye_front) next = back;

    intersect_info info1(eye_ray);
    if (next) info1 = next->traverse(eye_ray);

    intersect_info&& org_info = find_min_intersect(intersect, eye_ray);
    bool front_intersect = org_info.intersect && (org_info.dist <= dist_to_plane - FABS_EPS_F);
    if (front_intersect)
    {
        if (info1.intersect && org_info.intersect)
        {
            if (info1.dist < org_info.dist)
                return info1;
            return org_info;
        }
        if (org_info.intersect) return org_info;
    }
    if (info1.intersect) return info1;

    // overlap
    {
        intersect_info&& min_info = find_min_intersect(overlap, eye_ray);
        if (min_info.intersect) return min_info;
    }

    // front or back
    next = back;
    if (!eye_front) next = front;

    intersect_info result(eye_ray);
    if (next) result = next->traverse(eye_ray);
    if (result.intersect && org_info.intersect)
    {
        if (result.dist < org_info.dist)
            return result;
        return org_info;
    }
    if (org_info.intersect) return org_info;
    if (result.intersect) return result;

    return intersect_info(eye_ray);
}

// BSP_TREE

struct data
{
    data(pair<float, int> fit_inter, shared_ptr<const object> obj)
        : fitness(fit_inter.first), intersect(fit_inter.second), obj(obj)
    {
    }

    float fitness;
    int intersect;
    shared_ptr<const object> obj;
};

//pair<float, int> counter(const plane_t& plane, const vector<shared_ptr<const object>>& objs)
pair<float, int> counter(const plane_t& plane, const vector<shared_ptr<object>>& objs)
{
    int front = 0;
    int back = 1; // Div by zero
    int intersect = 0;
    int overlap = 0;

    for (auto it = objs.begin(); it != objs.end(); ++it)
    {
        int result = (*it)->front_or_back(plane);
        switch (result)
        {
            case BSP_FRONT:
                ++front;
                break;
            case BSP_INTERSECT:
                ++intersect;
                break;
            case BSP_OVERLAP:
                ++overlap;
                break;
            case BSP_BACK:
                ++back;
                break;
            default:
                assert(false);
                break;
        }
    }
    return make_pair((float)front/(float)(front + back), intersect);
}

void bsp_tree::build(scene& s)
{
    //vector<shared_ptr<const object>> objs;
    vector<shared_ptr<object>> objs;
    for (auto it = s.objs.begin(); it != s.objs.end(); ++it)
    {
        objs.push_back(*it);
        /*
        for (int i = 0; i < (*it)->get_size(); ++i)
        {
            objs.push_back((*it)->get_item(i));
        }
        */
    }
    cout << "objs size " << objs.size() << endl;

    const int threshold = (objs.size() - 1)/2;
    for (auto it = objs.begin(); it != objs.end(); ++it)
    {
        float min_fit = 2.0;
        int min_inter = numeric_limits<int>::max();

        if (!(*it)->is_sphere()) continue;

        sphere* obj = dynamic_cast<sphere*>((*it).get());
        assert(obj);

        vector3 min_normal = obj->plane_n;
        for (int i = 0; i < 1024; ++i)
        {
            vector3 tmp_normal;
            if (i < 3)
                tmp_normal = vector3().cardinal(i);
            else
                random_unit(tmp_normal);

            obj->plane_n = tmp_normal;

            float tmp_fit = counter(obj->get_plane(0.0), objs).first;
            float tmp_inter = counter(obj->get_plane(0.0), objs).second;
            if (tmp_inter >= threshold)
                continue;

            double tmp_f = fabs(0.5 - tmp_fit);
            double min_f = fabs(0.5 - min_fit);
            if (
                tmp_f < min_f ||
                (fabs(tmp_f - min_f) < FABS_EPS_F && tmp_inter < min_inter)
               )
            {
                min_fit = tmp_fit;
                min_inter = tmp_inter;
                min_normal = tmp_normal;
                cout <<"min_fit " << min_fit << " " << min_inter << endl;
            }
        }

        obj->plane_n = min_normal;
    }

    vector<data> fitness;
    transform(objs.begin(), objs.end(), back_inserter(fitness), [objs](shared_ptr<const object> obj) { return data(counter(obj->get_plane(0.0), objs), obj); });

    vector<data> useful;
    remove_copy_if(fitness.begin(), fitness.end(), back_inserter(useful), [](const data& d) { return !(d.fitness > FABS_EPS_F && d.fitness < 1 - FABS_EPS_F); });

    vector<data> not_useful;
    remove_copy_if(fitness.begin(), fitness.end(), back_inserter(not_useful), [](const data& d) { return !(d.fitness <= FABS_EPS_F || d.fitness >= 1 - FABS_EPS_F); });

    // useful
    stable_sort(useful.begin(), useful.end(), [](const data& l, const data& r) { return l.intersect < r.intersect; });
    stable_sort(useful.begin(), useful.end(), [threshold](const data& l, const data& r) { return l.intersect < threshold && fabs(0.5 - l.fitness) < fabs(0.5 - r.fitness); });

    // not useful
    stable_sort(not_useful.begin(), not_useful.end(), [](const data& l, const data& r) { return l.intersect < r.intersect; });
    stable_sort(not_useful.begin(), not_useful.end(), [threshold](const data& l, const data& r) { return l.intersect < threshold && fabs(0.5 - l.fitness) < fabs(0.5 - r.fitness); });

    root = shared_ptr<bsp_tree_node>(new bsp_tree_node(useful.front().obj->get_plane(0.0)));
    for (auto i = useful.begin(); i != useful.end(); ++i)
    {
        cout << i->fitness << " " << i->intersect << endl;
        root->add(i->obj, false);
    }
    for (auto i = not_useful.begin(); i != not_useful.end(); ++i)
    {
        cout << i->fitness << " " << i->intersect << endl;
        root->add(i->obj, true);
    }
}

intersect_info bsp_tree::traverse(const ray& eye_ray) const
{
    assert(root);
    if (root) return root->traverse(eye_ray);
}
