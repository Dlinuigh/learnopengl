#include "camera.h"
void framebuffer_size_callback(GLFWwindow *, int width, int height) {
  glViewport(0, 0, width, height);
}
void Program::process() {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    if (scale > 0.0f) {
      scale -= 0.01f;
    }
  } else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    if (scale < 1.0f) {
      scale += 0.01f;
    }
  }
  camera->process();
}
void Program::run() {
  program_shader->use();
  // 可以用for处理,但是此处简便
  program_shader->set("texture1", 0);
  program_shader->set("texture2", 1);
  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (auto t = 0; t < (int)textures.size(); t++) {
      textures[t]->activate(t);
    }
    camera->run(program_shader);
    program_shader->set("model", model);
    program_shader->set("scale", scale);
    draw();
    glfwSwapBuffers(window);
    glfwPollEvents();
    process();
  }
}
int main() {
  Program program({800, 600});
  program.program_shader->load_shader("assets/glsl/vertex_coordinate.glsl",
                                      GL_VERTEX_SHADER);
  program.program_shader->load_shader("assets/glsl/fragment_coordinate.glsl",
                                      GL_FRAGMENT_SHADER);
  program.program_shader->link();
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
  std::shared_ptr<Poly> poly = std::make_shared<Poly>(true, vertices, indices);
  program.push_back(poly);
  std::shared_ptr<ImageTexture> texture1 =
      std::make_shared<ImageTexture>("assets/img/wall.jpg");
  program.push_back(texture1);
  std::shared_ptr<ImageTexture> texture2 =
      std::make_shared<ImageTexture>("assets/img/awesomeface.png");
  program.push_back(texture2);
  program.run();
}