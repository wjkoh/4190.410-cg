#include "common.h"

class triangle
{
    public:
        triangle(const vector3& v0, const vector3& v1, const vector3& v2, bool cube = false)
            : material_name(""),
            cube(cube)
        {
            v[0] = v0;
            v[1] = v1;
            v[2] = v2;

            n[0] = n[1] = n[2] = get_normal();
            
            if (cube) material_name = "cube";
        }

        triangle(const vector3& v0, const vector3& v1, const vector3& v2,
                   const vector3& n0, const vector3& n1, const vector3& n2, bool cube = false)
            : material_name(""), 
            cube(cube)
        {
            v[0] = v0;
            v[1] = v1;
            v[2] = v2;

            n[0] = n0;
            n[1] = n1;
            n[2] = n2;

            if (cube) material_name = "cube";
        }

        void set_vertex(const vector3& v0, const vector3& v1, const vector3& v2)
        {
            v[0] = v0;
            v[1] = v1;
            v[2] = v2;
        }

        void set_normal(const vector3& n0, const vector3& n1, const vector3& n2)
        {
            n[0] = n0;
            n[1] = n1;
            n[2] = n2;
        }

        void display() const
        {
            if (!material_name.empty())
            {
                material_map[material_name].apply(cube);
            }
            else
                material_t().apply(cube);

            for (int i = 0; i < 3; ++i)
            {
                glNormal3fv(n[i].data());
                glVertex3fv(v[i].data());
            }
        }

        vector3 get_normal() const
        {
            vector3 ba = v[0] - v[1];
            vector3 bc = v[2] - v[1];
            return unit_cross(bc, ba);
        }

        const vector3& operator[](int idx) const    { return v[idx]; }
        vector3& operator[](int idx)                { return v[idx]; }

        bool operator<(const triangle& rhs) const
        { 
            if (!this->cube && rhs.cube) return true;
            else if (this->cube && !rhs.cube) return false;
            return this->area() < rhs.area();
            //return (!this->cube && rhs.cube) || ( || (this->area() < rhs.area());
        }

        float area() const
        {
            vector3 ba = v[0] - v[1];
            vector3 bc = v[2] - v[1];
            return cross(ba, bc).length() / 2;
        }

        list<triangle> split() const
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
           assert(this->material_name == t.material_name);

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

        vector3 v[3];   // 3 points of a triangle
        vector3 n[3];   // normal vector
        bool cube;      // cube polygon
        string material_name;
};

ostream& operator<<(ostream& os, const triangle& t)
{
    os << t[0] << " | ";
    os << t[1] << " | ";
    os << t[2] << endl;
    return os;
}

plane_t triangle_to_plane(const triangle& polygon)
{
    vector3 normal = polygon.get_normal();
    return plane_t(normal, -dot(normal, polygon[1])); // 0, 1, 2 아무거나 상관없음
}
