#include <print>
#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include <chrono>
#include <set>
#include <mutex>
#include <utility>
#include <cstdint>
#include <cctype>
#include <algorithm>
#include <cstddef>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h> // main header
#include <SDL3/SDL_main.h> // entry point

#include <glbinding/gl/gl.h> // opengl headers
#include <glbinding/glbinding.h> // main header
#include <glbinding-aux/debug.h> // utils
using namespace gl;

#include <glm/glm.hpp> // main header
#include <glm/ext/matrix_transform.hpp> // translation, rotation, etc
#include <glm/gtc/type_ptr.hpp> // obtain pointers

#include <libfreenect.hpp>
