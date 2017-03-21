// CPSC 587 Created By: Andrew Owens
// This is a (very) basic program to
// 1) load shaders from external files, and make a shader program
// 2) make Vertex Array Object and Vertex Buffer Object for the triangle

// take a look at the following sites for further readings:
// opengl-tutorial.org -> The first triangle (New OpenGL, great start)
// antongerdelan.net -> shaders pipeline explained
// ogldev.atspace.co.uk -> good resource

// NOTE: this dependencies (include/library files) will need to be tweaked if
// you wish to run this on a non lab computer

// This code was modified by Manorie Vachon. The wave function and included parameters
// were adapted to work with springs and masses.

#include <iostream>
#include <cmath>
#include <chrono>
#include <limits>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Mesh.h"
#include "ShaderTools.h"
#include "Vec3f.h"
#include "Mat4f.h"
#include "OpenGLMatrixTools.h"
#include "Camera.h"
#include "HomoVec4f.h"
#include "Mass.h"
#include "Spring.h"

bool g_cursorLocked;
float g_cursorX, g_cursorY;

using std::cout;
using std::endl;
using std::cerr;

GLuint vaoID;
GLuint basicProgramID, loadColorProgramID;

// Could store these two in an array GLuint[]
GLuint vertBufferID;
GLuint triangleIndexBufferID;

Mat4f MVP;
Mat4f M;
Mat4f V;
Mat4f P;

Mesh massSpringSys;
Mass m;
Spring s;
int sampleID = -1;

Camera camera;
int g_moveUpDown = 0;
int g_moveLeftRight = 0;
int g_moveBackForward = 0;
int g_rotateLeftRight = 0;
int g_rotateUpDown = 0;

bool g_play = false;

int WIN_WIDTH = 800, WIN_HEIGHT = 600;
int FB_WIDTH = 800, FB_HEIGHT = 600;
float WIN_FOV = 60;
float WIN_NEAR = 0.01;
float WIN_FAR = 1000;

// function declarations... just to keep things kinda organized.
void displayFunc();
void resizeFunc();
void init();
void initSysMesh();
void generateIDs();
void deleteIDs();
void setupVAO();
void loadBuffer();
void reloadProjectionMatrix();
void loadModelViewMatrix();
void setupModelViewProjectionTransform();
void reloadMVPUniform();
std::string GL_ERROR();
// Sets up the initial scene of a mass on a spring
void setUpMassOnSpring();
int main(int, char **);
// function declarations

void displayFunc() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Use our shader
  glUseProgram(basicProgramID);

  // Use VAO that holds buffer bindings
  // and attribute config of buffers
  glBindVertexArray(vaoID);

  glDrawElements(GL_TRIANGLES,               // mode
                 massSpringSys.indiceCount(), // count
                 GL_UNSIGNED_INT,            // type
                 (void *)0                   // element array buffer offset
                 );

  if (sampleID != -1) {
    glUseProgram(loadColorProgramID);
    glDrawArrays(GL_POINTS, sampleID, 1);
  }
}

void generateIDs() {
  std::string vsSource = loadShaderStringfromFile("./shaders/phong_vs.glsl");
  std::string fsSource = loadShaderStringfromFile("./shaders/phong_fs.glsl");
  std::string gsSource = loadShaderStringfromFile("./shaders/phong_gs.glsl");
  basicProgramID = CreateShaderProgram(vsSource, gsSource, fsSource);

  fsSource = loadShaderStringfromFile("./shaders/basic_fs.glsl");
  vsSource = loadShaderStringfromFile("./shaders/loadColor_vs.glsl");
  loadColorProgramID = CreateShaderProgram(vsSource, fsSource);

  // load IDs given from OpenGL
  glGenVertexArrays(1, &vaoID);
  glGenBuffers(1, &vertBufferID);
  glGenBuffers(1, &triangleIndexBufferID);
}

void deleteIDs() {
  glDeleteProgram(basicProgramID);
  glDeleteVertexArrays(1, &vaoID);
  glDeleteBuffers(1, &vertBufferID);
  glDeleteBuffers(1, &triangleIndexBufferID);
}

void reloadProjectionMatrix() {
  // Perspective Only

  // field of view angle 60 degrees
  // window aspect ratio
  // near Z plane > 0
  // far Z plane

  P = PerspectiveProjection(WIN_FOV, // FOV
                            static_cast<float>(WIN_WIDTH) /
                                WIN_HEIGHT, // Aspect
                            WIN_NEAR,       // near plane
                            WIN_FAR);       // far plane depth
}

void loadModelViewMatrix() {
  M = IdentityMatrix();
  // view doesn't change, but if it did you would use this
  V = camera.lookatMatrix();
}

void reloadViewMatrix() { V = camera.lookatMatrix(); }

void setupModelViewProjectionTransform() {
  MVP = P * V * M; // transforms vertices from right to left (odd huh?)
}

void reloadMVPUniform() {
  GLint id = glGetUniformLocation(basicProgramID, "MVP");

  glUseProgram(basicProgramID);
  glUniformMatrix4fv(id,        // ID
                     1,         // only 1 matrix
                     GL_TRUE,   // transpose matrix, Mat4f is row major
                     MVP.data() // pointer to data in Mat4f
                     );

  id = glGetUniformLocation(basicProgramID, "V");
  glUniformMatrix4fv(id,      // ID
                     1,       // only 1 matrix
                     GL_TRUE, // transpose matrix, Mat4f is row major
                     V.data() // pointer to data in Mat4f
                     );

  id = glGetUniformLocation(basicProgramID, "M");
  glUniformMatrix4fv(id,      // ID
                     1,       // only 1 matrix
                     GL_TRUE, // transpose matrix, Mat4f is row major
                     M.data() // pointer to data in Mat4f
                     );

  id = glGetUniformLocation(loadColorProgramID, "MVP");

  glUseProgram(loadColorProgramID);
  glUniformMatrix4fv(id,        // ID
                     1,         // only 1 matrix
                     GL_TRUE,   // transpose matrix, Mat4f is row major
                     MVP.data() // pointer to data in Mat4f
                     );
}

void setupVAO() {
  glBindVertexArray(vaoID);

  glEnableVertexAttribArray(0); // match layout # in shader
  glBindBuffer(GL_ARRAY_BUFFER, vertBufferID);
  glVertexAttribPointer(
      0,                                     // attribute layout # above
      3,                                     // # of components (ie XYZ )
      GL_FLOAT,                              // type of components
      GL_FALSE,                              // need to be normalized?
      sizeof(Mesh::Vertex),                  // stride
      (void *)Mesh::Vertex::positionOffset() // array buffer offset
      );

  glEnableVertexAttribArray(1); // match layout # in shader
  glBindBuffer(GL_ARRAY_BUFFER, vertBufferID);
  glVertexAttribPointer(1,                    // attribute layout # above
                        3,                    // # of components (ie XYZ )
                        GL_FLOAT,             // type of components
                        GL_FALSE,             // need to be normalized?
                        sizeof(Mesh::Vertex), // stride
                        (void *)Mesh::Vertex::rgbOffset() // array buffer offset
                        );

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexBufferID);

  glBindVertexArray(0); // reset to default
}

void setPickingColor() {
  glUseProgram(loadColorProgramID);
  GLuint id = glGetUniformLocation(loadColorProgramID, "inputColor");
  glUniform3f(id, 1, 0, 0);
}

void loadmassSpringSys(float t) {
//  float sinHeight = 1;
//  float cosHeight = 4;

  for (auto &vert : massSpringSys.vertices()) {
    auto &pos = vert.pos;
    auto &rgb = vert.rgb;
  //  float radius = pos.x() * pos.x() + pos.y() * pos.y();
  //  float r = (radius + t) * 0.05;

//    float z = (sinHeight * std::sin(r) + cosHeight * std::cos(0.5f * r));
//    float i = (z + 3) / (6); // just to get a changing color...

    pos.x(pos.x() + t);
    pos.y(pos.y() + t);

//    pos.z(z);
//    rgb.x(i);
//    rgb.y(1. - i);
  }
}

void reloadVertexBuffer() {
  glBindBuffer(GL_ARRAY_BUFFER, vertBufferID);
  glBufferData(
      GL_ARRAY_BUFFER,
      sizeof(Mesh::Vertex) *
          massSpringSys.vertexCount(), // byte size of Vec3f, 4 of them
      massSpringSys.vertexData(),      // pointer (Vec3f*) to contents of verts
      GL_DYNAMIC_DRAW);               // Usage pattern of GPU buffer
}

void loadBuffer() {
  reloadVertexBuffer(); // called every frame

  // but this is only needed once here
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexBufferID);
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      sizeof(Mesh::Triangle) *
          massSpringSys.triangleCount(), // byte size of Vec3f, 4 of them
      massSpringSys.triangleData(),      // pointer (Vec3f*) to contents of verts
      GL_STATIC_DRAW);                  // Usage pattern of GPU buffer
}

// Creates triangle and vertex information for each spring and mass
void initSysMesh() {
  Mesh::Vertices verts;
  Mesh::Triangles tris;

  for (int i = 0; i < m.Masses.size(); i++) {
    cout << "the index number is " << i << endl;
    int size = 2;
    float scale = 1.f; //0.075f;

    // Used to make sin function
     for (int r = 0; r < size; ++r) {
       for (int c = 0; c < size; ++c) {
         // push back Vertex( position, rgb )
         float x = (c - size * 0.5) * scale;
         float y = (r - size * 0.5) * scale;

         x = x + m.Masses[i].getPos().x();
         y = y + m.Masses[i].getPos().y();
cout << "a vert is being pushed back" << endl;
         verts.emplace_back(Vec3f(x, y, 0),
                            Vec3f((r) / float(size), (c) / float(size), 1));
       }
     }

    // helper lambda function to get array id from row, column
    auto id = [size](int r, int c) { return (size)*r + c; };

    // c----d
    // |\   |
    // | \  |
    // |  \ |
    // |   \|
    // a----b

    for (int row = 0; row < size - 1; ++row) {
      for (int col = 0; col < size - 1; ++col) {
        int a = id(row, col);
        int b = id(row, col + 1);
        int c = id(row + 1, col);
        int d = id(row + 1, col + 1);

        cout << "triangle a is " << a << endl;
        cout << "triangle b is " << a << endl;
        cout << "triangle c is " << a << endl;
        cout << "triangle d is " << a << endl;


        tris.emplace_back(a, b, c);
        tris.emplace_back(c, b, d);
      }
    }
  }

  massSpringSys = Mesh(verts, tris);
  printf("Triangle count %zu",massSpringSys.triangleCount());

}

void init() {
  glEnable(GL_DEPTH_TEST);
  glPointSize(50);

  camera = Camera(Vec3f{0, 0, 50}, Vec3f{0, 0, -1}, Vec3f{0, 1, 0});

  // SETUP SHADERS, BUFFERS, VAOs
cout << "coming into init" << endl;
  initSysMesh();
  loadmassSpringSys(0);

  generateIDs();
  setupVAO();
  loadBuffer();

  setPickingColor();

  loadModelViewMatrix();
  reloadProjectionMatrix();
  setupModelViewProjectionTransform();
  reloadMVPUniform();
}

// GLFW window callbacks //
void windowSetSizeFunc(GLFWwindow *window, int width, int height) {
  WIN_WIDTH = width;
  WIN_HEIGHT = height;

  reloadProjectionMatrix();
  setupModelViewProjectionTransform();
  reloadMVPUniform();
}

void windowSetFramebufferSizeFunc(GLFWwindow *window, int width, int height) {
  FB_WIDTH = width;
  FB_HEIGHT = height;

  glViewport(0, 0, FB_WIDTH, FB_HEIGHT);
}

int getClosestProjectedPointTo(int x, int y) {
  HomoVec4f vHomo;
  Vec3f v;
  Mesh::Vertices const &verts = massSpringSys.vertices();
  float screenX, screenY;

  int foundID = -1;
  float minDist = std::numeric_limits<float>::max();
  for (int i = 0; i < verts.size(); ++i) {
    vHomo = HomoVec4f(verts[i].pos);

    vHomo = MVP * vHomo;
    v = vHomo.perspectiveDivided(); // Now in NDC
    if (std::abs(v.x()) <= 1 && std::abs(v.y()) <= 1 && std::abs(v.z()) <= 1) {
      screenX = FB_WIDTH * (v.x() + 1) * 0.5;
      screenY = FB_HEIGHT * (1 - v.y()) * 0.5;

      float distSqrd =
          (x - screenX) * (x - screenX) + (y - screenY) * (y - screenY);

      if (distSqrd < minDist && distSqrd < 20) // 20 pxl radius
      {
        minDist = distSqrd;
        foundID = i;
      }
    }
  }

  return foundID;
}

void windowMouseButtonFunc(GLFWwindow *window, int button, int action,
                           int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      g_cursorLocked = GL_TRUE;
      if (mods == GLFW_MOD_CONTROL) {
        g_cursorLocked = GL_FALSE;
        double x, y;
        glfwGetCursorPos(window, &x, &y);

        int foundID = getClosestProjectedPointTo(x, y);
        if (foundID != -1) {
          sampleID = foundID;
          std::cout << " found " << foundID << std::endl;
        }
      }
    } else {
      g_cursorLocked = GL_FALSE;
    }
  }
}

void windowMouseMotionFunc(GLFWwindow *window, double x, double y) {
  if (g_cursorLocked) {
    float deltaX = (x - g_cursorX) * 0.01;
    float deltaY = (y - g_cursorY) * 0.01;
    camera.rotateAroundFocus(deltaX, deltaY);

    reloadViewMatrix();
    setupModelViewProjectionTransform();
    reloadMVPUniform();
  }

  g_cursorX = x;
  g_cursorY = y;
}

void moveCamera() {
  Vec3f dir;
  float speed = 0.5;
  float rotateSpeed = 0.01;

  if (g_moveBackForward) {
    dir += Vec3f(0, 0, g_moveBackForward * speed);
  }
  if (g_moveLeftRight) {
    dir += Vec3f(g_moveLeftRight * speed, 0, 0);
  }
  if (g_moveUpDown) {
    dir += Vec3f(0, g_moveUpDown * speed, 0);
  }

  if (g_rotateUpDown) {
    camera.rotateUpDown(g_rotateUpDown * rotateSpeed);
  }
  if (g_rotateLeftRight) {
    camera.rotateLeftRight(g_rotateLeftRight * rotateSpeed);
  }

  if (g_moveUpDown || g_moveLeftRight || g_moveBackForward ||
      g_rotateLeftRight || g_rotateUpDown) {
    camera.move(dir);
    reloadViewMatrix();
    setupModelViewProjectionTransform();
    reloadMVPUniform();
  }
}

void windowKeyFunc(GLFWwindow *window, int key, int scancode, int action,
                   int mods) {
  bool set = action != GLFW_RELEASE;
  switch (key) {
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(window, GL_TRUE);
    break;
  case GLFW_KEY_W:
    g_moveBackForward = set ? 1 : 0;
    break;
  case GLFW_KEY_S:
    g_moveBackForward = set ? -1 : 0;
    break;
  case GLFW_KEY_A:
    g_moveLeftRight = set ? 1 : 0;
    break;
  case GLFW_KEY_D:
    g_moveLeftRight = set ? -1 : 0;
    break;
  case GLFW_KEY_Q:
    g_moveUpDown = set ? -1 : 0;
    break;
  case GLFW_KEY_E:
    g_moveUpDown = set ? 1 : 0;
    break;
  case GLFW_KEY_UP:
    g_rotateUpDown = set ? -1 : 0;
    break;
  case GLFW_KEY_DOWN:
    g_rotateUpDown = set ? 1 : 0;
    break;
  case GLFW_KEY_LEFT:
    g_rotateLeftRight = set ? 1 : 0;
    break;
  case GLFW_KEY_RIGHT:
    g_rotateLeftRight = set ? -1 : 0;
    break;
  case GLFW_KEY_SPACE:
    g_play = set ? !g_play : g_play;
    break;
  case GLFW_KEY_1:
//    setUpMassOnSpring();
  default:
    break;
  }
}

void setUpMassOnSpring() {
  Mass *massA = new Mass(100.f, Vec3f(0, 20, 0));
  Mass *massB = new Mass(100.f, Vec3f(0, 0, 0));
  Spring *spring1 = new Spring(5.f, massA, massB, 20.0f);

  m.Masses.push_back(*massA);
  m.Masses.push_back(*massB);
  s.Springs.push_back(*spring1);

  init();
}


int main(int argc, char **argv) {
  GLFWwindow *window;

  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Physics Mass-Spring Simulation", NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glfwSetWindowSizeCallback(window, windowSetSizeFunc);
  glfwSetFramebufferSizeCallback(window, windowSetFramebufferSizeFunc);
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwGetFramebufferSize(window, &WIN_WIDTH, &WIN_HEIGHT);
  glfwSetKeyCallback(window, windowKeyFunc);
  glfwSetCursorPosCallback(window, windowMouseMotionFunc);
  glfwSetMouseButtonCallback(window, windowMouseButtonFunc);

  glewExperimental = true; // Needed in Core Profile
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialise GLEW" << std::endl;
    exit(EXIT_FAILURE);
  }
  cout << "GL Version: :" << glGetString(GL_VERSION) << endl;
  cout << GL_ERROR() << endl;

//  init(); // our own initialize stuff func
  setUpMassOnSpring();

  float const deltaT = 0.001f;
  float t = 0;

  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         !glfwWindowShouldClose(window)) {

    if (g_play) {
      t += deltaT;
      loadmassSpringSys(t);
      reloadVertexBuffer();
    }

    displayFunc();
    moveCamera();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // clean up after loop
  deleteIDs();

  return 0;
}

std::string GL_ERROR() {
  GLenum code = glGetError();

  switch (code) {
  case GL_NO_ERROR:
    return "GL_NO_ERROR";
  case GL_INVALID_ENUM:
    return "GL_INVALID_ENUM";
  case GL_INVALID_VALUE:
    return "GL_INVALID_VALUE";
  case GL_INVALID_OPERATION:
    return "GL_INVALID_OPERATION";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  case GL_OUT_OF_MEMORY:
    return "GL_OUT_OF_MEMORY";
  default:
    return "Non Valid Error Code";
  }
}
