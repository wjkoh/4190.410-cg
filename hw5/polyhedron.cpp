#include "polyhedron.h"

template<>
polyhedron<20>::polyhedron(const vector3& pos, float r)
    : sphere(pos, r)
{
    /* r: any radius in which the polyhedron is inscribed */
    double Pi = 3.141592653589793238462643383279502884197;
    double phiaa  = 26.56505; /* phi needed for generation */

    double phia = Pi*phiaa/180.0; /* 2 sets of four points */
    double theb = Pi*36.0/180.0;  /* offset second set 36 degrees */
    double the72 = Pi*72.0/180;   /* step 72 degrees */

    vertices[0][0]=0.0;
    vertices[0][1]=0.0;
    vertices[0][2]=r;

    vertices[11][0]=0.0;
    vertices[11][1]=0.0;
    vertices[11][2]=-r;
    double the = 0.0;

    for(int i=1; i<6; i++)
    {
        vertices[i][0]=r*cos(the)*cos(phia);
        vertices[i][1]=r*sin(the)*cos(phia);
        vertices[i][2]=r*sin(phia);
        the = the+the72;
    }

    the=theb;
    for(int i=6; i<11; i++)
    {
        vertices[i][0]=r*cos(the)*cos(-phia);
        vertices[i][1]=r*sin(the)*cos(-phia);
        vertices[i][2]=r*sin(-phia);
        the = the+the72;
    }

    make_polygon();
}

template<>
polyhedron<6>::polyhedron(const vector3& pos, float r)
    : sphere(pos, r)
{
    double Pi = 3.141592653589793238462643383279502884197;
    double phiaa = 35.264391; /* the phi needed for generation */

    double phia = Pi*phiaa/180.0; /* 2 sets of four points */
    double phib = -phia;
    double the90 = Pi*90.0/180.0;
    double the = Pi/4.0; //0.0;

    for(int i=0; i<4; i++)
    {
        vertices[i][0]=r*cos(the)*cos(phia);
        vertices[i][1]=r*sin(the)*cos(phia);
        vertices[i][2]=r*sin(phia);
        the = the+the90;
    }

    the = Pi/4.0; //0.0;
    for(int i=4; i<8; i++)
    {
        vertices[i][0]=r*cos(the)*cos(phib);
        vertices[i][1]=r*sin(the)*cos(phib);
        vertices[i][2]=r*sin(phib);
        the = the+the90;
    }

    /* map vertices to 6 faces */
    make_polygon();
}
