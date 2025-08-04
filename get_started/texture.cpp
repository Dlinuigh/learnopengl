#include <cmath>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
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
};
class Mesh {
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
  unsigned int vao;

public:
  Mesh(bool texture, std::vector<float> _vertices,
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
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
                            (void *)0);
      glEnableVertexAttribArray(0);
      // color attribute
      glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
                            (void *)(3 * sizeof(float)));
      glEnableVertexAttribArray(1);
      // texture coord attribute
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
                            (void *)(7 * sizeof(float)));
      glEnableVertexAttribArray(2);
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
  ~Mesh() { glDeleteVertexArrays(1, &vao); }
  void draw() {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  }
};
void framebuffer_size_callback(GLFWwindow *, int width, int height) {
  glViewport(0, 0, width, height);
}
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
      glTexImage2D(GL_TEXTURE_2D, 0, mode, width,
                   height, 0, mode, GL_UNSIGNED_BYTE, data);
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
class Program {
  float scale=0.5f;
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
  std::vector<std::shared_ptr<ImageTexture>> textures;

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
    for (auto &texture : textures) {
      texture.reset();
    }
    program_shader.reset();
    glfwDestroyWindow(window);
    glfwTerminate();
  }
  void push_back(std::shared_ptr<Mesh> &poly) { children.push_back(poly); }
  void push_back(std::shared_ptr<ImageTexture> &texture) {
    textures.push_back(texture);
  }
  void draw() {
    for (auto &it : children) {
      it->draw();
    }
  }
  void process() {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);
    else if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
      if(scale>0.0f){
        scale-=0.01f;
      }
    }
    // else if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE){
    //   if(scale>0.0f){
    //     scale-=0.1f;
    //   }
    // }
    else if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
      if(scale<1.0f){
        scale+=0.01f;
      }
    }
    // else if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE){
    //   if(scale<1.0f){
    //     scale+=0.1f;
    //   }
    // }
  }
  void run() {
    program_shader->use();
    // 可以用for处理,但是此处简便
    program_shader->set("texture1", 0);
    program_shader->set("texture2", 1);
    while (!glfwWindowShouldClose(window)) {
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
      for (auto t = 0; t < textures.size(); t++) {
        textures[t]->activate(t);
      }
      program_shader->set("scale", scale);
      draw();
      glfwSwapBuffers(window);
      glfwPollEvents();
      process();
    }
  }
};
int main() {
  Program program({800, 600});
  program.program_shader->load_shader("assets/glsl/vertex_texture.glsl",
                                      GL_VERTEX_SHADER);
  program.program_shader->load_shader("assets/glsl/fragment_texture.glsl",
                                      GL_FRAGMENT_SHADER);
  program.program_shader->link();
  // std::vector<float> triangle = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f,
  //                                0.0f,  0.0f,  0.5f, 0.0f};
  // std::vector<unsigned int> indices = {0, 1, 2};
  // std::shared_ptr<Mesh> poly = std::make_shared<Mesh>(triangle, indices);
  // program.push_back(poly);
  std::vector<float> vertices = {
      // positions          // colors           // texture coords
      0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top right
      0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom right
      -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, // bottom left
      -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f  // top left
  };
  std::vector<unsigned int> indices = {
      0, 1, 3, // first triangle
      1, 2, 3  // second triangle
  };
  std::shared_ptr<Mesh> poly = std::make_shared<Mesh>(true, vertices, indices);
  program.push_back(poly);
  std::shared_ptr<ImageTexture> texture1 =
      std::make_shared<ImageTexture>("assets/img/wall.jpg");
  program.push_back(texture1);
  std::shared_ptr<ImageTexture> texture2 =
      std::make_shared<ImageTexture>("assets/img/awesomeface.png");
  program.push_back(texture2);
  program.run();
}