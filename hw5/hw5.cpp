#include "common.h"
using namespace std;

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include "ray_tracer.h"

// ToDo
// triangle -done
// illumination, shadow oversampling -done
// cube, icosahedron -done
// DOF, jitter
// motion blur
// light attenuation
// area light, soft shadow
// viewpoint change
// quad
// octree or BSP

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
            glVertex2f(j, i);
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

    shared_ptr<object> sphere1(new sphere(vector3(1.0, 1.0, -0.0)));
    sphere1->mat.diffuse = vector3(1.0, 0.0, 0.0);
    sphere1->mat.specular = vector3(1.0, 1.0, 1.0);
    sphere1->mat.transparency = 0.2;
    sphere1->mat.shininess = 100;
    s.objs.push_back(sphere1);

    shared_ptr<object> sphere2(new sphere(vector3(-1.5, 0, -1.0)));
    sphere2->mat.diffuse = vector3(0.0, 0.0, 1.0);
    sphere2->mat.specular = vector3(1.0, 1.0, 1.0);
    sphere2->mat.transparency = 1.0;
    sphere2->mat.shininess = 20;
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
    
    s.lights.push_back(light(vector3(0, 0, -1)));
    s.lights.push_back(light(vector3(1, 5, -2)));
    s.lights.push_back(light(vector3(0, 0, 7)));
    //s.lights.push_back(light(vector3(-1, -2, 0)));
    //s.lights.push_back(light(vector3(0, 2, -1)));

    rt.run(IMG_WIDTH, IMG_HEIGHT, s);

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
