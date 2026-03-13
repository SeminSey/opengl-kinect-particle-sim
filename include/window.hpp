#pragma once

struct Window {
    void init(int width, int height, int multisample_count = 1) {
        // init the SDL video subsystem before anything else
        bool result_sdl = SDL_InitSubSystem(SDL_INIT_VIDEO);
        if (!result_sdl) std::println("SDL_InitSubSystem: {}", SDL_GetError());

        // OpenGL context settings
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        // set up multisampling capabilities
        if (multisample_count > 1) {
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1); // enable multisampling
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multisample_count); // set number of samples per pixel
        }

        // create a window specifically with OpenGL support
        _window_p = SDL_CreateWindow("OpenGL Renderer", width, height, SDL_WINDOW_OPENGL);
        if (_window_p == nullptr) std::println("SDL_CreateWindow: {}", SDL_GetError());

        // create opengl context
        _context = SDL_GL_CreateContext(_window_p);
        if (_context == nullptr) std::println("SDL_GL_CreateContext: {}", SDL_GetError());

        // OpenGL function loader
        glbinding::initialize(SDL_GL_GetProcAddress, false);
        glbinding::aux::enableGetErrorCallback();

        // OpenGL settings after initialization
        glEnable(GL_DEPTH_TEST); // enable depth buffer and depth testing
        glEnable(GL_CULL_FACE); // cull backfaces
        glEnable(GL_FRAMEBUFFER_SRGB); // gamma corrected framebuffer
        glEnable(GL_BLEND); // blend colors by source alpha
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // standard alpha blending
        SDL_GL_SetSwapInterval(1); // vsync

        // multisampling allows us to do blending and anti-aliasing (MSAA)
        if (multisample_count > 1) {
            glEnable(GL_MULTISAMPLE);  // enable multisampling application-wide
            glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE); // enable color blending via multisampling
        }
    }
    void destroy() {
        SDL_DestroyWindow(_window_p);
        SDL_Quit();
    }

    SDL_Window* _window_p;
    SDL_GLContext _context;
};
