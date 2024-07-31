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
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#define MAX_BONE_INFLUENCE 4
/**
 * @brief
 * model化之前的成果，然后这里我需要阐述将会出现的设计，因为这里并不是之前代码简单的
 * 复制粘贴
 *
 * @version 0.1
 * @author lion_claw (lion_claw@163.com)
 * @date 2024-07-30
 * @copyright Copyright (c) 2024
 * @par 详细的设计
 * 结合地看，目前glsl只能使用限制数量的光照，那么更多的光照可能需要复杂的设计，比如如果我在房间设计了一盏台灯，然后窗户外面有月光，那么台灯可以开关，月光可以在晚上散发光照，那么可变的激活可能是使用外部变量控制原本代码里面已经实现的光照。glsl中不存在可变的光照数量？
 *
 */
/*
  全局gl错误检查函数，当然可以进行错误阻截，尽早触发的获取错误会禁止后面的重复报错？
*/
void checkError(std::string function);
struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
  glm::vec3 Tangent;
  // bitangent
  glm::vec3 Bitangent;
  // bone indexes which will influence this vertex
  int m_BoneIDs[MAX_BONE_INFLUENCE];
  // weights from each bone
  float m_Weights[MAX_BONE_INFLUENCE];
};
class ShaderProgram {
  unsigned int id;
  void check(void fun(unsigned int, unsigned int, int *), unsigned int sid,
             unsigned int flag, std::string message,
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
  void load_shader(std::string filename, int flag) {
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
  void set(std::string name, glm::ivec4 value) const {
    glUniform4i(glGetUniformLocation(id, name.c_str()), value.x, value.y,
                value.z, value.w);
  }
  void set(std::string name, glm::fvec4 value) const {
    glUniform4f(glGetUniformLocation(id, name.c_str()), value.x, value.y,
                value.z, value.w);
  }
  void set(std::string name, glm::fvec3 value) const {
    glUniform3f(glGetUniformLocation(id, name.c_str()), value.x, value.y,
                value.z);
  }
  void set(std::string name, bool value) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
  }
  void set(std::string name, int value) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
  }
  void set(std::string name, float value) const {
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
  }
  void set(std::string name, glm::mat4 &value) const {
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE,
                       glm::value_ptr(value));
  }
};
class Texture {
  unsigned int id; // gl初始化时候自动赋值，用来区分不同的texture

public:
  int index; // 用来对应不同的texture0，后面的数字就是index，用来激活
  int type; // aiTextureType_DIFFUSE
  Texture(std::string filename, int _type) : type(_type) {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data =
        stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
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
  ~Texture() { glDeleteTextures(1, &id); }
};
class TextureMgr {
  std::map<std::string, std::shared_ptr<Texture>> texture_lut;
  TextureMgr() {};
  TextureMgr(TextureMgr &) = delete;
  void operator=(TextureMgr const &) = delete;

public:
  static TextureMgr &getInstance() {
    static TextureMgr stance;
    return stance;
  }
  void free(){
    for (auto &t : texture_lut) {
      t.second.reset();
    }
  }
  ~TextureMgr() {
    
  }
  void set(std::string name, std::shared_ptr<Texture> &texture) {
    texture_lut[name] = std::move(texture);
  }
  bool has(std::string name) { return texture_lut.contains(name); }
  std::shared_ptr<Texture> &get(std::string name) { return texture_lut[name]; }
};
class Mesh {
  // FIXME 过去的删除reset的代码并没有解决问题，之后就是手动控制了
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<std::string> diffuse;
  std::vector<std::string> specular;
  std::vector<std::string> normal;
  std::vector<std::string> ambient;
  unsigned int vao;

public:
  Mesh(std::vector<Vertex> _vertices, std::vector<unsigned int> _indices,
       std::vector<std::string> &d, std::vector<std::string> &s,
       std::vector<std::string> &n, std::vector<std::string> &a)
      : vertices(std::move(_vertices)), indices(std::move(_indices)),
        diffuse(std::move(d)), specular(std::move(s)), normal(std::move(n)),
        ambient(std::move(a)) {
    unsigned int vbo, ebo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
                 vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, Bitangent));
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex),
                           (void *)offsetof(Vertex, m_BoneIDs));

    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, m_Weights));
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
  }
  ~Mesh() { glDeleteVertexArrays(1, &vao); }
  void process(GLFWwindow *) {}
  void draw() {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }
  void activate(std::shared_ptr<ShaderProgram> program) {
    int idx = 0;
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    TextureMgr &mgr = TextureMgr::getInstance();
    for (auto &d : diffuse) {
      mgr.get(d)->activate(idx);
      program->set("diffuse" + std::to_string(diffuseNr++), idx++);
    }
    for (auto &s : specular) {
      mgr.get(s)->activate(idx);
      program->set("specular" + std::to_string(specularNr++), idx++);
    }
    for (auto &n : normal) {
      mgr.get(n)->activate(idx);
      program->set("normal" + std::to_string(normalNr++), idx++);
    }
    for (auto &a : ambient) {
      mgr.get(a)->activate(idx);
      program->set("ambient" + std::to_string(heightNr++), idx++);
    }
  }
};

class Light {
  // 上一节没有介绍对于Light的任何抽象，那么我将沿用自己的设计
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
  unsigned int vao;

public:
  glm::vec3 lightPos;
  glm::vec4 light_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  glm::vec4 ambient;
  glm::vec4 diffuse;
  glm::vec4 specular;
  glm::vec3 direction;
  glm::mat4 model;
  float constant;
  float linear;
  float quadratic;
  float cutOff;
  float outerCutoff;
  int light_type = 1;
  std::shared_ptr<ShaderProgram> program = std::make_shared<ShaderProgram>();
  Light(std::vector<float> _vertices, std::vector<unsigned int> _indices)
      : vertices(std::move(_vertices)), indices(std::move(_indices)) {
    unsigned int vbo, ebo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(),
                 vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
  }
  ~Light() {
    // program.reset();
    glDeleteVertexArrays(1, &vao);
  }
  void process(GLFWwindow *window) {
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
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
      lightPos.y -= 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_SEMICOLON) == GLFW_PRESS) {
      lightPos.y += 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
      lightPos.x -= 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS) {
      lightPos.x += 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
      light_type = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
      light_type = 2;
    }
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
      light_type = 0;
    }
  }
  void set() {
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.2f));
    program->set("model", model);
    program->set("light_color", light_color);
  }
  void draw() {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  }
};
class Model {
  // mesh但是包含了数据的载入，自然最好也包含图片，模型，渲染代码
  bool gammaCorrection;
  std::string directory;
  void loadModel(std::string path); // 调用节点处理载入模型
  void processNode(aiNode *node, const aiScene *scene);
  // 调用mesh处理并递归访问
  std::shared_ptr<Mesh> processMesh(aiMesh *mesh,
                                    const aiScene *scene); // 处理点与材质
  std::vector<std::string> loadMaterialTextures(aiMaterial *mat,
                                                aiTextureType type);
  // 处理材质，将会写入信息到texturemgr中。使用材质名称加diffuse类型访问
public:
  std::vector<std::shared_ptr<Mesh>> meshes;
  std::shared_ptr<ShaderProgram> program;
  Model(std::string path, bool gamma = false) : gammaCorrection(gamma) {
    loadModel(path);
  }
  ~Model() {
    program.reset();
    for (auto &m : meshes) {
      m.reset();
    }
  }
  void draw() {
    for (auto &m : meshes) {
      m->draw();
    }
  }
  void set(glm::mat4 &v, glm::mat4 &p, glm::mat4 &m, glm::vec3 &cp);
  void use() { program->use(); }
  void process(GLFWwindow *) {}
  void link() { program->link(); }
  void activate();
};
class Camera {
  bool firstMouse = true;
  float yaw = -90.0f, pitch = 0.0f, fov = 45.0f;
  double lastX, lastY;
  glm::ivec2 scr_size;
  glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
  const float cameraSpeed = 0.05f;

public:
  glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
  glm::mat4 view = glm::mat4(1.0f), projection = glm::mat4(1.0f);
  Camera(glm::ivec2 _scr_size) : scr_size(_scr_size) {
    // TODO 我想设计一个更加方便的办法，而不是需要设置这个指针
    lastX = scr_size.x / 2.0;
    lastY = scr_size.y / 2.0;
  }
  void process(GLFWwindow *window) {
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
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    projection = glm::perspective(glm::radians(fov),
                                  (float)scr_size.x / scr_size.y, 0.1f, 100.0f);
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
};
class Program {
  glm::ivec2 scr_size = {};
  std::shared_ptr<Camera> camera = nullptr;
  TextureMgr &mgr = TextureMgr::getInstance();
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
  static void mouse_callback_handler(GLFWwindow *window, double x, double y) {
    // 需要在外面设置这个指针，如果当前的指针更换了就失效
    // 直接在program中创建handler然后由于program会创建所有的类，那么就可以调用这些类里面的处理函数
    Program *program = static_cast<Program *>(glfwGetWindowUserPointer(window));
    if (program)
      program->camera->mouse_callback(window, x, y);
  }
  static void scroll_callback_handler(GLFWwindow *window, double x, double y) {
    Program *program = static_cast<Program *>(glfwGetWindowUserPointer(window));
    if (program)
      program->camera->scroll_callback(window, x, y);
  }
  static void framebuffer_size_callback_handler(GLFWwindow *window, int x,
                                                int y) {
    Program *program = static_cast<Program *>(glfwGetWindowUserPointer(window));
    if (program)
      program->framebuffer_size_callback(window, x, y);
  }
  void framebuffer_size_callback(GLFWwindow *, int width, int height) {
    glViewport(0, 0, width, height);
  }

public:
  GLFWwindow *window;
  std::shared_ptr<Model> model = nullptr;
  std::vector<std::shared_ptr<Light>> light_src;
  Program(glm::ivec2 _size) : scr_size(_size) {
    glfwInit();
    window = glfwCreateWindow(scr_size.x, scr_size.y, "Hello Window", nullptr,
                              nullptr);
    glfwMakeContextCurrent(window);
    glad_init();
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, scr_size.x, scr_size.y);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, mouse_callback_handler);
    glfwSetScrollCallback(window, scroll_callback_handler);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback_handler);
    camera = std::make_shared<Camera>(scr_size);
  }
  ~Program() {
    model.reset();
    camera.reset();
    mgr.free();
    glfwTerminate();
  }
  void push_back(std::shared_ptr<Light> &light) { light_src.push_back(light); }
  void set_light(std::shared_ptr<ShaderProgram> &program);
  void process();
  void run();
};
#endif