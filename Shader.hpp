// File: Shader.hpp
#ifndef SHADER_HPP
#define SHADER_HPP
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class Shader {
public:
    GLuint ID;
    Shader(const char* vertexPath, const char* fragmentPath);
    void use() const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
};
#endif
