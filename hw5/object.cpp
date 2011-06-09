#include "object.h"
using namespace std;

intersect_info object::check(const ray& in_ray, float time, std::pair<float, float> dist) const
{
    intersect_info result(in_ray);

    if (fabs(dist.first) <= FABS_EPS_F) dist.first = 0.0;
    if (fabs(dist.second) <= FABS_EPS_F) dist.second = 0.0;

    if (dist.first < -EPS_F && dist.second < -EPS_F) // -, -
    {
        return result;
    }
    else if (dist.first < -EPS_F && dist.second > EPS_F) // -, +
    {
        result.intersect = true;
        result.from_in_to_out = true;
        result.dist = dist.second;
    }
    else if (dist.first > EPS_F) // +, +
    {
        result.intersect = true;
        result.dist = dist.first;
    }
    else if (fabs(dist.first) <= EPS_F && dist.second > EPS_F) // 0, +
    {
        dist.first = 0.0;
        result.intersect = true;
        result.from_in_to_out = true;
        result.dist = dist.second;
    }
    else  // 0, 0 && -, 0
    {
        point3 pt = in_ray*dist.second;

        bool is_dir_outward = dot(in_ray.dir, get_normal(pt, time)) >= -EPS_F;

        result.intersect = !is_dir_outward;
        result.dist = dist.first;
    }

    if (DEBUG_MODE)
        if (result.intersect && !is_sphere())
        {
            //std::cout << dist.first << " " << dist.second << "     " << result.dist << " " << EPS_F<< std::endl;
        }

    if (result.intersect)
    {
        result.obj = shared_from_this();
        result.normal = get_normal(result.get_pt(), time);
        //result.normal = get_normal(result.get_pt(), time, true); // displacement mapping
    }
    return result;
}

std::pair<ray, ray> object::calc_reflect_refract(const intersect_info& info) const
{
    assert(info.intersect);

    const ray& in_ray = info.in_ray;
    const bool from_in_to_out = info.from_in_to_out;
    const point3 pt = info.get_pt();
    const vector3& u = info.in_ray.dir;
    vector3 n = info.normal;

    float out_refr_idx = refr_idx;
    if (from_in_to_out)
    {
        out_refr_idx = REFR_AIR;
        n = -n;
    }

    /*
       if (!is_sphere())
       {
       out_refr_idx = REFR_AIR;
       n = -n;
       }
       */

    vector3 refl = get_reflect(n, u);
    vector3 refr = get_refract(n, u, in_ray.refr_idx, out_refr_idx);

    //jittering reflection
    const float jitter = ((float)rand() / (float)RAND_MAX) - 0.5;

    if (JITTER_REFL_ON)
    {
        const float zone_size = cml::rad((float)JITTER_ANGLE_DEG) / (float)(JITTER*JITTER);
        refl = rotate_vector(refl, unit_cross(n, u), zone_size*(in_ray.code - JITTER*JITTER/2 + jitter));
    }

    if (JITTER_REFR_ON)
    {
        const float zone_size_r = cml::rad((float)JITTER_ANGLE_DEG_R) / (float)(JITTER*JITTER);
        refr = rotate_vector(refr, unit_cross(n, u), zone_size_r*(in_ray.code - JITTER*JITTER/2 + jitter));
    }

    ray refl_ray(pt, refl, in_ray.refr_idx);
    refl_ray.code = in_ray.code;

    ray refr_ray(pt, refr, out_refr_idx);
    refr_ray.code = in_ray.code;

    if (from_in_to_out)// || (is_sphere() && dot(in_ray.dir, pt - get_pos(time)) >= 0))
    {
        refl_ray.flag = false;
    }

    if (/*fabs(dot(n, u)) <= EPS_F ||*/ mat.transparency >= 1.0 - FABS_EPS_F)
    {
        refr_ray.flag = false;
    }

    return std::make_pair(refl_ray, refr_ray);
}

vector3 object::calc_local_illu(const point3& pt, const vector3& n__, const light& light, const vector3& u) const
{
    const vector3 n = get_normal(pt, 0.0, true);
    const float light_dist = ((light.get_pos(0.0) - pt)*0.2).length_squared();
    const vector3 l = normalize(light.get_pos(0.0) - pt);
    const vector3 v = -u;
    const vector3 r = 2*dot(n, l)*n - l;

    // l, v 모두 바깥 방향
    vector3 local_illu(0, 0, 0);

    if (dot(n, l) > EPS_F)
        local_illu += get_texture(pt)*light.intensity/light_dist*dot(n, l);

    if (dot(v, r) > EPS_F && dot(n, l) > EPS_F)
        local_illu += mat.specular*light.intensity/light_dist*pow(dot(v, r), mat.shininess);

    return local_illu;
} 

vector3 object::get_texture(const point3& pt) const
{
    if (!texture) return mat.diffuse;

    const vector2&& tex_coord = pt_to_tex_coord(pt);
    vector3 color;
    for (int i = 0; i < 3; ++i)
        color[i] = texture->cubic_atXY(tex_coord[0], tex_coord[1], 0, i) / 255.0;
    //std::cout << std::endl << color << " " << u_v.first << " " << u_v.second << " " << texture->width() << std::endl;
    return color;
}

vector2 sphere::pt_to_tex_coord(const point3& pt, bool bump) const
{
    vector3 v = r*get_normal(pt, 0.0);

    float theta = 0; // 180 deg
    float phi = 0; // 360 deg
    float radius = 0;
    cml::cartesian_to_spherical(v, radius, theta, phi, 1, cml::colatitude);

    shared_ptr<CImg<float>> tex(texture);
    if (bump)
        tex = bump_map;

    assert(tex);

    float x = (theta/cml::constants<float>::two_pi() + 0.5)*tex->width();
    float y = (phi/cml::constants<float>::pi())*tex->height();
    if (x < 0 || x > tex->width() || y < 0 || y > tex->height())
    {
        stringstream ss;
        ss << cml::deg(phi) << " " << cml::deg(theta) << endl;
        ss << "XY " << x << " " << y << " " << tex->width() << tex->height() << endl;
        cout << ss.str();
    }
    return vector2(x, y);
}

vector3 sphere::get_normal(const point3& pt, const float time, bool bump) const
{
    vector3 n = (pt - get_pos(time)).normalize();

    if (bump && bump_map)
    {
        vector2 tex_coord = pt_to_tex_coord(pt, true);
        vector3 normal;
        for (int i = 0; i < 3; ++i)
        {
            normal[i] = bump_map->cubic_atXY(tex_coord[0], tex_coord[1], 0, i) / 255.0 * 2.0 - 1.0;
            if (i == 0) // invert Y
                normal[i] *= -1.0;
        }
        normal.normalize();

        matrix m;
        vector3 x = unit_cross(Y, n);
        matrix_rotation_align(m, n, x, true, cml::axis_order_zxy);
        m.inverse();

        vector3 r_n = transform_vector(m, normal);
        return r_n.normalize();
    }

    return n;
}
