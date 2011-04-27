/*
 * 2011-1 Computer Graphics Homework 3
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
using namespace std;

#include <cmath>
#include <cstdio>

#include "cml/cml.h"
typedef cml::vector3f vector3;
typedef cml::vector3d vector3d;
typedef cml::matrix44f_c matrix;

#define _D_ //cout << __LINE__ << endl;
const double PI = 4.0*atan(1.0);
const vector3 zero(0,0,0);
const float INITIAL_VIEW_DISTANCE = 5;
const float INITIAL_FOV = 60.0;

// TODO
// dot(cos) cross(sin) arctan2 사용하기
// seek - look at point로 지정하기
// Cylinder의 bounding box 수정하기
// 우상단->좌하단 드래그 시 문제

// Camera position
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
GLfloat light_position[] = {-3.0, 3.0, -5.0, 0.0};
GLfloat light1_position[] = {1.0, 1.0, 1.0, 1.0};

#include <set>
#include <list>
typedef list<vector3> set_t;
set_t g_vertices;


/*
affine_trans(vector<>, vector<vector3>)
{
}

affine_trans(vector<>, vector<quaternionf_p>)
{
    // multilinear
}
*/

vector<vector3> tangents;

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

typedef vector<vector3> cross_sect_t;
vector<cross_sect_t> draw_pt_list;
vector<cross_sect_t> surfaces;
vector<vector3> draw_pos_list;
vector<vector3> pos_spline;
vector<vector3> scale_factors;
vector<vector3> scale_spline;

vector<vector3> Catmull_Rom(const vector<vector3>& pt, vector<vector3>& tangents, bool closed = true, float resolution = 0.05)
{
    vector3 prev_tan(0, 0, 0);
    prev_tan = (pt[1] - pt.back()) / 2;

    tangents.clear();
    vector<vector3> result;
    for (int i = 0; i < pt.size(); ++i)
    {
        vector3 tan = (pt[(i + 2) % pt.size()] - pt[i]) / 2;

        if (closed || (i > 0 && i < pt.size() - 1))
        {
            vector3 b0 = pt[i];
            vector3 b1 = b0 + prev_tan;
            vector3 b3 = pt[(i + 1) % pt.size()];
            vector3 b2 = b3 - tan;

            for (float t = 0; t <= 1; t += resolution)
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


// Function prototypes
void set_cam_dist(float view_dist = view_distance);

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

void init(void)
{
    glEnable(GL_NORMALIZE); // 이 코드가 없으면 scale시 normal vector가 엉망이 됨.
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL); // 이 코드가 없으면 전부 흑백으로

    // 첫 번째 light
    GLfloat mat_specular[] = {0.7, 0.7, 0.7, 1.0};
    GLfloat mat_shininess[] = {50.0};
    GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat mat_diffuse[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat lmodel_ambient[] = {0.4, 0.4, 0.4, 1.0};

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    glEnable(GL_DEPTH_TEST);

    // 두 번째 light
    GLfloat light1_ambient[] = {0.2, 0.2, 0.2, 1.0};
    GLfloat light1_diffuse[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat light1_specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat spot_direction[] = {-1.0, 1.0, 0.0};

    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.5);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.5);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.2);

    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 180.0);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 2.0);
    //glEnable(GL_LIGHT1);
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
        //glRotatef(big_rot, 1.0, 0.0, 0.0);
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    if (first_run)
        glLoadIdentity();
    else
        set_cam_dist(view_distance);

    glPushMatrix(); // top

    // rotate lights and objects together (stationary)
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);

    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_TRIANGLE_STRIP);
    for (vector<cross_sect_t>::iterator it = surfaces.begin(); it != surfaces.end(); ++it)
    {
        if (it + 1 == surfaces.end())
            break;

        cross_sect_t& c = *it;
        cross_sect_t& c_n = *(it + 1);

        for (int i = 0; i < c.size(); ++i)
        {
            vector3 new_c = c[i];
            vector3 new_c_n = c_n[i];

            vector3d normal = rotate_vector(tangents[i], vector3().cardinal(2), -PI / 2);
            glNormal3f(normal[0], normal[1], normal[2]);
            glVertex3f(new_c[0], new_c[1], new_c[2]);
            glVertex3f(new_c_n[0], new_c_n[1], new_c_n[2]);
        }

        vector3 new_c = c[0];
        glVertex3f(new_c[0], new_c[1], new_c[2]);
    }
    glEnd( );

    // FLOOR
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1, 1, 1, 0.3);
    draw_floor();
    glDisable(GL_BLEND);

    /*
    // WALL-E
    draw_wall_e();

    // ROBOT ARM
    glPushMatrix();
    glTranslatef(3.5, 1.0, 0.0);
    glRotatef(90, 0.0, 0.0, 1.0);
    draw_robot_arm(true, 0, big_elbow, big_finger1);
    glPopMatrix();
    */

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
    gluLookAt(eye_pos[0], eye_pos[1], eye_pos[2], rot_origin[0], rot_origin[1], rot_origin[2], cam_up_vec[0], cam_up_vec[1], cam_up_vec[2]);
}

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, (GLfloat)w/(GLfloat)h, 1.0, 20.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
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

            // Zoom in/out
        case 'z':
        case 'Z':
            {
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
                    view_distance = max(view_distance, 0.0f);
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
        case '1':
        case '!':
            elbow = (elbow + (key == '1' ? 1 : -1) * 10) % 360;
            if (elbow < 0) elbow = 0;
            if (elbow > 120) elbow = 120;
            glutPostRedisplay();
            break;
        case '2':
        case '@':
            shoulder_roll = (shoulder_roll + (key == '2' ? 1 : -1) * 5) % 360;
            if (shoulder_roll < -90) shoulder_roll = -90;
            if (shoulder_roll > 90) shoulder_roll = 90;
            glutPostRedisplay();
            break;
        case '3':
        case '#':
            shoulder_yaw1 = (shoulder_yaw1 + (key == '3' ? 1 : -1) * 5) % 360;
            if (shoulder_yaw1 > 90) shoulder_yaw1 = 90;
            if (shoulder_yaw1 < 0) shoulder_yaw1 = 0;
            shoulder_yaw2 = shoulder_yaw1;
            glutPostRedisplay();
            break;
        case '4':
        case '$':
            finger1 = (finger1 + (key == '4' ? 1 : -1) * 10) % 360;
            if (finger1 < 0) finger1 = 0;
            if (finger1 > 60) finger1 = 60;
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
                start_vec = screen_to_sphere(x, y);
                cam_vec_start = cam_vec;
                cam_up_vec_start = cam_up_vec;
                break;
            case GLUT_RIGHT_BUTTON:
                rot_origin = screen_to_object(x, y); // 새 위치를 origin으로
                set_cam_dist(view_distance);
                glutPostRedisplay();
                break;
        }
    }
    else // GLUT_UP
    {
        vector3 x, y, z;
        orthonormal_basis(cam_vec, cam_up_vec, x, y, z);
        cam_vec = z;
        cam_up_vec = y;
    }
}

GLvoid motion(GLint x, GLint y)
{
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

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);

    init();

    draw_pos_list.push_back(vector3(0,0,0));
    draw_pos_list.push_back(vector3(0,4,-3));
    draw_pos_list.push_back(vector3(0,0,-6));
    draw_pos_list.push_back(vector3(0,4,-9));
    pos_spline = Catmull_Rom(draw_pos_list, tangents, false);

    scale_factors.push_back(vector3(1.0, 0, 0));
    scale_factors.push_back(vector3(0.5, 0, 0));
    scale_factors.push_back(vector3(0.7, 0, 0));
    scale_factors.push_back(vector3(0.2, 0, 0));
    scale_spline = Catmull_Rom(scale_factors, tangents);

    vector<vector3> pts;
    pts.push_back(vector3(0,1,0));
    pts.push_back(vector3(2,2,0));
    pts.push_back(vector3(1,0,0));
    pts.push_back(vector3(2,-2,0));
    pts.push_back(vector3(0,-1,0));
    pts.push_back(vector3(-2,-2,0));
    pts.push_back(vector3(-1,0,0));
    pts.push_back(vector3(-2,2,0));

    draw_pt_list.push_back(Catmull_Rom(pts, tangents));
    draw_pt_list.push_back(Catmull_Rom(pts, tangents));
    draw_pt_list.push_back(Catmull_Rom(pts, tangents));
    draw_pt_list.push_back(Catmull_Rom(pts, tangents));

    for (int i = 0; i < pos_spline.size(); ++i)
    {
        int j = i / (1 / 0.1);

        cross_sect_t c;
        for (int k = 0; k < draw_pt_list[j].size(); ++k)
            c.push_back(scale_spline[i][0] * draw_pt_list[j][k] + pos_spline[i]);
        surfaces.push_back(c);
    }

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    glutMainLoop();
    return 0;
}
