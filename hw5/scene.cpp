#include "scene.h"
using namespace std;

#include "triangle.h"
#include "polyhedron.h"

#if 0
void make_quad(const vector3& v0, const vector3& v2)
{
    vector3 v0();
    vector3 v1();
    vector3 v2();
    vector3 v3();

    material mat;
    mat.diffuse = vector3(1.0, 1.0, 1.0);
    mat.specular = vector3(0.2, 0.2, 0.2);
    mat.transparency = 1.0;
    mat.shininess = 20;

    shared_ptr<object> triangle1(new triangle(v1, v2, v3));
    triangle1->mat = mat;
    triangle1->texture = bricks;
    s.objs.push_back(triangle1);

    shared_ptr<object> triangle1(new triangle(v3, v0, v1));
    triangle1->mat = mat;
    triangle1->texture = bricks;
    s.objs.push_back(triangle1);
}
#endif


const string tex_dir("./textures/");
#define TEX_DIR(X) (tex_dir + (X)).c_str()

shared_ptr<CImg<float>> image(new CImg<float>(TEX_DIR("52964_Black-Metal-Texture.jpg")));
shared_ptr<CImg<float>> wall(new CImg<float>(TEX_DIR("docwallorigin2dt.jpg")));
shared_ptr<CImg<float>> circle(new CImg<float>(TEX_DIR("bumpmap.jpg")));
shared_ptr<CImg<float>> water(new CImg<float>(TEX_DIR("Fresh_water_ocean_texture.jpg")));
shared_ptr<CImg<float>> bricks(new CImg<float>(TEX_DIR("bricks.jpg")));
shared_ptr<CImg<float>> frosted_glass(new CImg<float>(TEX_DIR("frosted_glass_bathroom_4060478.JPG")));
shared_ptr<CImg<float>> glass(new CImg<float>(TEX_DIR("glass_texture_P1012107.JPG")));
shared_ptr<CImg<float>> blue(new CImg<float>(TEX_DIR("3783098647_fb208001b5_b.jpg")));
shared_ptr<CImg<float>> stone(new CImg<float>(TEX_DIR("hrt-stone-texture-3.jpg")));
shared_ptr<CImg<float>> frosted_glass2(new CImg<float>(TEX_DIR("frosted_glass_4060475.JPG")));
shared_ptr<CImg<float>> emboss(new CImg<float>(TEX_DIR("Normal Map.bmp")));

#undef TEX_DIR

void scene_aux_0(scene& s)
{
    shared_ptr<object> sphere1(new sphere(vector3(1.0, 1.0, -0.0), 1.0));
    sphere1->mat.diffuse = vector3(1.0, 0.0, 0.0);
    sphere1->mat.specular = vector3(1.0, 1.0, 1.0);
    sphere1->mat.transparency = 0.2;
    sphere1->mat.shininess = 100;
    s.objs.push_back(sphere1);

    shared_ptr<object> sphere2(new sphere(vector3(-1.5, 0, -1.0), 1.0));
    sphere2->mat.diffuse = vector3(0.0, 0.0, 1.0);
    sphere2->mat.specular = vector3(1.0, 1.0, 1.0);
    sphere2->mat.transparency = 1.0;
    sphere2->mat.shininess = 20;
    sphere2->move_dir = vector3(0, 2.5, 0);
    s.objs.push_back(sphere2);

    shared_ptr<object> triangle1(new triangle(vector3(2.0, 0, -3.0), vector3(0.0, 2.0, -3.0), vector3(-2.0, 0, -3.0)));
    triangle1->mat.diffuse = vector3(0.0, 1.0, 0.0);
    triangle1->mat.specular = vector3(0.2, 0.2, 0.2);
    triangle1->mat.transparency = 0.5;
    triangle1->mat.shininess = 20;
    s.objs.push_back(triangle1);

    const float floor_y = -1.5;
    const float floor_y_up = 0.0;
    {
        shared_ptr<object> triangle1(new triangle(vector3(10.0, floor_y + floor_y_up, -10.0), vector3(-10.0, floor_y + floor_y_up, -10.0), vector3(-10.0, floor_y, 10.0)));
        triangle1->mat.diffuse = vector3(1.0, 1.0, 1.0);
        triangle1->mat.specular = vector3(0.2, 0.2, 0.2);
        triangle1->mat.transparency = 1.0;
        triangle1->mat.shininess = 20;
        //s.objs.push_back(triangle1);
    }
    {
        shared_ptr<object> triangle1(new triangle(vector3(-10.0, floor_y, 10.0), vector3(10.0, floor_y, 10.0), vector3(10.0, floor_y + floor_y_up, -10.0)));
        triangle1->mat.diffuse = vector3(1.0, 1.0, 1.0);
        triangle1->mat.specular = vector3(0.2, 0.2, 0.2);
        triangle1->mat.transparency = 1.0;
        triangle1->mat.shininess = 20;
        //s.objs.push_back(triangle1);
    }

    shared_ptr<object> cube1(new cube(vector3(1.0, 2.0, -2.0), 1));
    cube1->mat.diffuse = vector3(1.0, 1.0, 1.0);
    cube1->mat.specular = vector3(1.0, 1.0, 1.0);
    cube1->mat.transparency = 1.0;
    cube1->mat.shininess = 20;
    s.objs.push_back(cube1);

    shared_ptr<object> ico1(new icosahedron(vector3(0.0, -1.0, -0.5), 1));
    ico1->mat.diffuse = vector3(0.0, 1.0, 0.0);
    ico1->mat.specular = vector3(1.0, 1.0, 1.0);
    ico1->mat.transparency = 0.1;
    ico1->mat.shininess = 100;
    ico1->mat.reflection = 0.5;
    s.objs.push_back(ico1);

    s.lights.push_back(light(vector3(0, 0, 5)));
    s.lights.back().dir = vector3(0, 0, -1);
    s.lights.push_back(light(vector3(1, 5, -2)));
    s.lights.back().dir = vector3(0, -1, 0);
    //s.lights.push_back(light(vector3(0, 0, 7)));
    //s.lights.push_back(light(vector3(-1, -2, 0)));
    //s.lights.push_back(light(vector3(0, 2, -1)));
}

void scene_aux_1(scene& s)
{
    shared_ptr<object> sphere1(new sphere(vector3(1.0, 1.0, -0.0), 1.0));
    sphere1->mat.diffuse = vector3(1.0, 0.0, 0.0);
    sphere1->mat.specular = vector3(1.0, 1.0, 1.0);
    sphere1->mat.transparency = 0.2;
    sphere1->mat.shininess = 100;
    sphere1->mat.reflection = 1;
    sphere1->texture = image;
    //s.objs.push_back(sphere1);

    shared_ptr<object> sphere2(new sphere(vector3(-0.2, 0.2, 0.0), 0.7));
    sphere2->mat.diffuse = vector3(0.5, 0.5, 0.5);
    sphere2->mat.specular = vector3(1.0, 1.0, 1.0);
    sphere2->mat.transparency = 0.4;
    sphere2->mat.shininess = 100;
    sphere2->mat.reflection = 0.2;
    //sphere2->move_dir = vector3(0, 2.5, 0);
    //sphere2->bump_map = emboss;
    s.objs.push_back(sphere2);

    shared_ptr<object> triangle1(new triangle(vector3(2.0, 0, -3.0), vector3(0.0, 2.0, -3.0), vector3(-2.0, 0, -3.0)));
    triangle1->mat.diffuse = vector3(1.0, 1.0, 1.0);
    triangle1->mat.specular = vector3(0.2, 0.2, 0.2);
    triangle1->mat.transparency = 0.5;
    triangle1->mat.shininess = 20;
    triangle1->texture = image;
    //triangle1->bump_map = circle;
    s.objs.push_back(triangle1);

    const float floor_y =-0.5;
    const float floor_y_up = 0.0;
    const float len = 10.0;
    {
        shared_ptr<object> triangle1(new triangle(vector3(-len, floor_y + floor_y_up, -len), vector3(-len, floor_y + floor_y_up, len), vector3(len, floor_y, len)));
        triangle1->mat.diffuse = vector3(1.0, 1.0, 1.0);
        triangle1->mat.specular = vector3(0.2, 0.2, 0.2);
        triangle1->mat.transparency = 1.0;
        triangle1->mat.shininess = 0;
        triangle1->mat.reflection = 0;
        triangle1->texture = bricks;
        //triangle1->bump_map = wall;
        s.objs.push_back(triangle1);
    }
    {
        shared_ptr<triangle> triangle1(new triangle(vector3(len, floor_y, len), vector3(len, floor_y, -len), vector3(-len, floor_y + floor_y_up, -len)));
        triangle1->mat.diffuse = vector3(1.0, 1.0, 1.0);
        triangle1->mat.specular = vector3(0.2, 0.2, 0.2);
        triangle1->mat.transparency = 1.0;
        triangle1->mat.shininess = 20;
        triangle1->texture = bricks;
        triangle1->mat.reflection = 0;
        triangle1->inverted_xy = true;
        s.objs.push_back(triangle1);
    }

    shared_ptr<object> cube1(new cube(vector3(1.0, 2.0, -2.0), 1));
    cube1->mat.diffuse = vector3(1.0, 1.0, 1.0);
    cube1->mat.specular = vector3(1.0, 1.0, 1.0);
    cube1->mat.transparency = 1.0;
    cube1->mat.shininess = 20;
    s.objs.push_back(cube1);

    shared_ptr<object> ico1(new icosahedron(vector3(0.0, -1.0, -0.5), 1));
    ico1->mat.diffuse = vector3(0.0, 1.0, 0.0);
    ico1->mat.specular = vector3(1.0, 1.0, 1.0);
    ico1->mat.transparency = 0.1;
    ico1->mat.shininess = 100;
    ico1->mat.reflection = 0.5;
    //s.objs.push_back(ico1);

    s.lights.push_back(light(vector3(-1.5, 5, -1)));
    s.lights.back().dir = vector3(0, -1, 0);
    s.lights.push_back(light(vector3(0, 0.5, 5)));
    s.lights.back().dir = vector3(0, -0.5, -1);
    //s.lights.push_back(light(vector3(0, 0, 7)));
    //s.lights.push_back(light(vector3(-1, -2, 0)));
    //s.lights.push_back(light(vector3(0, 2, -1)));
}

scene::scene(int scene_num)
    : g_amb_light(0.4, 0.4, 0.4)
{
    switch (scene_num)
    {
        case 0: scene_aux_0(*this); break;
        case 1: scene_aux_1(*this); break;
        default: assert(false);
    }
}

