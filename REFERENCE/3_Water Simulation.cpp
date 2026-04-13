#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <iostream>

using namespace std;

// =======================
// WAVE SETTINGS (TOP)
// =======================
const float WAVE_AMP   = 0.15f;
const float WAVE_FREQ  = 4.5f;
const float WAVE_SPEED = 3.0f;

// =======================
// WINDOW RESIZE
// =======================
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// =======================
// GRID DATA
// =======================
vector<float> vertices;
vector<unsigned int> indices;

void generateGrid(int N, float size) {
    vertices.clear();
    indices.clear();

    float half = size / 2.0f;

    for (int z = 0; z <= N; z++) {
        for (int x = 0; x <= N; x++) {
            float xPos = -half + size * (float)x / (float)N;
            float zPos = -half + size * (float)z / (float)N;

            vertices.push_back(xPos);
            vertices.push_back(0.0f);
            vertices.push_back(zPos);
        }
    }

    for (int z = 0; z < N; z++) {
        for (int x = 0; x < N; x++) {
            int tl = z * (N + 1) + x;
            int tr = tl + 1;
            int bl = (z + 1) * (N + 1) + x;
            int br = bl + 1;

            indices.push_back(tl);
            indices.push_back(bl);
            indices.push_back(tr);

            indices.push_back(tr);
            indices.push_back(bl);
            indices.push_back(br);
        }
    }
}

// =======================
// SHADERS
// =======================
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 MVP;
uniform float uTime;
uniform float uAmp;
uniform float uFreq;
uniform float uSpeed;

void main() {
    float wave1 = sin(aPos.x * uFreq + uTime * uSpeed) * uAmp;
    float wave2 = cos(aPos.z * uFreq + uTime * uSpeed * 0.8) * (uAmp * 0.6);

    vec3 pos = aPos;
    pos.y += wave1 + wave2;

    gl_Position = MVP * vec4(pos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

void main() {
    FragColor = vec4(0.2, 0.7, 1.0, 1.0);
}
)";

// =======================
// SHADER UTILS
// =======================
unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        cout << "Shader compile error:\n" << infoLog << endl;
    }

    return shader;
}

unsigned int createShaderProgram() {
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    unsigned int program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        cout << "Shader link error:\n" << infoLog << endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

// =======================
// MAIN
// =======================
int main() {
    if (!glfwInit()) {
        cout << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(800, 600, "Water Displacement", NULL, NULL);
    if (!window) {
        cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD\n";
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glViewport(0, 0, 800, 600);

    glEnable(GL_DEPTH_TEST);

    // Grid
    generateGrid(100, 5.0f);

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(float),
                 vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(unsigned int),
                 indices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    unsigned int shaderProgram = createShaderProgram();

    int mvpLoc = glGetUniformLocation(shaderProgram, "MVP");
    int timeLoc = glGetUniformLocation(shaderProgram, "uTime");
    int ampLoc  = glGetUniformLocation(shaderProgram, "uAmp");
    int freqLoc = glGetUniformLocation(shaderProgram, "uFreq");
    int speedLoc = glGetUniformLocation(shaderProgram, "uSpeed");

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.05f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        float time = (float)glfwGetTime();
        glUniform1f(timeLoc, time);
        glUniform1f(ampLoc, WAVE_AMP);
        glUniform1f(freqLoc, WAVE_FREQ);
        glUniform1f(speedLoc, WAVE_SPEED);

        glm::mat4 proj = glm::perspective(glm::radians(45.0f),
                                          800.0f / 600.0f,
                                          0.1f, 100.0f);

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, -1.3f, -7.0f));
        view = glm::rotate(view, glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        glm::mat4 model = glm::mat4(1.0f);

        glm::mat4 mvp = proj * view * model;
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}