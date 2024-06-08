#include "lvar_opengl_gnulinux.h"
#include "lvar_math.h"
#include "lvar_timer.h"
#include "lvar_math_debug.h"

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace lvar;

constexpr float WORLD_UNIT_TO_PIXEL{ 10.0f };

inline constexpr float worldToPixels(float const worldunits)
{
  return worldunits * WORLD_UNIT_TO_PIXEL;
}

inline constexpr float pixelsToWorld(float const pixels)
{
  return pixels / WORLD_UNIT_TO_PIXEL;
}

float constexpr window_width { 1920.f };
float constexpr window_height{ 1080.f };

float constexpr level_width_game_units { 10.0f };
float constexpr level_height_game_units{ 10.0f };
float constexpr level_depth_game_units { 10.0f };

class grid final {
public:
  grid() noexcept
  {
    // this shouldn't be "random", choosing a right value depends on the avg size of entities
    cell_width  = 5.0f;         // game units
    cell_height = 5.0f;         // "
    cell_depth  = 5.0f;         // "
    rows   = std::ceil(level_width_game_units  / cell_height);
    cols   = std::ceil(level_height_game_units / cell_width);
    layers = std::ceil(level_depth_game_units  / cell_depth);
  }

  inline auto id(v3 const& pos) const noexcept
  {
    auto const xIndex = std::floor(pos.x / cell_width);
    auto const yIndex = std::floor(pos.y / cell_height);
    auto const zIndex = std::floor(pos.z / cell_depth);
    // hash that you've seen in the internet, just copy&paste p much
    return xIndex + yIndex * cols + zIndex * cols * rows;
  }

  void update(v3 const& pos) const noexcept
  {
    const auto i = id(pos);
    // std::clog << "current pos -> (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
    // std::clog << "computed id -> " << id(pos) << std::endl;
  }
private:
  int rows;
  int cols;
  int layers;
  float cell_width;
  float cell_height;
  float cell_depth;
};

inline auto useShaderProgram(const unsigned int id) noexcept
{
  glUseProgram(id);
}

inline auto setUniformFloat(const unsigned int id, const char* uniname, const float v)
{
  glUniform1f(glGetUniformLocation(id, uniname), v);
}

inline auto setUniformMat4(const unsigned int id, const char* uniname, const m4& m)
{
  glUniformMatrix4fv(glGetUniformLocation(id, uniname), 1, false, &m.get(0, 0));
}

inline auto setUniformInt(const unsigned int id, const char* uniname, const int value)
{
  glUniform1i(glGetUniformLocation(id, uniname), value);
}

inline auto setUniformVec4(const unsigned int id, const char* uniname, const v4& value)
{
  glUniform4f(glGetUniformLocation(id, uniname), value.x, value.y, value.z, value.w);
}

class Shader final {
public:
  Shader(unsigned int i, unsigned int va, unsigned int vb)
    : id{ i }, VAO{ va }, VBO{ vb }
  {
  }
  unsigned int id;
  unsigned int VAO;
  unsigned int VBO;
};

unsigned int loadCompileShaders(const char* vertpath, const char* fragpath)
{
  std::ifstream vertFileStream(vertpath);
  if(!vertFileStream) {
    std::cerr << __FUNCTION__ << " Couldn't open vertex file: " << vertpath << '\n';
    std::exit(EXIT_FAILURE);
  }
  std::ifstream fragFileStream(fragpath);
  if(!fragFileStream) {
    std::cerr << __FUNCTION__ << "Couldn't open fragment file: " << fragpath << '\n';
    std::exit(EXIT_FAILURE);
  }
  std::stringstream vertFileSS;
  vertFileSS << vertFileStream.rdbuf();
  std::stringstream fragFileSS;
  fragFileSS << fragFileStream.rdbuf();
  unsigned int vertId, fragId;
  const std::string vertCode = vertFileSS.str();
  const std::string fragCode = fragFileSS.str();
  const char* vertCodeC = vertCode.c_str();
  const char* fragCodeC = fragCode.c_str();
  vertId = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertId, 1, &vertCodeC, nullptr);
  glCompileShader(vertId);
  int success;
  char infoLog[512];
  glGetShaderiv(vertId, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertId, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
  fragId = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragId, 1, &fragCodeC, nullptr);
  glCompileShader(fragId);
  glGetShaderiv(vertId, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertId, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
  unsigned int programId = glCreateProgram();
  glAttachShader(programId, vertId);
  glAttachShader(programId, fragId);
  glLinkProgram(programId);
  glGetProgramiv(programId, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(programId, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }
  glDeleteShader(vertId);
  glDeleteShader(fragId);
  return programId;
}

Shader loadBackgroundShader(const char* vertpath,
                            const char* fragpath)
{
  const auto id = loadCompileShaders(vertpath, fragpath);
  if(!id) {
    std::cerr << __FUNCTION__ << " Error creating shader\n";
    exit(EXIT_FAILURE);
  }
  float constexpr vertices[] {
    // Back face
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
    // Front face
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
    // Left face
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
    // Right face
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
    // Bottom face
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
    // Top face
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f  // bottom-left
  };
  unsigned int VBO, VAO, EBO;
  glGenBuffers(1, &VBO);
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &EBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // texture coord attribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  useShaderProgram(id);
  setUniformInt(id, "image", 0);
  return Shader(id, VAO, VBO);
}

Shader loadWallsShader(char const* vertpath,
                       char const* fragpath)
{
  auto const id = loadCompileShaders(vertpath, fragpath);
  if(!id) {
    std::cerr << __FUNCTION__ << " Error creating shader\n";
    exit(EXIT_FAILURE);
  }
  float constexpr vertices[] = {
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom-left
     0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // bottom-right
    -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, // top-left

    -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, // top-left
     0.5f,  0.5f, 0.0f, 1.0f, 1.0f, // top-right
     0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // bottom-right
  };
  unsigned int VAO, VBO;
  glGenBuffers(1, &VBO);
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(sizeof(float) * 3));
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  useShaderProgram(id);
  setUniformInt(id, "image1", 0);
  return Shader(id, VAO, VBO);
}

// in X, hiding a pointer means creating a transparent one
static void hidePointer(Display* d, Window& w)
{
  // GC -> graphics context
  // create transparent pixelmap of 1x1
  Pixmap pointerPixmap{ XCreatePixmap(d, w, 1, 1, 1) };
  XGCValues values;
  GC gc{ XCreateGC(d, pointerPixmap, 0, &values) };
  // set the pointer to 0 (transparent)
  XSetForeground(d, gc, 0);
  XDrawPoint(d, pointerPixmap, gc, 0, 0);
  XFreeGC(d, gc);
  XColor colour;
  colour.flags = DoRed | DoGreen | DoBlue;
  colour.red = colour.green = colour.blue = 0;
  Cursor transparentPointer{ XCreatePixmapCursor(d, pointerPixmap, pointerPixmap, &colour, &colour, 0, 0) };
  // apply it to the window
  XDefineCursor(d, w, transparentPointer);
  // cleanup
  XFreePixmap(d, pointerPixmap);
  XFreeCursor(d, transparentPointer);
}

int main()
{
  std::ios::sync_with_stdio(false);
  init_opengl_ptrs();
  static int attrs[] = {
    GLX_X_RENDERABLE    , True,
    GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
    GLX_RENDER_TYPE     , GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
    GLX_RED_SIZE        , 8,
    GLX_GREEN_SIZE      , 8,
    GLX_BLUE_SIZE       , 8,
    GLX_ALPHA_SIZE      , 8,
    GLX_DEPTH_SIZE      , 24,
    GLX_STENCIL_SIZE    , 8,
    GLX_DOUBLEBUFFER    , True,
    GLX_SAMPLE_BUFFERS  , 1,
    GLX_SAMPLES         , 4,
    None
  };
  XSetWindowAttributes setWindowAttrs;
  XWindowAttributes gwa;
  Display* display = XOpenDisplay(NULL);
  if(!display) {
    std::cerr << "Couldn't connect to X server\n";
    std::exit(EXIT_FAILURE);
  }
  int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
  int fbcount;
  GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), attrs, &fbcount);
  for(int i = 0; i < fbcount; ++i) {
    XVisualInfo* vi = glXGetVisualFromFBConfig(display, fbc[i]);
    if(vi) {
      int samp_buf, samples;
      glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
      glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLES       , &samples  );
      if(best_fbc < 0 || (samp_buf && samples > best_num_samp)) {
	best_fbc = i;
	best_num_samp = samples;
      }
      if(worst_fbc < 0 || !samp_buf || samples < worst_num_samp) {
	worst_fbc = i;
	worst_num_samp = samples;
      }
    }
    XFree(vi);
  }
  GLXFBConfig bestFbc = fbc[best_fbc];
  XFree(fbc);
  Window root = DefaultRootWindow(display);
  XVisualInfo* visualInfo = glXGetVisualFromFBConfig(display, bestFbc);
  if(!visualInfo) {
    std::cerr << "Couldn't find appropiate visual info\n";
    std::exit(EXIT_FAILURE);
  }
  Colormap cmap = XCreateColormap(display, root, visualInfo->visual, AllocNone);
  // listening for key presses, releases and exposures
  setWindowAttrs.colormap = cmap;
  setWindowAttrs.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask;
  Window window = XCreateWindow(display, root, 0, 0,
                                window_width, window_height, 0, visualInfo->depth,
                                InputOutput, visualInfo->visual, CWColormap | CWEventMask, &setWindowAttrs);
  XFree(visualInfo);
  XStoreName(display, window, "Testing Spatial Grid");
  XMapWindow(display, window);
  XSync(display, False);
  // disable resizing
  XSizeHints hints;
  hints.flags = PMinSize | PMaxSize;
  hints.min_width = hints.max_width = window_width;
  hints.min_height = hints.max_height = window_height;
  XSetWMNormalHints(display, window, &hints);
  // closing window gracefully when user clicks on the X btn
  Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wmDeleteMessage, 1);
  // pretty important, setup opengl context for renderdoc
  glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
  glXCreateContextAttribsARB =
    (glXCreateContextAttribsARBProc) glXGetProcAddressARB((const GLubyte *) "glXCreateContextAttribsARB");
  int contextAttribs[] = {
    GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
    GLX_CONTEXT_MINOR_VERSION_ARB, 3,
    GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
    None
  };
  // !!!renderdoc!!!
  GLXContext glContext = glXCreateContextAttribsARB(display, bestFbc, 0, True, contextAttribs);
  XSync(display, False);
  glXMakeCurrent(display, window, glContext);
  // enable VSync
  glXSwapIntervalEXT(display, window, 1);
  XEvent xev;
  auto res = XGrabPointer(display, window, False, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, window, None, CurrentTime);
  if(res != GrabSuccess) {
    switch (res) {
    case GrabNotViewable:
      std::cerr << "Failed to grab pointer: GrabNotViewable\n";
      break;
    case AlreadyGrabbed:
      std::cerr << "Failed to grab pointer: AlreadyGrabbed\n";
      break;
    case GrabFrozen:
      std::cerr << "Failed to grab pointer: GrabFrozen\n";
      break;
    case GrabInvalidTime:
      std::cerr << "Failed to grab pointer: GrabInvalidTime\n";
      break;
    default:
      std::cerr << "Failed to grab pointer: Unknown error\n";
    }
  }
  hidePointer(display, window);
  // run the game
  unsigned int texture1, texture2, wallTexture, groundTexture;
  // texture 1
  // ---------
  glGenTextures(1, &texture1);
  glBindTexture(GL_TEXTURE_2D, texture1);
  // set the texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // load image, create texture and generate mipmaps
  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
  unsigned char *data = stbi_load("./res/despera.jpg", &width, &height, &nrChannels, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  stbi_image_free(data);
  // texture 2
  // ---------
  glGenTextures(1, &texture2);
  glBindTexture(GL_TEXTURE_2D, texture2);
  // set the texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // load image, create texture and generate mipmaps
  data = stbi_load("./res/sky.png", &width, &height, &nrChannels, 0);
  if (data){
    // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  stbi_image_free(data);
  // wall texture
  glGenTextures(1, &wallTexture);
  glBindTexture(GL_TEXTURE_2D, wallTexture);
  // set the texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // load image, create texture and generate mipmaps
  data = stbi_load("./res/blocks1.jpg", &width, &height, &nrChannels, 0);
  if (data){
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  stbi_image_free(data);
  // ground texture
  glGenTextures(1, &groundTexture);
  glBindTexture(GL_TEXTURE_2D, groundTexture);
  // set the texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // load image, create texture and generate mipmaps
  data = stbi_load("./res/Cobblestone.png", &width, &height, &nrChannels, 0);
  if (data){
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  stbi_image_free(data);
  m4 const projection{ perspective(45.0f, 1920.0f / 1080.0f, 0.1f, 100.f) };
  auto s = loadBackgroundShader("./res/basic.vert",
                                "./res/basic.frag");
  useShaderProgram(s.id);
  setUniformInt(s.id, "image1", 0);
  setUniformInt(s.id, "image2", 1);
  setUniformMat4(s.id, "projection", projection);
  // load wall
  auto s2 = loadWallsShader("./res/wall.vert", "./res/wall.frag");
  useShaderProgram(s2.id);
  setUniformMat4(s2.id, "projection", projection);
  // 3D
  grid levelGrid;
  v3 cubePositions[] = {
    v3( 5.0f,  2.0f,  5.0f),
    v3( 8.0f,  8.0f,  8.0f),
  };
  // wall data, you want to form a square
  v3 const wallScales[] = {
    v3(level_width_game_units, level_height_game_units, 1.0f), // front wall
    v3(level_width_game_units, level_height_game_units, 1.0f), // back wall
    v3(level_width_game_units, level_height_game_units, 1.0f), // left wall
    v3(level_width_game_units, level_height_game_units, 1.0f), // right wall
  };
  v3 const wallPositions[] = {
    v3( level_width_game_units / 2.0f, level_height_game_units / 2.0f, level_depth_game_units), // front wall
    v3( level_width_game_units / 2.0f, level_height_game_units / 2.0f, 0.0f), // back wall
    v3( 0.0f, level_height_game_units / 2.0f, level_width_game_units / 2.0f), // left wall
    v3( level_width_game_units, level_height_game_units / 2.0f, level_width_game_units / 2.0f), // right wall
  };
  v3 const groundScale {
    level_width_game_units, level_height_game_units, 1.0f,
  };
  v3 const groundPosition {
    level_width_game_units / 2.0f, 0, level_depth_game_units / 2.0f,
  };
  // ground
  m4 groundModel{ identity() };
  scale(groundModel, groundScale);
  rotate(groundModel, 90.0f, v3i{ 1, 0, 0 });
  translate(groundModel, groundPosition);
  // OpenGL stuff
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  // debug
  bool wireframe{ false };
  // Camera stuff
  v3 const cameraUp{ 0.0f, 1.0f, 0.0f };
  v3 cameraFront{ 0.0f, 0.0f, -1.0f };
  v3 cameraPosition{ 5.0f, 5.0f, 5.0f };
  auto lastFrame = 0.0f;
  auto quit = false;
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "OpenGL error: " << err << std::endl;
  }
  // Euler angles
  float yaw{ -90.0f };          // looking right or left?
  float pitch{ 0.0f };          // looking up or down?
  // Mouse stuff
  float lastMouseX{ window_width / 2.0f };
  float lastMouseY{ window_height / 2.0f };
  constexpr float mouseSensitivity{ 0.2f };
  while(!quit) {
    auto currentFrame = time();
    auto delta = currentFrame - lastFrame;
    lastFrame = currentFrame;
    float const cameraSpeed{ 1.5f * delta };
    glViewport(0, 0, gwa.width, gwa.height);
    // -------------------------------------------------------------------------------------------------------
    // mouse
    int root_x, root_y, win_x, win_y;
    unsigned int mask_return;
    Window root_return, child_return;
    XQueryPointer(display, window, &root_return, &child_return, &root_x, &root_y, &win_x, &win_y, &mask_return);
    // right after getting pointer's X and Y, you need to see if it's close to window's limits bc in that case
    // you want to change the pointer to the center of the window again, which will simulate the effect of being
    // able to do 360 turns in the game.
    bool shouldWarp{ false };
    if(win_x < 5 || win_x > window_width - 5) {
      shouldWarp = true;
      XWarpPointer(display, None, window, 0, 0, 0, 0, window_width / 2.0f, win_y);
    }
    float xMouseOffset{ win_x - lastMouseX };
    float yMouseOffset{ lastMouseY - win_y };
    lastMouseX = win_x;
    lastMouseY = win_y;
    if(shouldWarp) {
      XFlush(display);          // apply the warp immediately
      xMouseOffset = 0;         // to avoid the effect of jumping to the center of the screen, say that there isn't any diff
      lastMouseX = window_width / 2.0f; // yes, go back to the center of the screen, like you said in XWarpPointer
    }
    xMouseOffset *= mouseSensitivity;
    yMouseOffset *= mouseSensitivity;
    yaw += xMouseOffset;
    pitch += yMouseOffset;
    // clamp, need a function
    if(pitch > 89.0f) {
      pitch =  89.0f;
    } else if(pitch < -89.0f) {
      pitch = -89.0f;
    }
    // camera direction changes based on mouse input
    v3 const cameraDirection{
      std::cos(radians(yaw)) * std::cos(radians(pitch)),
      std::sin(radians(pitch)),
      std::sin(radians(yaw)) * std::cos(radians(pitch))
    };
    cameraFront = normalise(cameraDirection);
    auto prevCameraPosition = cameraPosition;
    // polling for x11 window events, keyboard
    while(XPending(display)) {
      XNextEvent(display, &xev);
      if(xev.type == KeyPress || xev.type == KeyRelease) {
        // const auto is_pressed = xev.type == KeyPress;
        const auto x_key = XLookupKeysym(&xev.xkey, 0);
        switch(x_key) {
        case XK_Escape:
          quit = true;
          break;
        case XK_a:
          cameraPosition = sub(cameraPosition, scale(normalise(cross(cameraFront, cameraUp)), cameraSpeed));
          break;
        case XK_d:
          cameraPosition = add(cameraPosition, scale(normalise(cross(cameraFront, cameraUp)), cameraSpeed));
          break;
        case XK_w:
          cameraPosition = add(cameraPosition, scale(cameraFront, cameraSpeed));
          break;
        case XK_s:
          cameraPosition = sub(cameraPosition, scale(cameraFront, cameraSpeed));
          break;
        case XK_F3:
          if(!wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            wireframe = true;
          } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            wireframe = false;
          }
          break;
        default:
          break;
        }
      } else if(xev.type == ClientMessage) {
        if(static_cast<Atom>(xev.xclient.data.l[0]) == wmDeleteMessage) {
          quit = true;
        }
      }
    }
    // @NOTE: modify this is you clip thru game level's walls
    if(cameraPosition.x > level_width_game_units || cameraPosition.y > level_height_game_units || cameraPosition.z > level_depth_game_units || cameraPosition.x < 0.0f || cameraPosition.y < 0.0f || cameraPosition.z < 0.0f) {
      cameraPosition = prevCameraPosition;
    }
    levelGrid.update(cameraPosition);
    // view matrix (camera)
    m4 const view{ lookAt(cameraPosition, add(cameraPosition, cameraFront), cameraUp) };
    // -------------------------------------------------------------------------------------------------------
    // start render code
    glClearColor(0.3f, 0.2f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    useShaderProgram(s.id);
    setUniformMat4(s.id, "view", view);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glBindVertexArray(s.VAO);
    // model matrix contains translations, rotations and scales
    for(int i{ 0 }; i < 2; ++i) {
      m4 model = identity();
      const float rotation = 20.0f * i;
      rotate(model, rotation, v3i{0, 0, 1});
      translate(model, cubePositions[i]);
      setUniformMat4(s.id, "model", model);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    // wall
    useShaderProgram(s2.id);
    setUniformMat4(s2.id, "view", view);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, wallTexture);
    glBindVertexArray(s2.VAO);
    for(int i{ 0 }; i < 4; ++i) {
      m4 model{ identity() };
      scale(model, wallScales[i]);
      if(i > 1) {
        rotate(model, 90.0f, v3i{0, 1, 0});
      }
      translate(model, wallPositions[i]);
      setUniformMat4(s2.id, "model", model);
      glDrawArrays(GL_TRIANGLES, 0, 18);
    }
    // ground
    glBindTexture(GL_TEXTURE_2D, groundTexture);
    setUniformMat4(s2.id, "model", groundModel);
    glDrawArrays(GL_TRIANGLES, 0, 18);
    // -------------------------------------------------------------------------------------------------------
    // end render code
    XGetWindowAttributes(display, window, &gwa);
    glXSwapBuffers(display, window);
  }
  XUngrabPointer(display, CurrentTime);
  glXMakeCurrent(display, None, nullptr);
  glXDestroyContext(display, glContext);
  XDestroyWindow(display, window);
  XCloseDisplay(display);
  return EXIT_SUCCESS;
}
