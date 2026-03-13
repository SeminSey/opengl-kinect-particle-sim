#pragma once
#include "time.hpp"
#include "input.hpp"
#include "window.hpp"
#include "camera.hpp"
#include "pipeline.hpp"
#include "kinect_device.hpp"
#include "point_cloud_renderer.hpp"
#include "particle_simulator.hpp"

struct Engine {
    Engine() {
        _time.init();

        // create render components
        _window.init(1280, 720, 4);
        _pipeline.init("points.vert", "points.frag");
        _points.init(_max_render_points);
        _kinect.init(0);

        // camera setup for depth space in millimeters
        _camera._position = { 0.0f, 0.0f, 0.0f };
        _camera._rotation.y = glm::radians(180.0f);
        _camera._near_plane = 10.0f;
        _camera._far_plane = 12000.0f;
        _camera._fov = glm::radians(50.0f);
        _camera._width = _window_width;
        _camera._height = _window_height;

        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }
    ~Engine() {
        // destroy in reversed init() order
        _kinect.destroy();
        _points.destroy();
        _pipeline.destroy();
        _window.destroy();
    }

    void handle_inputs() {
        // move via WASDQE
        float speed = _move_speed_mm_per_sec * _time._delta;
        if (Keys::keyPressed('w')) _camera.translate(0, 0, -speed);
        if (Keys::keyPressed('a')) _camera.translate(-speed, 0, 0);
        if (Keys::keyPressed('s')) _camera.translate(0, 0, +speed);
        if (Keys::keyPressed('d')) _camera.translate(+speed, 0, 0);
        if (Keys::keyPressed('q')) _camera.translate(0, -speed, 0);
        if (Keys::keyPressed('e')) _camera.translate(0, +speed, 0);

        // let go of mouse capture when we press ESCAPE
        if (Mouse::captured() && Keys::keyDown(SDLK_ESCAPE)) {
            Input::register_capture(false);
            SDL_SetWindowRelativeMouseMode(_window._window_p, Mouse::captured());
        }
        // grab mouse capture when we click into the window
        if (!Mouse::captured() && Mouse::pressed(Mouse::ids::left)) {
            Input::register_capture(true);
            SDL_SetWindowRelativeMouseMode(_window._window_p, Mouse::captured());
        }
        // camera rotation
        if (Mouse::captured()) {
            float mouse_sensitivity = 0.003f;
            _camera._rotation.x -= mouse_sensitivity * Mouse::delta().second;
            _camera._rotation.y -= mouse_sensitivity * Mouse::delta().first;
        }

        // depth control via FG
        if (Keys::keyDown('f')) _max_depth_mm = std::max<uint16_t>(500, _max_depth_mm - 100);
        if (Keys::keyDown('g')) _max_depth_mm = std::min<uint16_t>(10000, _max_depth_mm + 100);
        // c: cycle point cloud -> particles -> both
        if (Keys::keyDown('c')) {
            switch (_render_mode) {
                case render_mode_point_cloud: _render_mode = render_mode_particles; break;
                case render_mode_particles: _render_mode = render_mode_both; break;
                case render_mode_both:
                default: _render_mode = render_mode_point_cloud; break;
            }
        }
    }
    auto handle_sdl_event(SDL_Event& event) -> SDL_AppResult {
        switch (event.type) {
            case SDL_EventType::SDL_EVENT_QUIT: return SDL_AppResult::SDL_APP_SUCCESS;
            case SDL_EventType::SDL_EVENT_WINDOW_RESIZED:
            case SDL_EventType::SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                _window_width = std::max(1, event.window.data1);
                _window_height = std::max(1, event.window.data2);
                _camera._width = _window_width;
                _camera._height = _window_height;
                break;
            default: break;
        }
        Input::register_event(event);
        return SDL_AppResult::SDL_APP_CONTINUE;
    }
    auto handle_sdl_frame() -> SDL_AppResult {
        _time.update();
        handle_inputs();

        _kinect.fetch(_rgb_frame, _depth_frame);
        rebuild_point_cloud();

        _pipeline.bind();
        glViewport(0, 0, _window_width, _window_height);
        glClearColor(0.01f, 0.01f, 0.01f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        _camera.bind();
        _points.draw();

        // present drawn image to screen
        SDL_GL_SwapWindow(_window._window_p);
        // clear single-frame inputs
        Input::flush();
        return SDL_AppResult::SDL_APP_CONTINUE;
    }

    void rebuild_point_cloud() {
        _particle_sim.update_from_depth(
            _depth_frame,
            _frame_width,
            _frame_height,
            _focal_length,
            _principal_point_x,
            _principal_point_y,
            _max_depth_mm,
            _time._delta,
            _particle_vertices
        );

        bool draw_point_cloud = (_render_mode & render_mode_point_cloud) != 0;
        bool draw_particles = (_render_mode & render_mode_particles) != 0;

        _point_vertices.clear();
        // particles only: skip depth
        if (!draw_point_cloud && draw_particles) {
            _point_vertices = _particle_vertices;
            _points.upload(_point_vertices);
            return;
        }

        // reserve depth points (+ particles in both mode)
        _point_vertices.reserve(_frame_pixel_count + (draw_particles ? _particle_vertices.size() : 0));
        for (size_t pixel_index = 0; pixel_index < _frame_pixel_count; pixel_index++) {
            uint16_t depth_mm = _depth_frame[pixel_index];
            if (depth_mm == 0 || depth_mm > _max_depth_mm) continue;
            float z = depth_mm;

            // flat index -> image x/y
            float pixel_x = pixel_index % _frame_width;
            float pixel_y = pixel_index / _frame_width;

            // depth xyz -> world xyz in mm
            // acts like a pinhole cam
            float world_x = (pixel_x - _principal_point_x) * z / _focal_length;
            float world_y = (_principal_point_y - pixel_y) * z / _focal_length;

            // keep kinect rgb, add render point
            _point_vertices.push_back(PointVertex{
                .x = world_x,
                .y = world_y,
                .z = z,
                .r = _rgb_frame[3 * pixel_index + 0],
                .g = _rgb_frame[3 * pixel_index + 1],
                .b = _rgb_frame[3 * pixel_index + 2],
                .a = 255,
            });
        }

        // both mode: append particles
        if (draw_particles) {
            _point_vertices.insert(_point_vertices.end(), _particle_vertices.begin(), _particle_vertices.end());
        }

        _points.upload(_point_vertices);
    }

    Time _time;
    Window _window;
    Camera _camera;
    Pipeline _pipeline;
    KinectStream _kinect;
    PointCloudRenderer _points;

    int _window_width = 1280;
    int _window_height = 720;

    static constexpr size_t _frame_width = 640;
    static constexpr size_t _frame_height = 480;
    static constexpr size_t _frame_pixel_count = _frame_width * _frame_height;
    static constexpr size_t _max_render_points = _frame_pixel_count + ParticleSimulator::max_particles;

    enum RenderMode : uint8_t {
        render_mode_point_cloud = 1,
        render_mode_particles = 2,
        render_mode_both = render_mode_point_cloud | render_mode_particles,
    };

    float _focal_length = 595.0f;
    float _principal_point_x = (_frame_width - 1.0f) * 0.5f;
    float _principal_point_y = (_frame_height - 1.0f) * 0.5f;
    float _move_speed_mm_per_sec = 1000.0f;
    uint16_t _max_depth_mm = 4000;

    std::vector<uint8_t> _rgb_frame = std::vector<uint8_t>(_frame_pixel_count * 3, 0);
    std::vector<uint16_t> _depth_frame = std::vector<uint16_t>(_frame_pixel_count, 0);
    std::vector<PointVertex> _point_vertices;
    std::vector<PointVertex> _particle_vertices;
    ParticleSimulator _particle_sim;
    RenderMode _render_mode = render_mode_both;
};
