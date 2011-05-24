#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "common.h"
#include <map>
#include <string>

class material
{
    public:
        material()
            : diffuse(vector3(0.8, 0.8, 0.8)),
            specular(vector3(1.0, 1.0, 1.0)),
            ambient(vector3(0.2, 0.2, 0.2)),
            shininess(10),
            transparency(1.0)
    {
    }

        material(vector3 diffuse, vector3 specular, vector3 ambient, float shininess, float transparency)
            : diffuse(diffuse),
            specular(specular),
            ambient(ambient),
            shininess(shininess),
            transparency(transparency)
    {
    }

        vector3 diffuse;
        vector3 specular;
        vector3 ambient;
        float shininess;
        float transparency;
};

//std::map<std::string, material> material_map;
#endif
