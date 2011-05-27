#include "triangle.h"
using namespace std;

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
