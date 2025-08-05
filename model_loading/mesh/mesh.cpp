#include "mesh.h"
void checkError(const char *function) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << function << ": " << err << std::endl;
  }
}
void framebuffer_size_callback(GLFWwindow *, int width, int height) {
  glViewport(0, 0, width, height);
}
void Program::set_light(std::shared_ptr<ShaderProgram> &program) {
  int i = 0;
  for (auto &l : light_src) {
    std::string prefix = "lights[" + std::to_string(i);
    program->set((prefix + std::string("].position")).c_str(), l->lightPos);
    program->set((prefix + std::string("].ambient")).c_str(), l->ambient);
    program->set((prefix + std::string("].diffuse")).c_str(), l->diffuse);
    program->set((prefix + std::string("].specular")).c_str(), l->specular);
    program->set((prefix + std::string("].direction")).c_str(),
                 l->light_type == 0 ? l->direction : camera->cameraFront);
    program->set((prefix + std::string("].light_type")).c_str(), l->light_type);
    program->set((prefix + std::string("].constant")).c_str(), l->constant);
    program->set((prefix + std::string("].linear")).c_str(), l->linear);
    program->set((prefix + std::string("].quadratic")).c_str(), l->quadratic);
    program->set((prefix + std::string("].cutOff")).c_str(), l->cutOff);
    program->set((prefix + std::string("].outerCutoff")).c_str(),
                 l->outerCutoff);
    i++;
  }
}
void Program::process() {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  for (auto &it : children) {
    it->process();
  }
  for (auto &l : light_src) {
    l->process();
  }
  camera->process();
}
void Program::run() {
  for (auto &it : children) {
    it->program->link();
  }
  for (auto &l : light_src) {
    l->program->link();
  }
  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (auto &it : children) {
      it->program->use();
      it->program->set("view", camera->view);
      it->program->set("projection", camera->projection);
      it->activate_textures();
      it->program->set("material.shininess", 64.0f);
      set_light(it->program);
      it->program->set("viewPos", camera->cameraPos);
      it->set();
      it->draw();
    }
    for (auto &l : light_src) {
      if (l->light_type != 0) {
        l->program->use();
        l->program->set("view", camera->view);
        l->program->set("projection", camera->projection);
        l->set();
        l->draw();
      }
    }
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
    std::shared_ptr<Mesh> poly =
        std::make_shared<Mesh>(vertices, indices, program.window);
    std::shared_ptr<ImageTexture> texture1 =
        std::make_shared<ImageTexture>("assets/img/container2.png");
    poly->insert("material.diffuse", texture1);
    std::shared_ptr<ImageTexture> texture2 =
        std::make_shared<ImageTexture>("assets/img/container2_specular.png");
    poly->insert("material.specular", texture2);
    poly->program->load_shader("assets/glsl/vertex_multi.glsl",
                               GL_VERTEX_SHADER);
    poly->program->load_shader("assets/glsl/fragment_multi.glsl",
                               GL_FRAGMENT_SHADER);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, cubePositions[i]);
    float angle = 20.0f * i;
    model =
        glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

    poly->model = model;
    program.push_back(poly);
  }
  glm::vec3 pointLightPositions[] = {
      glm::vec3(0.7f, 0.2f, 2.0f), glm::vec3(2.3f, -3.3f, -4.0f),
      glm::vec3(-4.0f, 2.0f, -12.0f), glm::vec3(0.0f, 0.0f, -3.0f)};
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
  for (auto &p : pointLightPositions) {
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
    light->lightPos = p;
    light->light_type = rand()%3;
    program.light_src.push_back(std::move(light));
  }
  program.run();
}