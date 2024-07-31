#include "basic.h"
void checkError(const char *function) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << function << ": " << err << std::endl;
  }
}
void framebuffer_size_callback(GLFWwindow *, int width, int height) {
  glViewport(0, 0, width, height);
}
void Program::process() {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  for (auto &it : children) {
    it->process();
  }
  // for (auto &it : light_src) {
  //   it->process();
  // }
  light_src->process();
  camera->process();
}
void Program::run() {
  for (auto &it : children) {
    it->program->link();
  }
  // for(auto &t: light_src){
  //   t->program->link();
  // }
  light_src->program->link();
  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera->run();
    for (auto &it : children) {
      it->program->use();
      it->set("view", camera->get("view"));
      it->set("projection", camera->get("projection"));
      it->activate_textures();
      it->set("light_color", light_src->light_color);
      it->set("lightPos", light_src->lightPos);
      it->set("viewPos", camera->cameraPos);
      it->set();
      it->draw();
    }
    // for (auto &it : light_src) {
    //   it->program->use();
    //   it->set("view", camera->get("view"));
    //   it->set("projection", camera->get("projection"));
    //   it->set();
    //   it->draw();
    // }
    light_src->program->use();
    light_src->set("view", camera->get("view"));
    light_src->set("projection", camera->get("projection"));
    light_src->set();
    light_src->draw();
    glfwSwapBuffers(window);
    glfwPollEvents();
    process();
  }
  checkError("run");
}
int main() {
  Program program({800, 600});
  std::vector<float> vertices = //
      {
          -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, //
          0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,  //
          0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,   //
          0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,   //
          -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f,  //
          -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, //
          //
          -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, //
          0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  //
          0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,   //
          0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,   //
          -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,  //
          -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, //

          -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,   //
          -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,  //
          -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, //
          -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, //
          -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  //
          -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,   //
          //
          0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,   //
          0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  //
          0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, //
          0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, //
          0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  //
          0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,   //
          //
          -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, //
          0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,  //
          0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,   //
          0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,   //
          -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,  //
          -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, //
          //
          -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, //
          0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,  //
          0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   //
          0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   //
          -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  //
          -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f  //
      };
  std::vector<unsigned int> indices = {
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
      18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
  };
  std::shared_ptr<Mesh> poly =
      std::make_shared<Mesh>(vertices, indices, program.window);
  std::shared_ptr<ImageTexture> texture1 =
      std::make_shared<ImageTexture>("assets/img/wall.jpg");
  poly->insert("texture1", texture1);
  std::shared_ptr<ImageTexture> texture2 =
      std::make_shared<ImageTexture>("assets/img/awesomeface.png");
  poly->insert("texture2", texture2);
  poly->program->load_shader("assets/glsl/vertex_basic.glsl", GL_VERTEX_SHADER);
  poly->program->load_shader("assets/glsl/fragment_basic.glsl",
                             GL_FRAGMENT_SHADER);
  program.push_back(poly);
  std::vector<float> light_vectices = {
      -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,
      0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
      0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
      -0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f,
      0.5f,  -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
      0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  -0.5f,
      0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,
      0.5f,  0.5f,  -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f,
      0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f,
      -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,
      0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f};
  std::shared_ptr<Light> light =
      std::make_shared<Light>(light_vectices, indices, program.window);
  light->program->load_shader("assets/glsl/vertex_color.glsl",
                              GL_VERTEX_SHADER);
  light->program->load_shader("assets/glsl/fragment_color_light.glsl",
                              GL_FRAGMENT_SHADER);
  // program.push_back(light);
  program.light_src = std::move(light);
  program.run();
}