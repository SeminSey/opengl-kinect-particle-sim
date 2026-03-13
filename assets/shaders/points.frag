#version 460 core

layout(location = 0) in vec4 in_col;
layout(location = 0) out vec4 out_color;

void main() {
    out_color = in_col;
}
