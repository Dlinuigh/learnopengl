#include "caster.h"
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
  light_src->process();
  camera->process();
}
void Program::run() {
  for (auto &it : children) {
    it->program->link();
  }
  light_src->program->link();
  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (auto &it : children) {
      it->program->use();
      it->program->set("view", camera->view);
      it->program->set("projection", camera->projection);
      it->activate_textures();
      it->program->set("material.shininess", 64.0f);
      it->program->set("light.position", light_src->lightPos);
      it->program->set("viewPos", camera->cameraPos);
      it->program->set("light.ambient", light_src->ambient);
      it->program->set("light.diffuse", light_src->diffuse);
      it->program->set("light.specular", light_src->specular);
      it->program->set("light.direction", light_src->light_type == 0
                                              ? light_src->direction
                                              : camera->cameraFront);
      it->program->set("light_type", light_src->light_type);
      it->program->set("light.constant", light_src->constant);
      it->program->set("light.linear", light_src->linear);
      it->program->set("light.quadratic", light_src->quadratic);
      it->program->set("light.cutOff", light_src->cutOff);
      it->program->set("light.outerCutoff", light_src->outerCutoff);
      it->set();
      it->draw();
    }
    light_src->program->use();
    light_src->program->set("view", camera->view);
    light_src->program->set("projection", camera->projection);
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
  glm::vec3 cubePositions[] = {
      glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
      glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
      glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
      glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
      glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f)};
  for (unsigned int i = 0; i < 10; i++) {
    std::shared_ptr<Poly> poly =
        std::make_shared<Poly>(vertices, indices, program.window);
    std::shared_ptr<ImageTexture> texture1 =
        std::make_shared<ImageTexture>("assets/img/container2.png");
    poly->insert("material.diffuse", texture1);
    std::shared_ptr<ImageTexture> texture2 =
        std::make_shared<ImageTexture>("assets/img/container2_specular.png");
    poly->insert("material.specular", texture2);
    poly->program->load_shader("assets/glsl/vertex_caster.glsl",
                               GL_VERTEX_SHADER);
    poly->program->load_shader("assets/glsl/fragment_caster.glsl",
                               GL_FRAGMENT_SHADER);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, cubePositions[i]);
    float angle = 20.0f * i;
    model =
        glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

    poly->model = model;
    program.push_back(poly);
  }
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
  light->direction = glm::vec3(-0.2f, -1.0f, -0.3f);
  light->diffuse = glm::vec4(0.8f);
  light->ambient = glm::vec4(0.1f);
  light->specular = glm::vec4(1.0f);
  light->constant = 1.0f;
  light->linear = 0.09f;
  light->quadratic = 0.032f;
  light->cutOff = glm::cos(glm::radians(12.5f));
  light->outerCutoff = glm::cos(glm::radians(17.5f));
  program.light_src = std::move(light);
  program.run();
}