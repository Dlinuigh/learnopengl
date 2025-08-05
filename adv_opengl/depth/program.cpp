#include "program.h"
void checkError(std::string function) {
  GLenum err = glGetError();
  while (err != GL_NO_ERROR) {
    std::cerr << function << ": " << err << std::endl;
  }
}
void Model::loadModel(std::string path) {
  Assimp::Importer import;
  const aiScene *scene =
      import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
    return;
  }
  directory = path.substr(0, path.find_last_of('/'));
  processNode(scene->mRootNode, scene);
}
std::shared_ptr<Mesh> Model::processMesh(aiMesh *mesh, const aiScene *scene) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;
    vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y,
                                mesh->mVertices[i].z);
    vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y,
                              mesh->mNormals[i].z);
    vertex.TexCoords = (mesh->HasTextureCoords(0))
                           ? glm::vec2(mesh->mTextureCoords[0][i].x,
                                       mesh->mTextureCoords[0][i].y)
                           : glm::vec2(0.0f);
    vertices.push_back(vertex);
  }
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      indices.push_back(face.mIndices[j]);
  }
  aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
  std::vector<std::string> diffuseMaps =
      loadMaterialTextures(material, aiTextureType_DIFFUSE);
  std::vector<std::string> specularMaps =
      loadMaterialTextures(material, aiTextureType_SPECULAR);
  std::vector<std::string> normalMaps =
      loadMaterialTextures(material, aiTextureType_HEIGHT);
  std::vector<std::string> heightMaps =
      loadMaterialTextures(material, aiTextureType_AMBIENT);
  return std::make_shared<Mesh>(vertices, indices, diffuseMaps, specularMaps,
                                normalMaps, heightMaps);
}
void Model::processNode(aiNode *node, const aiScene *scene) {
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(processMesh(mesh, scene));
  }
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene);
  }
}
std::vector<std::string> Model::loadMaterialTextures(aiMaterial *mat,
                                                     aiTextureType type) {
  std::vector<std::string> textures;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    TextureMgr &mgr = TextureMgr::getInstance();
    mat->GetTexture(type, i, &str);
    if (!mgr.has(str.C_Str())) {
      std::string filename = directory + "/" + str.C_Str();
      auto texture = std::make_shared<Texture>(filename.c_str(), type);
      mgr.set(str.C_Str(), texture);
    }
    textures.push_back(str.C_Str());
  }
  return textures;
}
void Model::set(glm::mat4 &v, glm::mat4 &p, glm::mat4 &model, glm::vec3 &cp) {
  program->set("view", v);
  program->set("projection", p);
  program->set("model", model);
}
void Model::activate() {
  for (auto &m : meshes) {
    m->activate(program);
  }
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
  model->process(window);
  for (auto &l : light_src) {
    l->process(window);
  }
  camera->process(window);
}
void Program::run() {
  model->link();
  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 m = glm::mat4(1.0f);
    model->use();
    model->set(camera->view, camera->projection, m, camera->cameraPos);
    model->activate();
    model->draw();
    glfwSwapBuffers(window);
    glfwPollEvents();
    process();
    glUseProgram(0);
    checkError("run");
  }
}
int main() {
  Program program({800, 600});
  auto shader = std::make_shared<ShaderProgram>();
  shader->load_shader("assets/glsl/model/vertex.glsl", GL_VERTEX_SHADER);
  shader->load_shader("assets/glsl/model/fragment.glsl", GL_FRAGMENT_SHADER);
  auto model = std::make_shared<Model>("assets/model/backpack/backpack.obj");
  model->program = std::move(shader);
  program.model = std::move(model);
  program.run();
}