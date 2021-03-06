#include <omp.h>
#include "ray_tracer.h"
using namespace std;

void ray_tracer::run(int img_width, int img_height, scene& s)
{
    image.resize(img_width, vector<vector3>(img_height));

    const float term = RES / (float)JITTER;

    float f_ = 1.2;
    float o_ = (vector3().zero() - c_o_lens).length(); // focus<->lens
    float i_ = o_*f_/(o_-f_); // lens<->image plane
    assert(i_ > 0);

    // DOF
    vector3 center_line = normalize(c_o_lens - C_O_IMG_PLANE);
    C_O_IMG_PLANE = c_o_lens + (-i_*center_line);
    //s.move_scene(vector3(0, 0, -1.5));
    // DOF

    /* initialize random seed: */
    srand(time(NULL));

    //cout << (float)rand() / (float)RAND_MAX << endl;
#pragma omp parallel for
    for (int i = 0; i < img_height; ++i)
    {
        if (i % 100 == 0)
        {
            cout << endl;
            stringstream ss;
            ss << "(" << i << ")";
            cout << ss.str();
        }
        else
        {
            cout << ".";
            cout.flush();
        }

        for (int j = 0; j < img_width; ++j)
        {
            vector3 intensity;
            intensity.zero();

            vector<int> codes;
            vector<int> lens_positions;
            for (int k = 0; k < JITTER*JITTER; ++k)
            {
                codes.push_back(k);
                lens_positions.push_back(k);
            }

            // Randomly shuffle jitter codes
            /*
            {
                int idx = 0;
                std::generate_n(back_inserter(codes), JITTER*JITTER, [&idx]() { return idx++; });
            }
            */
            std::random_shuffle(codes.begin(), codes.end());

            // Randomly shuffle lens position
            /*
            {
                int idx = 0;
                std::generate_n(back_inserter(lens_positions), JITTER*JITTER, [&idx]() { return idx++; });
            }
            */
            std::random_shuffle(lens_positions.begin(), lens_positions.end());

            // DOF
            vector3 C_O_PIXEL((-img_width/2 + j)*RES + RES/2.0, (img_height/2 - i)*RES - RES/2.0, 0);
            C_O_PIXEL += C_O_IMG_PLANE;
            
            vector3 focal_line = normalize(c_o_lens - C_O_PIXEL);
            float s_ = o_/dot(focal_line, center_line);
            vector3 focus = c_o_lens + s_*focal_line;
            // DOF

            for (int k = 0; k < JITTER*JITTER; ++k)
            {
                int off_x = k % JITTER;
                int off_y = k / JITTER;

                // ?????? ?????? ????????? ????????? ??? ???!
                float x_random = ((float)rand()/(float)RAND_MAX)*term;
                float y_random = ((float)rand()/(float)RAND_MAX)*term;

                const point3 ray_org((-img_width/2 + j)*RES + off_x*term + x_random, (img_height/2 - i)*RES - off_y*term - y_random, 0);

                int l1 = lens_positions[k];
                int l1_x = l1 % JITTER;
                int l1_y = l1 / JITTER;

                float xx = LENS_WIDTH / (float)JITTER;
                float yy = LENS_HEIGHT / (float)JITTER;

                point3 ray_lens_org((-LENS_WIDTH/2 + xx*l1_x + 0.5*xx)*RES, (LENS_HEIGHT/2 - yy*l1_y - 0.5*yy)*RES, 0);
                ray_lens_org += c_o_lens;

                ray ray1(ray_org);
                if (DOF_ON)
                    ray1 = ray(ray_lens_org, focus - ray_lens_org);
                ray1.code = codes[k];

                float interval = 1.0/(float)(JITTER*JITTER);
                float interval_rand = ((float)rand()/(float)RAND_MAX); // 0.0~1.0
                float time = (codes[k] + interval_rand)*interval;

                // calculate an intensity of light
                shared_ptr<ray_tree_node> root(new ray_tree_node(s, ray1));
                root->process(s, ray1, time);

                // traverse a tree
                intensity += traverse_tree(root);
            }
            intensity /= JITTER*JITTER;

            // calculate an intensity of light
            image[j][i] = intensity;
        }
    }
    cout << endl;
}
