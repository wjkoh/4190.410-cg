#include "common.h"
using namespace std;

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif
#include "CImg.h"
using namespace cimg_library;

#include "ray_tracer.h"

// ToDo
// triangle -done
// illumination, shadow oversampling -done
// cube, icosahedron -done
// DOF, jitter -done
// area light, soft shadow -done
// motion blur -done
//
// light attenuation
// changing viewpoint
// quad
// octree or BSP
// moves global variable
// makes pos member var. private
// timer
// command-line arguments
// texture mapping
// bump mapping
// scene data file
// PLY import
// triangle transmission

void init(void)
{
    glDisable(GL_DEPTH_TEST);
}

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D (0, w, 0, h);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Ray Tracer
ray_tracer rt;

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);

    //glPointSize(10.0);
    glBegin(GL_POINTS);
    for (int i = 0; i < IMG_HEIGHT; ++i)
    {
        for (int j = 0; j < IMG_WIDTH; ++j)
        {
#if DOF_ON
            glColor3fv(rt.image[IMG_WIDTH - 1 - j][i].data());
#else
            // image[][] - upper left origin, OpenGL - lower left origin
            glColor3fv(rt.image[j][IMG_HEIGHT - 1 - i].data());
#endif
            glVertex2f(j + 0.5, i + 0.5);
        }
    }
    glEnd();

    glFlush();
    glutSwapBuffers();
}

int main(int argc, char** argv)
{
    // Ray Tracing
    scene s;

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
    s.objs.push_back(triangle1);
    }
    {
    shared_ptr<object> triangle1(new triangle(vector3(-10.0, floor_y, 10.0), vector3(10.0, floor_y, 10.0), vector3(10.0, floor_y + floor_y_up, -10.0)));
    triangle1->mat.diffuse = vector3(1.0, 1.0, 1.0);
    triangle1->mat.specular = vector3(0.2, 0.2, 0.2);
    triangle1->mat.transparency = 1.0;
    triangle1->mat.shininess = 20;
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
    s.objs.push_back(ico1);
    
    s.lights.push_back(light(vector3(0, 0, 5)));
    s.lights.back().dir = vector3(0, 0, -1);
    s.lights.push_back(light(vector3(1, 5, -2)));
    s.lights.back().dir = vector3(0, -1, 0);
    //s.lights.push_back(light(vector3(0, 0, 7)));
    //s.lights.push_back(light(vector3(-1, -2, 0)));
    //s.lights.push_back(light(vector3(0, 2, -1)));
   
    s.tree.build(s);

    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%Y-%m-%d_%H.%M.%S", timeinfo);
    cout << string(buffer) << endl;

    rt.run(IMG_WIDTH, IMG_HEIGHT, s);

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%Y-%m-%d_%H.%M.%S", timeinfo);
    cout << string(buffer) << endl;

    // Save as an image file
    CImg<unsigned char> img(IMG_WIDTH, IMG_HEIGHT, 1, 3, 0);
    cimg_forXY(img, x, y)
    {
        for (int i = 0; i < 3; ++i)
            img(x, y, i) = min(rt.image[IMG_WIDTH - 1 - x][IMG_HEIGHT -1 - y][i]*255.0, 255.0);
    }

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%Y-%m-%d_%H.%M.%S", timeinfo);
    string date_time(buffer);
    
#ifdef __APPLE__
    img.save_bmp(("img_" + date_time + ".bmp").c_str());
    //img.save_png(("img_" + date_time + ".png").c_str());
#else
    img.save_bmp(("img_" + date_time + ".bmp").c_str());
#endif

    // OpenGL & GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(IMG_WIDTH, IMG_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    //glutIdleFunc(idle);
    //glutKeyboardFunc(keyboard);
    //glutMouseFunc(mouse);
    //glutMotionFunc(motion);

    glutMainLoop();
	return 0;
}
