#include <cmath>
#include <iostream>
#include <fstream>
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <GLFW/glfw3.h>

void framebuffer_size_callback(GLFWwindow *, int width, int height) {
  glViewport(0, 0, width, height);
}
void check(void fun(unsigned int, unsigned int, int *), unsigned int pointer,
           unsigned int flag, const char *message,
           void log(unsigned int, int, int *, char *)) {
  int success;
  fun(pointer, flag, &success);
  char info[512];
  if (!success) {
    log(pointer, 512, NULL, info);
    std::cerr << message << ":\n  " << info << "\n";
  }
}
void load_shader(const char *filename, unsigned int pointer) {
  std::ifstream stream(filename, std::ios::in);
  if (stream.is_open()) {
    std::string tmp((std::istreambuf_iterator(stream)),
                    std::istreambuf_iterator<char>());
    const char *src = tmp.c_str();
    glShaderSource(pointer, 1, &src, NULL);
    glCompileShader(pointer);
    stream.close();
  }
}
void load_program(unsigned int &program_shader) {
  // 如果没有引用,就不会将这里的结果写入,颜色就是错误的
  unsigned int shader_vertex, shader_fragment;
  shader_vertex = glCreateShader(GL_VERTEX_SHADER);
  shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);
  program_shader = glCreateProgram();
  load_shader("assets/glsl/vertex.glsl", shader_vertex);
  load_shader("assets/glsl/fragment.glsl", shader_fragment);
  check(glGetShaderiv, shader_vertex, GL_COMPILE_STATUS, "Vertex",
        glGetShaderInfoLog);
  check(glGetShaderiv, shader_fragment, GL_COMPILE_STATUS, "Fragment",
        glGetShaderInfoLog);
  glAttachShader(program_shader, shader_vertex);
  glAttachShader(program_shader, shader_fragment);
  glLinkProgram(program_shader);
  check(glGetProgramiv, program_shader, GL_LINK_STATUS, "Shader Program",
        glGetProgramInfoLog);
  glDeleteShader(shader_vertex);
  glDeleteShader(shader_fragment);
}
void draw_triangle(unsigned int &vao) {
  // 一定需要使用引用,这里会给vao之类赋值
  unsigned int vbo;
  float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0); // 解绑
  glBindVertexArray(0);
  glDeleteBuffers(1, &vbo);
}
void draw_rect(unsigned int &vao) {
  // ebo的使用,只是vbo的组织方式而已,框架模式仍然是vao存储vbo的所有,vao是绘制的直接需求
  float vertices[] = {-0.5, 0.5, 0, 0.5, 0.5, 0, 0.5, -0.5, 0, -0.5, -0.5, 0};
  unsigned int indices[] = {0, 1, 2, 2, 3, 0};
  unsigned int vbo, ebo;
  glGenBuffers(1, &vbo);
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0); // 解绑
  glBindVertexArray(0);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
}
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

int main() {
  glfwInit();
  GLFWwindow *window =
      glfwCreateWindow(800, 600, "Hello Window", nullptr, nullptr);
  glfwMakeContextCurrent(window);
  glad_init();
  unsigned int program_shader, vao, rvao;
  load_program(program_shader);
  glViewport(0, 0, 800, 600);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  draw_triangle(vao);
  draw_rect(rvao);
  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // code below will transfer a variable to shader program
    float timeValue = glfwGetTime();
    float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
    int vertexColorLocation = glGetUniformLocation(program_shader, "color");
    glUseProgram(program_shader);
    glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(rvao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glDeleteVertexArrays(1, &vao);
  glDeleteProgram(program_shader);
  glfwTerminate();
  return 0;
}
