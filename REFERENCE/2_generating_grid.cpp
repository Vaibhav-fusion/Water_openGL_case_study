
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <iostream>

using namespace std;

// -------- SHADERS --------

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

void main() {
    FragColor = vec4(0.2, 0.7, 1.0, 1.0);
}
)";

// -------- GRID --------

vector<float> vertices;
vector<unsigned int> indices;

void generateGrid(int N, float size) {

    float half = size / 2.0f;

    for (int z = 0; z <= N; z++) {
        for (int x = 0; x <= N; x++) {

            float xPos = -half + size * x / N;
            float zPos = -half + size * z / N;

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

// -------- SHADER UTILS --------

unsigned int createShader() {

    int success;
    char infoLog[512];

    // Vertex
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, NULL);
    glCompileShader(vs);

    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vs, 512, NULL, infoLog);
        cout << "Vertex Shader Error:\n" << infoLog << endl;
    }

    // Fragment
    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, NULL);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fs, 512, NULL, infoLog);
        cout << "Fragment Shader Error:\n" << infoLog << endl;
    }

    // Program
    unsigned int program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        cout << "Shader Link Error:\n" << infoLog << endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

// -------- MAIN --------

int main() {

    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Wireframe Grid", NULL, NULL);
    if (!window) {
        cout << "Failed to create window\n";
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Init GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glEnable(GL_DEPTH_TEST);

    // Generate grid
    generateGrid(50, 5.0f);

    // Buffers
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // Shader
    unsigned int shader = createShader();

    // Wireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // -------- LOOP --------

    while (!glfwWindowShouldClose(window)) {

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);

        // Camera + MVP
        glm::mat4 proj = glm::perspective(glm::radians(45.0f),
                                          800.0f / 600.0f,
                                          0.1f, 100.0f);

        glm::mat4 view = glm::translate(glm::mat4(1.0f),
                                        glm::vec3(0.0f, -1.0f, -6.0f));

        glm::mat4 model = glm::mat4(1.0f);

        glm::mat4 mvp = proj * view * model;

        int loc = glGetUniformLocation(shader, "MVP");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mvp));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}