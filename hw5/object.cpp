#include <object.h>

vector3 object::calc_local_illu(const point3& pt, const light& light, const vector3& u) const
{
    vector3 n = get_normal(pt);
    vector3 l = normalize(light.get_pos() - pt);
    vector3 v = -u;
    vector3 r = 2*dot(n, l)*n - l;

    // l, v 모두 바깥 방향
    vector3 local_illu(0, 0, 0);
    if (dot(v, r) > EPS_F && dot(n, l) > EPS_F)
        local_illu += mat.specular*light.intensity*pow(dot(v, r), mat.shininess);
    if (dot(n, l) > EPS_F)
        local_illu += mat.diffuse*light.intensity*dot(n, l);

    return local_illu;
    //return mat.diffuse*light.intensity*dot(n, l) + mat.specular*light.intensity*pow(dot(r, u), mat.shininess);
} 
