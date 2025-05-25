#include "Shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode, fragmentCode;
    std::ifstream vShaderFile(vertexPath), fShaderFile(fragmentPath);
    std::stringstream vStream, fStream;
    vStream << vShaderFile.rdbuf(); fStream << fShaderFile.rdbuf();
    vertexCode = vStream.str(); fragmentCode = fStream.str();
    const char* vCode = vertexCode.c_str();
    const char* fCode = fragmentCode.c_str();

    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vCode, nullptr);
    glCompileShader(vert);
    {
        GLint success; char infoLog[512];
        glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vert, 512, NULL, infoLog);
            std::cerr << "VERTEX SHADER COMPILATION FAILED:\n" << infoLog << std::endl;
        }
    }
    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fCode, nullptr);
    glCompileShader(frag);
    {
        GLint success; char infoLog[512];
        glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(frag, 512, NULL, infoLog);
            std::cerr << "FRAGMENT SHADER COMPILATION FAILED:\n" << infoLog << std::endl;
        }
    }
    ID = glCreateProgram();
    glAttachShader(ID, vert);
    glAttachShader(ID, frag);
    // GLSL 1.2: attribute 위치 지정
    glBindAttribLocation(ID, 0, "aPos");
    glBindAttribLocation(ID, 1, "aNormal");
    glLinkProgram(ID);
    {
        GLint success; char infoLog[512];
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(ID, 512, NULL, infoLog);
            std::cerr << "SHADER LINKING FAILED:\n" << infoLog << std::endl;
        }
    }
    glDeleteShader(vert);
    glDeleteShader(frag);
}

void Shader::use() const {
    glUseProgram(ID);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const {
    GLint loc = glGetUniformLocation(ID, name.c_str());
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}
