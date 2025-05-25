// File: main.cpp
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include "Shader.hpp"

// OpenGL object IDs
static GLuint VAO, VBO, NBO;
static Shader* shader;

// Camera control
static float cameraAngleX = 20.0f, cameraAngleY = -30.0f;
static bool isDragging = false;
static int lastX = 0, lastY = 0;
static float cameraDistance = 60.0f;

// --- OBJ loader storage ---
static std::vector<glm::vec3> temp_vertices;
static std::vector<unsigned int> vertex_indices;
static std::vector<float> vertices;  // VBO
static std::vector<float> normals;   // NBO

// --- [1] OBJ parsing + smooth normal calculation + center alignment ---
void loadOBJ(const char* path) {
    temp_vertices.clear();
    std::vector<glm::vec3> temp_normals;
    std::vector<unsigned int> vertex_indices, normal_indices;
    vertices.clear();
    normals.clear();

    std::ifstream file(path);
    if (!file) { std::cerr << "Failed to open OBJ: " << path << "\n"; exit(-1); }
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string p; iss >> p;
        if (p == "v") {
            glm::vec3 v; iss >> v.x >> v.y >> v.z;
            temp_vertices.push_back(v);
        } else if (p == "vn") {
            glm::vec3 n; iss >> n.x >> n.y >> n.z;
            temp_normals.push_back(n);
        } else if (p == "f") {
            // 정점 인덱스 쿼드(4개) 또는 삼각형(3개) 모두 지원
            std::vector<std::string> tokens;
            std::string tok;
            while (iss >> tok) tokens.push_back(tok);
            if (tokens.size() < 3) continue;
            // 삼각형 (0,1,2)
            for (int t = 1; t < tokens.size() - 1; ++t) {
                for (int k = 0; k < 3; ++k) {
                    std::string& s = (k == 0) ? tokens[0] : (k == 1) ? tokens[t] : tokens[t+1];
                    unsigned int vi = 0, ni = 0;
                    if (sscanf(s.c_str(), "%u/%*u/%u", &vi, &ni) == 2 ||
                        sscanf(s.c_str(), "%u//%u", &vi, &ni) == 2) {
                        vertex_indices.push_back(vi - 1);
                        normal_indices.push_back(ni - 1);
                    } else if (sscanf(s.c_str(), "%u", &vi) == 1) {
                        vertex_indices.push_back(vi - 1);
                        normal_indices.push_back(vi - 1);
                    }
                }
            }
        }
    }
    // 버텍스/노멀 쌍 배열 채우기
    for (size_t i = 0; i < vertex_indices.size(); ++i) {
        glm::vec3 v = temp_vertices[vertex_indices[i]];
        glm::vec3 n = temp_normals.size() > 0 ? temp_normals[normal_indices[i]] : glm::vec3(0,1,0);
        vertices.push_back(v.x);
        vertices.push_back(v.y);
        vertices.push_back(v.z);
        normals.push_back(n.x);
        normals.push_back(n.y);
        normals.push_back(n.z);
    }
    // 모델 중심 정렬
    glm::vec3 center(0.0f);
    size_t vCount = vertices.size() / 3;
    for (size_t i = 0; i < vertices.size(); i += 3)
        center += glm::vec3(vertices[i], vertices[i+1], vertices[i+2]);
    center /= float(vCount);
    for (size_t i = 0; i < vertices.size(); i += 3) {
        vertices[i]   -= center.x;
        vertices[i+1] -= center.y;
        vertices[i+2] -= center.z;
    }
}


// --- [2] Mouse callbacks ---
void mouseButton(int b, int s, int x, int y) {
    if (b == GLUT_LEFT_BUTTON) {
        isDragging = (s == GLUT_DOWN);
        lastX = x; lastY = y;
    }
}
void mouseMotion(int x, int y) {
    if (!isDragging) return;
    cameraAngleY += (x - lastX) * 0.3f;
    cameraAngleX += (y - lastY) * 0.3f;
    cameraAngleX = glm::clamp(cameraAngleX, -89.0f, 89.0f);
    lastX = x; lastY = y;
    glutPostRedisplay();
}

// --- [3] Initialization ---
void initGL() {
    loadOBJ("model.obj");
    shader = new Shader("shader.vertx", "shader.frag");

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &NBO);
    glBindVertexArray(VAO);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // NBO
    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size()*sizeof(float), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);

    shader->use();
    glUniform3f(glGetUniformLocation(shader->ID, "lightDir"), 1.0f, 1.0f, -1.0f);
    glUniform3f(glGetUniformLocation(shader->ID, "objectColor"), 1.0f, 1.0f, 1.0f);

    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
}

// --- [4] Render loop ---
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader->use();

    // [apply scale, rotate, translate]
    float t = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model,    glm::vec3(1.2f));                     // scaling
    model = glm::rotate(model,   t, glm::vec3(0.0f, 1.0f, 0.0f));     // rotation
    model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));      // translation

    float rx = glm::radians(cameraAngleX),
          ry = glm::radians(cameraAngleY);
    glm::vec3 camPos(
        cameraDistance * cos(rx) * sin(ry),
        cameraDistance * sin(rx),
        cameraDistance * cos(rx) * cos(ry)
    );
    glm::mat4 view = glm::lookAt(camPos,
                                 glm::vec3(0.0f),
                                 glm::vec3(0,1,0));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f),
                                      800.0f/600.0f,
                                      0.1f, 100.0f);

    shader->setMat4("model",      model);
    shader->setMat4("view",       view);
    shader->setMat4("projection", proj);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size()/3);
    glBindVertexArray(0);

    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Smooth-Shaded Viewer");

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW init failed\n";
        return -1;
    }

    initGL();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
