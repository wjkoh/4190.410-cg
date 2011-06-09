#include "triangle.h"
using namespace std;

vector2 triangle::pt_to_tex_coord(const point3& pt, bool bump) const
{
    std::pair<float, float> u_v = get_barycentric_coord(pt);

    shared_ptr<CImg<float>> tex(texture);
    if (bump)
        tex = bump_map;
    assert(tex);

    vector3 u_ = v[2] - v[1];
    vector3 v_ = v[0] - v[1];

    vector3 x, y, z;
    orthonormal_basis_axial(v_, u_, x, y, z, true, cml::axis_order_yxz);

    vector2 u(u_.length(), 0);
    vector2 v(dot(v_, x), dot(v_, y));
        
    float ratio = min(128.0f, min(tex->width()/(u.length()), tex->height()/(v[1]))); // 256
    u *= ratio;
    v *= ratio;

    vector2 result = u*(u_v.first) + v*(u_v.second);
    if (inverted_xy)
        result = u*(1.0-u_v.first) + v*(1.0-u_v.second);

    int div = result[0]/tex->width();

    if (result[0] < 0)
    {
        result[0] = -(result[0] - div*tex->width()); // mirror
    }
    if (result[0] > tex->width())
    {
        //result[0] = (result[0] - div*tex->width()); // wrap
        result[0] = tex->width() - (result[0] - div*tex->width()); // mirror
    }
    if (result[1] < 0)
    {
        result[1] = -(result[1] - div*tex->height()); 
    }
    if (result[1] > tex->height())
    {
        result[1] = tex->height() - (result[1] - div*tex->height()); 
    }
    return result;
}

vector3 triangle::get_normal(const point3& pt, const float time, bool bump) const
{
    vector3 ba = v[0] - v[1];
    vector3 bc = v[2] - v[1];
    vector3 n = unit_cross(bc, ba);

    if (bump && bump_map)
    {
        vector2 pt_xy = pt_to_tex_coord(pt, true);

        vector3 normal;
        for (int i = 0; i < 3; ++i)
        {
            normal[i] = bump_map->cubic_atXY(pt_xy[0], pt_xy[1], 0, i) / 255.0 * 2.0 - 1.0;
            if (i == 1)
                normal[i] *= -1.0;
        }

        matrix m;
        vector3 x = unit_cross(Y, n);
        matrix_rotation_align(m, n, x, true, cml::axis_order_zxy);
        //m.inverse();

        vector3 r_n = transform_vector(m, normal);
        return r_n.normalize();
    }

    return n;
}

/*
vector3 triangle::get_texture(const point3& pt) const
{
    if (!texture) return vector3(1, 1, 1);

    vector2 pt_xy = pt_to_tex_coord(pt);

    vector3 color;
    for (int i = 0; i < 3; ++i)
        color[i] = texture->cubic_atXY(pt_xy[0], pt_xy[1], 0, i) / 255.0;
    //std::cout << std::endl << color << " " << u_v.first << " " << u_v.second << " " << texture->width() << std::endl;
    return color;
}
*/

list<triangle> triangle::split() const
{
    vector<vector3> vertices;
    vector<vector3> normals;
    assert(n[0] == n[1] && n[1] == n[2]);

    for (int i = 0; i < 3; ++i)
    {
        const int i_n = (i+1) % 3;

        vertices.push_back(v[i]);
        normals.push_back(n[i]);

        vector3 vertex = (v[i] + v[i_n]) / 2.0;
        vector3 normal = n[i]; //slerp(n[i], n[i_n], 0.5);

        vertices.push_back(vertex);
        normals.push_back(normal);
    }

    // 0, 1, 5 
    // 1, 2, 3 
    // 3, 4, 5 
    // 1, 3, 5 
    list<triangle> l;
    triangle t(*this);
    //assert(this->material_name == t.material_name);

    t.set_vertex(vertices[0], vertices[1], vertices[5]);
    t.set_normal(normals[0], normals[1], normals[5]);
    l.push_back(t);

    t.set_vertex(vertices[1], vertices[2], vertices[3]);
    t.set_normal(normals[1], normals[2], normals[3]);
    l.push_back(t);

    t.set_vertex(vertices[3], vertices[4], vertices[5]);
    t.set_normal(normals[3], normals[4], normals[5]);
    l.push_back(t);

    t.set_vertex(vertices[1], vertices[3], vertices[5]);
    t.set_normal(normals[1], normals[3], normals[5]);
    l.push_back(t);
    return l;
}
