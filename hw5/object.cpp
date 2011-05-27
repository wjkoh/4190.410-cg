#include <object.h>

vector3 object::calc_local_illu(const point3& pt, const vector3& n, const light& light, const vector3& u) const
{
    //const vector3 n = normal;
    const float light_dist = ((light.get_pos() - pt)*0.2).length_squared();
    const vector3 l = normalize(light.get_pos() - pt);
    const vector3 v = -u;
    const vector3 r = 2*dot(n, l)*n - l;

    // l, v 모두 바깥 방향
    vector3 local_illu(0, 0, 0);

    if (dot(n, l) > EPS_F)
        local_illu += mat.diffuse*light.intensity/light_dist*dot(n, l);

    if (dot(v, r) > EPS_F && dot(n, l) > EPS_F)
        local_illu += mat.specular*light.intensity/light_dist*pow(dot(v, r), mat.shininess);

    return local_illu;
} 
