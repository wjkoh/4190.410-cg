#define NOMINMAX

#ifndef _COMMON_H_
#define _COMMON_H_

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// CML
#include "cml/cml.h"
typedef cml::vector2f vector2;
typedef cml::vector3f vector3;
typedef cml::vector3d vector3d;
typedef cml::vector4f vector4;
typedef cml::quaternionf_p quaternionf_p;
typedef cml::quaterniond_p quaterniond_p;
typedef cml::matrix44f_c matrix;
typedef cml::matrixd_c matrixd;
typedef quaternionf_p plane_t; // ax+by+cz+d=0 -> (a, b, c, d)

// typedefs
typedef vector3 point3;

#include <limits>
const double FABS_EPS_F = std::numeric_limits<float>::epsilon();
const float EPS_F = 20*std::numeric_limits<float>::epsilon();
const double EPS_D = std::numeric_limits<double>::epsilon();

// Axes
const vector3 X = vector3().cardinal(0);
const vector3 Y = vector3().cardinal(1);
const vector3 Z = vector3().cardinal(2);

// Refractive indexes
const float REFR_AIR = 1.000277;
const float REFR_GLASS = 1.510;
const float REFR_WATER = 1.3330;

// n - normal, u - incidence, 그러니까 normal의 origin으로 향하는 벡터.
inline vector3 get_reflect(const vector3& n, const vector3& u)
{
    return (u - 2*dot(u, n)*n).normalize();
}

inline vector3 get_refract(const vector3& n, const vector3& u, const float refr_idx_in, const float refr_idx_refr)
{
    double n_i_r = refr_idx_in / refr_idx_refr; // n_i / n_r
    double cos_theta_i = dot(-u, n);
    double cos_theta_r = sqrt(1 - pow(n_i_r, 2)*(1 - pow(cos_theta_i, 2)));

    return (n_i_r*u - (cos_theta_r - n_i_r*cos_theta_i)*n).normalize();
}

inline vector3 operator*(const vector3& lhs, const vector3& rhs)
{
    return vector3(lhs[0]*rhs[0], lhs[1]*rhs[1], lhs[2]*rhs[2]);
}

const vector3 BACKGND_COLOR = vector3(0.0, 0.0, 0.0);
const vector3 c_o_lens(0, 0, 2.0);
static vector3 C_O_IMG_PLANE(0, 0, 3.0);

/*
const int IMG_WIDTH = 1024;
const int IMG_HEIGHT = 768;
const int LENS_WIDTH = 2; // 사실은 aperture size
const int LENS_HEIGHT = 2;
const float RES = 0.01;
const int MAX_DEPTH = 10;
const int JITTER = 3; // JITTER*JITTER 개의 subpixel ray
const int SHADOW_RAY = 2; // SHADOW_RAY*SHADOW_RAY 개의 shadow ray
const float JITTER_ANGLE_DEG = 5; // 주의! int면 rad()에서 0이 나온다.
const float JITTER_ANGLE_DEG_R = 5; // 주의! int면 rad()에서 0이 나온다.
*/

extern int MAX_DEPTH;
extern float RES;
extern int IMG_WIDTH;
extern int IMG_HEIGHT;
extern int LENS_WIDTH;
extern int LENS_HEIGHT;
extern int JITTER; // JITTER*JITTER 개의 subpixel ray
extern int SHADOW_RAY; // SHADOW_RAY*SHADOW_RAY 개의 shadow ray
extern float JITTER_ANGLE_DEG; // 주의! int면 rad()에서 0이 나온다.
extern float JITTER_ANGLE_DEG_R; // 주의! int면 rad()에서 0이 나온다.

// Function On/Off
extern bool JITTER_REFR_ON;
extern bool JITTER_REFL_ON;
extern bool DOF_ON;
extern bool BSP_ENABLED;

// for debugging
#include <cassert>

#if 1
#define _D_ std::cout << __FILE__ << " " << __LINE__ << std::endl;
#else
#define _D_
#endif

static bool DEBUG_MODE = false;

// BSP front or back
enum
{
    BSP_FRONT = 0,
    BSP_INTERSECT,
    BSP_OVERLAP,
    BSP_BACK,
};

#include "CImg.h"
using namespace cimg_library;

#endif // _COMMON_H_
