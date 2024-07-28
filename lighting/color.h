#ifndef CAMERA_H
#define CAMERA_H
#include <cmath>
#include <iostream>
#include <fstream>
#include <map>
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
  void set(const char *name, glm::fvec3 value) const {
    glUniform3f(glGetUniformLocation(id, name), value.x, value.y, value.z);
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
class ImageTexture {
  unsigned int id; // texture id

public:
  int index; // activate id
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
    index = idx;
  }
  ~ImageTexture() { glDeleteTextures(1, &id); }
};
class Poly {
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
  unsigned int vao;
  glm::mat4 model; // 每一个物体都应该有的
  std::map<std::string, glm::mat4> program_mat4; // 设置到program的变量
  std::map<std::string, int> program_int;
  std::map<std::string, float> program_float;
  GLFWwindow *window;
  float scale = 0.5f;

public:
  std::map<std::string, std::shared_ptr<ImageTexture>> textures;
  std::shared_ptr<ShaderProgram> program = std::make_shared<ShaderProgram>();
  // FIXME
  // 使用下面的代码会出现错误，可能是program的指针的问题，有两点使用move可能导致新的指针失去了某些特性
  Poly(bool texture, std::vector<float> _vertices,
       std::vector<unsigned int> _indices, GLFWwindow *_window)
      : vertices(std::move(_vertices)), indices(std::move(_indices)),
        window(_window) {
    if (texture) { // 区别是多了后面的材质坐标
      unsigned int vbo, ebo;
      glGenBuffers(1, &vbo);
      glGenBuffers(1, &ebo);
      glGenVertexArrays(1, &vao);
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
      glGenBuffers(1, &ebo);
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
    model = glm::mat4(1.0f);
  }
  ~Poly() {
    for (auto &texture : textures) {
      texture.second.reset();
    }
    program.reset();
    glDeleteVertexArrays(1, &vao);
  }
  void insert(std::string name, std::shared_ptr<ImageTexture> &texture) {
    textures[name] = std::move(texture);
  }
  void activate_textures() {
    if (!textures.empty()) {
      int i = 0;
      for (auto &t : textures) {
        t.second->activate(i);
        program->set(t.first.c_str(), i);
        i++;
      }
    }
  }
  void set() {
    // model = std::move(_model);
    program->set("model", model);
    for (auto &it : program_mat4) {
      program->set(it.first.c_str(), it.second);
    }
    for (auto &it : program_int) {
      program->set(it.first.c_str(), it.second);
    }
    for (auto &it : program_float) {
      program->set(it.first.c_str(), it.second);
    }
    program->set("scale", scale);
  }
  void process() {
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
      if (scale > 0.0f) {
        scale -= 0.01f;
      }
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
      if (scale < 1.0f) {
        scale += 0.01f;
      }
    }
  }
  void set(std::string name, glm::mat4 value) { program_mat4[name] = value; }
  void set(std::string name, int value) { program_int[name] = value; }
  void set(std::string name, float value) { program_float[name] = value; }
  void draw() {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  }
};
void framebuffer_size_callback(GLFWwindow *, int width, int height);
class Camera;
class Light {
  glm::vec3 lightPos = glm::vec3(1.2f, 1.0f, 1.0f);
  /* 为什么这个有pos,而前面的物体没有,不,前面也有,只是默认一个物体会处于中心位置,而他的位置会被1.
  定义的坐标 2. model所改变.然后opengl的坐标是一个基于屏幕尺寸的相对量
  下面为什么有process,这个地方是因为我要调节他的一些变量,通过按键,那么一个物体是否也可以修改呢?我的回应是肯定的,比如我想要移动一个方块,那么最佳的方法就是在方块里面写一个process然后通过这个控制model的变化.或者固定这个但是修改pos,然后经过model的处理将pos转换.
  之前在相机那里也有处理,但是我一开始是放到了program的地方,后来我创建camera类的时候完成了这个整理,将camera的控制自己的权力还给了camera.
  那么一个完整的物体还需要有这两个东西.
  但是camera不用draw,他是完全不同的类型,因为它会影响所有物体的view
  projection,所以camera不是物体.
  TODO 但是我在这个地方不会进行这样的抽象,仅仅是对于每一个物体做一个抽象,
  */
  glm::mat4 model;
  std::map<std::string, glm::mat4> program_mat4; // 设置到program的变量
  GLFWwindow *window;
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
  unsigned int vao;

public:
  glm::vec4 light_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  std::shared_ptr<ShaderProgram> program = std::make_shared<ShaderProgram>();
  Light(std::vector<float> _vertices, std::vector<unsigned int> _indices,
        GLFWwindow *_window)
      : window(_window), vertices(std::move(_vertices)),
        indices(std::move(_indices)) {
    unsigned int vbo, ebo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // vector这样写等效数组,但是定义更方便
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(),
                 vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // 解绑
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
  }
  ~Light() {
    program.reset();
    glDeleteVertexArrays(1, &vao);
  }
  void process() {
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
      light_color.x -= light_color.x > 0.0f ? 0.01f : 0.0f;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
      light_color.x += light_color.x < 1.0f ? 0.01f : 0.0f;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
      light_color.y -= light_color.y > 0.0f ? 0.01f : 0.0f;
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
      light_color.y += light_color.y < 1.0f ? 0.01f : 0.0f;
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
      light_color.z -= light_color.z > 0.0f ? 0.01f : 0.0f;
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
      light_color.z += light_color.z < 1.0f ? 0.01f : 0.0f;
    if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) {
      lightPos.z -= 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) {
      lightPos.z += 0.1f;
    }
  }
  void set(std::string name, glm::mat4 value) { program_mat4[name] = value; }
  void set() {
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);    // 平移
    model = glm::scale(model, glm::vec3(0.2f)); //  缩放
    program->set("model", model);
    program->set("light_color", light_color);
    for (auto &it : program_mat4) {
      program->set(it.first.c_str(), it.second);
    }
  }
  void draw() {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  }
};
class Program {
  float scale = 0.5f;
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

public:
  GLFWwindow *window;
  std::shared_ptr<Light> light_src = nullptr;
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
    camera = std::make_shared<Camera>(window, scr_size);
  }
  ~Program() {
    // 并且可以控制他的析构
    for (auto &child : children) {
      child.reset();
    }
    // for (auto &t : light_src) {
    //   t.reset();
    // }
    light_src.reset();
    glfwDestroyWindow(window);
    glfwTerminate();
  }
  void push_back(std::shared_ptr<Poly> &poly) { children.push_back(poly); }
  // void push_back(std::shared_ptr<Light> &light) { light_src.push_back(light);
  // }
  void process();
  void run();
};
class Camera {
  bool firstMouse = true;
  float yaw = -90.0f, pitch=0.0f, fov=45.0f;
  double lastX, lastY;
  glm::ivec2 scr_size;
  GLFWwindow *window;
  glm::mat4 view = glm::mat4(1.0f), projection = glm::mat4(1.0f);
  glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
  glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
  const float cameraSpeed = 0.05f; // adjust accordingly
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
  void mouse_callback(GLFWwindow *, double xpos, double ypos) {
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
  void scroll_callback(GLFWwindow *, double, double yoffset) {
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
    lastX = scr_size.x / 2.0;
    lastY = scr_size.y / 2.0;
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
  void run() {
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    projection = glm::perspective(glm::radians(fov),
                                  (float)scr_size.x / scr_size.y, 0.1f, 100.0f);
  }
  glm::mat4 get(std::string name) {
    if (name == "view") {
      return view;
    } else if (name == "projection") {
      return projection;
    } else {
      return view;
    }
  }
};
#endif