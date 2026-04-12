#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
using namespace std;

/*
    ===============================
    WINDOW RESIZE HANDLER
    ===============================
    Whenever window size changes,
    we update OpenGL viewport.
*/
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

/*
    ===============================
    SIMPLE SHADER CREATION
    ===============================
*/
unsigned int createShaderProgram() {

    // Vertex Shader (runs per vertex)
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;

        void main() {
            gl_Position = vec4(aPos.x, aPos.z, 0.0, 1.0);
        }
    )";

    // Fragment Shader (runs per pixel)
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;

        void main() {
            FragColor = vec4(0.2, 0.7, 1.0, 1.0);
        }
    )";

    // Compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Compile fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Link shaders into a program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Delete shaders (not needed after linking)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

/*
    ===============================
    MAIN PROGRAM
    ===============================
*/
int main() {

    /*
        -------------------------------
        INIT GLFW (window system)
        -------------------------------
    */
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Water Simulation", NULL, NULL);

    if (!window) {
        cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    /*
        -------------------------------
        LOAD OPENGL (GLAD)
        -------------------------------
    */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    /*
        -------------------------------
        CREATE SHADER
        -------------------------------
    */
    unsigned int shaderProgram = createShaderProgram();

    /*
        ===============================
        GRID GENERATION
        ===============================
        We create a flat surface using many vertices
    */
    int gridSize = 100;
    float spacing = 0.05f;

    vector<float> vertices;
    vector<unsigned int> indices;

    // Generate vertices
    for (int z = 0; z < gridSize; z++) {
        for (int x = 0; x < gridSize; x++) {

            // Normalize to [-1,1] so it fits screen
            float xpos = (x * spacing) - 2.5f;
            float zpos = (z * spacing) - 2.5f;

            vertices.push_back(xpos);   // X
            vertices.push_back(0.0f);   // Y (height)
            vertices.push_back(zpos);   // Z
        }
    }

    // Generate indices (triangles)
    for (int z = 0; z < gridSize - 1; z++) {
        for (int x = 0; x < gridSize - 1; x++) {

            int topLeft = z * gridSize + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * gridSize + x;
            int bottomRight = bottomLeft + 1;

            // Triangle 1
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Triangle 2
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    /*
        ===============================
        SEND DATA TO GPU
        ===============================
    */
    unsigned int VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Tell OpenGL how to read vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    /*
        ===============================
        WIREFRAME MODE
        ===============================
        Makes grid visible clearly
    */
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    /*
        ===============================
        RENDER LOOP
    */
    while (!glfwWindowShouldClose(window)) {

        // Clear screen
        glClearColor(0.05f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw grid
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    /*
        CLEANUP
    */
    glfwTerminate();
    return 0;
}