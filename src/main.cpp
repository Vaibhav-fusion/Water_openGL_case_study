#include <glad/glad.h>
#include <GLFW/glfw3.h>
// #include <iostream>
// #include <bits.h>
#include <bits/stdc++.h>
using namespace std;


// temp shader 

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

void main() {
    gl_Position = vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0); // orange color
}
)";
// 


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    // std::cout << "Starting program...\n";

    if (!glfwInit()) {
        cout << "GLFW init failed\n";
        return -1;
    }

    cout << "GLFW initialized\n";

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Setup", NULL, NULL);

    if (window == NULL) {
        cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    cout << "Window created\n";

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD\n";
        return -1;
    }

    cout << "GLAD initialized\n";

    while (!glfwWindowShouldClose(window)) {

        glClearColor(1.0f, 0.0f, 1.0f, 1.0f); // RED screen
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}