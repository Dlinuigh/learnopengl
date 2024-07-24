#ifndef CAMERA_H
#define CAMERA_H
#include <cmath>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
class ShaderProgram {
  unsigned int id;
  void check(void fun(unsigned int, unsigned int, int *), unsigned int sid,
             unsigned int flag, const char *message,
             void log(unsigned int, int, int *, char *)) {
    int success;
    fun(sid, flag, &success);
    char info[512];
    if (!success) {
      log(sid, 512, NULL, info);
      std::cerr << message << ":\n  " << info << "\n";
    }
  }

public:
  ShaderProgram() { id = glCreateProgram(); }
  ~ShaderProgram() { glDeleteProgram(id); }
  void load_shader(const char *filename, int flag) {
    std::ifstream stream(filename, std::ios::in);
    if (stream.is_open()) {
      std::string tmp((std::istreambuf_iterator(stream)),
                      std::istreambuf_iterator<char>());
      const char *src = tmp.c_str();
      unsigned int sid = glCreateShader(flag);
      glShaderSource(sid, 1, &src, NULL);
      glCompileShader(sid);
      check(glGetShaderiv, sid, GL_COMPILE_STATUS, "Shader",
            glGetShaderInfoLog);
      glAttachShader(id, sid);
      glDeleteShader(sid);
      stream.close();
    }
  }
  void link() {
    glLinkProgram(id);
    check(glGetProgramiv, id, GL_LINK_STATUS, "Shader Program",
          glGetProgramInfoLog);
  }
  void use() { glUseProgram(id); }
  void set(const char *name, glm::ivec4 value) const {
    glUniform4i(glGetUniformLocation(id, name), value.x, value.y, value.z,
                value.w);
  }
  void set(const char *name, glm::fvec4 value) const {
    glUniform4f(glGetUniformLocation(id, name), value.x, value.y, value.z,
                value.w);
  }
  void set(const char *name, int value) const {
    glUniform1i(glGetUniformLocation(id, name), value);
  }
  void set(const char *name, float value) const {
    glUniform1f(glGetUniformLocation(id, name), value);
  }
  void set(const char *name, glm::mat4 &value) const {
    glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_FALSE,
                       glm::value_ptr(value));
  }
};
class Poly {
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
  unsigned int vao;

public:
  Poly(bool texture, std::vector<float> _vertices,
       std::vector<unsigned int> _indices)
      : vertices(std::move(_vertices)), indices(std::move(_indices)) {
    if (texture) {
      unsigned int vbo, ebo;
      glGenBuffers(1, &vbo);
      glGenBuffers(1, &ebo);
      glBindVertexArray(vao);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(),
                   vertices.data(), GL_STATIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   sizeof(unsigned int) * indices.size(), indices.data(),
                   GL_STATIC_DRAW);
      // position attribute
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                            (void *)0);
      glEnableVertexAttribArray(0);
      // texture coord attribute
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                            (void *)(3 * sizeof(float)));
      glEnableVertexAttribArray(1);
    } else {
      unsigned int vbo, ebo;
      glGenBuffers(1, &vbo);
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      // vector这样写等效数组,但是定义更方便
      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(),
                   vertices.data(), GL_STATIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   sizeof(unsigned int) * indices.size(), indices.data(),
                   GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                            (void *)0);
      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0); // 解绑
      glBindVertexArray(0);
      glDeleteBuffers(1, &vbo);
      glDeleteBuffers(1, &ebo);
    }
  }
  ~Poly() { glDeleteVertexArrays(1, &vao); }
  void draw() {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  }
};
void framebuffer_size_callback(GLFWwindow *, int width, int height);
class ImageTexture {
  unsigned int id;

public:
  ImageTexture(const char *filename) {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    // set the texture wrapping/filtering options (on the currently bound
    // texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data) {
      int mode = nrChannels == 3 ? GL_RGB : GL_RGBA;
      glTexImage2D(GL_TEXTURE_2D, 0, mode, width, height, 0, mode,
                   GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
    } else {
      std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
  }
  void activate(int idx) {
    glActiveTexture(GL_TEXTURE0 + idx);
    glBindTexture(GL_TEXTURE_2D, id);
  }
  ~ImageTexture() { glDeleteTextures(1, &id); }
};
class Camera;
class Program {
  float scale = 0.5f;
  GLFWwindow *window;
  glm::ivec2 scr_size = {};
  std::shared_ptr<Camera> camera = nullptr;
  void glad_init() {
    int version = gladLoadGL(glfwGetProcAddress);
    if (!version) {
      std::cout << "Failed to initialize GLAD\n" << version << std::endl;
      exit(-1);
    } else {
      printf("GL %d.%d\n", GLAD_VERSION_MAJOR(version),
             GLAD_VERSION_MINOR(version));
    }
  }
  std::vector<std::shared_ptr<Poly>> children;
  std::vector<std::shared_ptr<ImageTexture>> textures;
  glm::mat4 model = glm::mat4(1.0f);
  // make sure to initialize matrix to identity matrix first

public:
  std::shared_ptr<ShaderProgram> program_shader = nullptr;
  Program(glm::ivec2 _size) : scr_size(_size) {
    glfwInit();
    window = glfwCreateWindow(scr_size.x, scr_size.y, "Hello Window", nullptr,
                              nullptr);
    glfwMakeContextCurrent(window);
    glad_init();
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, scr_size.x, scr_size.y);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // 使用shared_ptr为了将program的构建函数放到glad后面运行
    program_shader = std::make_shared<ShaderProgram>();
    camera = std::make_shared<Camera>(window, scr_size);
  }
  ~Program() {
    // 并且可以控制他的析构
    for (auto &child : children) {
      child.reset();
    }
    for (auto &texture : textures) {
      texture.reset();
    }
    program_shader.reset();
    glfwDestroyWindow(window);
    glfwTerminate();
  }
  void push_back(std::shared_ptr<Poly> &poly) { children.push_back(poly); }
  void push_back(std::shared_ptr<ImageTexture> &texture) {
    textures.push_back(texture);
  }
  void draw() {
    for (auto &it : children) {
      it->draw();
    }
  }
  void process();
  void run();
};
class Camera {
  bool firstMouse;
  float yaw = -90.0f, pitch, fov;
  double lastX, lastY;
  glm::ivec2 scr_size;
  GLFWwindow *window;
  glm::mat4 view = glm::mat4(1.0f), projection = glm::mat4(1.0f);
  glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
  glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
  // float deltaTime = 0.0f;          // Time between current frame and last
  // frame float lastFrame = 0.0f;          // Time of last frame
  const float cameraSpeed = 0.05f; // adjust accordingly
  glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
  static void mouse_callback_handler(GLFWwindow *window, double xpos,
                                     double ypos) {
    Camera *camera = static_cast<Camera *>(glfwGetWindowUserPointer(window));
    if (camera)
      camera->mouse_callback(window, xpos, ypos);
  }
  static void scroll_callback_handler(GLFWwindow *window, double xoffset,
                                      double yoffset) {
    Camera *camera = static_cast<Camera *>(glfwGetWindowUserPointer(window));
    if (camera)
      camera->scroll_callback(window, xoffset, yoffset);
  }
  void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
      lastX = xpos;
      lastY = ypos;
      firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
      pitch = 89.0f;
    if (pitch < -89.0f)
      pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
  }
  void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    fov -= (float)yoffset;
    if (fov < 1.0f)
      fov = 1.0f;
    if (fov > 45.0f)
      fov = 45.0f;
  }

public:
  Camera(GLFWwindow *_window, glm::ivec2 _scr_size)
      : scr_size(_scr_size), window(_window) {
    glfwSetWindowUserPointer(_window, this);
    glfwSetCursorPosCallback(_window, mouse_callback_handler);
    glfwSetScrollCallback(_window, scroll_callback_handler);
    lastX = scr_size.x;
    lastY = scr_size.y;
  }
  void process() {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      cameraPos -=
          glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      cameraPos +=
          glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  }
  void run(std::shared_ptr<ShaderProgram> program_shader) {
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    projection = glm::perspective(glm::radians(fov),
                                  (float)scr_size.x / scr_size.y, 0.1f, 100.0f);
    program_shader->set("view", view);
    program_shader->set("projection", projection);
  }
};
#endif