#pragma once
#include "point_cloud_renderer.hpp"

struct Particle {
    glm::vec3 pos;
    glm::vec3 vel;
    float age;
    float life;
    float depth;
};

struct ParticleSimulator {
    static constexpr size_t max_particles = 50000;

    void update_from_depth(
        const std::vector<uint16_t>& depth_frame,
        size_t frame_width,
        size_t frame_height,
        float focal_length,
        float principal_point_x,
        float principal_point_y,
        uint16_t max_depth_mm,
        float delta_seconds,
        std::vector<PointVertex>& out_vertices
    ) {
        // on startup set init depth buffer
        if (_previous_depth_frame.empty()) {
            _previous_depth_frame.assign(depth_frame.size(), 0);
            _has_previous_depth_frame = false;
        }

        // spawn pass: compare current vs previous depth
        if (_has_previous_depth_frame) {
            size_t max_new_particles = std::min(_max_new_particles_per_frame, max_particles - _active_particles.size());

            size_t spawn_count = 0;
            for (size_t y = 0; y < frame_height && spawn_count < max_new_particles; y += _depth_sample_step) {
                size_t row = y * frame_width;
                for (size_t x = 0; x < frame_width && spawn_count < max_new_particles; x += _depth_sample_step) {
                    size_t i = row + x;
                    uint16_t z_now = depth_frame[i];
                    uint16_t z_prev = _previous_depth_frame[i];
                    // skip invalid
                    if (z_now == 0 || z_prev == 0 || z_now > max_depth_mm || z_prev > max_depth_mm) continue;

                    int dz = z_now - z_prev;
                    // "filter" noise
                    if (std::abs(dz) < _motion_depth_delta_threshold_mm) continue;

                    float z = z_prev;
                    _active_particles.push_back(Particle{
                        // spawn at previous position for trailing fx
                        .pos = glm::vec3{
                            (x - principal_point_x) * z / focal_length,
                            (principal_point_y - y) * z / focal_length,
                            z,
                        },
                        // velocity = y lift + modifier from depth
                        .vel = glm::vec3{ 0.0f, _initial_upward_velocity_mm_per_sec, dz * _depth_velocity_scale },
                        .age = 0.0f,
                        .life = _particle_lifetime_sec,
                        .depth = z,
                    });
                    spawn_count++;
                }
            }
        }
        _previous_depth_frame = depth_frame;
        _has_previous_depth_frame = true;
        out_vertices.clear();
        out_vertices.reserve(_active_particles.size());

        for (size_t i = 0; i < _active_particles.size();) {
            Particle& p = _active_particles[i];
            p.age += delta_seconds;
            if (p.age >= p.life) {
                _active_particles[i] = _active_particles.back();
                _active_particles.pop_back();
                continue;
            }

            float life_ratio = 1.0f - (p.age / p.life);
            float gravity_scale = 1.0f + 2.0f * (1.0f - life_ratio);
            p.vel.y += _gravity_y_mm_per_sec2 * gravity_scale * delta_seconds;
            float drag = 1.0f / (1.0f + _drag_per_sec * delta_seconds);
            p.vel *= drag;
            p.pos += p.vel * delta_seconds;

            float depth_ratio = glm::clamp(1.0f - (p.depth / max_depth_mm), 0.45f, 1.0f);
            uint8_t r = static_cast<uint8_t>(glm::clamp(90.0f * life_ratio * depth_ratio, 0.0f, 255.0f));
            uint8_t g = static_cast<uint8_t>(glm::clamp(190.0f * life_ratio * depth_ratio, 0.0f, 255.0f));
            uint8_t b = static_cast<uint8_t>(glm::clamp(255.0f * life_ratio * depth_ratio, 0.0f, 255.0f));
            uint8_t a = static_cast<uint8_t>(glm::clamp(255.0f * life_ratio, 0.0f, 255.0f));

            out_vertices.push_back(PointVertex{
                .x = p.pos.x,
                .y = p.pos.y,
                .z = p.pos.z,
                .r = r,
                .g = g,
                .b = b,
                .a = a,
            });
            i++;
        }
    }

    std::vector<Particle> _active_particles;
    std::vector<uint16_t> _previous_depth_frame;
    bool _has_previous_depth_frame = false;

    size_t _depth_sample_step = 4;
    size_t _max_new_particles_per_frame = 1400;
    int _motion_depth_delta_threshold_mm = 42;
    float _particle_lifetime_sec = 2.2f;
    float _initial_upward_velocity_mm_per_sec = 280.0f;
    float _depth_velocity_scale = 5.0f;
    float _gravity_y_mm_per_sec2 = -900.0f;
    float _drag_per_sec = 0.8f;
};
