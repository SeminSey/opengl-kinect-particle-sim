#version 460 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec4 in_col;

layout(location = 0) out vec4 out_col;

layout(location = 8) uniform mat4x4 camera_transform;
layout(location = 12) uniform mat4x4 camera_projection;

void main() {
    gl_Position = camera_projection * camera_transform * vec4(in_pos, 1.0);
    gl_PointSize = 2.0;
    out_col = in_col;
}
