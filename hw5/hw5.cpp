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
// DOF, jitter -done
// area light, soft shadow -done
// motion blur -done
// makes pos member var. private -done
// octree or BSP -done
//
// light attenuation
// changing viewpoint
// quad
// moves global variable
// timer
// command-line arguments
// texture mapping
// bump mapping
// scene data file
// PLY import
// triangle transmission

// Global variables
int IMG_WIDTH = 1024;
int IMG_HEIGHT = 768;
int LENS_WIDTH = 2; // 사실은 aperture size
int LENS_HEIGHT = 2;
float RES = 0.01;
int MAX_DEPTH = 10;
int JITTER = 1; // JITTER*JITTER 개의 subpixel ray
int SHADOW_RAY = 2; // SHADOW_RAY*SHADOW_RAY 개의 shadow ray
float JITTER_ANGLE_DEG = 5; // 주의! int면 rad()에서 0이 나온다.
float JITTER_ANGLE_DEG_R = 5; // 주의! int면 rad()에서 0이 나온다.

// Function On/Off
bool JITTER_REFR_ON = false;
bool JITTER_REFL_ON = false;
bool DOF_ON = true;
bool BSP_ENABLED = false;

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
            if (DOF_ON)
            {
                glColor3fv(rt.image[IMG_WIDTH - 1 - j][i].data());
            }
            else
            {
                // image[][] - upper left origin, OpenGL - lower left origin
                glColor3fv(rt.image[j][IMG_HEIGHT - 1 - i].data());
            }
            glVertex2f(j + 0.5, i + 0.5);
        }
    }
    glEnd();

    glFlush();
    glutSwapBuffers();
}

int main(int argc, char* argv[])
{
    int scene_num = 0;
    for (int i = 1; i < argc; ++i)
    {
        switch (i)
        {
            case 1:
                scene_num = atoi(argv[i]);
                break;
            case 2:
                JITTER = atoi(argv[i]);
                break;
            case 3:
                SHADOW_RAY = atoi(argv[i]);
                break;
            case 4:
                IMG_WIDTH = atoi(argv[i]);
                IMG_HEIGHT = IMG_WIDTH*3/4;
                break;
            case 5:
                BSP_ENABLED = argv[i];
                break;
        }
    }

    cout << "* scene #: " << scene_num << endl;
    cout << "* # of sampling: " << pow(JITTER, 2) << endl;
    cout << "* # of shadow rays: " << pow(SHADOW_RAY, 2) << endl;
    cout << "* image size: " << IMG_WIDTH << "x" << IMG_HEIGHT << endl;
    cout << "* BSP on/off: " << BSP_ENABLED << endl;
    cout << endl;
    cout << endl;

    // Ray Tracing
    scene s(scene_num);
   
    // Build a BSP tree
    s.tree.build(s);

    time_t start;
    struct tm* timeinfo;
    char buffer[80];

    time(&start);
    timeinfo = localtime(&start);
    strftime(buffer, 80, "%Y-%m-%d_%H.%M.%S", timeinfo);
    cout << string(buffer) << endl;

    rt.run(IMG_WIDTH, IMG_HEIGHT, s);

    time_t end;
    time(&end);
    timeinfo = localtime(&end);
    strftime(buffer, 80, "%Y-%m-%d_%H.%M.%S", timeinfo);
    cout << string(buffer) << endl;
    string date_time(buffer);

    double dif = difftime(end, start);
    cout << "* done." << endl;
    cout << "* " << (int)dif/60 << ":" << (int)dif%60 << " elapsed." << endl;

    // Save as an image file
    CImg<unsigned char> img(IMG_WIDTH, IMG_HEIGHT, 1, 3, 0);
    cimg_forXY(img, x, y)
    {
        for (int i = 0; i < 3; ++i)
            img(x, y, i) = min(rt.image[IMG_WIDTH - 1 - x][IMG_HEIGHT -1 - y][i]*255.0, 255.0);
    }

    const string output_dir("./output/");
#ifdef __APPLE__
    try
    {
        img.save_png((output_dir + "img_" + date_time + ".png").c_str());
    }
    catch (...)
    {
        img.save_bmp(("img_" + date_time + ".bmp").c_str());
    }
#else
    try
    {
        img.save_bmp((output_dir + "img_" + date_time + ".bmp").c_str());
    }
    catch (...)
    {
        img.save_bmp(("img_" + date_time + ".bmp").c_str());
    }
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
