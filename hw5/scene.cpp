#include "scene.h"
using namespace std;

#include "triangle.h"
#include "polyhedron.h"
#include "obj_file.h"

const string tex_dir("./textures/");
#define TEX_DIR(X) (tex_dir + string(X) + ".bmp").c_str() // GraphicsMagick이 깔려 있으면 .bmp는 빼도 된다.

shared_ptr<CImg<float>> wall(new CImg<float>(TEX_DIR("docwallorigin2dt.jpg")));
shared_ptr<CImg<float>> image(new CImg<float>(TEX_DIR("52964_Black-Metal-Texture.jpg")));
shared_ptr<CImg<float>> water(new CImg<float>(TEX_DIR("Fresh_water_ocean_texture.jpg")));
shared_ptr<CImg<float>> bricks(new CImg<float>(TEX_DIR("bricks.jpg")));
shared_ptr<CImg<float>> checker(new CImg<float>(TEX_DIR("CB_Texture_02_by_CB_Stock.jpg")));
shared_ptr<CImg<float>> grunge(new CImg<float>(TEX_DIR("Grunge_texture_by_darkrose42_stock.jpg")));

/*
shared_ptr<CImg<float>> blue(new CImg<float>(TEX_DIR("3783098647_fb208001b5_b.jpg")));
shared_ptr<CImg<float>> circle(new CImg<float>(TEX_DIR("bumpmap.jpg")));
shared_ptr<CImg<float>> frosted_glass(new CImg<float>(TEX_DIR("frosted_glass_bathroom_4060478.JPG")));
shared_ptr<CImg<float>> glass(new CImg<float>(TEX_DIR("glass_texture_P1012107.JPG")));
shared_ptr<CImg<float>> stone(new CImg<float>(TEX_DIR("hrt-stone-texture-3.jpg")));
shared_ptr<CImg<float>> frosted_glass2(new CImg<float>(TEX_DIR("frosted_glass_4060475.JPG")));
shared_ptr<CImg<float>> emboss(new CImg<float>(TEX_DIR("Normal Map.bmp")));
*/

#undef TEX_DIR

void scene::make_quad(const vector3& v0, const vector3& v1, const vector3& v2,
                      const material& mat, shared_ptr<CImg<float>> tex, shared_ptr<CImg<float>> bump)
{
    vector3 v3(v0 + (v2 - v1));

    shared_ptr<object> triangle1(new triangle(v0, v1, v2));
    triangle1->mat = mat;
    triangle1->texture = tex;
    triangle1->bump_map = bump;
    objs.push_back(triangle1);

    {
        shared_ptr<triangle> triangle1(new triangle(v2, v3, v0));
        triangle1->mat = mat;
        triangle1->texture = tex;
        triangle1->bump_map = bump;
        triangle1->inverted_xy = true;
        objs.push_back(triangle1);
    }
}

void scene_aux_1(scene& s)
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
    s.moving_objs.push_back(sphere2);

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

    shared_ptr<object> icosa1(new icosahedron(vector3(0.0, -1.0, -0.5), 1));
    icosa1->mat.diffuse = vector3(0.0, 1.0, 0.0);
    icosa1->mat.specular = vector3(1.0, 1.0, 1.0);
    icosa1->mat.transparency = 0.1;
    icosa1->mat.shininess = 100;
    icosa1->mat.reflection = 0.5;
    s.objs.push_back(icosa1);

    s.lights.push_back(light(vector3(0, 0, 5)));
    s.lights.back().dir = vector3(0, 0, -1);
    s.lights.push_back(light(vector3(1, 5, -2)));
    s.lights.back().dir = vector3(0, -1, 0);
    //s.lights.push_back(light(vector3(0, 0, 7)));
    //s.lights.push_back(light(vector3(-1, -2, 0)));
    //s.lights.push_back(light(vector3(0, 2, -1)));
}

void scene_aux_2(scene& s)
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

    shared_ptr<object> icosa1(new icosahedron(vector3(0.0, -1.0, -0.5), 1));
    icosa1->mat.diffuse = vector3(0.0, 1.0, 0.0);
    icosa1->mat.specular = vector3(1.0, 1.0, 1.0);
    icosa1->mat.transparency = 0.1;
    icosa1->mat.shininess = 100;
    icosa1->mat.reflection = 0.5;
    //s.objs.push_back(icosa1);

    s.lights.push_back(light(vector3(-1.5, 5, -1)));
    s.lights.back().dir = vector3(0, -1, 0);
    s.lights.push_back(light(vector3(0, 0.5, 5)));
    s.lights.back().dir = vector3(0, -0.5, -1);
    //s.lights.push_back(light(vector3(0, 0, 7)));
    //s.lights.push_back(light(vector3(-1, -2, 0)));
    //s.lights.push_back(light(vector3(0, 2, -1)));
}

void scene_aux_0(scene& s)
{
    material wall_m;
    wall_m.diffuse = vector3(0.5, 0.5, 0.5);
    wall_m.transparency = 1.0;
    wall_m.shininess = 0;
    wall_m.reflection = 0;

    material floor_m;
    floor_m.diffuse = vector3(1.0, 1.0, 1.0);
    floor_m.transparency = 1.0;
    floor_m.shininess = 0;
    floor_m.reflection = 0;

    material bump_m;
    bump_m.diffuse = vector3(0.4, 0.4, 0.4);
    bump_m.transparency = 1.0;
    bump_m.shininess = 1;
    bump_m.reflection = 0.0;

    const float floor_len = 4.0;
    const float wall_len = 6.0;

    const float floor_y = -1.5;
    const float wall_y = floor_y;
    s.make_quad(
                vector3(-floor_len, floor_y, -floor_len),
                vector3(-floor_len, floor_y, floor_len),
                vector3(floor_len, floor_y, floor_len),
                floor_m,
                checker
               );

    s.make_quad(
                vector3(-floor_len, floor_y + wall_len, floor_len),
                vector3(-floor_len, floor_y, floor_len),
                vector3(-floor_len, floor_y, -floor_len),
                bump_m,
                grunge,
                shared_ptr<CImg<float>>()
               );

    s.make_quad(
                vector3(-floor_len, floor_y + wall_len, -floor_len),
                vector3(-floor_len, floor_y, -floor_len),
                vector3(floor_len, floor_y, -floor_len),
                wall_m,
                grunge,
                shared_ptr<CImg<float>>()
               );

    s.make_quad(
                vector3(floor_len, floor_y + wall_len, -floor_len),
                vector3(floor_len, floor_y, -floor_len),
                vector3(floor_len, floor_y, floor_len),
                wall_m,
                grunge
               );

    shared_ptr<object> sphere1(new sphere(vector3(-0.0, 0.0, 0.0), 0.8));
    sphere1->mat.diffuse = vector3(1.0, 0.0, 0.0);
    sphere1->mat.specular = vector3(1.0, 1.0, 1.0);
    sphere1->mat.transparency = 0.4;
    sphere1->mat.shininess = 100;
    sphere1->mat.reflection = 0.1;
    s.objs.push_back(sphere1);

    {
        shared_ptr<object> sphere1(new sphere(vector3(-1.0, 1.5, -1.5), 0.8));
        sphere1->mat.diffuse = vector3(0.2, 0.2, 0.2);
        sphere1->mat.specular = vector3(1.0, 1.0, 1.0);
        sphere1->mat.transparency = 1.0;
        sphere1->mat.shininess = 0;
        sphere1->mat.reflection = 1.0;
        s.objs.push_back(sphere1);
    }

    {
        shared_ptr<object> sphere1(new sphere(vector3(1.2, 0.0, 1.0), 0.5));
        sphere1->mat.diffuse = vector3(0.2, 0.2, 0.2);
        sphere1->mat.specular = vector3(1.0, 1.0, 1.0);
        sphere1->mat.transparency = 1.0;
        sphere1->mat.shininess = 100;
        sphere1->mat.reflection = 1;
        s.objs.push_back(sphere1);
    }

    {
        shared_ptr<object> sphere1(new sphere(vector3(-1.0, -1.0, 0.3), 0.5));
        sphere1->mat.diffuse = vector3(1.0, 1.0, 1.0);
        sphere1->mat.specular = vector3(1.0, 1.0, 1.0);
        sphere1->mat.transparency = 1.0;
        sphere1->mat.shininess = 100;
        sphere1->mat.reflection = 0.1;
        sphere1->texture = water;
        s.objs.push_back(sphere1);
    }

    shared_ptr<cube> cube1(new cube(vector3(1.2, 1.0, 0.0), 1));
    cube1->mat.diffuse = vector3(0.2, 0.2, 0.2);
    cube1->mat.specular = vector3(1.0, 1.0, 1.0);
    cube1->mat.transparency = 1.0;
    cube1->mat.shininess = 1;
    cube1->mat.reflection = 0.0;
    cube1->bump_map = wall;
    cube1->make_polygon();
    s.objs.push_back(cube1);

    shared_ptr<icosahedron> icosa1(new icosahedron(vector3(-2.0, -0.0, -1.0), 1));
    icosa1->mat.diffuse = vector3(0.1, 0.5, 0.0);
    icosa1->mat.specular = vector3(1.0, 1.0, 1.0);
    icosa1->mat.transparency = 1.0;
    icosa1->mat.shininess = 100;
    icosa1->mat.reflection = 0.5;
    icosa1->make_polygon();
    s.objs.push_back(icosa1);

    s.lights.push_back(light(vector3(0, 0, 8)));
    s.lights.back().dir = vector3(0, 0, -1);

    s.lights.push_back(light(vector3(0, 9, 0)));
    s.lights.back().dir = vector3(0, -1, 0);
}

void scene_aux_3(scene& s)
{
    auto tri_lst = read_obj_file("cube.obj");
    for (auto i = tri_lst.begin(); i != tri_lst.end(); ++i)
    {
        s.objs.push_back(*i);
    }

    float phi = 0.0;
    int MAX = 36;
    float r = 4;
    for (int i = 0; i < MAX; ++i)
    {
        float theta = cml::constants<float>::two_pi() * i / (float)MAX;
        vector3 v;
        spherical_to_cartesian(r, theta, phi, 1, cml::latitude, v);
        v += vector3(0, 0, -2);

        shared_ptr<object> sphere1(new sphere(v, 0.2));
        sphere1->mat.diffuse = vector3(1.0, 0.0, 0.0);
        sphere1->mat.specular = vector3(1.0, 1.0, 1.0);
        sphere1->mat.transparency = 0.2;
        sphere1->mat.shininess = 100;
        sphere1->mat.reflection = 1.0;
        s.objs.push_back(sphere1);
    }

    s.lights.push_back(light(vector3(0, 2, 2)));
    s.lights.back().dir = vector3(0, -1, -1);
    s.lights.push_back(light(vector3(0, 0, 0)));
    s.lights.back().dir = vector3(0, 0, -1);
}

    scene::scene(int scene_num)
: g_amb_light(0.4, 0.4, 0.4)
{
    switch (scene_num)
    {
        case 0: scene_aux_0(*this); break;
        case 1: scene_aux_1(*this); break;
        case 2: scene_aux_2(*this); break;
        case 3: scene_aux_3(*this); break;
        default: assert(false);
    }
}
