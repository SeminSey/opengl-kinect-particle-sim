#pragma once

struct Camera {
    // translate relative to camera direction
    void translate(float x, float y, float z) {
        _position += glm::quat(_rotation) * glm::vec3(x, y, z);
    }

    void bind() {
        // projection matrix
        glm::mat4x4 projection_matrix = glm::perspectiveFov(_fov, _width, _height, _near_plane, _far_plane);

        // transformation matrix (inverse of normal transform, since it is the camera)
        glm::mat4x4 transform_matrix(1.0f);
        transform_matrix = glm::rotate(transform_matrix, - _rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        transform_matrix = glm::rotate(transform_matrix, - _rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        transform_matrix = glm::translate(transform_matrix, - _position);
        // upload to GPU
        glUniformMatrix4fv(8, 1, false, glm::value_ptr(transform_matrix));
        glUniformMatrix4fv(12, 1, false, glm::value_ptr(projection_matrix));
    }

    glm::vec3 _position{ 0, 0, 0 };
    glm::vec3 _rotation{ 0, 0, 0 }; // euler rotation
    float _near_plane = 0.1f;  // nearest distance the camera can render
    float _far_plane = 100.0f; // furthest distance the camera can render
    float _fov = 70.0f; // field of view
    float _width = 1280.0f; // should be equal to window width
    float _height = 720.0f; // should be equal to window height
};
