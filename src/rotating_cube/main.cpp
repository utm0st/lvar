#include "lvar_opengl_gnulinux.h"
#include "lvar_math.h"
#include "lvar_timer.h"

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

constexpr float window_width{ 1920.f };
constexpr float window_height{ 1080.f };

inline auto useShaderProgram(const unsigned int id) noexcept
{
  glUseProgram(id);
}
// NOTE: you can cache uniform locations to avoid a context switch
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
  constexpr float vertices[] {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f  };
  // unsigned int indices[] = {
  //   0, 1, 3, // first triangle
  //   1, 2, 3  // second triangle
  // };
  unsigned int VBO, VAO, EBO;
  glGenBuffers(1, &VBO);
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &EBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
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
  unsigned int texture1, texture2;
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
  m4 const projection{ perspective(45.0f, 1920.0f / 1080.0f, 0.1f, 100.f) };
  stbi_image_free(data);
  auto s = loadBackgroundShader("./res/basic.vert",
                                "./res/basic.frag");
  useShaderProgram(s.id);
  setUniformInt(s.id, "image1", 0);
  setUniformInt(s.id, "image2", 1);
  setUniformMat4(s.id, "projection", projection);
  // 3D
  v3 cubePositions[] = {
    v3( 0.0f,  0.0f,  3.0f),
    v3( 2.0f,  5.0f, -15.0f),
    v3(-1.5f, -2.2f, -2.5f),
    v3(-3.8f, -2.0f, -12.3f),
    v3( 2.4f, -0.4f, -3.5f),
    v3(-1.7f,  3.0f, -7.5f),
    v3( 1.3f, -2.0f, -2.5f),
    v3( 1.5f,  2.0f, -2.5f),
    v3( 1.5f,  0.2f, -1.5f),
    v3(-1.3f,  1.0f, -1.5f)
  };
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
  // Camera stuff
  v3 const cameraUp{ 0.0f, 1.0f, 0.0f };
  v3 cameraFront{ 0.0f, 0.0f, -1.0f };
  v3 cameraPosition{ 0.0f, 0.0f, -3.0f };
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
        default:
          break;
        }
      } else if(xev.type == ClientMessage) {
        if(static_cast<Atom>(xev.xclient.data.l[0]) == wmDeleteMessage) {
          quit = true;
        }
      }
    }
    // -------------------------------------------------------------------------------------------------------
    // start render code
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    useShaderProgram(s.id);
    m4 const view{ lookAt(cameraPosition, add(cameraPosition, cameraFront), cameraUp) };
    setUniformMat4(s.id, "view", view);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glBindVertexArray(s.VAO);
    // model matrix contains translations, rotations and scales
    for(int i{ 0 }; i < 10; ++i) {
      m4 model = identity();
      const float rotation = 20.0f * i;
      rotate(model, rotation, v3i{0, 0, 1});
      translate(model, cubePositions[i]);
      setUniformMat4(s.id, "model", model);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
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
