#include <iostream>             // cerr, cout, etc
#include <cstdlib>              // exit codes

#include "lvar_opengl_gnulinux.h" // ogl fxs & cnts
#include "lvar_math.h"
#include "lvar_timer.h"
#include "lvar_input.h"
#include "lvar_camera.h"

// one trans unit
#include "../../lvar_camera.cpp"
#include "../../lvar_resource.cpp"

#include <X11/Xatom.h>

using namespace lvar;

v3 const light_pos{ 1.2f, 1.0f, 2.0f };

float constexpr window_width { 2560.f };
float constexpr window_height{ 1440.f };

static void mouse_hide(Display* d, Window& w)
{
  // GC -> graphics context
  // create transparent pixelmap of 1x1
  Pixmap pointer_pixmap{ XCreatePixmap(d, w, 1, 1, 1) };
  XGCValues values;
  GC gc{ XCreateGC(d, pointer_pixmap, 0, &values) };
  // set the pointer to 0 (transparent)
  XSetForeground(d, gc, 0);
  XDrawPoint(d, pointer_pixmap, gc, 0, 0);
  XFreeGC(d, gc);
  XColor colour;
  colour.flags = DoRed | DoGreen | DoBlue;
  colour.red = colour.green = colour.blue = 0;
  Cursor pointer_transparent{ XCreatePixmapCursor(d, pointer_pixmap, pointer_pixmap, &colour, &colour, 0, 0) };
  // apply it to the window
  XDefineCursor(d, w, pointer_transparent);
  // cleanup
  XFreePixmap(d, pointer_pixmap);
  XFreeCursor(d, pointer_transparent);
}

static input::key x_key_to_game(KeySym const& k)
{
  switch(k) {
  case XK_w:      return input::key::w;
  case XK_a:      return input::key::a;
  case XK_s:      return input::key::s;
  case XK_d:      return input::key::d;
  case XK_Escape: return input::key::esc;
  case XK_Up:     return input::key::up;
  case XK_Left:   return input::key::left;
  case XK_Right:  return input::key::right;
  case XK_Down:   return input::key::down;
  case XK_F1:     return input::key::f1;
  default:        return input::key::unknown;
  }
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
  XSetWindowAttributes swa;
  XWindowAttributes gwa;
  Display* display{ XOpenDisplay(nullptr) };
  if(!display) {
    std::cerr << "Couldn't connect to X server\n";
    std::exit(EXIT_FAILURE);
  }
  int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
  int fbcount;
  GLXFBConfig* fbc{ glXChooseFBConfig(display, DefaultScreen(display), attrs, &fbcount) };
  for(int i = 0; i < fbcount; ++i) {
    XVisualInfo* vi{ glXGetVisualFromFBConfig(display, fbc[i]) };
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
  GLXFBConfig bfbc{ fbc[best_fbc] };
  XFree(fbc);
  Window root{ DefaultRootWindow(display) };
  XVisualInfo* vinfo{ glXGetVisualFromFBConfig(display, bfbc) };
  if(!vinfo) {
    std::cerr << "Couldn't find appropiate visual info\n";
    std::exit(EXIT_FAILURE);
  }
  Colormap cmap{ XCreateColormap(display, root, vinfo->visual, AllocNone) };
  // listening for key presses, releases and exposures
  swa.colormap = cmap;
  swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask;
  Window window{ XCreateWindow(display, root, 0, 0,
                               window_width, window_height, 0, vinfo->depth,
                               InputOutput, vinfo->visual, CWColormap | CWEventMask, &swa) };
  XFree(vinfo);
  XStoreName(display, window, "Colours!!");
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
  int ctxattrs[] = {
    GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
    GLX_CONTEXT_MINOR_VERSION_ARB, 3,
    GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
    None
  };
  // renderdoc!!!
  GLXContext glContext{ glXCreateContextAttribsARB(display, bfbc, 0, True, ctxattrs) };
  XSync(display, False);
  glXMakeCurrent(display, window, glContext);
  // debug!!
  bool wireframe{ false };
  // full screen borderless
  XEvent xev;
  Atom wm_state{ XInternAtom(display, "_NET_WM_STATE", False) };
  Atom wm_fullscreen{ XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False) };
  memset(&xev, 0, sizeof(xev));
  xev.type = ClientMessage;
  xev.xclient.window = window;
  xev.xclient.message_type = wm_state;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = 1;
  xev.xclient.data.l[1] = wm_fullscreen;
  xev.xclient.data.l[2] = 0;
  XSendEvent(display, DefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask,
             &xev);
  XFlush(display);
  // mouse!!
  auto res = XGrabPointer(display, window, False, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                          GrabModeAsync, GrabModeAsync, window, None, CurrentTime);
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
  mouse_hide(display, window);
  // input manager
  input::manager input_manager;
  // camera
  camera cam(v3{ 0.0f, 0.0f, 0.0f }, input_manager);
  // resource manager
  resource::manager resource_manager;
  if(resource_manager.error()) {
    std::cerr << "Failed to create resource manager\n";
    return EXIT_FAILURE;
  }
  // get shader handles
  auto const* shader_cube_light = resource_manager.get_shader(SHADER_VERT_LIGHT_CUBE);
  auto const* shader_cube_object = resource_manager.get_shader(SHADER_VERT_LIGHTING_COLOURS);
  // opengl stuff
  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  // mouse stuff
  float mouse_last_x{ window_width / 2.0f };
  float mouse_last_y{ window_height / 2.0f };
  float constexpr mouse_sens{ 0.2f };
  // -----------------------------------------
  // spec demo stuff!!!!!!!!!!
  // -----------------------------------------
  // define cube colour
  v3 const colour_coral{ 1.0f, 0.5f, 0.31f };
  v3 const colour_light{ 1.0f, 1.0f, 1.0f };
  m4 const projection{ perspective(45.0f, window_width / window_height, 0.1f, 100.f) };
  // uniform data for shaders
  resource::uni_buff_obj ubo_data{ .proj = projection, .view = identity() };
  m4 light_model{ identity( )};
  translate(light_model, light_pos);
  m4 light_model_trans{ transpose(inverse_transform_noscale(light_model)) };
  resource_manager.use_shader(shader_cube_light->id);
  resource_manager.set_uni_mat4(shader_cube_light->id, "model", light_model);
  resource_manager.use_shader(shader_cube_object->id);
  resource_manager.set_uni_mat4(shader_cube_object->id, "model", identity());
  resource_manager.set_uni_vec3(shader_cube_object->id, "colour_object", colour_coral);
  resource_manager.set_uni_vec3(shader_cube_object->id, "colour_light", colour_light);
  resource_manager.set_uni_vec3(shader_cube_object->id, "light_pos", light_pos);
  resource_manager.set_uni_mat4(shader_cube_object->id, "model_trans", light_model_trans);
  float lastframe{ 0.0f };
  bool quit{ false };
  while(!quit) {
    input_manager.begin_frame_kb();
    float currframe{ time() };
    float delta{ currframe - lastframe };
    lastframe = currframe;
    // float const cam_speed{ 1.5f * delta };
    glViewport(0, 0, gwa.width, gwa.height);
    // -------------------------------------------------------------------------------------------------------
    // mouse
    int root_x, root_y, win_x, win_y;
    unsigned int mask_return;
    Window root_return, child_return;
    XQueryPointer(display, window, &root_return, &child_return, &root_x, &root_y, &win_x, &win_y, &mask_return);
    // right after getting mouse's X and Y, you need to see if it's close to window's limits bc in that case
    // you want to change the pointer to the center of the window again, which will simulate the effect of being
    // able to do 360 turns in the game.
    bool warp{ false };
    if(win_x < 5 || win_x > window_width - 5) {
      warp = true;
      XWarpPointer(display, None, window, 0, 0, 0, 0, window_width / 2.0f, win_y);
    }
    float mouse_x_offset{ win_x - mouse_last_x };
    float mouse_y_offset{ mouse_last_y - win_y };
    mouse_last_x = win_x;
    mouse_last_y = win_y;
    if(warp) {
      XFlush(display);    // apply warp immediately
      mouse_x_offset = 0; // to avoid the effect of jumping to the center of the screen, say that there isn't any diff
      mouse_last_x = window_width / 2.0f; // yes, go back to the center of the screen, like you said in XWarpPointer
    }
    mouse_x_offset *= mouse_sens;
    mouse_y_offset *= mouse_sens;
    // polling for x window events, keyboard
    while(XPending(display)) {
      XNextEvent(display, &xev);
      if(xev.type == KeyPress || xev.type == KeyRelease) {
        auto const is_pressed = xev.type == KeyPress;
        auto const x_key = XLookupKeysym(&xev.xkey, 0);
        auto const key = x_key_to_game(x_key);
        input_manager.update_key(key, is_pressed);
        // debug
        if(input_manager.key_pressed(input::key::f1)) {
          if(!wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            wireframe = true;
          } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            wireframe = false;
          }
        }
        else if(input_manager.key_pressed(input::key::esc)) {
          quit = true;
        }
      } else if(xev.type == ClientMessage) {
        if(static_cast<Atom>(xev.xclient.data.l[0]) == wmDeleteMessage) {
          quit = true;
        }
      }
    }
    // update camera position etc based on input, it also updates the view matrix
    cam.update(mouse_x_offset, mouse_y_offset, delta);
    ubo_data.view = cam.get_view();
    resource_manager.update_ubo(ubo_data);
    // clear screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // -------------------------------------------------------------------------------------------------------
    // start render code
    // -------------------------------------------------------------------------------------------------------
    resource_manager.use_shader(shader_cube_object->id);
    glBindVertexArray(shader_cube_object->vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    resource_manager.use_shader(shader_cube_light->id);
    glBindVertexArray(shader_cube_light->vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // end render code
    XGetWindowAttributes(display, window, &gwa);
    glXSwapBuffers(display, window);
  }
  // cleanup!
  XUngrabPointer(display, CurrentTime);
  glXMakeCurrent(display, None, nullptr);
  glXDestroyContext(display, glContext);
  XDestroyWindow(display, window);
  XCloseDisplay(display);
  return EXIT_SUCCESS;
}
