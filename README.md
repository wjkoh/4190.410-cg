# Distributed Ray Tracer
It is a distributed ray tracer using stochastic sampling.

## Features
* Multithreaded using [OpenMP](http://openmp.org/)
* Ray tracing 3D geometric primitives
* Distributed ray tracing
  * Soft shadows, depth of field, and motion blur
* Recursive reflection/refraction.
* Texture mapping
* Bump mapping
* Phong illumination
* Spatial partitioning using BSP trees
* Importing geometry files such as OBJ

## Screenshots
![Ray tracing #0](https://bitbucket.org/wjkoh/4190.410-cg/raw/f92a0d8a0cb4/docs/images/ray_tracing_0.png)
![Ray tracing #1](https://bitbucket.org/wjkoh/4190.410-cg/raw/f92a0d8a0cb4/docs/images/ray_tracing_1.png)
![Ray tracing #2](https://bitbucket.org/wjkoh/4190.410-cg/raw/f92a0d8a0cb4/docs/images/ray_tracing_2.png)

## Dependencies
1. [CML](http://cmldev.net/) (included)
1. [CImg](http://cimg.sourceforge.net/) (included)

## Authors
* Woojong Koh  <wjngkoh@gmail.com>
