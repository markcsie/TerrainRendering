/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields
  C++ starter code

  Student username: <kaichiem>
 */

#include <iostream>
#include <vector>
#include <array>
#include "openGLHeader.h"
#include "glutHeader.h"

#include "imageIO.h"
#include "openGLMatrix.h"
#include "basicPipelineProgram.h"

#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib, "glew32d.lib")
#else
#pragma comment(lib, "glew32.lib")
#endif
#endif

#ifdef WIN32
char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum {
  ROTATE, TRANSLATE, SCALE
} CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
std::array<float, 3> landRotate = {0.0f, 0.0f, 0.0f};
std::array<float, 3> landTranslate = {0.0f, 0.0f, 0.0f};
std::array<float, 3> landScale = {1.0f, 1.0f, 1.0f};

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

ImageIO * heightmapImage;

size_t imageWidth = 0;
size_t imageHeight = 0;

BasicPipelineProgram pipeline;
OpenGLMatrix openGLMatrix;
GLuint vao;
GLuint vertexBufferName;
GLuint colorBufferName;
GLuint numVertices = 0;
GLuint numTriangles = 0;
GLuint numTrianglesPerY = 0; 
// write a screenshot to the specified filename

void saveScreenshot(const char * filename) {
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
    cout << "File " << filename << " saved successfully." << endl;
  else cout << "Failed to save file " << filename << '.' << endl;

  delete [] screenshotData;
}

void displayFunc() {
  // Projection
  float projectionMatrix[16];
  openGLMatrix.SetMatrixMode(OpenGLMatrix::Projection);
  openGLMatrix.GetMatrix(projectionMatrix);
  pipeline.SetProjectionMatrix(projectionMatrix);
  
  // Model View
  float modelViewMatrix[16];
  openGLMatrix.SetMatrixMode(OpenGLMatrix::ModelView);
  openGLMatrix.LoadIdentity();
  openGLMatrix.LookAt(0, 0, 2, 0, 0, 0, 0, 1, 0);
  openGLMatrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
  openGLMatrix.Rotate(landRotate[0], 1, 0, 0);
  openGLMatrix.Rotate(landRotate[1], 0, 1, 0);
  openGLMatrix.Rotate(landRotate[2], 0, 0, 1);
  openGLMatrix.Scale(landScale[0], landScale[1], landScale[2]);
  openGLMatrix.GetMatrix(modelViewMatrix);
  pipeline.SetModelViewMatrix(modelViewMatrix);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  for (size_t y = 0; y < imageHeight - 1; y++) {
    glDrawElements(GL_TRIANGLES, numTrianglesPerY * 3, GL_UNSIGNED_INT, (const GLvoid *)(y * sizeof(unsigned int) * numTrianglesPerY * 3));
  }

  glutSwapBuffers();
}

void idleFunc() {
  // do some stuff... 

  // for example, here, you can save the screenshots to disk (to make the animation)

  // make the screen update 
  glutPostRedisplay();
}

void reshapeFunc(int w, int h) {
  glViewport(0, 0, w, h);
  // setup perspective matrix...
  openGLMatrix.SetMatrixMode(OpenGLMatrix::Projection);
  openGLMatrix.LoadIdentity();
  openGLMatrix.Perspective(90.0, 1.0 * w / h, 0.01, 10.0);
  openGLMatrix.SetMatrixMode(OpenGLMatrix::ModelView);
}

void mouseMotionDragFunc(int x, int y) {
  // mouse has moved and one of the mouse buttons is pressed (dragging)

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = {x - mousePos[0], y - mousePos[1]};

  switch (controlState) {
      // translate the landscape
    case TRANSLATE:
      if (leftMouseButton) {
        // control x,y translation via the left mouse button
        landTranslate[0] += mousePosDelta[0] * 0.01f;
        landTranslate[1] -= mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton) {
        // control z translation via the middle mouse button
        landTranslate[2] += mousePosDelta[1] * 0.01f;
      }
      break;

      // rotate the landscape
    case ROTATE:  
      if (leftMouseButton) {
        // control x,y rotation via the left mouse button
        landRotate[0] += mousePosDelta[1] * 0.1f;
        landRotate[1] += mousePosDelta[0] * 0.1f;
      }
      if (middleMouseButton) {
        // control z rotation via the middle mouse button
        landRotate[2] += mousePosDelta[1] * 0.1f;
      }
      break;

      // scale the landscape
    case SCALE:
      if (leftMouseButton) {
        // control x,y scaling via the left mouse button
        landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
        landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton) {
        // control z scaling via the middle mouse button
        landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseMotionFunc(int x, int y) {
  // mouse has moved
  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y) {
  // a mouse button has has been pressed or depressed

  // keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
  switch (button) {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
      break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
      break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
      break;
  }

  // keep track of whether CTRL and SHIFT keys are pressed
  switch (glutGetModifiers()) {
    case GLUT_ACTIVE_CTRL:
      controlState = TRANSLATE;
      break;

    case GLUT_ACTIVE_SHIFT:
      controlState = SCALE;
      break;

      // if CTRL and SHIFT are not pressed, we are in rotate mode
    default:
      controlState = ROTATE;
      break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y) {
  switch (key) {
    case 27: // ESC key
      exit(0); // exit the program
      break;

    case ' ':
      cout << "You pressed the spacebar." << endl;
      std::fill(landRotate.begin(), landRotate.end(), 0.0f);
      std::fill(landTranslate.begin(), landTranslate.end(), 0.0f);
      std::fill(landScale.begin(), landScale.end(), 1.0f);
      break;

    case 'x':
      // take a screenshot
      saveScreenshot("screenshot.jpg");
      break;
  }
}

void initScene(int argc, char *argv[]) {
  // load the image from a jpeg disk file to main memory
  heightmapImage = new ImageIO();
  if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK) {
    cout << "Error reading image " << argv[1] << "." << endl;
    exit(EXIT_FAILURE);
  }
  
  imageWidth = heightmapImage->getWidth();
  imageHeight = heightmapImage->getHeight();

  std::cout << "Image Width: " << imageWidth << std::endl;
  std::cout << "Image Height: " << imageHeight << std::endl;
  std::cout << "BytesPerPixel: " << heightmapImage->getBytesPerPixel() << std::endl;

  numVertices = imageWidth * imageHeight;
  numTrianglesPerY = (imageWidth - 1) * 2;
  numTriangles = numTrianglesPerY * (imageHeight - 1);
  
  std::cout << "numVertices " << numVertices << std::endl;
  std::cout << "numTriangles " << numTriangles << std::endl;
  std::cout << "numTriangles * 3 " << numTriangles * 3 << std::endl;
  
  // do additional initialization here...
  GLfloat vertices[3 * numVertices];
  GLfloat colors[4 * numVertices];
  
  for (size_t y = 0; y < imageHeight; y++) {
    for (size_t x = 0; x < imageWidth; x++) {
      unsigned int grayValue = 0;
      for (size_t channel = 0; channel < heightmapImage->getBytesPerPixel(); channel++) {
//        std::cout << static_cast<unsigned int>(heightmapImage->getPixel(x, y, channel)) << " ";
        grayValue += heightmapImage->getPixel(x, y, channel);
      }
      // TODO: 3 channel??

      vertices[(y * imageWidth + x) * 3 + 0] = -1.0f + (x / static_cast<GLfloat> (imageWidth)) * 2.0f;
      vertices[(y * imageWidth + x) * 3 + 1] = -1.0f + (y / static_cast<GLfloat> (imageHeight)) * 2.0f;
      vertices[(y * imageWidth + x) * 3 + 2] = grayValue / 255.0f;

      colors[(y * imageWidth + x) * 4 + 0] = 1.0f ;
      colors[(y * imageWidth + x) * 4 + 1] = 1.0f ;
      colors[(y * imageWidth + x) * 4 + 2] = 1.0f ;
      colors[(y * imageWidth + x) * 4 + 3] = 0.0f;
      // std::cout << vertices[(y * imageWidth + x) * 3 + 0] << " " << vertices[(y * imageWidth + x) * 3 + 1] << " " << vertices[(y * imageWidth + x) * 3 + 2] << std::endl;
    }
  }
  
  std::vector<unsigned int> indices;
  for (size_t y = 0; y < imageHeight - 1; y++) {
    for (size_t x = 0; x < imageWidth - 1; x++) {
      unsigned int baseIndex = y * imageWidth + x;
      
      indices.push_back(baseIndex);
      indices.push_back(baseIndex + 1);
      indices.push_back(baseIndex + imageWidth);
      
      indices.push_back(baseIndex + imageWidth);
      indices.push_back(baseIndex + 1);
      indices.push_back(baseIndex + 1 + imageWidth);
      
    }
  }
  std::cout << "indices.size() " << indices.size() << std::endl;

  // Create shaders
  pipeline.Init(shaderBasePath);
  pipeline.Bind();

  // vao
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // vertex vbo
  glGenBuffers(1, &vertexBufferName);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBufferName);
  glBufferData(GL_ARRAY_BUFFER, sizeof (vertices), vertices, GL_STATIC_DRAW);
  GLuint posLocation = glGetAttribLocation(pipeline.GetProgramHandle(), "position");
  glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(posLocation);

  // color vbo
  glGenBuffers(1, &colorBufferName);
  glBindBuffer(GL_ARRAY_BUFFER, colorBufferName);
  glBufferData(GL_ARRAY_BUFFER, sizeof (colors), colors, GL_STATIC_DRAW);
  GLuint colLocation = glGetAttribLocation(pipeline.GetProgramHandle(), "color");
  glVertexAttribPointer(colLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(colLocation);
  
  // element vbo
  GLuint elementbuffer;
  glGenBuffers(1, &elementbuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);


  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << "The arguments are incorrect." << endl;
    cout << "usage: ./hw1 <heightmap file>" << endl;
    exit(EXIT_FAILURE);
  }

  cout << "Initializing GLUT..." << endl;
  glutInit(&argc, argv);
  glutInitContextVersion(3, 3);
  glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  glutSetOption(
          GLUT_ACTION_ON_WINDOW_CLOSE,
          GLUT_ACTION_GLUTMAINLOOP_RETURNS
          );

  cout << "Initializing OpenGL..." << endl;

#ifdef __APPLE__
  glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#else
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
#endif

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);
  glutCreateWindow(windowTitle);

  cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
  cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
  cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  // tells glut to use a particular display function to redraw 
  glutDisplayFunc(displayFunc);
  // perform animation inside idleFunc
  glutIdleFunc(idleFunc);
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  glutKeyboardFunc(keyboardFunc);

  // init glew
#ifdef __APPLE__
  // nothing is needed on Apple
#else
  // Windows, Linux
  glewExperimental = GL_TRUE;
  GLint result = glewInit();
  if (result != GLEW_OK) {
    cout << "error: " << glewGetErrorString(result) << endl;
    exit(EXIT_FAILURE);
  }
#endif

  // do initialization
  initScene(argc, argv);

  // sink forever into the glut loop
  glutMainLoop();
}


