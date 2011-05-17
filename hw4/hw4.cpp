/*
 * 2011-1 Computer Graphics Homework 4
 * SNUCSE 2004-11881 고우종 Woojong Koh (jstrane@gmail.com)
 *
 */

#include <stdlib.h>
//#include <GL/glew.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include <iostream>
#include <limits>
#include <string>
#include <fstream>
#include <sstream>
#include <queue>
#include <map>
#include <list>
using namespace std;

#include <cmath>
#include <cstdio>
#include <cassert>

#include "cml/cml.h"
typedef cml::vector3f vector3;
typedef cml::vector3d vector3d;
typedef cml::vector4f vector4;
typedef cml::matrix44f_c matrix;
typedef cml::quaternionf_p quaternionf_p;
typedef cml::matrixf_c matrix_d;

#define _D_ //cout << __LINE__ << endl;
const double PI = 4.0*atan(1.0);
const vector3 zero(0,0,0);
const float INITIAL_VIEW_DISTANCE = 5;
const float INITIAL_FOV = 60.0;

// TODO
// Cylinder의 bounding box 수정하기
// 우상단->좌하단 드래그 시 문제
//
// normal vector 추가 -done
// 각 면마다 다른 material 설정 -done
// show all용으로 g_vertices에 vertex 추가하기?
// Triangle들의 CW, CCW 다시 체크 -done
//
// cube 옆에 light 추가 -done

bool g_2d_mode = false;
int scr_width = 800;
int scr_height = 600;
float scr_mag = 100;

int target_cs_idx = 0;
int target_cp = 0;
bool drag_cp = false;

// Camera position
const vector3 INIT_CAM_VEC(0.0, 0.0, 5); // origin to camera
const vector3 INIT_CAM_UP_VEC(0.0, 1.0, 0); // origin to camera

vector3 cam_vec(0.0, 0.0, 5); // origin to camera
vector3 cam_vec_start(0.0, 0.0, 5); // origin to camera

vector3 cam_up_vec(0, 1, 0);
vector3 cam_up_vec_start(0, 1, 0); // origin to camera

// Virtual trackball
// mouse drag start
vector3 start_vec; // on a sphere
////

// Zoom in/out
float fov = INITIAL_FOV;

// Dolly in/out
float view_distance = INITIAL_VIEW_DISTANCE;

// Seek
// rotation origin & look at position (origin)
vector3d rot_origin(0, 0, 0);

/////
// Show All
vector3 mid_pt(0, 0, 0);
vector3 normal(0, 0, 1); // origin to camera
float max_rad = 0;

// first run은 Show All시 필요한 normal vector와 중점을 찾기 위한 1-pass.
// World coordinates를 얻기 위해 카메라 위치 바꾸지 않고 돌아간다.
bool first_run = true;
/////

// Wall-e
int head_rot = 0;
int shoulder_roll = 90;
int shoulder_yaw1 = 90;
int shoulder_yaw2 = 90;
int elbow = 0;
int finger1 = 30;
float position_z = 0; // move

// 큰 로봇팔 
float big_rot = 0;
float big_shoulder_yaw = 100;
float big_elbow = 0;
float big_finger1 = 30;

// to make light stationary
GLfloat light_position[] = {0.0, 1.0, 0.0, 0.0};
GLfloat light1_position[] = {0.0, 2.5, 2.5, 1.0};
GLfloat light2_position[] = {0.0, 2.5, -2.5, 1.0};

float light_dist = 2.0;
GLfloat light3_position[] = {light_dist, -5.5 + 1, 0.0, 1.0};
GLfloat light4_position[] = {0.0, -5.5 + 1, light_dist, 1.0};
GLfloat light5_position[] = {-light_dist, -5.5 + 1, 0.0, 1.0};
GLfloat light6_position[] = {0.0, -5.5 + 1, -light_dist, 1.0};
GLfloat light7_position[] = {0.0, -5.5 - light_dist, 0.0, 1.0};

#include <set>
#include <list>
typedef list<vector3> set_t;
set_t g_vertices;

bool swept_transparent = false;
const float INIT_TRANSPARENCY = 0.7;
float transparency = INIT_TRANSPARENCY;

vector3 mult_mat(int matrix, vector3 pt)
{
    float mv[16];
    glGetFloatv(matrix, mv);

    float x = pt[0];
    float y = pt[1];
    float z = pt[2];

    float xp = mv[0] * x + mv[4] * y + mv[8] * z + mv[12];
    float yp = mv[1] * x + mv[5] * y + mv[9] * z + mv[13];
    float zp = mv[2] * x + mv[6] * y + mv[10] * z + mv[14];
    float wp = mv[3] * x + mv[7] * y + mv[11] * z + mv[15];

    xp /= wp;
    yp /= wp;
    zp /= wp;

    return vector3(xp, yp, zp);
}

typedef quaternionf_p plane_t; // ax+by+cz+d=0 -> (a, b, c, d)

class material_t
{
    public:
        material_t()
            : diffuse(vector4(0.8, 0.8, 0.8, 1.0)),
            specular(vector4(0, 0, 0, 1)),
            ambient(vector4(0.2, 0.2, 0.2, 1.0)),
            shininess(0)
    {
    }

        material_t(vector4 diffuse, vector4 specular, vector4 ambient, float shininess)
            : diffuse(diffuse),
            specular(specular),
            ambient(ambient),
            shininess(shininess)
    {
    }

        void apply(bool transparent) const
        {
            vector4 tmp(diffuse);
            if (transparent)
                tmp[3] = transparency;

            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, tmp.data());
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular.data());
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient.data());
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
        }

        vector4 diffuse;
        vector4 specular;
        vector4 ambient;
        float shininess;
};

map<string, material_t> material_map;

class triangle_t
{
    public:
        triangle_t(const vector3& v0, const vector3& v1, const vector3& v2, bool cube = false)
            : material_name(""),
            cube(cube)
        {
            v[0] = v0;
            v[1] = v1;
            v[2] = v2;

            n[0] = n[1] = n[2] = get_normal_vector();
            
            if (cube) material_name = "cube";
        }

        triangle_t(const vector3& v0, const vector3& v1, const vector3& v2,
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

        vector3 get_normal_vector() const
        {
            vector3 ba = v[0] - v[1];
            vector3 bc = v[2] - v[1];
            return unit_cross(bc, ba);
        }

        const vector3& operator[](int idx) const    { return v[idx]; }
        vector3& operator[](int idx)                { return v[idx]; }

        bool operator<(const triangle_t& rhs) const
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

        list<triangle_t> split() const
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
           list<triangle_t> l;
           triangle_t t(*this);
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

ostream& operator<<(ostream& os, const triangle_t& t)
{
    os << t[0] << " | ";
    os << t[1] << " | ";
    os << t[2] << endl;
    return os;
}

plane_t triangle_to_plane(const triangle_t& polygon)
{
    vector3 normal = polygon.get_normal_vector();
    return plane_t(normal, -dot(normal, polygon[1])); // 0, 1, 2 아무거나 상관없음
}

typedef pair<int, triangle_t> f_or_b_t;

enum
{
    BACK = -1
        , SAME = 0
        , FRONT = 1
};

class node_t
{
    public:
        node_t()
            : left(NULL), right(NULL) {}

        ~node_t()
        {
            clear();
        }

        void clear()
        {
            polygons.clear();
            delete left;
            left = NULL;
            delete right;
            right = NULL;
        }

        vector<triangle_t> polygons;
        node_t* left;
        node_t* right;
};

node_t root;
vector<triangle_t> triangle_list;

// Determine whether a polygon is front of plane or behind of.
// Also split a polygon into three triangles if needed.
vector<f_or_b_t> is_front_or_back(const triangle_t& plane, const triangle_t& polygon)
{
    plane_t p = triangle_to_plane(plane);

    int f_or_b[3] = {0};
    for (int i = 0; i < 3; ++i)
    {
        float det = dot(p.imaginary(), polygon[i]) + p.real();

        f_or_b[i] = SAME;
        if (det > numeric_limits<float>::epsilon())
            f_or_b[i] = FRONT;
        else if (det < -numeric_limits<float>::epsilon())
            f_or_b[i] = BACK;
    }

    vector<f_or_b_t> result;

    // 불투명 polygon 끼리는 쪼개지 않고 같은 node에 둔다.
    // (leaf 일 때, 해당 node에서 더 내려가지 않고 쌓임.)
    if (!plane.cube && !polygon.cube)
    {
        result.push_back(f_or_b_t(SAME, polygon));
        return result;
    }

    // 교점 0개 or 겹침
    if (f_or_b[0] == f_or_b[1] && f_or_b[1] == f_or_b[2])
    {
        result.push_back(f_or_b_t(f_or_b[0], polygon));
        return result;
    }

    // 교점이 vertex면서 1개 or 2개
    // 나머지 vertex들은 FRONT나 BACK 한 쪽에 몰려있다.
    int sum = f_or_b[0] + f_or_b[1] + f_or_b[2];
    if (f_or_b[0] * f_or_b[1] * f_or_b[2] == 0 && sum != 0)
    {
        result.push_back(f_or_b_t(sum > 0 ? FRONT : BACK, polygon));
        return result;
    }

    // 교점이 vertex가 아님. 즉 2개.
    vector<pair<int, vector3> > quad;
    vector<vector3> quad_normal;
    for (int i = 0; i < 3; ++i) // loop
    {
        int j = (i + 1) % 3;
        quad.push_back(make_pair(f_or_b[i], polygon[i]));
        quad_normal.push_back(polygon.n[i]);

        if (f_or_b[i] != SAME && f_or_b[j] != SAME)
        {
            if (f_or_b[i] != f_or_b[j])
            {
                vector3 dir = polygon[j] - polygon[i];
                float t = (-p.real() - dot(p.imaginary(), polygon[i])) / dot(p.imaginary(), dir); // p = p_i + t*(p_j - p_i)
                //assert(t >= 0 && t <= 1);

                vector3 pt = polygon[i] + t * dir;
                vector3 normal = slerp(polygon.n[i], polygon.n[j], t);

                quad.push_back(make_pair((int)SAME, pt));
                quad_normal.push_back(normal);
            }
        }
    }

    int start = 0;
    for (int i = 0; i < quad.size(); ++i)
        if (quad[i].first == SAME && quad[(i + 2) % quad.size()].first == SAME)
            start = i;

    rotate(quad.begin(), quad.begin() + start, quad.end());
    rotate(quad_normal.begin(), quad_normal.begin() + start, quad_normal.end());

    assert(quad.front().first == SAME);

    // quad to triangle
    // (0, 1, 2) - (0, 3, 4) - (0, 2, 3)
    if (quad.size() == 5)
    {
        triangle_t tri1(quad[0].second, quad[1].second, quad[2].second,
                        quad_normal[0], quad_normal[1], quad_normal[2], polygon.cube);
        tri1.material_name = polygon.material_name;
        result.push_back(f_or_b_t(quad[1].first, tri1));

        triangle_t tri2(quad[0].second, quad[2].second, quad[3].second,
                        quad_normal[0], quad_normal[2], quad_normal[3], polygon.cube);
        tri2.material_name = polygon.material_name;
        result.push_back(f_or_b_t(quad[3].first, tri2));

        triangle_t tri3(quad[0].second, quad[3].second, quad[4].second,
                        quad_normal[0], quad_normal[3], quad_normal[4], polygon.cube);
        tri3.material_name = polygon.material_name;
        result.push_back(f_or_b_t(quad[4].first, tri3));
    }
    else if (quad.size() == 4)
    {
        triangle_t tri1(quad[0].second, quad[1].second, quad[2].second,
                        quad_normal[0], quad_normal[1], quad_normal[2], polygon.cube);
        tri1.material_name = polygon.material_name;
        result.push_back(f_or_b_t(quad[1].first, tri1));

        triangle_t tri2(quad[2].second, quad[3].second, quad[0].second,
                        quad_normal[2], quad_normal[3], quad_normal[0], polygon.cube);
        tri2.material_name = polygon.material_name;
        result.push_back(f_or_b_t(quad[3].first, tri2));
    }
    else
    {
        cout << quad.size() << endl;
        cout << f_or_b[0] << " " << f_or_b[1] << " " << f_or_b[2] << endl;
        assert(false);
    }
    return result;
}

// BSP tree
void add_tree(node_t* node, const triangle_t& triangle)
{
    assert(node != NULL);

    if (node->polygons.empty())
    {
        node->polygons.push_back(triangle);
        return;
    }

    vector<f_or_b_t> result = is_front_or_back(node->polygons.front(), triangle);
    for (int i = 0; i < result.size(); ++i)
    {
        switch (result[i].first)
        {
            case FRONT:
                if (!node->left)
                    node->left = new node_t();
                add_tree(node->left, result[i].second);
                break;
            case SAME:
                node->polygons.push_back(result[i].second);
                break;
            case BACK:
                if (!node->right)
                    node->right = new node_t();
                add_tree(node->right, result[i].second);
                break;
        }
    }
}

float blue = 100;

void traverse_tree(node_t* node, bool preorder)
{
    if (!node) return;

    // find an eye location
    {
        plane_t p = triangle_to_plane(node->polygons.front());
        vector3 eye_pos = rot_origin + cam_vec;
        float det = dot(p.imaginary(), eye_pos) + p.real();

        preorder = true;
        if (det > numeric_limits<float>::epsilon())
            preorder = false;
        else if (det < -numeric_limits<float>::epsilon())
            preorder = true;
    }

    node_t* next = node->left;
    if (!preorder)
        next = node->right;
    traverse_tree(next, preorder);

    //if (location != 0)
    {
        const vector<triangle_t>& p = node->polygons;
        for (int i = 0; i < p.size(); ++i)
        {
            p[i].display();
        }
        //blue += 50;
    }

    next = node->right;
    if (!preorder)
        next = node->left;
    traverse_tree(next, preorder);
}

typedef vector<vector3> cross_sect_t;

struct data
{
    enum spline_t
    {
        BSPLINE = 1,
        INTERPOLATION = 2, // Catmull-Rom
        NATURAL_CUBIC = 3,
        B_SUBDIVISION = 4,
        INTERPOLATING_SUBDIVISION = 5,
    };

    data()
        : curve_type(BSPLINE)
          , sweep_type(INTERPOLATION)
    {
    }

    void clear()
    {
        pos.clear();
        angle.clear();
        axis.clear();
        scale_factor.clear();
        con_pts.clear();
    }

    spline_t curve_type;
    spline_t sweep_type;

    vector<vector3> pos;
    vector<float> angle;
    vector<vector3> axis;
    vector<quaternionf_p> orient;
    vector<vector3> scale_factor;

    vector<cross_sect_t> con_pts;
} g_data;

bool wireframe_mode = false;
bool depth_ordering = true;

vector<vector3> tangents; // tangent vector 값 전달을 위한 임시 global variable

// draw text on screen
void DrawText(int x, int y, string msg, void* font, vector3 Color)
{
    double FontWidth = 0.1;
    glColor3f(Color[0], Color[1], Color[2]);

    glRasterPos2i(x, y);
    for (int i = 0 ; i < msg.length(); ++i)
    {
        glutBitmapCharacter(font, msg[i]);
    }
}

quaternionf_p Bezier_curve(float t, quaternionf_p b0, quaternionf_p b1, quaternionf_p b2, quaternionf_p b3)
{
    // De Casteljau's algorithm
    quaternionf_p p1 = slerp(b0, b1, t);
    quaternionf_p p2 = slerp(b1, b2, t);
    quaternionf_p p3 = slerp(b2, b3, t);

    quaternionf_p p12 = slerp(p1, p2, t);
    quaternionf_p p23 = slerp(p2, p3, t);

    quaternionf_p p = slerp(p12, p23, t);
    return p;
} 

vector3 Bezier_curve(float t, vector3 b0, vector3 b1, vector3 b2, vector3 b3)
{
    vector3 pt = pow(1 - t, 3)*b0 \
                 + 3*t*pow(1 - t, 2)*b1 \
                 + 3*pow(t, 2)*(1 - t)*b2 \
                 + pow(t, 3)*b3;
    return pt;
} 

vector3 Bezier_curve_deriv(float t, vector3 b0, vector3 b1, vector3 b2, vector3 b3)
{
    vector3 pt = -3*(
                     pow(t - 1, 2)*b0 \
                     + (-3*pow(t, 2) + 4*t - 1)*b1 \
                     + t*(3*b2*t - 2*b2 - b3*t)
                    );
    return pt;
} 

vector3 B_spline(float t, vector3 b0, vector3 b1, vector3 b2, vector3 b3)
{
    vector3 pt = (pow(1 - t, 3)*b0 \
                  + (3*pow(t, 3) - 6*pow(t, 2) + 4)*b1 \
                  + (-3*pow(t, 3) + 3*pow(t, 2) + 3*t + 1)*b2 \
                  + pow(t, 3)*b3)/6;
    return pt;
} 

vector3 B_spline_deriv(float t, vector3 b0, vector3 b1, vector3 b2, vector3 b3)
{
    vector3 pt = (-pow(t-1, 2)*b0 \
                  + (3*pow(t, 2) - 4*t)*b1 \
                  + (-3*pow(t, 2) + 2*t + 1)*b2 \
                  + pow(t, 2)*b3)/2;
    return pt;
} 


vector<cross_sect_t> surfaces;
vector<cross_sect_t> surface_normals;

vector<quaternionf_p> Catmull_Rom(const vector<quaternionf_p>& pt,
                                  vector<quaternionf_p>& tangents,
                                  bool closed = false,
                                  float resolution = 0.01)
{
    assert(pt.size() > 0);

    quaternionf_p prev_tan = log(inverse(pt.back())*pt[1])/2;
    if (!closed)
        prev_tan = log(inverse(pt[0])*pt[2]) / 2;

    float pieces = 1.0 / resolution;
    float piece_res = 1.0 / (pieces / (closed ? (float)pt.size() : (float)(pt.size() - 3)));

    tangents.clear();
    vector<quaternionf_p> result;
    for (int i = 0; i < pt.size(); ++i)
    {
        quaternionf_p tan = log(inverse(pt[i])*pt[(i + 2) % pt.size()])/2;

        if (closed || (i > 0 && i < pt.size() - 2))
        {
            const quaternionf_p& b0 = pt[i];
            const quaternionf_p& b1 = b0 + prev_tan/3;
            const quaternionf_p& b3 = pt[(i + 1) % pt.size()];
            const quaternionf_p& b2 = b3 - tan/3;

            for (float t = 0; t < 1; t += piece_res)
            {
                quaternionf_p pt = Bezier_curve(t, b0, b1, b2, b3);
                result.push_back(pt);

                //quaternionf_p tangent = Bezier_curve_deriv(t, b0, b1, b2, b3);
                //tangents.push_back(tangent);
            }
        }
        prev_tan = tan;
    }
    return result;
}

vector<vector3> Catmull_Rom(const vector<vector3>& pt,
                            vector<vector3>& tangents,
                            bool closed = false,
                            float resolution = 0.01)
{
    assert(pt.size() >= 4);
    vector3 prev_tan = (pt[1] - pt.back()) / 2;
    if (!closed)
        prev_tan = (pt[2] - pt[0]) / 2;

    float pieces = 1.0 / resolution;
    float piece_res = 1.0 / (pieces / (closed ? (float)pt.size() : (float)(pt.size() - 3)));

    tangents.clear();
    vector<vector3> result;
    for (int i = 0; i < pt.size(); ++i)
    {
        vector3 tan = (pt[(i + 2) % pt.size()] - pt[i]) / 2;

        if (closed || (i > 0 && i < pt.size() - 2))
        {
            const vector3& b0 = pt[i];
            const vector3& b1 = b0 + prev_tan/3;
            const vector3& b3 = pt[(i + 1) % pt.size()];
            const vector3& b2 = b3 - tan/3;

            for (float t = 0; t < 1; t += piece_res)
            {
                vector3 pt = Bezier_curve(t, b0, b1, b2, b3);
                result.push_back(pt);

                vector3 tangent = Bezier_curve_deriv(t, b0, b1, b2, b3);
                tangents.push_back(tangent);
            }
        }
        prev_tan = tan;
    }
    return result;
}

vector<vector3> B_Spline(const vector<vector3>& pt,
                         vector<vector3>& tangents,
                         bool closed = false,
                         float resolution = 0.01)
{
    assert(pt.size() >= 4);

    float pieces = 1.0 / resolution;
    float piece_res = 1.0 / (pieces / (closed ? (float)pt.size() : (float)(pt.size() - 3)));

    tangents.clear();
    vector<vector3> result;

    int max_idx = (closed ? pt.size() - 1: pt.size() - 4); 
    for (int i = 0; i <= max_idx; ++i)
    {
        //if (closed || (i > 0 && i < pt.size() - 2)) //???
        {
            const vector3& b0 = pt[i];
            const vector3& b1 = pt[(i+1) % pt.size()];
            const vector3& b2 = pt[(i+2) % pt.size()];
            const vector3& b3 = pt[(i+3) % pt.size()];

            for (float t = 0; t < 1; t += piece_res)
            {
                vector3 pt = B_spline(t, b0, b1, b2, b3);
                result.push_back(pt);

                vector3 tangent = B_spline_deriv(t, b0, b1, b2, b3);
                tangents.push_back(tangent);
            }
        }
    }
    return result;
}

vector<vector3> Natural_Cubic_Spline(const vector<vector3>& pt,
                                     vector<vector3>& tangents,
                                     bool closed = false,
                                     float resolution = 0.01)
{
    vector<vector3> new_pts(pt);
    if (!closed)
    {
        // zero vector 삽입
        new_pts.insert(new_pts.begin(), vector3().zero());
        new_pts.push_back(vector3().zero());
    }

    const int pt_size = new_pts.size();
    matrix_d conv_mat(pt_size, pt_size);
    conv_mat.zero();

    int j = pt_size - 1;
    for (int i = 0; i < pt_size; ++i)
    {
        if (!closed && (i == 0 || i == pt_size - 1))
        {
            if (i == 0)
            {
                conv_mat(i, 0) = 1;
                conv_mat(i, 1) = -2;
                conv_mat(i, 2) = 1;
            }
            else
            {
                conv_mat(i, pt_size - 3) = 1;
                conv_mat(i, pt_size - 2) = -2;
                conv_mat(i, pt_size - 1) = 1;
            }
            j = (j + 1) % pt_size;
            continue;
        }

        int k = j;
        j = (j + 1) % pt_size;

        conv_mat(i, k) = 1;
        k = (k + 1) % pt_size;
        conv_mat(i, k) = 4;
        k = (k + 1) % pt_size;
        conv_mat(i, k) = 1;
        k = (k + 1) % pt_size;
    }
    conv_mat /= 6;
    conv_mat.inverse();

    vector<vector3> cp;
    for (int i = 0; i < pt_size; ++i)
    {
        vector3 result;
        result.zero();
        for (int j = 0; j < pt_size; ++j)
        {
            result += conv_mat(i, j) * new_pts[j];
        }
        cp.push_back(result);
    }
    if (!closed)
    {
        cp.erase(cp.end() - 1);
        cp.erase(cp.begin());
    }

    return B_Spline(cp, tangents, closed, resolution);
}

vector<vector3> B_Subdivision(const vector<vector3>& pt,
                              vector<vector3>& tangents,
                              bool closed = false,
                              float resolution = 0.01)
{
    assert(pt.size() >= 4);

    float pieces = 1.0 / resolution;
    float piece_res = 1.0 / (pieces / (closed ? (float)pt.size() : (float)(pt.size() - 3)));

    tangents.clear();
    vector<vector3> result = pt;
    while (result.size() < pieces)
    {
        int max_idx = (closed ? result.size() - 2: result.size() - 4); // -2가 맞을까? -1이 맞을까? 

        vector<vector3> temp;
        for (int i = 0; i <= max_idx; ++i)
        {
            const vector3& b0 = result[i];
            const vector3& b1 = result[(i+1) % result.size()];
            const vector3& b2 = result[(i+2) % result.size()];
            const vector3& b3 = result[(i+3) % result.size()];

            if (i == 0)
            {
                const vector3& p0 = (4*b0 + 4*b1)/8;
                const vector3& p1 = (1*b0 + 6*b1 + 1*b2)/8;
                const vector3& p2 = (4*b1 + 4*b2)/8;
                temp.push_back(p0);
                temp.push_back(p1);
                temp.push_back(p2);
            }
            const vector3& p3 = (1*b1 + 6*b2 +1*b3)/8;
            const vector3& p4 = (4*b2 + 4*b3)/8;
            temp.push_back(p3);
            temp.push_back(p4);

            /*
               if (fabs(p4[0] - result.front()[0]) >= numeric_limits<float>::epsilon() ||
               fabs(p4[1] - result.front()[1]) >= numeric_limits<float>::epsilon() ||
               fabs(p4[2] - result.front()[2]) >= numeric_limits<float>::epsilon())
               {
               temp.push_back(p4);
               }
               */
        }
        result = temp;
    }

    tangents.clear();
    for (int i = 0; i < result.size() + (closed ? 0 : -1); ++i)
    {
        tangents.push_back(normalize(result[(i+1) % result.size()] - result[i]));
    }
    if (!closed)
        tangents.push_back(tangents.back());

    return result;
}

vector<vector3> Interpolating_Subdivision(const vector<vector3>& pt,
                                          vector<vector3>& tangents,
                                          bool closed = false,
                                          float resolution = 0.01)
{
    assert(pt.size() >= 4);

    float pieces = 1.0 / resolution;
    float piece_res = 1.0 / (pieces / (closed ? (float)pt.size() : (float)(pt.size() - 3)));

    tangents.clear();
    vector<vector3> result = pt;
    while (result.size() < pieces)
    {
        int min_idx = (closed ? 0 : 1);
        int max_idx = (closed ? result.size() - 1: result.size() - 3);

        vector<vector3> temp;
        if (!closed && min_idx == 1)
            temp.push_back(result[0]);

        for (int i = min_idx; i <= max_idx; ++i)
        {
            vector3& b0 = result[(i-1) % result.size()];
            if (!closed && i - 1 < 0)
                b0 = result[i];

            const vector3& b1 = result[i];
            const vector3& b2 = result[(i+1) % result.size()];
            vector3& b3 = result[(i+2) % result.size()];
            if (!closed && i + 2 >= result.size())
                b3 = result.back();

            const vector3& p = (-1*b0 + 9*b1 + 9*b2 -1*b3)/16;
            temp.push_back(b1);
            temp.push_back(p);
        }
        if (!closed)
        {
            for (int i = max_idx + 1; i < result.size(); ++i)
                temp.push_back(result[i]);
        }
        result = temp;
    }
    result.erase(result.end() - 1);
    result.erase(result.begin());

    tangents.clear();
    for (int i = 0; i < result.size() + (closed ? 0 : -1); ++i)
    {
        tangents.push_back(normalize(result[(i+1) % result.size()] - result[i]));
    }
    if (!closed)
        tangents.push_back(tangents.back());

    return result;
}

// Function prototypes
void set_cam_dist(float view_dist = view_distance);
void draw_swept_surface();

// Picking
vector3d screen_to_object(int x, int y)
{
    GLdouble modelview[16];
    GLdouble projection[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLfloat winX, winY, winZ = 0;
    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

    vector3d result;
    if (gluUnProject(winX, winY, winZ, modelview, projection, viewport, &result[0], &result[1], &result[2]) == GL_FALSE)
    {
        result[0] = 0;
        result[1] = 0;
        result[2] = 0;
    }
    return result;
}

vector3 vertices[12]; /* 12 vertices with x, y, z coordinates */

void polygon(int i, int j, int k)
{
    const vector3 trans_vec(0, 0, 0);
    triangle_t t(vertices[i], vertices[j], vertices[k], true);

    // Triangle tesselation
    //list<triangle_t> lst = t.split();
    list<triangle_t> lst;
    lst.push_back(t);
    for (list<triangle_t>::iterator i = lst.begin(); i != lst.end(); ++i)
    {
        (*i)[0] += trans_vec;
        (*i)[1] += trans_vec;
        (*i)[2] += trans_vec;

        triangle_list.push_back(*i);
    }
}

void polygon(int i_, int j, int k, int l, const string& name)
{
    const vector3 trans_vec(0, -5.5, 0);

    list<triangle_t> result;

    triangle_t t1(vertices[i_], vertices[j], vertices[k], true);
    t1.material_name = name;
    result.push_back(t1);
    
    triangle_t t2(vertices[k], vertices[l], vertices[i_], true);
    t2.material_name = name;
    result.push_back(t2);

    // Triangle tessellation
    const int N = 5;
    for (int i = 0; i < N; ++i)
    {
        list<triangle_t> tmp;
        for (list<triangle_t>::iterator j = result.begin(); j != result.end(); ++j)
        {
            list<triangle_t> lst = j->split();
            tmp.splice(tmp.end(), lst);
        }
        result.swap(tmp);
    }

    for (list<triangle_t>::iterator i = result.begin(); i != result.end(); ++i)
    {
        (*i)[0] += trans_vec;
        (*i)[1] += trans_vec;
        (*i)[2] += trans_vec;

        triangle_list.push_back(*i);
    }
}

void polyhedron(int num_of_faces = 20, float r = 1.5)
{
    /* r: any radius in which the polyhedron is inscribed */

    if (num_of_faces == 20)
    {
        double Pi = 3.141592653589793238462643383279502884197;
        double phiaa  = 26.56505; /* phi needed for generation */

        double phia = Pi*phiaa/180.0; /* 2 sets of four points */
        double theb = Pi*36.0/180.0;  /* offset second set 36 degrees */
        double the72 = Pi*72.0/180;   /* step 72 degrees */

        vertices[0][0]=0.0;
        vertices[0][1]=0.0;
        vertices[0][2]=r;

        vertices[11][0]=0.0;
        vertices[11][1]=0.0;
        vertices[11][2]=-r;
        double the = 0.0;

        for(int i=1; i<6; i++)
        {
            vertices[i][0]=r*cos(the)*cos(phia);
            vertices[i][1]=r*sin(the)*cos(phia);
            vertices[i][2]=r*sin(phia);
            the = the+the72;
        }

        the=theb;
        for(int i=6; i<11; i++)
        {
            vertices[i][0]=r*cos(the)*cos(-phia);
            vertices[i][1]=r*sin(the)*cos(-phia);
            vertices[i][2]=r*sin(-phia);
            the = the+the72;
        }

        polygon(0,1,2);
        polygon(0,2,3);
        polygon(0,3,4);
        polygon(0,4,5);
        polygon(0,5,1);
        //polygon(11,6,7); // cw
        polygon(7,6,11);
        //polygon(11,7,8); // cw
        polygon(8,7,11);
        //polygon(11,8,9); // cw
        polygon(9,8,11);
        //polygon(11,9,10); // cw
        polygon(10,9,11);
        //polygon(11,10,6); // cw
        polygon(6,10,11);
        //polygon(1,2,6); // cw
        polygon(6,2,1);
        //polygon(2,3,7); // cw
        polygon(7,3,2);
        //polygon(3,4,8); // cw
        polygon(8,4,3);
        //polygon(4,5,9); // cw
        polygon(9,5,4);
        //polygon(5,1,10); //cw
        polygon(10,1,5);
        polygon(6,7,2);
        polygon(7,8,3);
        polygon(8,9,4);
        polygon(9,10,5);
        polygon(10,6,1);
    }
    else if (num_of_faces == 4)
    {
        double Pi = 3.141592653589793238462643383279502884197;
        double phiaa = 35.264391; /* the phi needed for generation */

        double phia = Pi*phiaa/180.0; /* 2 sets of four points */
        double phib = -phia;
        double the90 = Pi*90.0/180.0;
        double the = PI/4.0; //0.0;

        for(int i=0; i<4; i++)
        {
            vertices[i][0]=r*cos(the)*cos(phia);
            vertices[i][1]=r*sin(the)*cos(phia);
            vertices[i][2]=r*sin(phia);
            the = the+the90;
        }

        the = PI/4.0; //0.0;
        for(int i=4; i<8; i++)
        {
            vertices[i][0]=r*cos(the)*cos(phib);
            vertices[i][1]=r*sin(the)*cos(phib);
            vertices[i][2]=r*sin(phib);
            the = the+the90;
        }

        /* map vertices to 6 faces */
        map<string, material_t>::const_iterator it = material_map.begin();
        if (it->first == string("cube")) ++it;

        polygon(0,1,2,3, it->first); ++it;
        if (it->first == string("cube")) ++it;

        polygon(7, 6, 5, 4, it->first); ++it;
        if (it->first == string("cube")) ++it;

        polygon(4,5,1,0, it->first); ++it;
        if (it->first == string("cube")) ++it;

        polygon(5,6,2,1, it->first); ++it;
        if (it->first == string("cube")) ++it;

        polygon(6,7,3,2, it->first); ++it;
        if (it->first == string("cube")) ++it;

        polygon(7,4,0,3, it->first);// ++it;
        //cout << it->first << endl;
    }
    else
        assert(false);
}

void init(void)
{
    glEnable(GL_NORMALIZE); // 이 코드가 없으면 scale시 normal vector가 엉망이 됨.
    //glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    //glEnable(GL_COLOR_MATERIAL); // 이 코드가 없으면 전부 흑백으로
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glShadeModel(GL_SMOOTH);

    // Back-face culling
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);
    //glFrontFace(GL_CW);

    if (g_2d_mode)
        glDisable(GL_DEPTH_TEST);
    else
        glEnable(GL_DEPTH_TEST);

    // Two-side lighting
    //glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
   
    // Global ambient light
    GLfloat lmodel_ambient[] = {0.4, 0.4, 0.4, 1.0};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);


    // 첫 번째 light
    /*
    //GLfloat mat_diffuse[] = {1.0, 193/255.0, 37/255.0, 1.0};
    GLfloat mat_diffuse[] = {1.0, 165/255.0, 0/255.0, 1.0}; // orange
    GLfloat mat_ambient[] = {0/255.0, 0/255.0, 128/255.0, 1.0}; // navy?
    //GLfloat mat_ambient[] = {25/255.0, 25/255.0, 112/255.0, 1.0};
    */
    GLfloat mat_diffuse[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat mat_ambient[] = {0.2, 0.2, 0.2, 1.0};
    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};

    //glLightfv(GL_LIGHT0, GL_AMBIENT, mat_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, mat_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    //glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    // 두 번째 light
    GLfloat light1_diffuse[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat light1_ambient[] = {0.4, 0.4, 0.4, 1.0};
    GLfloat light1_specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat spot_direction[] = {0.0, -1.0, -1.0};

    //glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);
    //glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.5);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.2);
    //glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.2);

    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 90.0);
    //glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 2.0);
    glEnable(GL_LIGHT1);

    {
    // 세 번째 light
    GLfloat mat_diffuse[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat mat_ambient[] = {0.2, 0.2, 0.2, 1.0};
    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat spot_direction[] = {0.0, -1.0, 1.0};

    //glLightfv(GL_LIGHT2, GL_AMBIENT, mat_ambient);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, mat_specular);
    glLightfv(GL_LIGHT2, GL_POSITION, light2_position);
    glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spot_direction);
    //glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.5);
    glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.2);

    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 90.0);
    glEnable(GL_LIGHT2);
    }

    {
    vector3 spot_direction;
    float exponent = 2;
    float cutoff = 180; // 90은 Mac과 Win에서 서로 다르게 보임
    float const_att = 1.1;

    // 네 번째 light
    GLfloat mat_diffuse[] = {1.0, 1.0, 1.0, 1.0};
    //GLfloat mat_ambient[] = {0.2, 0.2, 0.2, 1.0};
    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};

    //glLightfv(GL_LIGHT3, GL_AMBIENT, mat_ambient);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT3, GL_SPECULAR, mat_specular);
    glLightfv(GL_LIGHT3, GL_POSITION, light3_position);

    spot_direction = vector3(-1.0, -1.0, 0.0);
    glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, spot_direction.data());
    glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION, const_att);
    //glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, 0.5);
    //glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, 0.2);

    glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, cutoff);
    glLightf(GL_LIGHT3, GL_SPOT_EXPONENT, exponent);
    glEnable(GL_LIGHT3);

    // 세 번째 light
    //glLightfv(GL_LIGHT4, GL_AMBIENT, mat_ambient);
    glLightfv(GL_LIGHT4, GL_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT4, GL_SPECULAR, mat_specular);
    glLightfv(GL_LIGHT4, GL_POSITION, light4_position);
    spot_direction = vector3(0.0, -1.0, -1.0);
    glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, spot_direction.data());
    glLightf(GL_LIGHT4, GL_CONSTANT_ATTENUATION, const_att);
    //glLightf(GL_LIGHT4, GL_LINEAR_ATTENUATION, 0.5);
    //glLightf(GL_LIGHT4, GL_QUADRATIC_ATTENUATION, 0.2);
    glLightf(GL_LIGHT4, GL_SPOT_CUTOFF, cutoff);
    glLightf(GL_LIGHT4, GL_SPOT_EXPONENT, exponent);
    glEnable(GL_LIGHT4);

    // 세 번째 light
    //glLightfv(GL_LIGHT5, GL_AMBIENT, mat_ambient);
    glLightfv(GL_LIGHT5, GL_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT5, GL_SPECULAR, mat_specular);
    glLightfv(GL_LIGHT5, GL_POSITION, light5_position);
    spot_direction = vector3(1.0, -1.0, 0.0);
    glLightfv(GL_LIGHT5, GL_SPOT_DIRECTION, spot_direction.data());
    glLightf(GL_LIGHT5, GL_CONSTANT_ATTENUATION, const_att);
    //glLightf(GL_LIGHT5, GL_LINEAR_ATTENUATION, 0.5);
    //glLightf(GL_LIGHT5, GL_QUADRATIC_ATTENUATION, 0.2);
    glLightf(GL_LIGHT5, GL_SPOT_CUTOFF, cutoff);
    glLightf(GL_LIGHT5, GL_SPOT_EXPONENT, exponent);
    glEnable(GL_LIGHT5);

    // 세 번째 light
    //glLightfv(GL_LIGHT6, GL_AMBIENT, mat_ambient);
    glLightfv(GL_LIGHT6, GL_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT6, GL_SPECULAR, mat_specular);
    glLightfv(GL_LIGHT6, GL_POSITION, light6_position);
    spot_direction = vector3(0.0, -1.0, 1.0);
    glLightfv(GL_LIGHT6, GL_SPOT_DIRECTION, spot_direction.data());
    glLightf(GL_LIGHT6, GL_CONSTANT_ATTENUATION, const_att);
    //glLightf(GL_LIGHT6, GL_LINEAR_ATTENUATION, 0.5);
    //glLightf(GL_LIGHT6, GL_QUADRATIC_ATTENUATION, 0.2);
    glLightf(GL_LIGHT6, GL_SPOT_CUTOFF, cutoff);
    glLightf(GL_LIGHT6, GL_SPOT_EXPONENT, exponent);
    glEnable(GL_LIGHT6);

    // 세 번째 light
    //glLightfv(GL_LIGHT7, GL_AMBIENT, mat_ambient);
    glLightfv(GL_LIGHT7, GL_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT7, GL_SPECULAR, mat_specular);
    glLightfv(GL_LIGHT7, GL_POSITION, light7_position);
    spot_direction = vector3(0.0, 1.0, 0.0);
    glLightfv(GL_LIGHT7, GL_SPOT_DIRECTION, spot_direction.data());
    glLightf(GL_LIGHT7, GL_CONSTANT_ATTENUATION, const_att);
    //glLightf(GL_LIGHT7, GL_LINEAR_ATTENUATION, 0.5);
    //glLightf(GL_LIGHT7, GL_QUADRATIC_ATTENUATION, 0.2);
    glLightf(GL_LIGHT7, GL_SPOT_CUTOFF, cutoff);
    glLightf(GL_LIGHT7, GL_SPOT_EXPONENT, exponent);
    glEnable(GL_LIGHT7);
    }

    // Turn off directional lights
#if 0
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT2);
#endif
}

void idle(void)
{
    int milliseconds = glutGet(GLUT_ELAPSED_TIME);
    float sin_1 = sinf((float)milliseconds * 0.001f);
    return; // 애니메이션 끄기 - a, b, c, d 키로 움직여보고 싶을 때

    // Wall-e
    // move
    position_z = sin_1 * 2;

    // head
    head_rot = -sin_1 * 30;
    head_rot = -sinf((float)milliseconds * 0.002f) * 30;

    // 0 <= finger1 <= 60
    finger1 = (sinf((float)milliseconds * 0.004f) + 1) * 30; // 0~2

    // 0 <= elbow <= 100
    elbow = (sin_1 + 1) * 25; // 0~2

    // shoulder_yaw (위아래)
    //shoulder_yaw1 = sin_1 * 10 + 90;
    //shoulder_yaw2 = -sin_1 * 10 + 90;

    // 큰 로봇팔
    big_rot = -sin_1 * 15;

    big_elbow = sin_1 * 70; // 0~2
    if (big_elbow < 0) big_elbow = -big_elbow;

    big_finger1 = sin_1 * 60; // 0~2
    if (big_finger1 < 0) big_finger1 = -big_finger1;

    glutPostRedisplay();
}

void call_vertex3f(const vector3& vec)
{
    glVertex3f(vec[0], vec[1], vec[2]);
}

void new_vertex3f(float x, float y, float z)
{
    vector3 temp(x, y, z);
    call_vertex3f(temp);

    if (!first_run) return;

    float mv[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mv);

    float xp = mv[0] * x + mv[4] * y + mv[8] * z + mv[12];
    float yp = mv[1] * x + mv[5] * y + mv[9] * z + mv[13];
    float zp = mv[2] * x + mv[6] * y + mv[10] * z + mv[14];
    float wp = mv[3] * x + mv[7] * y + mv[11] * z + mv[15];

    xp /= wp;
    yp /= wp;
    zp /= wp;

    temp[0] = xp;
    temp[1] = yp;
    temp[2] = zp;

    g_vertices.push_back(temp);

    glPushMatrix();
    glLoadIdentity();

    glBegin(GL_POINTS);
    glVertex3f(xp,yp,zp);
    glEnd( );
    glPopMatrix();
}

void new_solidcube(const double size)
{
    const double l = size / 2.0;

    glutSolidCube(size);
    new_vertex3f(l, l, l);
    new_vertex3f(l, l, -l);
    new_vertex3f(-l, l, l);
    new_vertex3f(-l, l, -l);

    new_vertex3f(l, -l, l);
    new_vertex3f(l, -l, -l);
    new_vertex3f(-l, -l, l);
    new_vertex3f(-l, -l, -l);
}

void draw_floor(void)
{
    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);

    glVertex3f(-18.0, -1.0, 27.0);
    glVertex3f(27.0, -1.0, 27.0);
    glVertex3f(27.0, -1.0, -18.0);
    glVertex3f(-18.0, -1.0, -18.0);
    glEnd();
    glEnable(GL_LIGHTING);
}

void TriangularPrism(bool solid)
{
    // back endcap
    glBegin(solid ? GL_TRIANGLES : GL_LINES);
    glVertex3f(-0.5, 0.0, -0.5);
    glVertex3f(0.0, 0.87, -0.5);
    glVertex3f(0.5, 0.0, -0.5);
    _D_;
    glEnd();

    new_vertex3f(-0.5, 0.0, -0.5);
    new_vertex3f(0.0, 0.87, -0.5);
    new_vertex3f(0.5, 0.0, -0.5);

    // front endcap
    glBegin(solid ? GL_TRIANGLES : GL_LINES);
    glVertex3f(-0.5, 0.0, 0.5);
    glVertex3f(0.0, 0.87, 0.5);
    glVertex3f(0.5, 0.0, 0.5);
    _D_;
    glEnd();

    new_vertex3f(-0.5, 0.0, 0.5);
    new_vertex3f(0.0, 0.87, 0.5);
    new_vertex3f(0.5, 0.0, 0.5);

    // bottom
    glBegin(solid ? GL_QUADS : GL_LINES);
    glVertex3f(-0.5, 0.0, -0.5);
    glVertex3f(0.5, 0.0, -0.5);
    glVertex3f(0.5, 0.0, 0.5);
    glVertex3f(-0.5, 0.0, 0.5);
    _D_;
    glEnd();

    new_vertex3f(-0.5, 0.0, -0.5);
    new_vertex3f(0.5, 0.0, -0.5);
    new_vertex3f(0.5, 0.0, 0.5);
    new_vertex3f(-0.5, 0.0, 0.5);

    // back
    glBegin(solid ? GL_QUADS : GL_LINES);
    glVertex3f(-0.5, 0.0, -0.5);
    glVertex3f(0.0, 0.87, -0.5);
    glVertex3f(0.0, 0.87, 0.5);
    glVertex3f(-0.5, 0.0, 0.5);
    _D_;
    glEnd();

    new_vertex3f(-0.5, 0.0, -0.5);
    new_vertex3f(0.0, 0.87, -0.5);
    new_vertex3f(0.0, 0.87, 0.5);
    new_vertex3f(-0.5, 0.0, 0.5);

    // top
    glBegin(solid ? GL_QUADS : GL_LINES);
    glVertex3f(0.0, 0.87, -0.5);
    glVertex3f(0.5, 0.0, -0.5);
    glVertex3f(0.5, 0.0, 0.5);
    glVertex3f(0.0, 0.87, 0.5);
    _D_;
    glEnd();

    new_vertex3f(0.0, 0.87, -0.5);
    new_vertex3f(0.5, 0.0, -0.5);
    new_vertex3f(0.5, 0.0, 0.5);
    new_vertex3f(0.0, 0.87, 0.5);
}

void draw_cyl(GLUquadric* quad, float base, float top, float height, float slices, float stacks) // base: radius
{
    gluCylinder(quad, base, top, height, slices, stacks);

    const float l = base;
    new_vertex3f(l, height, l);
    new_vertex3f(l, height, -l);
    new_vertex3f(-l, height, l);
    new_vertex3f(-l, height, -l);
    _D_;

    new_vertex3f(l, 0, l);
    new_vertex3f(l, 0, -l);
    new_vertex3f(-l, 0, l);
    new_vertex3f(-l, 0, -l);
    _D_;

    glRotatef(180, 1,0,0);
    gluDisk(quad, 0.0f, base, slices, 1);
    glRotatef(180, 1,0,0);
    glTranslatef(0.0f, 0.0f, height);
    gluDisk(quad, 0.0f, top, slices, 1);
    glTranslatef(0.0f, 0.0f, -height);
}

void draw_robot_arm(bool has_cylinder, int shoulder_yaw_local, int elbow, int finger1)
{
    glPushMatrix();
    glTranslatef(-1.5, 0.0, 0.0);

    if (has_cylinder)
    {
        float radius = 1.00f;

        glPushMatrix();
        glRotatef(90, 0.0, 1.0, 0.0);
        GLUquadric *quadric = gluNewQuadric();
        //gluQuadricDrawStyle(quadric, GLU_FILL);
        //gluQuadricOrientation(quadric, GLU_INSIDE);
        glColor3f(0, 100/255.0, 0);
        draw_cyl(quadric, radius, radius, 0.7f, 20, 20);
        glPopMatrix();

        glTranslatef(1.5 + 0.7/2, 0.0, 0.0);
        glRotatef(big_rot, 1.0, 0.0, 0.0);
        glColor3f(0/255.0, 0/255.0, 139/255.0);
        glPushMatrix();
        glScalef(3.0, 0.7, 0.7);
        new_solidcube(1.0);
        glPopMatrix();
    }

    if (has_cylinder)
        glColor3f(0/255.0, 0/255.0, 128/255.0);
    else
        glColor3f(217/255.0, 135/255.0, 25/255.0);

    glTranslatef(1.5, 0.0, 0.0);
    glRotatef((GLfloat)shoulder_roll, 0.0, 0.0, 1.0);
    glRotatef((GLfloat)shoulder_yaw_local, 0.0, 1.0, 0.0);
    glTranslatef(1.0, 0.0, 0.0);
    glPushMatrix();
    glScalef(2.0, 0.7, 0.7);
    new_solidcube(1.0);
    glPopMatrix();

    glTranslatef(1.0, 0.0, 0.0);
    glRotatef((GLfloat)elbow, 0.0, 0.0, 1.0);
    glTranslatef(1.0, 0.0, 0.0);
    glPushMatrix();
    glScalef(2.0, 0.7, 0.7);
    new_solidcube(1.0);
    glPopMatrix();

    //finger
    if (has_cylinder)
        glColor3f(1.0, 0.0, 0.0);
    else
        glColor3f(192/255.0, 192/255.0, 192/255.0);

    float x_len = 0.3;
    for (int i = 0; i < 3; ++i)
    {
        glPushMatrix();

        // 0.15 - 손가락 끝이 손목에 겹치지 않게 띄워주기
        glTranslatef(1 + 0.15, 0.0, 0.0); //전 모델의 중간부터 끝까지
        glRotatef((GLfloat)-20, 0.0, 0.0, 1.0);
        glTranslatef(x_len/2, -0.7/2 + 0.2/2, 0.35 - 0.2/2 - 0.25 * i);
        glPushMatrix();
        glScalef(x_len + 0.1, 0.2, 0.2);
        new_solidcube(1.0);
        glPopMatrix();

        glTranslatef(x_len/2, 0.0, 0.0); //전 모델의 중간부터 끝까지
        glRotatef((GLfloat)finger1, 0.0, 0.0, 1.0);
        glTranslatef((x_len+0.2)/2, 0.0, 0.0);
        glPushMatrix();
        glScalef(x_len+0.2, 0.2, 0.2);
        new_solidcube(1.0);
        glPopMatrix();
        glPopMatrix();
    }

    glPushMatrix();
    glTranslatef(1+0.1, 0.0, 0.0);//전 모델의 중간부터 끝까지
    glRotatef((GLfloat)20, 0.0, 0.0, 1.0);
    glTranslatef(x_len/2, 0.3, 0.35 - 0.2/2);
    glPushMatrix();
    glScalef(x_len, 0.2, 0.2);
    new_solidcube(1.0);
    glPopMatrix();

    glTranslatef(x_len/2, 0.0, 0.0);//전 모델의 중간부터 끝까지
    glRotatef((GLfloat)-finger1, 0.0, 0.0, 1.0);
    glTranslatef((x_len + 0.2)/2, 0.0, 0.0);
    glPushMatrix();
    glScalef(x_len + 0.2, 0.2, 0.2);
    new_solidcube(1.0);
    glPopMatrix();
    glPopMatrix();

    glPopMatrix();
}

void draw_wall_e()
{
    float radius = 0.22f;
    glPushMatrix(); // top, move
    glTranslatef(0.0, 0.0, position_z);

    // head
    glPushMatrix();
    glRotatef(head_rot, 0.0, 1.0, 0.0);
    glTranslatef(radius, 1.0, -0.2);
    glPushMatrix();
    glTranslatef(-2*radius+0.02, 0.0, 0.0);
    glColor3f(105/255.0, 105/255.0, 105/255.0);
    GLUquadric *quadric = gluNewQuadric();
    // 아래 코드를 쓰면 반사가 안 생김
    //gluQuadricDrawStyle(quadric, GLU_FILL);
    //gluQuadricOrientation(quadric, GLU_INSIDE);
    draw_cyl(quadric, radius, radius, 0.7f, 20, 20);
    glPopMatrix();

    GLUquadric *quadric2 = gluNewQuadric();
    //gluQuadricDrawStyle(quadric2, GLU_FILL);
    //gluQuadricOrientation(quadric2, GLU_INSIDE);
    draw_cyl(quadric2, radius, radius, 0.7f, 20, 20);
    glPopMatrix();

    // neck
    glPushMatrix();
    glTranslatef(0.0f, 0.4f, 0.0f);
    glScalef(0.17f, 1.0f, 0.17f);
    glColor3f(105/255.0, 105/255.0, 105/255.0);
    new_solidcube(1.0f);
    glPopMatrix();

    // body
    glColor3f(255/255.0, 140/255.0, 0/255.0);
    new_solidcube(1.0f);

    // hand
    for (int i = 0; i < 2; ++i)
    {
        glPushMatrix();
        glTranslatef((i ? -1.0 : 1.0) * (0.5f + 0.1f), 0.2, 0.0);
        glScalef((i ? -1.0 : 1.0) * 0.25, 0.25, -0.25);
        draw_robot_arm(false, (i == 0 ? shoulder_yaw1 : shoulder_yaw2), elbow, finger1);
        glPopMatrix();
    }

    // wheel
    float wheel_thick = 0.3;
    for (int i = 0; i < 2; ++i)
    {
        glPushMatrix();
        glTranslatef((i ? -1.0 : 1.0) * (0.5f + wheel_thick/2), -0.65, -0.05);

        glRotatef(90.0, 0.0, 1.0, 0.0); // 옆면 바라보도록
        glScalef(1.3, 0.7, wheel_thick);
        glColor3f(84/255.0, 84/255.0, 84/255.0);
        TriangularPrism(true);
        glPopMatrix();
    }

    glPopMatrix(); // top, move
}

void display(void)
{
    if (g_2d_mode)
    {
        glDisable(GL_LIGHTING);

        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);

        glColor3f(0, 0, 0);
        glBegin(GL_LINES);
        glVertex2f(scr_width/2, 0);
        glVertex2f(scr_width/2, scr_height);
        glVertex2f(0, scr_height/2);
        glVertex2f(scr_width, scr_height/2);
        glEnd();

        stringstream ss;
        ss << "cross-section #"; 
        ss << target_cs_idx;
        ss << "  magnifier:"; 
        ss << scr_mag;
        DrawText(10, 10, ss.str(), GLUT_BITMAP_8_BY_13, vector3(0, 0, 0));

        {
            stringstream ss;
            ss << "cross-section type: ";
            switch (g_data.curve_type)
            {
                case data::BSPLINE:
                    ss << "B-Spline";
                    break;
                case data::INTERPOLATION:
                    ss << "Catmull-Rom";
                    break;
                case data::NATURAL_CUBIC:
                    ss << "Natural Cubic";
                    break;
                case data::B_SUBDIVISION:
                    ss << "B Subdivision";
                    break;
                case data::INTERPOLATING_SUBDIVISION:
                    ss << "Interpolating Subdivision";
                    break;
            }
            DrawText(10, scr_height - 20, ss.str(), GLUT_BITMAP_8_BY_13, vector3(0, 0, 0));
        }

        {
            stringstream ss;
            ss << "sweep type: ";
            switch (g_data.sweep_type)
            {
                case data::BSPLINE:
                    ss << "B-Spline";
                    break;
                case data::INTERPOLATION:
                    ss << "Catmull-Rom";
                    break;
                case data::NATURAL_CUBIC:
                    ss << "Natural Cubic";
                    break;
                case data::B_SUBDIVISION:
                    ss << "B Subdivision";
                    break;
                case data::INTERPOLATING_SUBDIVISION:
                    ss << "Interpolating Subdivision";
                    break;
            }
            DrawText(10, scr_height - 35, ss.str(), GLUT_BITMAP_8_BY_13, vector3(0, 0, 0));
        }

        glColor3f(0, 0, 0);
        glPointSize(10.0);

        glBegin(GL_POINTS);
        cross_sect_t& cs = g_data.con_pts[target_cs_idx];
        for (int j = 0; j < cs.size(); ++j)
        {
            glVertex2f(scr_width/2 + cs[j][0]*scr_mag, scr_height/2 + cs[j][2]*scr_mag);
        }
        glEnd();

        glBegin(GL_LINE_LOOP);
        vector<vector3> tant;

        vector<vector3> (*func)(const vector<vector3>&, vector<vector3>&, bool, float) = Catmull_Rom;

        switch (g_data.curve_type)
        {
            default:
            case data::BSPLINE:
                func = B_Spline;
                break;
            case data::INTERPOLATION:
                func = Catmull_Rom;
                break;
            case data::NATURAL_CUBIC:
                func = Natural_Cubic_Spline;
                break;
            case data::B_SUBDIVISION:
                func = B_Subdivision;
                break;
            case data::INTERPOLATING_SUBDIVISION:
                func = Interpolating_Subdivision;
                break;
        }

        vector<vector3> spline = func(cs, tant, true, 0.01);
        for (int j = 0; j < spline.size(); ++j)
        {
            glVertex2f(scr_width/2 + spline[j][0]*scr_mag, scr_height/2 + spline[j][2]*scr_mag);
        }
        glEnd();

        glFlush();
        glutSwapBuffers();

        glEnable(GL_LIGHTING);
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, wireframe_mode ? GL_LINE : GL_FILL);

    glMatrixMode(GL_MODELVIEW);
    if (first_run)
        glLoadIdentity();
    else
        set_cam_dist(view_distance);

    // rotate lights and objects together (stationary)
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    glLightfv(GL_LIGHT2, GL_POSITION, light2_position);
    glLightfv(GL_LIGHT3, GL_POSITION, light3_position);
    glLightfv(GL_LIGHT4, GL_POSITION, light4_position);
    glLightfv(GL_LIGHT5, GL_POSITION, light5_position);
    glLightfv(GL_LIGHT6, GL_POSITION, light6_position);
    glLightfv(GL_LIGHT7, GL_POSITION, light7_position);

    glPushMatrix(); // top

    if (first_run)
    {
        // SWEPT SURFACES
        string mat_name("gold");

        triangle_list.clear();
        triangle_list.reserve(2 * surfaces.size() * surfaces.front().size() + 12); // reserve 20812, actual 20604

        for (int i = 0; i < surfaces.size(); ++i)
        {
            if (i + 1 == surfaces.size())
                break;

            for (int j = 0; j < surfaces[i].size(); ++j)
            {
                const vector3& x = surfaces[i+1][j];
                const vector3& y = surfaces[i][j];
                const vector3& z = surfaces[i][(j+1)%surfaces[i].size()];
                triangle_t tmp1(z, y, x); // 왜 내가 생각하는 거랑 CCW가 반대인 거지?
                if (!surface_normals.empty())
                {
                    const vector3& x = surface_normals[i+1][j];
                    const vector3& y = surface_normals[i][j];
                    const vector3& z = surface_normals[i][(j+1)%surface_normals[i].size()];
                    tmp1.set_normal(z, y, x);
                }
                tmp1.material_name = mat_name;
                tmp1.cube = swept_transparent;
                triangle_list.push_back(tmp1);

                const vector3& d = surfaces[i][(j+1)%surfaces[i].size()];
                const vector3& e = surfaces[i+1][(j+1)%surfaces[i+1].size()];
                const vector3& f = surfaces[i+1][j];
                triangle_t tmp2(f, e, d); // 왜 내가 생각하는 거랑 CCW가 반대인 거지?
                if (!surface_normals.empty())
                {
                    const vector3& d = surface_normals[i][(j+1)%surface_normals[i].size()];
                    const vector3& e = surface_normals[i+1][(j+1)%surface_normals[i+1].size()];
                    const vector3& f = surface_normals[i+1][j];
                    tmp2.set_normal(f, e, d);
                }
                tmp2.material_name = mat_name;
                tmp2.cube = swept_transparent;
                triangle_list.push_back(tmp2);
            }
        }

        // A TRANSLUCENT CUBE
        polyhedron(4);

        // A TRANSLUCENT ICOSAHEDRON
        polyhedron(20);

        // Sort by transparency, size
        sort(triangle_list.rbegin(), triangle_list.rend()); // sort into descending order

        // Build a BSP tree
        root.clear();
        for (int i = 0; i < triangle_list.size(); ++i)
        {
            add_tree(&root, triangle_list[i]);
        }
    }

    // Draw swept surface and a cube
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_TRIANGLES);

    blue = 255;
    if (depth_ordering)
    {
        traverse_tree(&root, true);
    }
    else
    {
        for (int i = 0; i < triangle_list.size(); ++i)
        {
            const triangle_t& t = triangle_list[i];
            t.display();
        }
    }

    glEnd();
    glDisable(GL_BLEND);

    /*
    // FLOOR
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1, 1, 1, 0.3);
    draw_floor();
    glDisable(GL_BLEND);

    // WALL-E
    draw_wall_e();

    // ROBOT ARM
    glPushMatrix();
    glTranslatef(3.5, 1.0, 0.0);
    glRotatef(90, 0.0, 0.0, 1.0);
    draw_robot_arm(true, 0, big_elbow, big_finger1);
    glPopMatrix();
    */

    // Show all
    if (first_run)
    {
        for (set_t::iterator it = g_vertices.begin(); it != g_vertices.end(); ++it)
            mid_pt = mid_pt + *it;
        mid_pt = mid_pt / g_vertices.size();

        vector3 max(0, 0, 0);
        vector3 max2(0, 0, 0);

        int i = 0;
        for (set_t::iterator it = g_vertices.begin(); it != g_vertices.end(); ++it)
        {
            vector3 dist = *it - mid_pt;
            if (dist.length() >= max.length())
            {
                max2 = max;
                max = dist;
            }
        }
        max_rad = max.length();

        if (max != max2 && max != zero && max2 != zero)
        {
            vector3 v = cml::cross(max, max2);
            normal = v;
        }
        else
            normal = zero;
        normal = normal.normalize();

        first_run = false;
        glutPostRedisplay();
    }
    ////

    glPopMatrix(); // top
    glutSwapBuffers();
}

void set_cam_dist(float view_distance)
{
    cam_vec = cam_vec.normalize() * view_distance;
    vector3 eye_pos = rot_origin + cam_vec;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (g_2d_mode)
    {
        //gluLookAt(0, 2, 0, 0, 0, 0, 0, 0, 1);
    }
    else
        gluLookAt(eye_pos[0], eye_pos[1], eye_pos[2], rot_origin[0], rot_origin[1], rot_origin[2], cam_up_vec[0], cam_up_vec[1], cam_up_vec[2]);
}

void reshape(int w, int h)
{
    scr_width = w;
    scr_height = h;

    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (g_2d_mode)
    {
        gluOrtho2D (0, w, 0, h);
    }
    else
    {
        gluPerspective(fov, (GLfloat)w/(GLfloat)h, 1.0, 20.0);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (!g_2d_mode)
        set_cam_dist(view_distance);
}

double calculateAngle(double size, double distance)
{
    double radtheta, degtheta; 

    radtheta = 2.0 * atan2 (size/2.0, distance);
    degtheta = (180.0 * radtheta) / PI;
    return (degtheta);
}

void keyboard(unsigned char key, int x, int y)
{
    vector3 x_vec, y_vec, z_vec;
    orthonormal_basis(cam_vec, cam_up_vec, x_vec, y_vec, z_vec);

    switch (key)
    {
        // Translate
        case ';':
            rot_origin -= x_vec * 0.1;
            set_cam_dist(view_distance);
            glutPostRedisplay();
            break;
        case '\'':
            rot_origin += x_vec * 0.1;
            set_cam_dist(view_distance);
            glutPostRedisplay();
            break;
        case '[':
            rot_origin += y_vec * 0.1;
            set_cam_dist(view_distance);
            glutPostRedisplay();
            break;
        case '/':
            rot_origin -= y_vec * 0.1;
            set_cam_dist(view_distance);
            glutPostRedisplay();
            break;

            // Reset
        case 'r':
            {
                // reset variables
                fov = INITIAL_FOV;
                rot_origin.zero();
                cam_vec = INIT_CAM_VEC;
                cam_up_vec = INIT_CAM_UP_VEC;
                view_distance = INITIAL_VIEW_DISTANCE;

                GLint viewport[4];
                glGetIntegerv(GL_VIEWPORT, viewport);
                GLint width = viewport[2];
                GLint height = viewport[3];

                reshape(width, height);
                set_cam_dist(view_distance);
                glutPostRedisplay();
                break;
            }
        case 'e':
            {
                g_2d_mode = !g_2d_mode;

                init();
                GLint viewport[4];
                glGetIntegerv(GL_VIEWPORT, viewport);
                GLint width = viewport[2];
                GLint height = viewport[3];

                draw_swept_surface();
                reshape(width, height);
                glutPostRedisplay();
                break;
            }
        case '+':
        case '-':
            {
                if (g_2d_mode)
                {
                    target_cs_idx = target_cs_idx + (key == '+' ? 1 : -1);
                    if (target_cs_idx < 0) target_cs_idx = 0;
                    else if (target_cs_idx >= g_data.con_pts.size()) target_cs_idx = g_data.con_pts.size() - 1;
                    glutPostRedisplay();
                }
                break;
            }
            // Zoom in/out
        case 'z':
        case 'Z':
            {
                if (g_2d_mode)
                {
                    if (key == 'z')
                    {
                        scr_mag -= 1.0;
                        if (scr_mag < 1)
                            scr_mag = 1;
                    }
                    else
                        scr_mag += 1.0;
                    glutPostRedisplay();
                    break;
                }

                if (key == 'z')
                {
                    fov -= 1.0;
                    fov = max(fov, 0.0f);
                }
                else
                    fov += 1.0;

                GLint viewport[4];
                glGetIntegerv(GL_VIEWPORT, viewport);
                GLint width = viewport[2];
                GLint height = viewport[3];

                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                gluPerspective(fov, (GLfloat)width/(GLfloat)height, 1, 20.0);
                glMatrixMode(GL_MODELVIEW);
                glutPostRedisplay();
                break;
            }

        case 's':
            {
                rot_origin = mid_pt;
                cam_up_vec = vector3().cardinal(1);
                cam_vec = normal;

                view_distance = max_rad + 1; // zNear
                fov = calculateAngle(2*max_rad, view_distance);

                GLint viewport[4];
                glGetIntegerv(GL_VIEWPORT, viewport);
                GLint width = viewport[2];
                GLint height = viewport[3];

                reshape(width, height);
                set_cam_dist(view_distance);
                glutPostRedisplay();
                break;
            }
            // Dolly in/out (일종의 zoom)
        case 'd':
        case 'D':
            {
                if (key == 'd')
                {
                    view_distance -= 0.1;
                    view_distance = max(view_distance, 0.1f);
                }
                else
                    view_distance += 0.1;

                set_cam_dist(view_distance);
                glutPostRedisplay();
                break;
            }

            // Rotate camera
            // 여러 관절 조종
            // (애니메이션 끄고 할 것)
        case '1': // Cross func
        case '2':
        case '3':
        case '4':
        case '5':
            {
                g_data.curve_type = static_cast<data::spline_t>(key - '0');
                draw_swept_surface();
                glutPostRedisplay();
                break;
            }
        case '6': // Sweep func
        case '7':
        case '8':
        case '9':
        case '0':
            {
                int type = key - '0';
                if (type == 0)
                    type = 10;
                type -= 5;
                g_data.sweep_type = static_cast<data::spline_t>(type);
                draw_swept_surface();
                glutPostRedisplay();
                break;
            }
        case 'w':
            wireframe_mode = !wireframe_mode;
            glutPostRedisplay();
            break;

            // HW 4
        case 'o':
            depth_ordering = !depth_ordering;
            glutPostRedisplay();
            break;
        case 't':
        case 'T':
            transparency += (key == 't' ? 1 : -1) * 0.05;
            transparency = cml::clamp(transparency, 0.0f, 1.0f);
            glutPostRedisplay();
            break;
        case 'f':
            swept_transparent = !swept_transparent;
            transparency = INIT_TRANSPARENCY;
            draw_swept_surface();
            glutPostRedisplay();
            break;
    }
}

vector3 screen_to_sphere(float x, float y)
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLint width = viewport[2];
    GLint height = viewport[3];

    float half_width = width / 2;
    float half_height = height / 2;

    //vector 1
    //(0, 0, r)

    //screen -> [-1, 1] plane
    //1. move screen orgin
    x -= half_width;
    y -= half_height;

    y = -y; // y축을 아래 방향에서 윗 방향으로

    //2. normalize
    x /= half_width;
    y /= half_height;

    //3.
    float z = sqrt(1 - pow(x, 2) - pow(y, 2));

    // outside a circle
    float sq_sum = pow(x, 2) + pow(y, 2);
    if (sq_sum > 1)
    {
        x /= sqrt(sq_sum);
        y /= sqrt(sq_sum);
        z = 0;
    }
    return vector3(x, y, z);
}

GLvoid mouse(GLint button, GLint state, GLint x, GLint y)
{
    if (state == GLUT_DOWN)
    {
        switch (button)
        {
            case GLUT_LEFT_BUTTON:
                if (g_2d_mode)
                {
                    int x_ = x;
                    int y_ = scr_height - y;

                    cross_sect_t& cs = g_data.con_pts[target_cs_idx];
                    for (int j = 0; j < cs.size(); ++j)
                    {
                        int t_x = scr_width/2 + cs[j][0]*scr_mag;
                        int t_y = scr_height/2 + cs[j][2]*scr_mag;

                        if (abs(t_x - x_) < 10 && abs(t_y - y_) < 10)
                        {
                            drag_cp = true;
                            target_cp = j;
                            break;
                        }
                    }
                }
                else
                {
                    start_vec = screen_to_sphere(x, y);
                    cam_vec_start = cam_vec;
                    cam_up_vec_start = cam_up_vec;
                }
                break;
            case GLUT_RIGHT_BUTTON:
                if (g_2d_mode)
                {
                    int x_ = x;
                    int y_ = scr_height - y;

                    int min_idx = 0;
                    float min_dist = -1;

                    bool add = true;
                    cross_sect_t& cs = g_data.con_pts[target_cs_idx];
                    for (int j = 0; j < cs.size(); ++j)
                    {
                        int t_x = scr_width/2 + cs[j][0]*scr_mag;
                        int t_y = scr_height/2 + cs[j][2]*scr_mag;

                        float dist = sqrt(pow((float)t_x - x_, 2) + pow((float)t_y - y_, 2));

                        if (min_dist < 0)
                        {
                            min_dist = dist;
                            min_idx = j;
                        }

                        if (dist < min_dist)
                        {
                            min_dist = dist;
                            min_idx = j;
                        }

                        if (dist < 10)
                        {
                            target_cp = 0;
                            cs.erase(cs.begin() + j);
                            add = false;
                            break;
                        }
                    }

                    if (add)
                    {
                        vector3 t((float)(x_-scr_width/2.0)/scr_mag, 0, (float)(y_-scr_height/2.0)/scr_mag);
                        cs.insert(cs.begin() + min_idx + 1, t);
                    }
                    glutPostRedisplay();
                }
                else
                {
                    rot_origin = screen_to_object(x, y); // 새 위치를 origin으로
                    set_cam_dist(view_distance);
                    glutPostRedisplay();
                    break;
                }
        }
    }
    else // GLUT_UP
    {
        drag_cp = false;

        vector3 x, y, z;
        orthonormal_basis(cam_vec, cam_up_vec, x, y, z);
        cam_vec = z;
        cam_up_vec = y;
    }
}

GLvoid motion(GLint x, GLint y)
{
    if (g_2d_mode)
    {
        if (drag_cp)
        {
            int x_ = x;
            int y_ = scr_height - y;

            g_data.con_pts[target_cs_idx][target_cp].set((float)(x_-scr_width/2)/scr_mag, 0, (float)(y_-scr_height/2)/scr_mag);
            glutPostRedisplay();
        }
        return;
    }

    vector3 end_vec = screen_to_sphere(x, y);

    // Sphere의 좌표계를 View 좌표계와 일치
    matrix rot_axis;
    matrix_rotation_align(rot_axis, cam_vec_start, cam_up_vec_start);

    vector3 start = transform_vector(rot_axis, start_vec);
    vector3 end = transform_vector(rot_axis, end_vec);

    // 이제 변환된 vector로 실제 회전을 구한다.
    matrix rot;
    matrix_rotation_vec_to_vec(rot, end, start);

    cam_vec = transform_vector(rot, cam_vec_start);
    cam_up_vec = transform_vector(rot, cam_up_vec_start);

    glutPostRedisplay();
}

const float cross_resolution  = 0.03;
const float sweep_resolution  = 0.03;

void draw_swept_surface()
{
    first_run = true; // BSP 새로 구성하기 위해

    vector<cross_sect_t> draw_pt_list;
    vector<cross_sect_t> normal_list;

    vector<vector3> pos_spline;
    vector<vector3> scale_spline;

    vector<vector3> (*func)(const vector<vector3>&, vector<vector3>&, bool, float) = B_Spline;
    vector<vector3> (*sweep_func)(const vector<vector3>&, vector<vector3>&, bool, float) = Catmull_Rom;

    switch (g_data.curve_type)
    {
        default:
        case data::BSPLINE:
            func = B_Spline;
            break;
        case data::INTERPOLATION:
            func = Catmull_Rom;
            break;
        case data::NATURAL_CUBIC:
            func = Natural_Cubic_Spline;
            break;
        case data::B_SUBDIVISION:
            func = B_Subdivision;
            break;
        case data::INTERPOLATING_SUBDIVISION:
            func = Interpolating_Subdivision;
            break;
    }

    switch (g_data.sweep_type)
    {
        case data::BSPLINE:
            sweep_func = B_Spline;
            break;
        default:
        case data::INTERPOLATION:
            sweep_func = Catmull_Rom;
            break;
        case data::NATURAL_CUBIC:
            sweep_func = Natural_Cubic_Spline;
            break;
        case data::B_SUBDIVISION:
            sweep_func = B_Subdivision;
            break;
        case data::INTERPOLATING_SUBDIVISION:
            sweep_func = Interpolating_Subdivision;
            break;
    }

    pos_spline = sweep_func(g_data.pos, tangents, false, sweep_resolution);
    scale_spline = sweep_func(g_data.scale_factor, tangents, false, sweep_resolution);

    vector<quaternionf_p> tant;  
    vector<quaternionf_p> ori_spline = Catmull_Rom(g_data.orient, tant, false, 0.005);


    vector<cross_sect_t> tangent_list;
    for (int i = 0; i < g_data.con_pts.size(); ++i)
    {
        if (g_data.con_pts[i].empty())
            continue;

        draw_pt_list.push_back(func(g_data.con_pts[i], tangents, true, cross_resolution));
        if (!tangents.empty())
            tangent_list.push_back(tangents);
    }

    {
        vector<vector<vector3> > result;
        vector<vector<vector3> > result2;
        // cross section의 한 point 당
        int j;
        for (int j = 0; j < draw_pt_list.front().size(); ++j)
        {
            vector<vector3> temp;
            vector<vector3> temp2;
            for (int i = 0; i < draw_pt_list.size(); ++i)
            {
                // 처음부터 끝까지 곡선을 이루는 각 point
                //if (j >= draw_pt_list[i].size()) break;
                temp.push_back(draw_pt_list[i][j % draw_pt_list[i].size()]);
                if (!tangent_list.empty())
                    temp2.push_back(tangent_list[i][j % tangent_list[i].size()]);
            }
            temp = sweep_func(temp, tangents, false, sweep_resolution);
            result.push_back(temp);

            if (!temp2.empty())
            {
                temp2 = sweep_func(temp2, tangents, false, sweep_resolution);
                result2.push_back(temp2);
            }

        }

        surfaces.clear();
        surface_normals.clear();

        for (int len_idx = 0; len_idx < result.front().size(); ++len_idx)
        {
            vector<vector3> c;
            vector<vector3> normals;
            for (int pt_idx = 0; pt_idx < result.size(); ++pt_idx)
            {
                vector3 temp = scale_spline[len_idx][0] * result[pt_idx][len_idx];

                vector3 axis;
                float angle;
                quaternion_to_axis_angle(ori_spline[min<int>(len_idx, ori_spline.size()-1)], axis, angle);
                temp = rotate_vector(temp, axis, angle) + pos_spline[len_idx];

                c.push_back(temp);

                if (!result2.empty())
                {
                    vector3 normal = rotate_vector(result2[pt_idx][len_idx], vector3().cardinal(2), PI / 2);
                    normal = rotate_vector(normal, axis, angle);
                    normals.push_back(normal.normalize());
                }
            }
            surfaces.push_back(c);
            if (!normals.empty())
                surface_normals.push_back(normals);
        }
    }

    /*
       for (int i = 0; i < pos_spline.size(); ++i)
       {
       int j = 0;

       cross_sect_t c;
       for (int k = 0; k < draw_pt_list[j].size(); ++k)
       {
       double s = scale_spline[i][0];
       cml::vector3d t = draw_pt_list[j][k];
       c.push_back(s*t + pos_spline[i]);
       }
       surfaces.push_back(c);
       }
       */
}

string getline_and_ss(fstream& fs)
{
    string temp;
    while (fs.good() && temp.empty())
    {
        getline(fs, temp);

        bool flag = true;
        for (int i = 0; i < temp.size(); ++i)
        {
            if (!isspace(temp[i]))
            {
                flag = false;
                break;
            }
        } 
        if (flag) temp.clear();

        if (!temp.empty() && temp[0] == '#')
        {
            temp.clear();
        }
    }
    return temp;
}

void read_data_file(const string& fname = "data.txt")
{
    fstream file(fname.c_str(), fstream::in);

    string curve_type;
    {
        stringstream ss(getline_and_ss(file));
        ss >> curve_type;
    }
    //cout << curve_type << endl;

    g_data.curve_type = data::BSPLINE;
    if (curve_type == "INTERPOLATION")
        g_data.curve_type = data::INTERPOLATION;

    int num_of_keyframes = 0;
    {
        stringstream ss(getline_and_ss(file));
        ss >> num_of_keyframes;
    }
    //cout << num_of_keyframes << endl;

    int num_of_control_points = 0;
    {
        stringstream ss(getline_and_ss(file));
        ss >> num_of_control_points;
    }
    //cout << num_of_control_points << endl;

    g_data.clear();
    for (int i = 0; i < num_of_keyframes; ++i)
    {
        cross_sect_t cs;
        for (int j = 0; j < num_of_control_points; ++j)
        {
            float x = 0;
            float z = 0;
            {
                stringstream ss(getline_and_ss(file));
                ss >> x >> z;
            }
            //cout << "x " << x << " z " << z << endl;

            vector3 t(x, 0, z);
            cs.push_back(t);
        }
        g_data.con_pts.push_back(cs);

        float scale_factor;
        {
            stringstream ss(getline_and_ss(file));
            ss >> scale_factor;
        }
        //cout << "scaling_factor " << scale_factor << endl;
        g_data.scale_factor.push_back(vector3(scale_factor, 0, 0));

        float angle = 0;
        vector3 axis;
        {
            stringstream ss(getline_and_ss(file));
            ss >> angle >> axis[0] >> axis[1] >> axis[2];
        }
        //cout << angle << " axis " << axis << endl;

        g_data.angle.push_back(angle);
        g_data.axis.push_back(axis);

        quaternionf_p orient;
        quaternion_rotation_axis_angle(orient, axis, angle);
        g_data.orient.push_back(orient);

        vector3 pos;
        {
            stringstream ss(getline_and_ss(file));
            ss >> pos[0] >> pos[1] >> pos[2];
        }
        //cout << "position " << pos << endl;
        if (pos[2] >= 25)
            pos[2] -= 25;
        g_data.pos.push_back(pos);
    }

    file.close();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(scr_width, scr_height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);

    // chrome
    material_t& chrome = material_map["chrome"];
    chrome.specular = vector4(0.774597, 0.774597, 0.77459797, 1.0);
    chrome.diffuse = vector4(0.4, 0.4, 0.4, 1.0);
    //chrome.diffuse = vector4(0.0,  1.0,  0.0, 1.0);
    chrome.ambient = vector4(0.25, 0.25, 0.25, 1.0);
    chrome.shininess = 76.8;

    // plastic (red)
    material_t& plastic = material_map["plastic"];
    plastic.ambient = vector4(0.0, 0.0, 0.0, 1.0);
    plastic.diffuse = vector4(0.5, 0.0, 0.0, 1.0);
    plastic.specular = vector4(0.7, 0.6, 0.6, 1.0);
    plastic.shininess = 32;

    // gold
    material_t& gold = material_map["gold"];
    gold.specular = vector4(0.628281, 0.555802, 0.366065, 1.0);
    gold.diffuse = vector4(0.75164, 0.60648, 0.22648, 1.0);
    gold.ambient = vector4(0.24725, 0.1995, 0.0745, 1.0);
    gold.shininess = 51.2;

    // silver
    material_t& silver = material_map["silver"];
    silver.ambient = vector4(0.19225,  0.19225,  0.19225, 1.0);
    silver.diffuse = vector4(0.50754,  0.50754,  0.50754, 1.0);
    silver.specular = vector4(0.508273, 0.508273, 0.508273,1.0);
    silver.shininess = 51.2;
    
    /*
    // polished silver
    silver.specular = vector4(0.773911, 0.773911, 0.773911, 1.0);
    silver.diffuse = vector4(0.2775, 0.2775, 0.2775, 1.0);
    silver.ambient = vector4(0.23125, 0.23125, 0.23125, 1.0);
    silver.shininess = 89.6;
    */

    // jade
    material_t& jade = material_map["jade"];
    jade.ambient = vector4(0.135, 0.2225, 0.1575, 1.0);
    jade.diffuse = vector4(0.54, 0.89, 0.63, 1.0);
    jade.specular = vector4(0.316228, 0.316228, 0.316228, 1.0);
    jade.shininess = 12.8;

	//23:Rubber(Yellow)
    material_t& rubber = material_map["rubber"];
    rubber.ambient = vector4(0.05, 0.05, 0.0, 1.0);
    rubber.diffuse = vector4(0.5, 0.5, 0.4, 1.0);
    rubber.specular = vector4(0.7, 0.7, 0.04, 1.0);
    rubber.shininess = 10.0;
    
    // brass
    //material_t& brass = material_map["brass"];
    //brass.specular = vector4(0.992157, 0.941176, 0.807843, 1.0);
    //brass.diffuse = vector4(0.780392, 0.568627, 0.113725, 1.0);
    //brass.ambient = vector4(0.329412, 0.223529, 0.027451, 1.0);
    //brass.shininess = 27.8974;

    // translucent cube material
    material_t& cube = material_map["cube"];
    cube.specular = vector4(1.0, 1.0, 1.0, 1.0);
    cube.diffuse = vector4(0.0, 0.0, 255/255.0, transparency);
    cube.ambient = vector4(0.1, 0.1, 0.1, 1.0);
    //cube.ambient = vector4(0.0, 0.0, 255/255.0, 1.0);
    //cube.ambient = vector4(25/255.0, 25/255.0, 112/255.0, 1.0);
    cube.shininess = 128.0;

    {
        /*
    material_t& rubber = material_map["b"];
    rubber.ambient = vector4(0.05, 0.05, 0.05, 1.0);
    rubber.diffuse = vector4(0.5, 0.5, 0.5, 1.0);
    rubber.specular = vector4(0.7, 0.7, 0.7, 1.0);
    rubber.shininess = 10.0;
    */
    }

    read_data_file();
    init();

    draw_swept_surface();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    glutMainLoop();
    return 0;
}
