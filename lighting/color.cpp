#include "color.h"
void framebuffer_size_callback(GLFWwindow *, int width, int height) {
  glViewport(0, 0, width, height);
}
void Program::process() {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  for (auto &it : children) {
    it->process();
  }
  for (auto &it : light_src) {
    it->process();
  }
  camera->process();
}
void Program::run() {
  for(auto &it: children){
    it->program->link();
  }
  for(auto &t:light_src){
    t->program->link();
  }
  // 不能在这里进行link,会导致texture出现问题
  /*
  我找到问题了，创建的shared_ptr通过参数传递进行赋值出现了错误，现在我直接在类里面进行初始化，而不是通过传入一个初始化的类。我测试了首先move是不行的，然后我之前的直接赋值也是不行的
  但是问题比这个还要麻烦，如果使用move，那么就会出现原先修改了link的方法不能fix新的状况，而以前的直接赋值可以通过立马进行link解决texture显示灰色的问题。
  */
  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera->run(); // set camera mat4
    for (auto &it : children) {
      it->program->use();
      // default object position, if you want to change just use set(model)
      it->set("view", camera->get("view"));
      it->set("projection", camera->get("projection"));
      it->activate_textures();
      it->set();
      it->draw();
    }
    for (auto &it : light_src) {
      it->program->use();
      it->set("view", camera->get("view"));
      it->set("projection", camera->get("projection"));
      it->set(); // set light position
      it->draw();
    }
    glfwSwapBuffers(window);
    glfwPollEvents();
    process();
  }
}
int main() {
  Program program({800, 600});
  std::vector<float> vertices = {
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f,
      0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
      -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
      -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,

      -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,

      0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
      0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,
      0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f,
      0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

      -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f};
  std::vector<unsigned int> indices = {
      0,  1,  2,  3,  4,  5,

      6,  7,  8,  9,  10, 11,

      12, 13, 14, 15, 16, 17,

      18, 19, 20, 21, 22, 23,

      24, 25, 26, 27, 28, 29,

      30, 31, 32, 33, 34, 35,
  };
  std::shared_ptr<Poly> poly =
      std::make_shared<Poly>(true, vertices, indices, program.window);
  std::shared_ptr<ImageTexture> texture1 =
      std::make_shared<ImageTexture>("assets/img/wall.jpg");
  poly->insert("texture1", texture1);
  std::shared_ptr<ImageTexture> texture2 =
      std::make_shared<ImageTexture>("assets/img/awesomeface.png");
  poly->insert("texture2", texture2);
  poly->program->load_shader("assets/glsl/vertex_coordinate.glsl",
                             GL_VERTEX_SHADER);
  poly->program->load_shader("assets/glsl/fragment_coordinate.glsl",
                             GL_FRAGMENT_SHADER);
  // poly->program->link();
  program.push_back(poly);
  std::vector<float> light_vectices = {
      -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f,
      0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f,

      -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,
      0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,

      -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f,
      -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,

      0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f,
      0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,

      -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,
      0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f,

      -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,
      0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f};
  std::shared_ptr<Light> light =
      std::make_shared<Light>(light_vectices, indices, program.window);
  light->program->load_shader("assets/glsl/vertex_color.glsl",
                              GL_VERTEX_SHADER);
  light->program->load_shader("assets/glsl/fragment_color.glsl",
                              GL_FRAGMENT_SHADER);
  // light->program->link();
  program.push_back(light);
  program.run();
}