#pragma once

struct PointVertex {
    float x;
    float y;
    float z;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct PointCloudRenderer {
    void init(size_t max_points) {
        _max_points = max_points;

        glCreateVertexArrays(1, &_vao);
        glCreateBuffers(1, &_vbo);
        glNamedBufferData(_vbo, static_cast<GLsizeiptr>(_max_points * sizeof(PointVertex)), nullptr, GL_DYNAMIC_DRAW);

        glVertexArrayVertexBuffer(_vao, 0, _vbo, 0, sizeof(PointVertex));

        glEnableVertexArrayAttrib(_vao, 0);
        glVertexArrayAttribFormat(_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(PointVertex, x));
        glVertexArrayAttribBinding(_vao, 0, 0);

        glEnableVertexArrayAttrib(_vao, 1);
        glVertexArrayAttribFormat(_vao, 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(PointVertex, r));
        glVertexArrayAttribBinding(_vao, 1, 0);
    }

    void destroy() {
        glDeleteBuffers(1, &_vbo);
        glDeleteVertexArrays(1, &_vao);
        _vbo = 0;
        _vao = 0;
        _max_points = 0;
    }

    void upload(const std::vector<PointVertex>& points) {
        _draw_count = std::min(points.size(), _max_points);
        if (_draw_count == 0) return;
        glNamedBufferSubData(_vbo, 0, static_cast<GLsizeiptr>(_draw_count * sizeof(PointVertex)), points.data());
    }

    void draw() {
        if (_draw_count == 0) return;
        glBindVertexArray(_vao);
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(_draw_count));
    }

    GLuint _vao = 0;
    GLuint _vbo = 0;
    size_t _max_points = 0;
    size_t _draw_count = 0;
};
