/*
 * 2011-1 Computer Graphics Homework 1
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

#include <cmath>
#include <cstdio>

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

// camera rotation
int rot = 0; 
float look_x = -4.0;
float look_z = 4.0;
float angle = 0;
float rad = 5;

// to make light stationary
GLfloat light_position[] = {-3.0, 3.0, -5.0, 0.0};

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
    GLfloat light1_position[] = {1.0, 1.0, 1.0, 1.0};
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
    glEnable(GL_LIGHT1);
}

void idle(void)
{
    int milliseconds = glutGet(GLUT_ELAPSED_TIME);
    float sin_1 = sinf((float)milliseconds * 0.001f);
    //return; // 애니메이션 끄기 - a, b, c, d 키로 움직여보고 싶을 때

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

void drawFloor(void)
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
    glEnd();

    // front endcap
    glBegin(solid ? GL_TRIANGLES : GL_LINES);
    glVertex3f(-0.5, 0.0, 0.5);
    glVertex3f(0.0, 0.87, 0.5);
    glVertex3f(0.5, 0.0, 0.5);
    glEnd();

    // bottom
    glBegin(solid ? GL_QUADS : GL_LINES);
    glVertex3f(-0.5, 0.0, -0.5);
    glVertex3f(0.5, 0.0, -0.5);
    glVertex3f(0.5, 0.0, 0.5);
    glVertex3f(-0.5, 0.0, 0.5);
    glEnd();

    // back
    glBegin(solid ? GL_QUADS : GL_LINES);
    glVertex3f(-0.5, 0.0, -0.5);
    glVertex3f(0.0, 0.87, -0.5);
    glVertex3f(0.0, 0.87, 0.5);
    glVertex3f(-0.5, 0.0, 0.5);
    glEnd();

    // top
    glBegin(solid ? GL_QUADS : GL_LINES);
    glVertex3f(0.0, 0.87, -0.5);
    glVertex3f(0.5, 0.0, -0.5);
    glVertex3f(0.5, 0.0, 0.5);
    glVertex3f(0.0, 0.87, 0.5);
    glEnd();
}

void draw_cyl(GLUquadric* quad, float base, float top, float height, float slices, float stacks)
{
    gluCylinder(quad, base, top, height, slices, stacks);
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
        gluQuadricDrawStyle(quadric, GLU_FILL);
        gluQuadricOrientation(quadric, GLU_INSIDE);
        glColor3f(0, 100/255.0, 0);
        draw_cyl(quadric, radius, radius, 0.7f, 20, 20);
        glPopMatrix();

        glTranslatef(1.5 + 0.7/2, 0.0, 0.0);
        glRotatef(big_rot, 1.0, 0.0, 0.0);
        glColor3f(0/255.0, 0/255.0, 139/255.0);
        glPushMatrix();
        glScalef(3.0, 0.7, 0.7);
        glutSolidCube(1.0);
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
    glutSolidCube(1.0);
    glPopMatrix();

    glTranslatef(1.0, 0.0, 0.0);
    glRotatef((GLfloat)elbow, 0.0, 0.0, 1.0);
    glTranslatef(1.0, 0.0, 0.0);
    glPushMatrix();
    glScalef(2.0, 0.7, 0.7);
    glutSolidCube(1.0);
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
        glutSolidCube(1.0);
        glPopMatrix();

        glTranslatef(x_len/2, 0.0, 0.0); //전 모델의 중간부터 끝까지
        glRotatef((GLfloat)finger1, 0.0, 0.0, 1.0);
        glTranslatef((x_len+0.2)/2, 0.0, 0.0);
        glPushMatrix();
        glScalef(x_len+0.2, 0.2, 0.2);
        glutSolidCube(1.0);
        glPopMatrix();
        glPopMatrix();
    }

    glPushMatrix();
    glTranslatef(1+0.1, 0.0, 0.0);//전 모델의 중간부터 끝까지
    glRotatef((GLfloat)20, 0.0, 0.0, 1.0);
    glTranslatef(x_len/2, 0.3, 0.35 - 0.2/2);
    glPushMatrix();
    glScalef(x_len, 0.2, 0.2);
    glutSolidCube(1.0);
    glPopMatrix();

    glTranslatef(x_len/2, 0.0, 0.0);//전 모델의 중간부터 끝까지
    glRotatef((GLfloat)-finger1, 0.0, 0.0, 1.0);
    glTranslatef((x_len + 0.2)/2, 0.0, 0.0);
    glPushMatrix();
    glScalef(x_len + 0.2, 0.2, 0.2);
    glutSolidCube(1.0);
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
    glutSolidCube(1.0f);
    glPopMatrix();

    // body
    glColor3f(255/255.0, 140/255.0, 0/255.0);
    glutSolidCube(1.0f);

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

    // FLOOR
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1, 1, 1, 0.3);
    drawFloor();
    glDisable(GL_BLEND);

    glPushMatrix(); // top
    //glRotatef(rot, 0.0, 1.0, 0.0); // camera rotation

    // WALL-E
    draw_wall_e();

    // ROBOT ARM
    glPushMatrix();
    glTranslatef(3.5, 1.0, 0.0);
    glRotatef(90, 0.0, 0.0, 1.0);
    draw_robot_arm(true, 0, big_elbow, big_finger1);
    glPopMatrix();

    glPopMatrix(); // top
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);
    gluPerspective(60.0, (GLfloat)w/(GLfloat)h, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(-1.0, 1.2, 4.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}


void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        // Dolly in/out (일종의 zoom)
        case 'z':
        case 'Z':
            if (key == 'z')
                rad -= 0.1;
            else
                rad += 0.1;

            look_x = rad*cosf(angle);
            look_z = rad*sinf(angle);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            gluLookAt(look_x, 1.2, look_z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
            glLightfv(GL_LIGHT0, GL_POSITION, light_position);

            glutPostRedisplay();
            break;
        // Rotate camera
        case 'r':
        case 'R':
            if (key == 'r')
                angle += 0.10;
            else
                angle -= 0.10;

            look_x = rad*cosf(angle);
            look_z = rad*sinf(angle);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            gluLookAt(look_x, 1.2, look_z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
            glLightfv(GL_LIGHT0, GL_POSITION, light_position); // 여기서 다시 지정해주지 않으면 조명이 카메라를 따라 돈다.

            glutPostRedisplay();
            break;
        // 여러 관절 조종
        // (애니메이션 끄고 할 것)
        case 'a':
        case 'A':
            elbow = (elbow + (key == 'a' ? 1 : -1) * 10) % 360;
            if (elbow < 0) elbow = 0;
            if (elbow > 120) elbow = 120;
            glutPostRedisplay();
            break;
        case 'b':
        case 'B':
            shoulder_roll = (shoulder_roll + (key == 'b' ? 1 : -1) * 5) % 360;
            if (shoulder_roll < -90) shoulder_roll = -90;
            if (shoulder_roll > 90) shoulder_roll = 90;
            glutPostRedisplay();
            break;
        case 'c':
        case 'C':
            shoulder_yaw1 = (shoulder_yaw1 + (key == 'c' ? 1 : -1) * 5) % 360;
            if (shoulder_yaw1 > 90) shoulder_yaw1 = 90;
            if (shoulder_yaw1 < 0) shoulder_yaw1 = 0;
            shoulder_yaw2 = shoulder_yaw1;
            glutPostRedisplay();
            break;
        case 'd':
        case 'D':
            finger1 = (finger1 + (key == 'd' ? 1 : -1) * 10) % 360;
            if (finger1 < 0) finger1 = 0;
            if (finger1 > 60) finger1 = 60;
            glutPostRedisplay();
            break;
    }
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
    glutMainLoop();
    return 0;
}