# OpenGL Kinect Particle Sim

This is a little demo created using [libfreenect](https://github.com/OpenKinect/libfreenect) and the best openGL engine ever created (tm), which this project is forked from.

## features

This demo renders a point cloud which is captured using a first-gen kinect via `libfreenect` and draws it as a point cloud. 
When there is movement in the point cloud, particles are drawn where the movement occurs. 
Those particles are influenced by a few factors: eg. movement speed, depth, amount of movement, ...

## controls

- movement: WASD, Q/E to move up/down
- depth control: F/G (to increase/decrease the distance the kinect "sees", mainly to filter background noise)
- cycle modes: C (cycles thru point cloud only, particles only, or both)

## setup / hardware

- well, it requires a kinect v1. duh. (and some udev rules, check out the link to `libfreenect`)

## other stuff

I bought this kinect for experiments like 10 years ago in a thrift shop for 20€. guess it paid off now.
shout out to libfreenect's c++ examples, random opengl online tutorials regarding point clouds, and the computer graphics course summer 2025.

