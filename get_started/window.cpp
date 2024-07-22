#include <iostream>
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <GLFW/glfw3.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

int main() {
  glfwInit();
  GLFWwindow *window =
      glfwCreateWindow(800, 600, "Hello Window", nullptr, nullptr);
  glfwMakeContextCurrent(window);
  // 只能把glad的命令放置在这个位置,否则会出现无法通过glfw的进程获取版本的问题
  // 我安装了所有的插件,但是不会影响程序的正常运行,vscode的term下运行的程序会缺少decor
  int version = gladLoadGL(glfwGetProcAddress);
  if (!version) {
    std::cout << "Failed to initialize GLAD\n" << version << std::endl;
    return -1;
  } else {
    printf("GL %d.%d\n", GLAD_VERSION_MAJOR(version),
           GLAD_VERSION_MINOR(version));
  }
  glViewport(0, 0, 800, 600);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  while (!glfwWindowShouldClose(window)) {
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
