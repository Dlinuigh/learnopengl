#include <cmath>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
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
};
class Mesh {
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
  unsigned int vao;

public:
  Mesh(std::vector<float> _vertices, std::vector<unsigned int> _indices)
      : vertices(std::move(_vertices)), indices(std::move(_indices)) {
    unsigned int vbo, ebo;
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // vector这样写等效数组,但是定义更方便
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(),
                 vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // 解绑
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
  }
  ~Mesh() { glDeleteVertexArrays(1, &vao); }
  void draw() {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  }
};
void framebuffer_size_callback(GLFWwindow *, int width, int height) {
  glViewport(0, 0, width, height);
}
class Program {
  GLFWwindow *window;
  glm::ivec2 scr_size = {};
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
  std::vector<std::shared_ptr<Mesh>> children;

public:
  std::shared_ptr<ShaderProgram> program_shader = nullptr;
  Program(glm::ivec2 _size) : scr_size(_size) {
    glfwInit();
    window = glfwCreateWindow(scr_size.x, scr_size.y, "Hello Window", nullptr,
                              nullptr);
    glfwMakeContextCurrent(window);
    glad_init();
    glViewport(0, 0, scr_size.x, scr_size.y);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // 使用shared_ptr为了将program的构建函数放到glad后面运行
    program_shader = std::make_shared<ShaderProgram>();
  }
  ~Program() {
    // 并且可以控制他的析构
    for (auto &child : children) {
      child.reset();
    }
    program_shader.reset();
    glfwDestroyWindow(window);
    glfwTerminate();
  }
  void push_back(std::shared_ptr<Mesh> &poly) { children.push_back(poly); }
  void draw() {
    for (auto &it : children) {
      it->draw();
    }
  }
  void run() {
    while (!glfwWindowShouldClose(window)) {
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
      float timeValue = glfwGetTime();
      float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
      program_shader->use();
      program_shader->set("color", glm::fvec4(0.0f, greenValue, 0.0f, 1.0f));
      draw();
      glfwSwapBuffers(window);
      glfwPollEvents();
    }
  }
};
int main() {
  Program program({800, 600});
  program.program_shader->load_shader("assets/glsl/vertex.glsl", GL_VERTEX_SHADER);
  program.program_shader->load_shader("assets/glsl/fragment.glsl", GL_FRAGMENT_SHADER);
  program.program_shader->link();
  std::vector<float> triangle = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f,
                                 0.0f,  0.0f,  0.5f, 0.0f};
  std::vector<unsigned int> indices = {0, 1, 2};
  std::shared_ptr<Mesh> poly = std::make_shared<Mesh>(triangle, indices);
  program.push_back(poly);
  program.run();
}