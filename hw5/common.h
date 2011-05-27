#ifndef _COMMON_H_
#define _COMMON_H_

#include <algorithm>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

// CML
#include "cml/cml.h"
typedef cml::vector3f vector3;
typedef cml::vector3d vector3d;
typedef cml::vector4f vector4;
typedef cml::quaternionf_p quaternionf_p;
typedef cml::quaterniond_p quaterniond_p;
typedef cml::matrix44f_c matrix;
typedef cml::matrixd_c matrixd;

//
typedef vector3 point3;

#include <limits>
const double FABS_EPS_F = std::numeric_limits<float>::epsilon();
const float EPS_F = 4*std::numeric_limits<float>::epsilon();
const double EPS_D = std::numeric_limits<double>::epsilon();

const vector3 X = vector3().cardinal(0);
const vector3 Y = vector3().cardinal(1);
const vector3 Z = vector3().cardinal(2);

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
const int IMG_WIDTH = 800;
const int IMG_HEIGHT = 600;
const float RES = 0.01;
const int MAX_DEPTH = 10;
const int JITTER = 3;
const float JITTER_ANGLE_DEG = 5; // 주의! int면 rad()에서 0이 나온다.
const float JITTER_ANGLE_DEG_R = 5; // 주의! int면 rad()에서 0이 나온다.

#define JITTER_REFR_ON 0
#define JITTER_REFL_ON 0

// for debugging
#include <cassert>

#if 1
#define _D_ cout << __FILE__ << " " << __LINE__ << endl;
#else
#define _D_
#endif

static bool DEBUG_MODE = false;

#endif // _COMMON_H_
