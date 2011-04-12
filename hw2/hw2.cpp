/*
 * 2011-1 Computer Graphics Homework 2
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
void set_cam_dist(float view_distance);

// compatibility with original GLUT
#if !defined(GLUT_WHEEL_UP)
#  define GLUT_WHEEL_UP   3
#  define GLUT_WHEEL_DOWN 4
#endif

#include <cmath>
#include <cstdio>

#include <iostream>
using namespace std;

#define _D_ //cout << __LINE__ << endl;

const double PI = 4.0*atan(1.0);


#include "cml/cml.h"
typedef cml::vector3f vector3;
typedef cml::matrix44f_c matrix;

vector3 zero_(0,0,0);


// TODO
// dot(cos) cross(sin) arctan2 사용하기
// seek - look at point로 지정하기

// Virtual trackball
#include "vector.h"

// rotation matrices
float m_prev[16];
float m_curr[16];
vector3 y_unit_vec(0,0,0);
float max_rad = 0;

// rotation origin
vector3d rot_origin(0,0,0);

// mouse drag start
vector3f start_vec; // on a sphere
////

// Translation
vector3f up_vec;
vector3f right_vec;
float u_trans = 0.0;
float r_trans = 0.0;

// Zoom in/out
float fov = 60.0;

// Dolly in/out
// Camera position
vector3f cam_vec(0.0, 0.0, 1); // origin to camera
float view_distance = 5;

// Seek
// look at position (origin)
vector3d look_at_pos;

vector3 sa_vec(0, 0, 0);
float sa_mag = 0;


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
//set<vector3f> g_vertices;
//typedef set<vector3> set_t;
#include <list>
typedef list<vector3> set_t;
set_t g_vertices;

typedef pair<vector3f, float> sphere_t;
set<sphere_t> g_spheres;

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
    if (gluUnProject(winX, winY, winZ, modelview, projection, viewport, &result.x, &result.y, &result.z) == GL_FALSE)
    {
        result.x = 0;
        result.y = 0;
        result.z = 0;
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

void call_vertex3f(const vector3f& vec)
{
    glVertex3f(vec.x, vec.y, vec.z);
}

void call_vertex3f(const vector3& vec)
{
    glVertex3f(vec[0], vec[1], vec[2]);
}

template<class T, int DIM> // x, y, z, w
wjk::vector<T, DIM - 1> MultMatVec(T m[DIM*DIM], wjk::vector<T, DIM - 1> v)
{
    wjk::vector<T, DIM - 1> result;
    for (int i = 0; i < DIM; ++i)
    {
        float mul_sum = 0;
        for (int j = 0; j < DIM - 1; ++j)
        {
            if (j == DIM)
            {
                mul_sum += m[i + 4*j] * 1; // w의 default 값은 1
            }
            else
            {
                mul_sum += m[i + 4*j] * v[j];
            }
        }
        result[i] = mul_sum;
    }
    return result;
}

void new_vertex3f(float x, float y, float z)
{
    GLfloat m[16];
    glGetFloatv (GL_MODELVIEW_MATRIX, m);

    vector3 temp(x, y, z);
    call_vertex3f(temp);

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
    /*
    if (temp[2] < 0)
        cout << temp << endl;
        */

    g_vertices.push_back(temp);

    glPushMatrix();
    glLoadIdentity();

    /*
    glBegin(GL_POINTS);
    glVertex3f(temp[0], temp[1], temp[2]);
    glEnd( );
    */
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

vector3 sum(0, 0, 0);
vector3 new_origin(0, 0, 0);
vector3 normal(-0.377993, 1.45917, -1.37781); // origin to camera

float rot = 0;

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix(); // top

    //// display normal vector
    glPushMatrix();
    //glLoadIdentity();
    normal = normal.normalize();
    glTranslatef(normal[0], normal[1], normal[2]);
    //glutSolidCube(0.3);
    glTranslatef(-normal[0], -normal[1], -normal[2]);

    //glTranslatef(sum[0], sum[1], sum[2]);
    //glutSolidCube(1.3);
    //glTranslatef(-sum[0], -sum[1], -sum[2]);

    /*
    glBegin(GL_POINTS);
    glVertex3fv(normal.data());
    glEnd( );
    */
    glPopMatrix();
    ////
    
    //cout << "MAX RAD" << max_rad << endl;
    glPushMatrix();

    //glutSolidSphere(max_rad, 20, 20);
    glTranslatef(-sum[0], -sum[1], 0);
    glPopMatrix();

    // Trackball rotation
    glTranslatef(-r_trans, -u_trans, 0);

    glMultMatrixf(m_curr); // curr next
    glMultMatrixf(m_prev); // prev first

    // Translation
    //vector3f horiz_move = right_vec * r_trans * -1;
    //vector3f vert_move = up_vec * u_trans * -1;

    //glTranslatef(horiz_move.x, horiz_move.y, horiz_move.z);
    //glTranslatef(vert_move.x, vert_move.y, vert_move.z);

    g_vertices.clear();

    // rotate lights and objects together (stationary)
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);

    // FLOOR
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1, 1, 1, 0.3);
    //draw_floor();
    glDisable(GL_BLEND);

    // WALL-E
    draw_wall_e();

    // ROBOT ARM
    glPushMatrix();
    glTranslatef(3.5, 1.0, 0.0);
    glRotatef(90, 0.0, 0.0, 1.0);
    draw_robot_arm(true, 0, big_elbow, big_finger1);
    glPopMatrix();

    /*
    new_vertex3f(0, 1, 0);
    y_unit_vec = g_vertices.back();
    cout << "y_vec " << y_unit_vec << endl;
    */

    for (set_t::iterator it = g_vertices.begin(); it != g_vertices.end(); ++it)
    {
        sum = sum + *it;
    }

    sum[0] = sum[0] / g_vertices.size();
    sum[1] = sum[1] / g_vertices.size();
    sum[2] = sum[2] / g_vertices.size();

    vector3 max(0, 0, 0);
    vector3 max2(0, 0, 0);
    int i = 0;
    for (set_t::iterator it = g_vertices.begin(); it != g_vertices.end(); ++it)
    {
        vector3 dist = *it - sum;
        if (dist.length() >= max.length())
        {
            max2 = max;
            max = dist;
        }
    }

    max_rad = max.length();

    if (max != max2 && max != zero_ && max2 != zero_)
    {
        vector3 v = cml::cross(max, max2);
        normal = v;
    }
    else
        normal = zero_;

    ////
    glPopMatrix(); // top
    glutSwapBuffers();
}

void set_cam_dist(float view_distance)
{
    float new_mag = view_distance / sqrt(cam_vec.sq_sum());
    vector3f res = cam_vec * new_mag;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(res.x, res.y, res.z, look_at_pos.x, look_at_pos.y, look_at_pos.z, 0.0, 1.0, 0.0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    //glutPostRedisplay();
}

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);
    gluPerspective(fov, (GLfloat)w/(GLfloat)h, 1.0, 20.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glGetFloatv (GL_MODELVIEW_MATRIX, m_prev); // set identity
    glGetFloatv (GL_MODELVIEW_MATRIX, m_curr); // set identity

    set_cam_dist(view_distance);

    // cross product
    //right_vec = vector3f(0, 1, 0) / cam_vec;
    right_vec = vector3f(1, 0, 0);
    right_vec.normalize();

    //up_vec = cam_vec / right_vec;
    up_vec = vector3f(0, 1, 0);
    up_vec.normalize();
}

#define PI 3.14159265389

double calculateAngle(double size, double distance)
{
    double radtheta, degtheta; 

    radtheta = 2.0 * atan2 (size/2.0, distance);
    degtheta = (180.0 * radtheta) / PI;
    return (degtheta);
}
  
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        // Translate
        case ';':
            r_trans -= 0.1;
            glutPostRedisplay();
            break;
        case '\'':
            r_trans += 0.1;
            glutPostRedisplay();
            break;
        case '[':
            u_trans += 0.1;
            glutPostRedisplay();
            break;
        case '/':
            u_trans -= 0.1;
            glutPostRedisplay();
            break;

            // Reset
        case 'r':
            {
                // reset variables
                fov = 60.0;
                view_distance = 5;
                r_trans = 0;
                u_trans = 0;

                GLint viewport[4];
                glGetIntegerv(GL_VIEWPORT, viewport);
                GLint width = viewport[2];
                GLint height = viewport[3];

                reshape(width, height);
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
                //cout << "SUM  " << sum << endl;
                r_trans = 0;
                u_trans = 0;
                //fov = 60;

                new_origin[0] = sum[0];
                new_origin[1] = sum[1];
                new_origin[2] = sum[2];

                look_at_pos[0] = sum[0];
                look_at_pos[1] = sum[1];
                look_at_pos[2] = sum[2];

				/*
                rot_origin[0] = 0;
                rot_origin[1] = 0;
                rot_origin[2] = 0;
                */
                //set_cam_dist(view_distance);

                vector3 start_vec(normal[0], normal[1], normal[2]);
                vector3 end_vec;
                end_vec[0] = cam_vec[0];
                end_vec[1] = cam_vec[1];
                end_vec[2] = cam_vec[2];

                start_vec.normalize();
                end_vec.normalize();

                //cout << "START " << start_vec << endl;
                //cout << "END " << end_vec << endl;

                float cos_theta = dot(start_vec, end_vec);
                float sin_theta = cross(start_vec, end_vec).length();
                //cout << "cos_theta " << cos_theta << endl;

                float theta = acos(cos_theta);
                //cout << "theta1 " << theta << endl;
                theta = atan2(sin_theta, cos_theta);
                //cout << "theta2 " << theta << endl;

                //theta = atan2(end_vec[1],end_vec[0]) - atan2(start_vec[1], start_vec[0]);
                //cout << "theta3 " << theta << endl;

                //cross product
                vector3 axis = cross(start_vec, end_vec);

                glMatrixMode(GL_MODELVIEW);

                // Save m_curr
                glPushMatrix();
                glLoadMatrixf(m_curr); // curr next
                glMultMatrixf(m_prev); // prev first
                glGetFloatv(GL_MODELVIEW_MATRIX, m_prev);
                glPopMatrix();

                glPushMatrix();
                glMatrixMode(GL_MODELVIEW);
                //glLoadMatrixf(m_curr); // curr next
                glLoadIdentity();

                //glTranslatef(-sum[0], -sum[1], 0);
                glRotatef(theta * 180 / PI, axis[0], axis[1], axis[2]);

                glGetFloatv(GL_MODELVIEW_MATRIX, m_curr);
                glPopMatrix();

                view_distance = max_rad + 1; // zNear
                /*
                vector3f res = cam_vec * view_distance;
                cout << "CAM " << cam_vec << endl;
                vector3 res_;
                res_[0] = res[0];
                res_[1] = res[1];
                res_[2] = res[2];

                float dist = (sum - res_).length();
                cout << "DIST " << dist << endl;
                */

                double angle = calculateAngle(2*max_rad, view_distance);
                fov = angle*1.15;

                //cout << "ANGLE " << angle << endl;
                GLint viewport[4];
                glGetIntegerv(GL_VIEWPORT, viewport);
                GLint width = viewport[2];
                GLint height = viewport[3];

                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                gluPerspective(fov, (GLfloat)width/(GLfloat)height, 1, 20.0);

                glMatrixMode(GL_MODELVIEW);
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

vector3f screen_to_sphere(float x, float y)
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
    return vector3f(x, y, z);
}

GLvoid mouse(GLint button, GLint state, GLint x, GLint y)
{
    /*
       GLint viewport[4];
       glGetIntegerv(GL_VIEWPORT, viewport);
       GLint width = viewport[2];
       GLint height = viewport[3];

       switch (button)
       {
       case GLUT_WHEEL_UP:
       {
    // Zoom in
    fov -= 1.0;
    fov = max(fov, 0.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, (GLfloat)w/(GLfloat)h, 1, 20.0);
    glMatrixMode(GL_MODELVIEW);
    glutPostRedisplay();
    break;
    }
    case GLUT_WHEEL_DOWN:
    {
    // Zoom out
    fov += 1.0;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, (GLfloat)w/(GLfloat)h, 1, 20.0);
    glMatrixMode(GL_MODELVIEW);
    glutPostRedisplay();
    break;
    }
    }
    */

    if (state == GLUT_DOWN)
    {
        switch (button)
        {
            case GLUT_LEFT_BUTTON:
                start_vec = screen_to_sphere(x, y);
                break;
            case GLUT_RIGHT_BUTTON:
                //look_at_pos = screen_to_object(x, y); // 새 위치를 origin으로
                rot_origin = screen_to_object(x, y); // 새 위치를 origin으로
                set_cam_dist(view_distance);
                glutPostRedisplay();
                break;
        }
    }
    else // GLUT_UP
    {
        // m_curr * m_prev -> m_prev
        // m_curr <- I
        glPushMatrix();
        glLoadMatrixf(m_curr); // curr next
        glMultMatrixf(m_prev); // prev first
        glGetFloatv(GL_MODELVIEW_MATRIX, m_prev);
        glLoadIdentity();
        glGetFloatv(GL_MODELVIEW_MATRIX, m_curr);
        glPopMatrix();
    }
}

GLvoid motion(GLint x, GLint y)
{
    vector3f end_vec = screen_to_sphere(x, y);

    //inner product
    float cos_theta = end_vec * start_vec;
    float theta = acos(cos_theta);

    //cross product
    vector3f axis = start_vec / end_vec;

    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //glTranslatef(look_at_pos.x, look_at_pos.y, look_at_pos.z);
    glTranslatef(rot_origin.x, rot_origin.y, rot_origin.z);
    glRotatef(theta * 180 / PI, axis.x, axis.y, axis.z);
    //glTranslatef(-look_at_pos.x, -look_at_pos.y, -look_at_pos.z);
    glTranslatef(-rot_origin.x, -rot_origin.y, -rot_origin.z);

    glGetFloatv(GL_MODELVIEW_MATRIX, m_curr);
    glPopMatrix();
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

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    glutMainLoop();
    return 0;
}