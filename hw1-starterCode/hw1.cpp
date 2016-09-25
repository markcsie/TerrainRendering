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
#include "texture.h"

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

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum {
  ROTATE, TRANSLATE, SCALE
} CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

typedef enum {
  POINTS, WIRE_FRAME, FILL, BOTH
} RENDER_MODE;
RENDER_MODE renderMode = POINTS;

// state of the world
std::array<GLfloat, 3> landRotate = {0.0f, 0.0f, 0.0f};
std::array<GLfloat, 3> landTranslate = {0.0f, 0.0f, 0.0f};
std::array<GLfloat, 3> landScale = {1.0f, 1.0f, 1.0f};

const std::array<GLfloat, 3> rgbCoefficients = {0.299f, 0.587f, 0.114f};

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
const GLfloat fillColor[4] = {1.0, 1.0, 1.0, 0.0};
const GLfloat frameColor[4] = {1.0, 0.0, 0.0, 0.0};

GLuint textureImage;

// write a screenshot to the specified filename
void saveScreenshot(const char * filename) {
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  std::cout << "wtfffff " << std::endl;
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);
  std::cout << "wtfffff " << std::endl;

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);
  std::cout << "wtfffff " << std::endl;

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK) {
    std::cout << "File " << filename << " saved successfully." << std::endl;
  } else {
    std::cout << "Failed to save file " << filename << '.' << std::endl;
  }

  delete [] screenshotData;
}

void displayFunc() {
  // Projection
  GLfloat projectionMatrix[16];
  openGLMatrix.SetMatrixMode(OpenGLMatrix::Projection);
  openGLMatrix.GetMatrix(projectionMatrix);
  pipeline.SetProjectionMatrix(projectionMatrix);

  // Model View
  GLfloat modelViewMatrix[16];
  openGLMatrix.SetMatrixMode(OpenGLMatrix::ModelView);
  openGLMatrix.LoadIdentity();

  // Camera
  openGLMatrix.LookAt(0, 0, 2, 0, 0, 0, 0, 1, 0);

  // T R S
  openGLMatrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
  openGLMatrix.Rotate(landRotate[0], 1, 0, 0);
  openGLMatrix.Rotate(landRotate[1], 0, 1, 0);
  openGLMatrix.Rotate(landRotate[2], 0, 0, 1);
  openGLMatrix.Scale(landScale[0], landScale[1], landScale[2]);

  // Fill in the matrix and set
  openGLMatrix.GetMatrix(modelViewMatrix);
  pipeline.SetModelViewMatrix(modelViewMatrix);
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  switch (renderMode) {
    case POINTS:
      pipeline.SetFColor(frameColor);
      glDrawArrays(GL_POINTS, 0, numVertices);
      break;
    case WIRE_FRAME:
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      pipeline.SetFColor(frameColor);
      for (size_t y = 0; y < imageHeight - 1; y++) { 
        glDrawElements(GL_TRIANGLE_STRIP, numTrianglesPerY * 3, GL_UNSIGNED_INT, (const GLvoid *) (y * sizeof (unsigned int) * numTrianglesPerY * 3));
      }
      break;
    case FILL:
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      pipeline.SetFColor(fillColor);
      for (size_t y = 0; y < imageHeight - 1; y++) {
        glDrawElements(GL_TRIANGLE_STRIP, numTrianglesPerY * 3, GL_UNSIGNED_INT, (const GLvoid *) (y * sizeof (unsigned int) * numTrianglesPerY * 3));
      }
      break;
    case BOTH:
      // Fill
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      pipeline.SetFColor(fillColor);
      for (size_t y = 0; y < imageHeight - 1; y++) {
        glDrawElements(GL_TRIANGLE_STRIP, numTrianglesPerY * 3, GL_UNSIGNED_INT, (const GLvoid *) (y * sizeof (unsigned int) * numTrianglesPerY * 3));
      }

      // Wire Frame
      pipeline.SetFColor(frameColor);
      glEnable(GL_POLYGON_OFFSET_LINE);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glPolygonOffset(-1.0f, -1.0f);
      for (size_t y = 0; y < imageHeight - 1; y++) {
        glDrawElements(GL_TRIANGLE_STRIP, numTrianglesPerY * 3, GL_UNSIGNED_INT, (const GLvoid *) (y * sizeof (unsigned int) * numTrianglesPerY * 3));
      }
      glPolygonOffset(0.0f, 0.0f);
      glDisable(GL_POLYGON_OFFSET_LINE);
  }



  glutSwapBuffers();
}

void idleFunc() {
  // do some stuff... 

  // for example, here, you can save the screenshots to disk (to make the animation)
//  saveScreenshot("screenshot.jpg");

  // make the screen update 
  glutPostRedisplay();
}

void reshapeFunc(int w, int h) {
  glViewport(0, 0, w, h);
  windowWidth = w;
  windowHeight = h;
  
  // setup perspective matrix...
  openGLMatrix.SetMatrixMode(OpenGLMatrix::Projection);
  openGLMatrix.LoadIdentity();
  openGLMatrix.Perspective(90.0, 1.0 * w / h, 0.01, 1000.0);
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
        landTranslate[0] += mousePosDelta[0] * 0.005f;
        landTranslate[1] -= mousePosDelta[1] * 0.005f;
      }
      if (middleMouseButton) {
        // control z translation via the middle mouse button
        landTranslate[2] += mousePosDelta[1] * 0.005f;
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

    case 'q':
      renderMode = POINTS;
      break;
    case 'w':
      renderMode = WIRE_FRAME;
      break;
    case 'e':
      renderMode = FILL;
      break;
    case 'r':
      renderMode = BOTH;
      break;
      
    case ' ':
      std::cout << "You pressed the spacebar." << std::endl;
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
    std::cout << "Error reading image " << argv[1] << "." << std::endl;
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

  std::vector<GLfloat> vertices(3 * numVertices);
  std::vector<GLfloat> colors(4 * numVertices);
  std::vector<GLfloat> uvData;

  for (size_t y = 0; y < imageHeight; y++) {
    for (size_t x = 0; x < imageWidth; x++) {
      unsigned int grayValue = 0;
      if (heightmapImage->getBytesPerPixel() == 3) {
        for (size_t channel = 0; channel < heightmapImage->getBytesPerPixel(); channel++) {
          //        std::cout << static_cast<unsigned int>(heightmapImage->getPixel(x, y, channel)) << " ";
          grayValue += heightmapImage->getPixel(x, y, channel) * rgbCoefficients[channel];
        }
      } else {
        grayValue = heightmapImage->getPixel(x, y, 0);
      }

      vertices[(y * imageWidth + x) * 3 + 0] = -1.0f + (x / static_cast<GLfloat> (imageWidth)) * 2.0f;
      vertices[(y * imageWidth + x) * 3 + 1] = -1.0f + (y / static_cast<GLfloat> (imageHeight)) * 2.0f;
      vertices[(y * imageWidth + x) * 3 + 2] = grayValue / 255.0f / 2.0;

      if (y % 2 == 0) {
        if (x % 2 == 0) {
          uvData.push_back(0.0);
          uvData.push_back(0.0);
        } else {
          uvData.push_back(1.0);
          uvData.push_back(0.0);
        }
      } else {
        if (x % 2 == 0) {
          uvData.push_back(0.0);
          uvData.push_back(1.0);
        } else {
          uvData.push_back(1.0);
          uvData.push_back(1.0);
        }
      }

      colors[(y * imageWidth + x) * 4 + 0] = 1.0f * grayValue / 255.0f;
      colors[(y * imageWidth + x) * 4 + 1] = 1.0f * grayValue / 255.0f;
      colors[(y * imageWidth + x) * 4 + 2] = 1.0f * grayValue / 255.0f;
      colors[(y * imageWidth + x) * 4 + 3] = 0.0f;
      // std::cout << vertices[(y * imageWidth + x) * 3 + 0] << " " << vertices[(y * imageWidth + x) * 3 + 1] << " " << vertices[(y * imageWidth + x) * 3 + 2] << std::endl;
    }
  }
  
  std::vector<unsigned int> triangleIndices;
  for (size_t y = 0; y < imageHeight - 1; y++) {
    for (size_t x = 0; x < imageWidth - 1; x++) {
      unsigned int baseIndex = y * imageWidth + x;

      triangleIndices.push_back(baseIndex);
      triangleIndices.push_back(baseIndex + 1);
      triangleIndices.push_back(baseIndex + imageWidth);
      
      triangleIndices.push_back(baseIndex + imageWidth);
      triangleIndices.push_back(baseIndex + 1);
      triangleIndices.push_back(baseIndex + 1 + imageWidth);      
    }
  }
  std::cout << "triangleIndices.size() " << triangleIndices.size() << std::endl;

  // Create shaders
  pipeline.Init(shaderBasePath);
  pipeline.Bind();
  
  textureImage = loadBMP_custom("./grass.bmp");

  // vao
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // vertex vbo
  glGenBuffers(1, &vertexBufferName);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBufferName);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof (GLfloat), &vertices[0], GL_STATIC_DRAW);
  GLuint posLocation = glGetAttribLocation(pipeline.GetProgramHandle(), "position");
  glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(posLocation);

  // color vbo
  glGenBuffers(1, &colorBufferName);
  glBindBuffer(GL_ARRAY_BUFFER, colorBufferName);
  glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof (GLfloat), &colors[0], GL_STATIC_DRAW);
  GLuint colLocation = glGetAttribLocation(pipeline.GetProgramHandle(), "color");
  glVertexAttribPointer(colLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(colLocation);

  // element vbo
  GLuint elementbuffer;
  glGenBuffers(1, &elementbuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleIndices.size() * sizeof (unsigned int), &triangleIndices[0], GL_STATIC_DRAW);

  // Bind our texture in Texture Unit 0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureImage);
  pipeline.SetTextureSampler();
  
  // uv vbo, for texture
  GLuint uvbuffer;
  glGenBuffers(1, &uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, uvData.size() * sizeof (GLfloat), &uvData[0], GL_STATIC_DRAW);
  GLuint uvLocation = glGetAttribLocation(pipeline.GetProgramHandle(), "vertexUV");
  glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(uvLocation);

  glEnable(GL_DEPTH_TEST);
  glClearColor(131.0 / 255.0, 175 / 255.0, 155.0 / 255.0, 0.0);

}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "The arguments are incorrect." << std::endl;
    std::cout << "usage: ./hw1 <heightmap file>" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::cout << "Initializing GLUT..." << std::endl;
  glutInit(&argc, argv);
  glutInitContextVersion(3, 3);
  glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

  std::cout << "Initializing OpenGL..." << std::endl;

#ifdef __APPLE__
  glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#else
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
#endif

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);
  glutCreateWindow(windowTitle);

  std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

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
    std::cout << "error: " << glewGetErrorString(result) << std::endl;
    exit(EXIT_FAILURE);
  }
#endif

  // do initialization
  initScene(argc, argv);

  // sink forever into the glut loop
  glutMainLoop();
}


