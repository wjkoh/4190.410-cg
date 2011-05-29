#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "common.h"
#include "material.h"

class ray
{
    public:
        ray(const point3& org, const vector3& dir = -Z, float refr_idx = REFR_AIR)
            : org(org), dir(normalize(dir)), refr_idx(refr_idx), code(0), flag(true)
        {
        }

        point3 operator*(const float s) const { return org + s*dir; }

        vector3 org;
        vector3 dir;

        float refr_idx;
        int code; // for subpixel ray index(code)
        bool flag; // valid/invalid
};

inline std::ostream& operator<<(std::ostream& os, const ray& r)
{
    os << "ray " << r.org << "/" << r.dir;
    return os;
}

struct intersect_info
{
    intersect_info(const ray& in_ray)
        : in_ray(in_ray),
        intersect(false),
        from_in_to_out(false),
        dist(std::numeric_limits<float>::max())
    {}

    /*
    void set_dist(float dist, const vector3& normal, bool from_in_to_out = false)
    {
        intersect = true;
        this->normal = normal;
        this->dist = dist;
        this->from_in_to_out = from_in_to_out;
    }
    */

    point3 get_pt() const { return (intersect ? in_ray*dist : point3()); }

    bool intersect;

    ray in_ray;
    float dist; // ray.org + dist*ray.dir
    bool from_in_to_out;

    vector3 normal;
};

class light;
class object
{
    public:
        object(const vector3& pos, const vector3& move_dir = vector3(0, 0, 0))
            : pos(pos), refr_idx(REFR_GLASS), move_dir(move_dir)//, time(0)
        {
        }

        virtual ~object() {}

        virtual bool is_sphere() const { return false; }
        virtual vector3 get_normal(const point3& pt, const float time) const = 0; // Phong shading

        point3 get_pos(float time) const { return pos + move_dir*time; }
        virtual void set_pos(const point3& new_pos) { pos = new_pos; }

        // get_hit_dist 함수를 상위 클래스 것으로 사용할 수 있게 하기 위해 분리
        virtual intersect_info check(const ray& in_ray, float time) { return check(in_ray, time, get_hit_dist(in_ray, time)); }
        virtual intersect_info check(const ray& in_ray, float time, std::pair<float, float> dist)
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
                result.normal = get_normal(result.get_pt(), time);
            return result;
        }

        // intersect with ray
        std::pair<ray, ray> calc_reflect_refract(const intersect_info& info)
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

        vector3 calc_local_illu(const point3& pt, const vector3& normal, const light& light, const vector3& v) const;

        material mat;
        vector3 move_dir;

    protected:
        virtual std::pair<float, float> get_hit_dist(const ray& ray, const float time) const = 0;

        float refr_idx;
        float time;
        point3 pos; // 절대 그냥 접근하지 말 것!! 반드시 get_pos(time) 사용.
};

class sphere : public object
{
    public:
        sphere(const vector3& pos) : object(pos), r(1.0) {}

        vector3 get_normal(const point3& pt, const float time) const { return (pt - get_pos(time)).normalize(); }
        virtual bool is_sphere() const { return true; }
        
    protected:
        std::pair<float, float> get_hit_dist(const ray& ray, const float time) const
        {
            const vector3 d_p = get_pos(time) - ray.org;
            float det = pow(r, 2) - (d_p - dot(ray.dir, d_p)*ray.dir).length_squared();
            if (det < -EPS_F) return std::make_pair(-1.0, -1.0);
            else if (fabs(det) <= FABS_EPS_F) det = 0.0;

            float x = dot(ray.dir, d_p) - sqrt(det);
            if (fabs(x) <= FABS_EPS_F) x = 0.0;

            float y = dot(ray.dir, d_p) + sqrt(det);
            if (fabs(y) <= FABS_EPS_F) x = 0.0;

            return std::make_pair(x, y); // + 는 outgoing
        }

    private:
        float r; // radius
};

class light : public object // point, directional, area
{
    public:
        light()
            : object(vector3(0, 3, -1)), intensity(1.0, 1.0, 1.0), side_len(1.5)
        {
            dir = vector3().zero() - get_pos(0.0);
        }
        light(const vector3& pos)
            : object(pos), intensity(1.0, 1.0, 1.0), side_len(1.5)
        {
            dir = vector3().zero() - get_pos(0.0);
        }

        vector3 get_normal(const point3& pt, const float time) const { return (pt - get_pos(time)).normalize(); } // TODO: area light는 triangle처럼 바꿔야 하지 않을까?

        point3 get_jittered_pos(int idx, const float time) const 
        {
            using namespace std;
            assert(idx >= 0 && idx < SHADOW_RAY*SHADOW_RAY);

            const point3 center = get_pos(time);
            const float half_len = side_len/2.0;
            const float space = side_len/(float)SHADOW_RAY;

            vector3 x_vec;
            if (fabs(dot(dir, Z)) < 1 - FABS_EPS_F)
                x_vec = unit_cross(dir, Z);
            else
                x_vec = unit_cross(dir, Y);
            vector3 y_vec = unit_cross(x_vec, dir);

            // left top
            const point3 lt = center - x_vec*half_len + y_vec*half_len;

            // lt --> x_pos
            // |
            // V
            // y_pos
            const int x_pos = idx % SHADOW_RAY;
            const int y_pos = idx / SHADOW_RAY;

            const point3 cof = lt + x_pos*space*x_vec - y_pos*space*y_vec;

            const float x_random = ((float)rand()/(float)RAND_MAX) - 0.5;
            const float y_random = ((float)rand()/(float)RAND_MAX) - 0.5;

            const point3 jittered = cof + space*x_random*x_vec + space*y_random*y_vec;
            return jittered;
        }

        vector3 intensity;
        vector3 dir;

    protected:
        std::pair<float, float> get_hit_dist(const ray& ray, const float time) const
        {
            float s = length(get_pos(time) - ray.org);
            if (s < -EPS_F) return std::make_pair(-1.0, -1.0);
            if (fabs(s) <= FABS_EPS_F) s = 0.0;
            return std::make_pair(s, s);
        }

    private:
        float side_len;
};

#endif //_OBJECT_H_
