#include "object.h"

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

#if JITTER_REFL_ON
    const float zone_size = cml::rad((float)JITTER_ANGLE_DEG) / (float)(JITTER*JITTER);
    refl = rotate_vector(refl, unit_cross(n, u), zone_size*(in_ray.code - JITTER*JITTER/2 + jitter));
#endif

#if JITTER_REFR_ON
    const float zone_size_r = cml::rad((float)JITTER_ANGLE_DEG_R) / (float)(JITTER*JITTER);
    refr = rotate_vector(refr, unit_cross(n, u), zone_size_r*(in_ray.code - JITTER*JITTER/2 + jitter));
#endif

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

vector3 object::calc_local_illu(const point3& pt, const vector3& n, const light& light, const vector3& u) const
{
    const float light_dist = ((light.get_pos(0.0) - pt)*0.2).length_squared();
    const vector3 l = normalize(light.get_pos(0.0) - pt);
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
