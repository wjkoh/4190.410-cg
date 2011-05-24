#include "common.h"

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include <iostream>
#include <vector>
using namespace std;

#include "ray_tracer.h"

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
            glColor3fv(rt.image[j][IMG_HEIGHT - 1 - i].data()); // image[][] - upper left origin, OpenGL - lower left origin
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

    shared_ptr<object> sphere1(new sphere(vector3(0.0, 0, -1)));
    sphere1->mat.diffuse = vector3(0.7, 0.0, 0.0);
    sphere1->mat.specular = vector3(1.0, 1.0, 1.0);
    sphere1->mat.transparency = 1.0;
    sphere1->mat.shininess = 20;
    s.objs.push_back(sphere1);

    shared_ptr<object> sphere2(new sphere(vector3(-1.5, 0, -2.0)));
    sphere2->mat.diffuse = vector3(0.0, 0.0, 0.7);
    sphere2->mat.specular = vector3(1.0, 1.0, 1.0);
    sphere2->mat.transparency = 1.0;
    sphere2->mat.shininess = 20;
    s.objs.push_back(sphere2);
    
    s.lights.push_back(light());
    s.lights.push_back(light(vector3(2, 0, 0)));
    s.lights.push_back(light(vector3(-1, -2, 0)));
    s.lights.push_back(light(vector3(0, 2, -1)));

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
