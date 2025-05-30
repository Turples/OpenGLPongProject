#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

ma_engine engine;

// callback functions 

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void playSound(const char* soundPath) {
    ma_engine_play_sound(&engine, soundPath, NULL);
}

int randomInt(int min, int max) {
    std::random_device rd;                          // Seed
    std::mt19937 gen(rd());                         // Mersenne Twister RNG
    std::uniform_int_distribution<> distr(min, max);   // Range: [1, 10]

    return distr(gen);
}

float randomFloat(float min, float max) {
    std::random_device rd; // seed
    std::mt19937 gen(rd()); // Mersenne Twister RNG
    std::uniform_real_distribution<float> dis(min, max); // range [0.0, 1.0)

    return dis(gen);
}

float Clamp(float value, float min, float max) {
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

class myCoolOpenGLApp {
public:
    GLFWwindow* window = nullptr;
    int windowWidth = 1920;
    int windowHeight = 1280;
    float deltaTime = 0.0f; // Time between current and last frame
    float lastFrame = 0.0f; // Time of last frame

	int init() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);


        window = glfwCreateWindow(mode->width, mode->height, "PING PANG THE SECOND", monitor, nullptr);
        if (window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return -1;
        }

        windowWidth = mode->width;
        windowHeight = mode->height;

        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return -1;
        }

        glViewport(0, 0, windowWidth, windowHeight);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glEnable(GL_DEPTH_TEST);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 460");
        io.IniFilename = nullptr;

        if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
            std::cerr << "Failed to initialize audio engine." << std::endl;
            return -1;
        }

        return 0;
	}

    void makeDeltaTime() {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
    }

    void mainLoop(std::function<void()> Input, std::function<void()> Render) {
        while (!glfwWindowShouldClose(window))
        {
            Input();
            Render();
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
};

class basicGraphicalThings {
public:
    unsigned int VBO = 0;
    unsigned int vertexShader = 0;
    unsigned int VAO = 0;
    unsigned int fragmentShader = 0;
    unsigned int EBO = 0;
    unsigned int shaderProgram = 0;

    void createVBO(std::vector<float> vertices) {
        glGenBuffers(1, &VBO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    }

    int createVertexShader(bool ThreeDs) {
        std::stringstream ss;
        if (ThreeDs) {
            ss << "#version 460 core\n"
                << "layout (location = 0) in vec3 aPos;\n"
                << "\n"
                << "uniform mat4 projection;\n"
                << "uniform mat4 view;\n"
                << "uniform mat4 model;\n"
                << "\n"
                << "void main()\n"
                << "{\n"
                << "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
                << "}";
        }
        else {
            ss << "#version 460 core\n"
                << "layout (location = 0) in vec3 aPos;\n"
                << "void main()\n"
                << "{\n"
                << "    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);"
                << "}";
        }
        std::string cool = ss.str();
        const char* vertexShaderSource = cool.c_str();

        vertexShader = glCreateShader(GL_VERTEX_SHADER);

        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        int  success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
            return -1;
        }

        return 0;
    }

    int createFragmentShader(float r, float g, float b, float a) {
        std::stringstream ss;
        ss << "#version 460 core\n"
            << "out vec4 FragColor;\n"
            << "void main()\n"
            << "{\n"
            << "    FragColor = vec4(" << r << ", " << g << ", " << b << ", " << a << "); "
            << "}\n";
        std::string cool = ss.str();
        const char* fragmentShaderSource = cool.c_str();

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        int  success;
        char infoLog[512];
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
            return -1;
        }

        return 0;
    }

    int createShaderProgram() {
        if (vertexShader == 0) {
            std::cout << "Vertex Shader hasn't been setup yet!";
            return -1;
        }
        if (fragmentShader == 0) {
            std::cout << "Fragment Shader hasn't been setup yet!";
            return -1;
        }

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        int  success;
        char infoLog[512];
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            return -1;
        }

        return 0;
    }

    void createVAO() {
        if (VBO == 0 || EBO == 0) {
            std::cerr << "VBO or EBO not initialized!\n";
            return;
        }
        glGenVertexArrays(1, &VAO);
        // 1. bind Vertex Array Object
        glBindVertexArray(VAO);
        // 2. copy our vertices array in a buffer for OpenGL to use
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        // 3. then set our vertex attributes pointers
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
        glEnableVertexAttribArray(0);
    }

    void createEBO(std::vector<unsigned int> indices) {
        glGenBuffers(1, &EBO);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    }

    void cleanUp() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteProgram(shaderProgram);
    }
};

class verticesAndIndicesForShapes {
public:
    void PositionsForRectangle(std::vector<float>& vertices, std::vector<unsigned int>& indices, float x, float y, float width, float height) {
        float halfWidth = width / 2;
        float halfHeight = height / 2;
        vertices.assign({
            x + halfWidth,  y + halfHeight, 0.0f,  // top right
            x + halfWidth, y - halfHeight, 0.0f,   // bottom right
            x - halfWidth, y - halfHeight, 0.0f,   // bottom left
            x - halfWidth,  y + halfHeight, 0.0f   // top left 
        });

        indices.assign({
            0, 1, 3,
            1, 2, 3
        });
    }

    void PositionsForCube(std::vector<float>& vertices, std::vector<unsigned int>& indices, float x, float y, float z, float width, float height, float depth) {
        float halfWidth = width / 2;
        float halfHeight = height / 2;
        float halfDepth = depth / 2;
        vertices.assign({
            x + halfWidth,  y + halfHeight, z - halfDepth,
            x + halfWidth, y - halfHeight, z - halfDepth,
            x - halfWidth, y - halfHeight, z - halfDepth,
            x - halfWidth,  y + halfHeight, z - halfDepth,

            x + halfWidth,  y + halfHeight, z + halfDepth,
            x + halfWidth, y - halfHeight, z + halfDepth,
            x - halfWidth, y - halfHeight, z + halfDepth,
            x - halfWidth,  y + halfHeight, z + halfDepth
            });

        indices.assign({
            // Front face (z+)
            4, 5, 6,
            4, 6, 7,

            // Back face (z-)
            0, 3, 2,
            0, 2, 1,

            // Left face (x-)
            3, 7, 6,
            3, 6, 2,

            // Right face (x+)
            0, 1, 5,
            0, 5, 4,

            // Top face (y+)
            0, 4, 7,
            0, 7, 3,

            // Bottom face (y-)
            1, 2, 6,
            1, 6, 5
            });
    }
};

class Camera {
public:
    GLFWwindow* window = nullptr;
    float windowWidth = 0;
    float windowHeight = 0;
    // cool camera variables

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f); // Initial position
    glm::vec3 cameraDir = glm::vec3(0.0f, 0.0f, -1.0f); // forward
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);   // Up direction

    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 view = glm::mat4(1.0f);

    double lastX = 0;
    double lastY = 0;
    float yaw = -90.0f;
    float pitch = 0.0f;
    bool firstMouse = true;
    bool cursorLocked = false;

    glm::mat4 getProjection() {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspectRatio = width / (float)height;
        return glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    }

    void cameraMovement(float deltaTime) {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !cursorLocked) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            cursorLocked = true;

            int width, height;
            glfwGetWindowSize(window, &width, &height);
            glfwSetCursorPos(window, width / 2.0, height / 2.0);
            firstMouse = true;
        }

        // Unlock on Escape
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && cursorLocked) {
            glm::vec3 n = cameraDir;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            cursorLocked = false;
            cameraDir = n;
        }

        // Key Movement
        glm::vec3 forward = glm::normalize(glm::vec3(cameraDir.x, cameraDir.y, cameraDir.z));
        glm::vec3 right = glm::normalize(glm::cross(forward, cameraUp));
        float speed = 1.0f * deltaTime;

        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            cameraPos.y += speed;

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            cameraPos.y -= speed;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += forward * speed;

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= forward * speed;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= right * speed;

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += right * speed;

        // Mouse Position Looking

        if (cursorLocked) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            if (firstMouse) {
                lastX = xpos;
                lastY = ypos;
                firstMouse = false;
            }

            float xoffset = xpos - lastX;
            float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top

            lastX = xpos;
            lastY = ypos;

            float sensitivity = 0.1f;
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            yaw += xoffset;
            pitch += yoffset;

            // Clamp pitch
            if (pitch > 89.0f) pitch = 89.0f;
            if (pitch < -89.0f) pitch = -89.0f;

            glm::vec3 direction;
            direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            direction.y = sin(glm::radians(pitch));
            direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            cameraDir = glm::normalize(direction);
        }

        view = glm::lookAt(cameraPos, cameraPos + cameraDir, cameraUp);
    }

    void setPosition(glm::vec3 position, glm::vec3 lookAt) {
        view = glm::lookAt(position, lookAt, cameraUp);
    }

    void setCameraThings(unsigned int shaderProgram) {
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    }

    Camera(GLFWwindow* window, float windowWidth, float windowHeight) {
        this->window = window;
        this->windowWidth = windowWidth;
        this->windowHeight = windowHeight;

        lastX = windowWidth / 2.0f;
        lastY = windowHeight / 2.0f;
        projection = getProjection();
        view = glm::lookAt(cameraPos, cameraPos + cameraDir, cameraUp);
    }
};

class renderCube {
public:
    basicGraphicalThings BGT;
    verticesAndIndicesForShapes VAIFS;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 position;
    glm::vec3 scale;

    int setup(float r, float g, float b, float a, glm::vec3 position, glm::vec3 WDH) {
        this->position = position;
        scale = WDH;

        VAIFS.PositionsForCube(vertices, indices, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

        BGT.createVBO(vertices);
        BGT.createEBO(indices);
        BGT.createVAO();

        if (BGT.createVertexShader(true) != 0)
            return -1;
        if (BGT.createFragmentShader(r, g, b, a) != 0)
            return -1;
        if (BGT.createShaderProgram() != 0)
            return -1;

        return 0;
    }

    void render() {
        glUseProgram(BGT.shaderProgram);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, scale);

        GLuint modelLoc = glGetUniformLocation(BGT.shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(BGT.VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    }
};

void resetCube(float& speedX, float& speedZ, renderCube& bouncingCube) {
    bouncingCube.position = glm::vec3(0.0f, -0.575f, 1.25f);

    const float maxSpeed = 2.0f;
    const float minXSpeed = 0.5f;
    const float maxXSpeed = 1.5f;

    do {
        speedX = randomFloat(-maxSpeed, maxSpeed);
    } while (std::abs(speedX) < minXSpeed || std::abs(speedX) > maxXSpeed);

    float remainingSpeed = sqrt(maxSpeed * maxSpeed - speedX * speedX);
    // Randomize direction of speedZ for variety
    speedZ = (randomFloat(0.0f, 1.0f) > 0.5f) ? remainingSpeed : -remainingSpeed;
}

class mainScreen {
public:
    void Input(myCoolOpenGLApp &App, renderCube &LeftPlayer, renderCube &RightPlayer, ImFont* smallFont) {
        static bool wireframeOn = false;     // Must be static to persist
        static bool rPressed = false;

        if (glfwGetKey(App.window, GLFW_KEY_W) == GLFW_PRESS)
            LeftPlayer.position.z += 0.75f * App.deltaTime;
        if (glfwGetKey(App.window, GLFW_KEY_S) == GLFW_PRESS)
            LeftPlayer.position.z -= 0.75f * App.deltaTime;

        if (glfwGetKey(App.window, GLFW_KEY_UP) == GLFW_PRESS)
            RightPlayer.position.z += 0.75f * App.deltaTime;
        if (glfwGetKey(App.window, GLFW_KEY_DOWN) == GLFW_PRESS)
            RightPlayer.position.z -= 0.75f * App.deltaTime;

        LeftPlayer.position.z = Clamp(LeftPlayer.position.z, 0.6f, 1.9f);
        RightPlayer.position.z = Clamp(RightPlayer.position.z, 0.6f, 1.9f);

        if (glfwGetKey(App.window, GLFW_KEY_R) == GLFW_PRESS) {
            if (!rPressed) {
                wireframeOn = !wireframeOn;
                rPressed = true;
            }
        }
        else {
            rPressed = false;
        }

        (wireframeOn ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(190, 60));
        ImGui::PushFont(smallFont);
        ImGui::Begin("Settings");
        ImGui::Checkbox("Wireframe (Press R)", &wireframeOn);
        ImGui::PopFont();
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Render(basicGraphicalThings &BGT, myCoolOpenGLApp &App, Camera &camera, renderCube &backgroundCube, renderCube &TopCube, renderCube &BottomCube, renderCube &RightCube, renderCube &LeftCube, renderCube &BouncingCube, renderCube &LeftPlayer, renderCube &RightPlayer, float &speedX, float &speedZ, ImFont* &bigFont, ImFont* &smallFont, int &leftPlayerScore, int &rightPlayerScore, int &screenOn, bool &Timed) {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        if (App.windowWidth != viewport[2] || App.windowHeight != viewport[3]) {
            App.windowWidth = viewport[2];
            App.windowHeight = viewport[3];

            camera.projection = camera.getProjection();
            glViewport(0, 0, App.windowWidth, App.windowHeight);
        }

        App.makeDeltaTime();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static bool hasBouncedX = false;
        static bool hasBouncedZ = false;

        static int TimerValue = 3;
        static bool timerStarted = false;
        static float timerStartTime = 0.0f;

        if (Timed) {
            if (!timerStarted) {
                playSound("./sounds/countdown.wav");
                timerStartTime = glfwGetTime();
                timerStarted = true;
                BouncingCube.position = glm::vec3(0.0f, -0.575f, 1.25f);
            }

            float currentTime = glfwGetTime();
            float elapsed = currentTime - timerStartTime;
            int TimerValue = 3 - floor(elapsed);

            if (TimerValue <= 0) {
                resetCube(speedX, speedZ, BouncingCube);
                Timed = false;
                timerStarted = false;
                TimerValue = 3;
            }

            ImGui::SetNextWindowBgAlpha(0.0f); // Fully transparent background
            ImGui::SetNextWindowPos(ImVec2(0.0f, 100.0f), ImGuiCond_Always);
            ImGui::Begin("Timer", nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoInputs |
                ImGuiWindowFlags_NoBackground); // Make it look like just text

            ImGui::PushFont(bigFont);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 1.0f, 1.0f));
            float textWidth = ImGui::CalcTextSize(std::to_string(TimerValue).c_str()).x;

            ImGui::SetCursorPosX((App.windowWidth - textWidth) * 0.5f); // Center horizontally
            ImGui::Text("%s", std::to_string(TimerValue).c_str());
            ImGui::PopStyleColor();
            ImGui::PopFont();
            ImGui::End();
        }
        else {

            BouncingCube.position.x += speedX * App.deltaTime;
            BouncingCube.position.z += speedZ * App.deltaTime;

            bool leftPlayerHit = (
                BouncingCube.position.x - (BouncingCube.scale.x / 2) < LeftPlayer.position.x + (LeftPlayer.scale.x / 2) &&
                BouncingCube.position.x + (BouncingCube.scale.x / 2) > LeftPlayer.position.x - (LeftPlayer.scale.x / 2) &&
                BouncingCube.position.z - (BouncingCube.scale.z / 2) < LeftPlayer.position.z + (LeftPlayer.scale.z / 2) &&
                BouncingCube.position.z + (BouncingCube.scale.z / 2) > LeftPlayer.position.z - (LeftPlayer.scale.z / 2)
                );

            bool rightPlayerHit = (
                BouncingCube.position.x - (BouncingCube.scale.x / 2) < RightPlayer.position.x + (RightPlayer.scale.x / 2) &&
                BouncingCube.position.x + (BouncingCube.scale.x / 2) > RightPlayer.position.x - (RightPlayer.scale.x / 2) &&
                BouncingCube.position.z - (BouncingCube.scale.z / 2) < RightPlayer.position.z + (RightPlayer.scale.z / 2) &&
                BouncingCube.position.z + (BouncingCube.scale.z / 2) > RightPlayer.position.z - (RightPlayer.scale.z / 2)
                );

            static float paddleHitCooldown = 0.0f;
            const float paddleHitDelay = 0.1f; // seconds

            if (paddleHitCooldown > 0.0f)
                paddleHitCooldown -= App.deltaTime;

            if (leftPlayerHit) {
                if (paddleHitCooldown <= 0.0f) {
                    playSound("./sounds/dink.wav");
                    paddleHitCooldown = paddleHitDelay;
                }
                    
                // Calculate overlap for left player separately
                float overlapXLeft = 0.0f;
                if (BouncingCube.position.x < LeftPlayer.position.x) {
                    overlapXLeft = (BouncingCube.position.x + BouncingCube.scale.x / 2) - (LeftPlayer.position.x - LeftPlayer.scale.x / 2);
                }
                else {
                    overlapXLeft = (LeftPlayer.position.x + LeftPlayer.scale.x / 2) - (BouncingCube.position.x - BouncingCube.scale.x / 2);
                }

                float overlapZLeft = 0.0f;
                if (BouncingCube.position.z < LeftPlayer.position.z) {
                    overlapZLeft = (BouncingCube.position.z + BouncingCube.scale.z / 2) - (LeftPlayer.position.z - LeftPlayer.scale.z / 2);
                }
                else {
                    overlapZLeft = (LeftPlayer.position.z + LeftPlayer.scale.z / 2) - (BouncingCube.position.z - BouncingCube.scale.z / 2);
                }

                // Reflect speeds based on which overlap is smaller (collision axis)
                if (overlapXLeft < overlapZLeft && speedX > 0 && !hasBouncedX) {  // The cube is moving towards the left player (speedX > 0)
                    (speedX > 0 ? speedX += 0.1f : speedX -= 0.1f);
                    speedX = -speedX;
                    hasBouncedX = true;
                }
                else if (overlapZLeft < overlapXLeft && !hasBouncedZ) {
                    (speedZ > 0 ? speedZ += 0.1f : speedZ -= 0.1f);
                    speedZ = -speedZ;
                    hasBouncedZ = true;
                }
            }

            if (rightPlayerHit) {
                if (paddleHitCooldown <= 0.0f) {
                    playSound("./sounds/dink.wav");
                    paddleHitCooldown = paddleHitDelay;
                }
                    
                // Calculate overlap for right player separately
                float overlapXRight = 0.0f;
                if (BouncingCube.position.x < RightPlayer.position.x) {
                    overlapXRight = (BouncingCube.position.x + BouncingCube.scale.x / 2) - (RightPlayer.position.x - RightPlayer.scale.x / 2);
                }
                else {
                    overlapXRight = (RightPlayer.position.x + RightPlayer.scale.x / 2) - (BouncingCube.position.x - BouncingCube.scale.x / 2);
                }

                float overlapZRight = 0.0f;
                if (BouncingCube.position.z < RightPlayer.position.z) {
                    overlapZRight = (BouncingCube.position.z + BouncingCube.scale.z / 2) - (RightPlayer.position.z - RightPlayer.scale.z / 2);
                }
                else {
                    overlapZRight = (RightPlayer.position.z + RightPlayer.scale.z / 2) - (BouncingCube.position.z - BouncingCube.scale.z / 2);
                }

                if (overlapXRight < overlapZRight && speedX < 0 && !hasBouncedX) {  // Moving toward right player
                    (speedX > 0 ? speedX += 0.1f : speedX -= 0.1f);
                    speedX = -speedX;
                    hasBouncedX = true;
                }
                else if (overlapZRight < overlapXRight && !hasBouncedZ) {
                    (speedZ > 0 ? speedZ += 0.1f : speedZ -= 0.1f);
                    speedZ = -speedZ;
                    hasBouncedZ = true;
                }
            }

            if (!leftPlayerHit && !rightPlayerHit) {
                hasBouncedX = false;
                hasBouncedZ = false;
            }

            // Calculate overlaps on Z axis

            if (BouncingCube.position.z > 1.975f && speedZ > 0) {
                speedZ = -speedZ;
            }
            if (BouncingCube.position.z < 0.525f && speedZ < 0) {
                speedZ = -speedZ;
            }

            if (BouncingCube.position.x < -1.225) {
                leftPlayerScore++;
                Timed = true;
                playSound("./sounds/getPoint.wav");
            }

            if (BouncingCube.position.x > 1.225) {
                rightPlayerScore++;
                Timed = true;
                playSound("./sounds/getPoint.wav");
            }

            if (leftPlayerScore > 8) {
                screenOn = 1;
                playSound("./sounds/win.wav");
            }
                
            if (rightPlayerScore > 8) {
                screenOn = 2;
                playSound("./sounds/win.wav");
            }
        }

        backgroundCube.render();
        camera.setCameraThings(backgroundCube.BGT.shaderProgram);
        TopCube.render();
        camera.setCameraThings(TopCube.BGT.shaderProgram);
        BottomCube.render();
        camera.setCameraThings(BottomCube.BGT.shaderProgram);
        RightCube.render();
        camera.setCameraThings(RightCube.BGT.shaderProgram);
        LeftCube.render();
        camera.setCameraThings(LeftCube.BGT.shaderProgram);
        BouncingCube.render();
        camera.setCameraThings(BouncingCube.BGT.shaderProgram);
        RightPlayer.render();
        camera.setCameraThings(RightPlayer.BGT.shaderProgram);
        LeftPlayer.render();
        camera.setCameraThings(LeftPlayer.BGT.shaderProgram);

        ImGui::SetNextWindowBgAlpha(0.0f); // Fully transparent background
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(App.windowWidth, App.windowHeight)); // Fullscreen
        ImGui::Begin("OverlayLeft", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoBackground); // Make it look like just text

        ImGui::PushFont(bigFont);
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255)); // Red color (RGBA)

        ImVec2 textSizeLeft = ImGui::CalcTextSize(std::to_string(leftPlayerScore).c_str());

        ImGui::SetCursorPos(ImVec2(
            ((App.windowWidth - textSizeLeft.x) * 0.5f) - 200.0f,
            150.0f
        ));
        ImGui::Text("%s", std::to_string(leftPlayerScore).c_str());

        ImGui::PopStyleColor();
        ImGui::PopFont();
        ImGui::End();

        ImGui::SetNextWindowBgAlpha(0.0f); // Fully transparent background
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(App.windowWidth, App.windowHeight)); // Fullscreen
        ImGui::Begin("OverlayRight", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoBackground); // Make it look like just text

        ImGui::PushFont(bigFont);
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255)); // Red color (RGBA)
        
        ImVec2 textSizeRight = ImGui::CalcTextSize(std::to_string(rightPlayerScore).c_str());

        ImGui::SetCursorPos(ImVec2(
            ((App.windowWidth - textSizeRight.x) * 0.5f) + 200.0f,
            150.0f
        ));
        ImGui::Text("%s", std::to_string(rightPlayerScore).c_str());

        ImGui::PopStyleColor();
        ImGui::PopFont();
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
};

class startScreen {
public:
    void Render(myCoolOpenGLApp &App, ImFont* &bigFont, ImFont* &mediumFont, int &screenOn) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(App.windowWidth, App.windowHeight));
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::Begin("Main Screen", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs); // Make it look like just text
        ImGui::End();

        ImGui::SetNextWindowBgAlpha(0.0f); // Fully transparent background
        ImGui::SetNextWindowPos(ImVec2(0.0f, 100.0f), ImGuiCond_Always);
        ImGui::Begin("Title", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoBackground); // Make it look like just text

        ImGui::PushFont(bigFont);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        const char* text = "PING PANG THE SECOND";
        float textWidth = ImGui::CalcTextSize(text).x;

        ImGui::SetCursorPosX((App.windowWidth - textWidth) * 0.5f); // Center horizontally
        ImGui::Text("%s", text);
        ImGui::PopStyleColor();
        ImGui::PopFont();

        ImGui::End();

        ImGui::SetNextWindowBgAlpha(0.0f); // Fully transparent background
        ImGui::SetNextWindowPos(ImVec2(0.0f, App.windowHeight / 2), ImGuiCond_Always);
        ImGui::Begin("Start Button", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBackground); // Make it look like just text

        ImGui::PushFont(mediumFont);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        float buttonWidth = 500;

        ImGui::SetCursorPosX((App.windowWidth - buttonWidth) * 0.5f); // Center horizontally
        if (ImGui::Button("Start Game", ImVec2(buttonWidth, 100))) {
            screenOn = 0;
        }
        ImVec2 p = ImGui::GetItemRectMin(); // Top-left of last item (button)
        ImVec2 q = ImGui::GetItemRectMax(); // Bottom-right of last item

        ImGui::GetWindowDrawList()->AddRect(p, q, IM_COL32(0, 255, 0, 255), 0.0f, 0, 5.0f);

        ImGui::PopStyleColor();
        ImGui::PopFont();

        ImGui::End();

        ImGui::SetNextWindowBgAlpha(0.0f); // Fully transparent background
        ImGui::SetNextWindowPos(ImVec2(0.0f, (App.windowHeight / 2) + 120), ImGuiCond_Always);
        ImGui::Begin("Quit Button", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBackground); // Make it look like just text

        ImGui::PushFont(mediumFont);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        float buttonWidthQuit = 500;

        ImGui::SetCursorPosX((App.windowWidth - buttonWidthQuit) * 0.5f); // Center horizontally
        if (ImGui::Button("Quit Game", ImVec2(buttonWidthQuit, 100))) {
            glfwSetWindowShouldClose(App.window, GL_TRUE);
        }
        ImVec2 pQuit = ImGui::GetItemRectMin(); // Top-left of last item (button)
        ImVec2 qQuit = ImGui::GetItemRectMax(); // Bottom-right of last item

        ImGui::GetWindowDrawList()->AddRect(pQuit, qQuit, IM_COL32(0, 255, 0, 255), 0.0f, 0, 5.0f);

        ImGui::PopStyleColor();
        ImGui::PopFont();

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
};

class leftPlayerWins {
public:
    void Render(myCoolOpenGLApp& App, ImFont*& bigFont, ImFont*& mediumFont, int& screenOn, int &leftPlayerScore, int &rightPlayerScore, float& speedX, float& speedZ, renderCube& bouncingCube) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(App.windowWidth, App.windowHeight));
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::Begin("Main Screen", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs); // Make it look like just text
        ImGui::End();

        ImGui::SetNextWindowBgAlpha(0.0f); // Fully transparent background
        ImGui::SetNextWindowPos(ImVec2(0.0f, 100.0f), ImGuiCond_Always);
        ImGui::Begin("Title", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoBackground); // Make it look like just text

        ImGui::PushFont(bigFont);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        const char* text = "Left Player Wins!";
        float textWidth = ImGui::CalcTextSize(text).x;

        ImGui::SetCursorPosX((App.windowWidth - textWidth) * 0.5f); // Center horizontally
        ImGui::Text("%s", text);
        ImGui::PopStyleColor();
        ImGui::PopFont();

        ImGui::End();

        ImGui::SetNextWindowBgAlpha(0.0f); // Fully transparent background
        ImGui::SetNextWindowPos(ImVec2(0.0f, App.windowHeight / 2), ImGuiCond_Always);
        ImGui::Begin("Play Again", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBackground); // Make it look like just text

        ImGui::PushFont(mediumFont);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        float buttonWidth = 500;

        ImGui::SetCursorPosX((App.windowWidth - buttonWidth) * 0.5f); // Center horizontally
        if (ImGui::Button("Play Again", ImVec2(buttonWidth, 100))) {
            screenOn = 0;
            leftPlayerScore = 0;
            rightPlayerScore = 0;
            resetCube(speedX, speedZ, bouncingCube);
        }
        ImVec2 p = ImGui::GetItemRectMin(); // Top-left of last item (button)
        ImVec2 q = ImGui::GetItemRectMax(); // Bottom-right of last item

        ImGui::GetWindowDrawList()->AddRect(p, q, IM_COL32(0, 255, 0, 255), 0.0f, 0, 5.0f);

        ImGui::PopStyleColor();
        ImGui::PopFont();

        ImGui::End();

        ImGui::SetNextWindowBgAlpha(0.0f); // Fully transparent background
        ImGui::SetNextWindowPos(ImVec2(0.0f, (App.windowHeight / 2) + 120), ImGuiCond_Always);
        ImGui::Begin("Quit Button", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBackground); // Make it look like just text

        ImGui::PushFont(mediumFont);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        float buttonWidthQuit = 500;

        ImGui::SetCursorPosX((App.windowWidth - buttonWidthQuit) * 0.5f); // Center horizontally
        if (ImGui::Button("Quit Game", ImVec2(buttonWidthQuit, 100))) {
            glfwSetWindowShouldClose(App.window, GL_TRUE);
        }
        ImVec2 pQuit = ImGui::GetItemRectMin(); // Top-left of last item (button)
        ImVec2 qQuit = ImGui::GetItemRectMax(); // Bottom-right of last item

        ImGui::GetWindowDrawList()->AddRect(pQuit, qQuit, IM_COL32(0, 255, 0, 255), 0.0f, 0, 5.0f);

        ImGui::PopStyleColor();
        ImGui::PopFont();

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
};

class rightPlayerWins {
public:
    void Render(myCoolOpenGLApp& App, ImFont*& bigFont, ImFont*& mediumFont, int& screenOn, int& leftPlayerScore, int& rightPlayerScore, float& speedX, float& speedZ, renderCube& bouncingCube) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(App.windowWidth, App.windowHeight));
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::Begin("Main Screen", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs); // Make it look like just text
        ImGui::End();

        ImGui::SetNextWindowBgAlpha(0.0f); // Fully transparent background
        ImGui::SetNextWindowPos(ImVec2(0.0f, 100.0f), ImGuiCond_Always);
        ImGui::Begin("Title", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoBackground); // Make it look like just text

        ImGui::PushFont(bigFont);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        const char* text = "Right Player Wins!";
        float textWidth = ImGui::CalcTextSize(text).x;

        ImGui::SetCursorPosX((App.windowWidth - textWidth) * 0.5f); // Center horizontally
        ImGui::Text("%s", text);
        ImGui::PopStyleColor();
        ImGui::PopFont();

        ImGui::End();

        ImGui::SetNextWindowBgAlpha(0.0f); // Fully transparent background
        ImGui::SetNextWindowPos(ImVec2(0.0f, App.windowHeight / 2), ImGuiCond_Always);
        ImGui::Begin("Play Again", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBackground); // Make it look like just text

        ImGui::PushFont(mediumFont);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        float buttonWidth = 500;

        ImGui::SetCursorPosX((App.windowWidth - buttonWidth) * 0.5f); // Center horizontally
        if (ImGui::Button("Play Again", ImVec2(buttonWidth, 100))) {
            screenOn = 0;
            leftPlayerScore = 0;
            rightPlayerScore = 0;
            resetCube(speedX, speedZ, bouncingCube);
        }
        ImVec2 p = ImGui::GetItemRectMin(); // Top-left of last item (button)
        ImVec2 q = ImGui::GetItemRectMax(); // Bottom-right of last item

        ImGui::GetWindowDrawList()->AddRect(p, q, IM_COL32(0, 255, 0, 255), 0.0f, 0, 5.0f);

        ImGui::PopStyleColor();
        ImGui::PopFont();

        ImGui::End();

        ImGui::SetNextWindowBgAlpha(0.0f); // Fully transparent background
        ImGui::SetNextWindowPos(ImVec2(0.0f, (App.windowHeight / 2) + 120), ImGuiCond_Always);
        ImGui::Begin("Quit Button", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBackground); // Make it look like just text

        ImGui::PushFont(mediumFont);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        float buttonWidthQuit = 500;

        ImGui::SetCursorPosX((App.windowWidth - buttonWidthQuit) * 0.5f); // Center horizontally
        if (ImGui::Button("Quit Game", ImVec2(buttonWidthQuit, 100))) {
            glfwSetWindowShouldClose(App.window, GL_TRUE);
        }
        ImVec2 pQuit = ImGui::GetItemRectMin(); // Top-left of last item (button)
        ImVec2 qQuit = ImGui::GetItemRectMax(); // Bottom-right of last item

        ImGui::GetWindowDrawList()->AddRect(pQuit, qQuit, IM_COL32(0, 255, 0, 255), 0.0f, 0, 5.0f);

        ImGui::PopStyleColor();
        ImGui::PopFont();

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
};

int main()
{
    myCoolOpenGLApp App;
    if (App.init() != 0)
        return -1;
    glfwSetWindowUserPointer(App.window, &App);

    basicGraphicalThings BGT;
    verticesAndIndicesForShapes VAIFS;
    Camera camera(App.window, App.windowWidth, App.windowHeight);
    mainScreen MAINSCREEN;
    startScreen STARTSCREEN;
    leftPlayerWins LEFTSCREEN;
    rightPlayerWins RIGHTSCREEN;

    // the screens are Main, Left Wins, Right Wins, and Start.
    int screenOn = 3;

    renderCube backgroundCube;
    backgroundCube.setup(0.0f, 1.0f, 0.0f, 1.0f, glm::vec3(0.0f, -0.75f, 1.25f), glm::vec3(3.0f, 0.1f, 2.0f));

    renderCube TopCube;
    TopCube.setup(0.0f, 0.0f, 0.0f, 1.0f, glm::vec3(0.0f, -0.75f, 2.25f), glm::vec3(3.0f, 0.3f, 0.3f));

    renderCube BottomCube;
    BottomCube.setup(0.0f, 0.0f, 0.0f, 1.0f, glm::vec3(0.0f, -0.75f, 0.25f), glm::vec3(3.0f, 0.3f, 0.3f));

    renderCube RightCube;
    RightCube.setup(0.0f, 0.0f, 0.0f, 1.0f, glm::vec3(1.5f, -0.75f, 1.25f), glm::vec3(0.3f, 0.3f, 2.3f));

    renderCube LeftCube;
    LeftCube.setup(0.0f, 0.0f, 0.0f, 1.0f, glm::vec3(-1.5f, -0.75f, 1.25f), glm::vec3(0.3f, 0.3f, 2.3f));

    renderCube BouncingCube;
    BouncingCube.setup(0.0f, 0.0f, 1.0f, 1.0f, glm::vec3(0.0f, -0.575f, 1.25f), glm::vec3(0.25f, 0.25f, 0.25f));

    renderCube RightPlayer;
    RightPlayer.setup(1.0f, 0.0f, 0.0f, 1.0f, glm::vec3(-1.0, -0.75, 1.25), glm::vec3(0.10f, 0.25f, 0.40f));

    renderCube LeftPlayer;
    LeftPlayer.setup(1.0f, 0.0f, 0.0f, 1.0f, glm::vec3(1.0, -0.75, 1.25), glm::vec3(0.10f, 0.25f, 0.40f));

    float speedX;
    float speedZ;
    int leftPlayerScore = 0;
    int rightPlayerScore = 0;

    bool Timed = true;

    ImGuiIO& io = ImGui::GetIO();
    ImFont* smallFont = io.Fonts->AddFontFromFileTTF("fonts\\VCR_OSD_MONO_1.001.ttf", 12.0f); // 32 px
    ImFont* bigFont = io.Fonts->AddFontFromFileTTF("fonts\\VCR_OSD_MONO_1.001.ttf", 120.0f); // 32 px
    ImFont* mediumFont = io.Fonts->AddFontFromFileTTF("fonts\\VCR_OSD_MONO_1.001.ttf", 60.0f); // 32 px

    camera.setPosition(glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    App.mainLoop(
        [&App, &LeftPlayer, &RightPlayer, &MAINSCREEN, &screenOn, &smallFont] {
            if (screenOn == 0)
                MAINSCREEN.Input(App, LeftPlayer, RightPlayer, smallFont);
        },
        [&BGT, &App, &camera, &backgroundCube, &TopCube, &BottomCube, &RightCube, &LeftCube,
        &BouncingCube, &LeftPlayer, &RightPlayer, &speedX, &speedZ, &bigFont, &smallFont,
        &leftPlayerScore, &rightPlayerScore, &MAINSCREEN, &STARTSCREEN, &LEFTSCREEN, &RIGHTSCREEN,
        &screenOn, &mediumFont, &Timed] {

            if (screenOn == 3)
                STARTSCREEN.Render(App, bigFont, mediumFont, screenOn);
            if (screenOn == 1)
                LEFTSCREEN.Render(App, bigFont, mediumFont, screenOn, leftPlayerScore, rightPlayerScore, speedX, speedZ, BouncingCube);
            if (screenOn == 2)
                RIGHTSCREEN.Render(App, bigFont, mediumFont, screenOn, leftPlayerScore, rightPlayerScore, speedX, speedZ, BouncingCube);
            if (screenOn == 0)
                MAINSCREEN.Render(BGT, App, camera,
                    backgroundCube, TopCube, BottomCube,
                    RightCube, LeftCube, BouncingCube,
                    LeftPlayer, RightPlayer, speedX, speedZ,
                    bigFont, smallFont, leftPlayerScore, rightPlayerScore, screenOn, Timed);
        }
    );

    // App Clean Up
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ma_engine_uninit(&engine);
    BGT.cleanUp();
    glfwTerminate();
}