#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <iostream>
#include <cmath>

using namespace std;

// ================= SETTINGS =================
const float WAVE_AMP   = 0.12f;
const float WAVE_FREQ  = 2.5f;
const float WAVE_SPEED = 1.8f;

const float CAMERA_SPEED = 3.5f;
const float MOUSE_SENSITIVITY = 0.12f;

// ================= WINDOW SIZE =================
int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;

// ================= CAMERA STATE =================
glm::vec3 cameraPos   = glm::vec3(0.0f, 3.0f, 6.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, -0.25f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw   = -90.0f;
float pitch = -15.0f;

float lastX = 400.0f;
float lastY = 300.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// ================= CALLBACKS =================
void framebuffer_size_callback(GLFWwindow*, int w, int h) {
    SCR_WIDTH = w;
    SCR_HEIGHT = h;
    glViewport(0, 0, w, h);
}

void mouse_callback(GLFWwindow*, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed because screen y goes down
    lastX = xpos;
    lastY = ypos;

    xoffset *= MOUSE_SENSITIVITY;
    yoffset *= MOUSE_SENSITIVITY;

    yaw   += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void processInput(GLFWwindow* window) {
    float cameraSpeed = CAMERA_SPEED * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;

    glm::vec3 right = glm::normalize(glm::cross(cameraFront, cameraUp));

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= right * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += right * cameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// ================= GRID DATA =================
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

// ================= SHADERS =================
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 MVP;
uniform float uTime;
uniform float uAmp;
uniform float uFreq;
uniform float uSpeed;

out vec3 FragPos;
out vec3 Normal;

void main() {
    float wave1 = sin(aPos.x * uFreq + uTime * uSpeed) * uAmp;
    float wave2 = cos(aPos.z * uFreq * 1.2 + uTime * uSpeed * 0.8) * uAmp;

    vec3 pos = aPos;
    pos.y += wave1 + wave2;

    float dx = uFreq * uAmp * cos(aPos.x * uFreq + uTime * uSpeed);
    float dz = -uFreq * uAmp * sin(aPos.z * uFreq * 1.2 + uTime * uSpeed * 0.8);

    vec3 tx = vec3(1.0, dx, 0.0);
    vec3 tz = vec3(0.0, dz, 1.0);

    Normal = normalize(cross(tz, tx));
    FragPos = pos;

    gl_Position = MVP * vec4(pos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightDir;
uniform vec3 cameraPos;

void main() {
    vec3 norm = normalize(Normal);

    float diff = max(dot(norm, normalize(-lightDir)), 0.0);

    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    float fresnel = pow(1.0 - max(dot(viewDir, norm), 0.0), 3.0);

    float heightFactor = clamp(FragPos.y * 2.0 + 0.5, 0.0, 1.0);

    vec3 deepColor = vec3(0.0, 0.18, 0.45);
    vec3 shallowColor = vec3(0.08, 0.55, 0.9);

    vec3 waterColor = mix(deepColor, shallowColor, heightFactor);

    vec3 finalColor = waterColor * (0.25 + 0.75 * diff);
    finalColor += spec * vec3(1.0);
    finalColor += fresnel * vec3(0.65, 0.8, 1.0);

    FragColor = vec4(finalColor, 1.0);
}
)";

// ================= SHADER UTILS =================
unsigned int compileShader(unsigned int type, const char* src) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
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

// ================= MAIN =================
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

    // GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Realistic Water Camera", NULL, NULL);
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    GLFWwindow* window = glfwCreateWindow(
        mode->width,
        mode->height,
        "Water",
        monitor,
        NULL
);
    if (!window) {
        cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD\n";
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    // glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glViewport(0, 0, mode->width, mode->height);

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int shader = createShaderProgram();

    int mvpLoc   = glGetUniformLocation(shader, "MVP");
    int timeLoc  = glGetUniformLocation(shader, "uTime");
    int ampLoc   = glGetUniformLocation(shader, "uAmp");
    int freqLoc  = glGetUniformLocation(shader, "uFreq");
    int speedLoc = glGetUniformLocation(shader, "uSpeed");
    int lightLoc = glGetUniformLocation(shader, "lightDir");
    int camLoc   = glGetUniformLocation(shader, "cameraPos");

    glm::vec3 lightDir = glm::vec3(-0.3f, -1.0f, -0.2f);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.05f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);

        glm::mat4 proj = glm::perspective(
            glm::radians(45.0f),
            (float)mode->width / (float)mode->height,
            0.1f,
            100.0f
        );

        glm::mat4 view = glm::lookAt(
            cameraPos,
            cameraPos + cameraFront,
            cameraUp
        );

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 mvp = proj * view * model;

        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniform1f(timeLoc, currentFrame);
        glUniform1f(ampLoc, WAVE_AMP);
        glUniform1f(freqLoc, WAVE_FREQ);
        glUniform1f(speedLoc, WAVE_SPEED);
        glUniform3fv(lightLoc, 1, glm::value_ptr(lightDir));
        glUniform3fv(camLoc, 1, glm::value_ptr(cameraPos));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}