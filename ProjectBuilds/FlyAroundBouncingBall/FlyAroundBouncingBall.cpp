#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <cmath>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

GLFWwindow* startItUp(int width, int height, std::string Title) {
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return nullptr;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(width, height, Title.c_str(), NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return nullptr;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return nullptr;
    }

    return window;
}

float degreesToRadians(float degrees) {
    return degrees * (3.14159265359f / 180.0f);
}

void computeModelMatrix(const std::map<std::string, float>& transform, float* model) {
    float x = transform.at("x");
    float y = transform.at("y");
    float z = transform.at("z");
    float xRot = transform.at("xRot");
    float yRot = transform.at("yRot");
    float zRot = transform.at("zRot");

    glm::mat4 mat = glm::mat4(1.0f);
    mat = glm::translate(mat, glm::vec3(x, y, z));
    mat = glm::rotate(mat, glm::radians(xRot), glm::vec3(1.0f, 0.0f, 0.0f));
    mat = glm::rotate(mat, glm::radians(yRot), glm::vec3(0.0f, 1.0f, 0.0f));
    mat = glm::rotate(mat, glm::radians(zRot), glm::vec3(0.0f, 0.0f, 1.0f));
    memcpy(model, glm::value_ptr(mat), sizeof(float) * 16);
}

unsigned int createShaderProgram(float r, float g, float b, float a)
{
    const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)glsl";

    // Dynamically create fragment shader source with color values injected
    std::ostringstream fragShaderStream;
    fragShaderStream << std::fixed << std::setprecision(3) <<
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "    FragColor = vec4(" << r << ", " << g << ", " << b << ", " << a << ");\n"
        "}\n";

    std::string fragShaderStr = fragShaderStream.str();
    const char* fragmentShaderSource = fragShaderStr.c_str();
    // Vertex Shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Check vertex shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::VERTEX_SHADER_COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Fragment Shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Check fragment shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::FRAGMENT_SHADER_COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Shader Program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Check linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER_PROGRAM_LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Delete shaders as they are linked now
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void rect(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO, const std::map<std::string, float>& transform, float* model) {
    computeModelMatrix(transform, model);

    float halfWidth = transform.at("width") / 2.0f;
    float halfHeight = transform.at("height") / 2.0f;

    float x = transform.at("x");
    float y = transform.at("y");

    float vertices[] = {
        halfWidth,  halfHeight, 0.0f,  // top right
        halfWidth, -halfHeight, 0.0f,  // bottom right
       -halfWidth, -halfHeight, 0.0f,  // bottom left
       -halfWidth,  halfHeight, 0.0f   // top left
    };

    unsigned int indices[] = {
        0, 1, 3,   // first triangle (top right, bottom right, top left)
        1, 2, 3    // second triangle (bottom right, bottom left, top left)
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // VBO for vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // EBO for indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // vertex attribute pointer (position)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // have rotation


    glBindVertexArray(0); // unbind VAO (optional but good practice)
}

void cube(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO, const std::map<std::string, float>& transform, float* model) {
    computeModelMatrix(transform, model);

    float halfWidth = transform.at("width") / 2.0f;
    float halfHeight = transform.at("height") / 2.0f;
    float halfDepth = transform.at("depth") / 2.0f;

    float x = transform.at("x");
    float y = transform.at("y");
    float z = transform.at("z");

    float vertices[] = {
        -halfWidth,  halfHeight,  halfDepth,  // 0: top-left-front
         halfWidth,  halfHeight,  halfDepth,  // 1: top-right-front
         halfWidth, -halfHeight,  halfDepth,  // 2: bottom-right-front
        -halfWidth, -halfHeight,  halfDepth,  // 3: bottom-left-front

        -halfWidth,  halfHeight, -halfDepth,  // 4: top-left-back
         halfWidth,  halfHeight, -halfDepth,  // 5: top-right-back
         halfWidth, -halfHeight, -halfDepth,  // 6: bottom-right-back
        -halfWidth, -halfHeight, -halfDepth   // 7: bottom-left-back
    };

    unsigned int indices[] = {
        // front face
        0, 1, 2,
        2, 3, 0,
        // right face
        1, 5, 6,
        6, 2, 1,
        // back face
        5, 4, 7,
        7, 6, 5,
        // left face
        4, 0, 3,
        3, 7, 4,
        // top face
        4, 5, 1,
        1, 0, 4,
        // bottom face
        3, 2, 6,
        6, 7, 3
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // VBO for vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // EBO for indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // vertex attribute pointer (position)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0); // unbind VAO (optional but good practice)
}

void drawAndClear(float r, float g, float b, float a, unsigned int shaderProgram, unsigned int VAO, float* model, int indexCount) {
    glClearColor(0.2f, 0.4f, 0.6f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);

    float aspect = 800.0f / 600.0f; // or get it dynamically from window size
    float orthoLeft = -1.0f * aspect;
    float orthoRight = 1.0f * aspect;
    float orthoBottom = -1.0f;
    float orthoTop = 1.0f;

    glm::mat4 projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, -1.0f, 1.0f);


    int projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(VAO);
    // Draw using indices
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model);

    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

int main(void)
{
    GLFWwindow* window = startItUp(800, 600, "OpenGL Setup");
    if (window == nullptr) {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    unsigned int shaderProgram = createShaderProgram(0.0f, 1.0f, 0.0f, 1.0f); // your function

    glm::vec3 cameraPos = glm::vec3(-5.0f, -5.0f, -5.0f); // Camera position
    glm::vec3 cameraTarget = glm::vec3(-0.5f, -0.5f, -0.5f); // Where it's looking
    glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f); // World up direction

    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, upVector);

    std::map<std::string, float> transform = { {"x", -0.5f}, {"y", -0.5f}, {"z", -0.5f}, {"width", 0.5f}, {"height", 0.5f}, {"depth", 0.5f}, {"xRot", 0}, {"yRot", 0}, {"zRot", 0}};

    float model[16];
    unsigned int VAO, VBO, EBO;
    cube(VAO, VBO, EBO, transform, model);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.f);
        int projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        float time = (float)glfwGetTime();
        transform["xRot"] = time * 90.0f;
        computeModelMatrix(transform, model);

        drawAndClear(0.0f, 0.0f, 0.0f, 1.0f, shaderProgram, VAO, model, 50);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}