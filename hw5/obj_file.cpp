#include "obj_file.h"
using namespace std;

#include "triangle.h"

string getline_and_ss(fstream& fs)
{
    string temp;
    while (fs.good() && temp.empty())
    {
        getline(fs, temp);

        bool flag = true;
        for (int i = 0; i < temp.size(); ++i)
        {
            if (!isspace(temp[i]))
            {
                flag = false;
                break;
            }
        } 
        if (flag) temp.clear();

        if (!temp.empty() && temp[0] == '#' && !(temp[0] == 'v' || temp[0] == 'f'))
        {
            temp.clear();
        }
    }
    return temp;
}

vector<shared_ptr<triangle>> read_obj_file(const string& fname)
{
    vector<vector3> vertices;
    fstream file(fname.c_str(), fstream::in);

    vector<shared_ptr<triangle>> result;
    float ratio = 1.0;
    float x_offset = -1.5;
    float y_offset = -1.5;
    while (file.good())
    {
        stringstream ss(getline_and_ss(file));

        string type;
        ss >> type;

        if (type == "v")
        {
            float x, y, z;
            ss >> x;
            ss >> y;
            ss >> z;
            vertices.push_back(vector3(x/ratio + x_offset, y/ratio + y_offset, z/ratio));
        }

        if (type == "f")
        {
			float v0, v1, v2;
            ss >> v0;
            ss >> v1;
            ss >> v2;
            result.push_back(shared_ptr<triangle>(new triangle(vertices[v0], vertices[v1], vertices[v2])));
        }
    }

    file.close();
    return result;
}
