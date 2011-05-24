#include "common.h"

#include <iostream>
#include <vector>
using namespace std;

#include "ray_tracer.h"

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

ray_tracer rt;
int scr_width = 800;
int scr_height = 600;

void init(void)
{
    glDisable(GL_DEPTH_TEST);
}

void reshape(int w, int h)
{
    scr_width = w;
    scr_height = h;

    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D (0, w, 0, h);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);

    //glPointSize(10.0);
    glBegin(GL_POINTS);
    for (int i = 0; i < scr_height; ++i)
    {
        for (int j = 0; j < scr_width; ++j)
        {
            glColor3fv(rt.image[j][scr_height - 1 - i].data()); // image[][] - upper left origin, OpenGL - lower left origin
            glVertex2f(j, i);
        }
    }
    glEnd();

    glFlush();
    glutSwapBuffers();
}

int main(int argc, char** argv)
{
    scene s;
    s.objs.push_back(shared_ptr<object>(new sphere(vector3(0.0, 0, -1))));
    s.objs.front()->mat.diffuse = vector3(0.7, 0.0, 0.0);
    s.objs.push_back(shared_ptr<object>(new sphere(vector3(-0.5, 0, -3.0))));
    s.objs.back()->mat.diffuse = vector3(0.0, 0.0, 1.0);
    s.lights.push_back(light());
    s.lights.push_back(light(vector3(2, 0, 0)));
    s.lights.push_back(light(vector3(-1, -2, 0)));

    scr_width = 800;
    scr_height = 600;
    rt.run(800, 600, s);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(scr_width, scr_height);
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
