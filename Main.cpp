// main.cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Include ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Include your custom classes
#include "Shader.h"
#include "Light.h"
#include "Camera.h"
#include "Model.h"

// Include standard libraries
#include <iostream>
#include <vector>
#include <string>
#include <fstream> // For file operations

// Include nlohmann/json for JSON serialization
#include "nlohmann/json.hpp" // Ensure you have this library installed
using json = nlohmann::json;

// stb_image implementation
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera and Cursor State
Camera camera;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f;

// Cursor State
bool cursorDisabled = true; // Start in Camera Mode
bool spacePressedLastFrame = false; // To track spacebar state

// Models
std::vector<Model> models;

// Lights
std::vector<Light> lights;

// Supported model file extensions
const std::vector<std::string> supportedExtensions = { ".obj", ".fbx", ".dae", ".3ds", ".ply", ".glb", ".gltf" };

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadCubemap(std::vector<std::string> faces);
void saveScene(const std::string& filepath);
void loadScene(const std::string& filepath);

// Skybox vertices
float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

// Cube vertices (for optional cube rendering)
float cubeVertices[] = {
    // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

    // Front face
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

    // Left face
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    // Right face
     0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
     0.5f,  0.5f, -0.5f, 1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
     0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
     0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
     0.5f, -0.5f,  0.5f, 1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
     0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

     // Bottom face
    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
     0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
     0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
     0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f,   0.0f, 1.0f,

    // Top face
   -0.5f,  0.5f, -0.5f, 0.0f, 1.0f,  0.0f,    0.0f, 1.0f,
    0.5f,  0.5f , -0.5f,0.0f, 1.0f,  0.0f,    1.0f, 1.0f,
    0.5f,  0.5f ,  0.5f,0.0f, 1.0f,  0.0f,    1.0f, 0.0f,
    0.5f,  0.5f ,  0.5f,0.0f, 1.0f,  0.0f,    1.0f, 0.0f,
   -0.5f,  0.5f ,  0.5f,0.0f, 1.0f,  0.0f,    0.0f, 0.0f,
   -0.5f,  0.5f , -0.5f,0.0f, 1.0f,  0.0f,    0.0f, 1.0f
};


// Function definitions

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum format;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << "\n";
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Prevent seams
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void saveScene(const std::string& filepath)
{
    std::string savePath = "saves/" + filepath; // Prepend 'saves/' to the filepath

    json sceneJson;

    // Save Models
    sceneJson["models"] = json::array();
    for (const auto& model : models)
    {
        json modelJson;
        modelJson["path"] = model.path; // Use 'path' instead of 'directory'
        modelJson["position"] = { model.position.x, model.position.y, model.position.z };
        modelJson["rotation"] = { model.rotation.x, model.rotation.y, model.rotation.z };
        modelJson["scaleFactor"] = { model.scaleFactor.x, model.scaleFactor.y, model.scaleFactor.z };
        sceneJson["models"].push_back(modelJson);
    }

    // Save Lights
    sceneJson["lights"] = json::array();
    for (const auto& light : lights)
    {
        json lightJson;
        lightJson["position"] = { light.position.x, light.position.y, light.position.z };
        lightJson["rotation"] = { light.rotation.x, light.rotation.y, light.rotation.z };
        lightJson["scale"] = { light.scale.x, light.scale.y, light.scale.z };
        lightJson["color"] = { light.color.x, light.color.y, light.color.z };
        lightJson["intensity"] = light.intensity;
        sceneJson["lights"].push_back(lightJson);
    }

    // Write to file
    std::ofstream file(savePath);
    if (file.is_open())
    {
        file << sceneJson.dump(4); // Pretty print with 4 spaces
        file.close();
        std::cout << "Scene saved to " << savePath << std::endl;
    }
    else
    {
        std::cout << "Failed to open file for saving: " << savePath << std::endl;
    }
}

void loadScene(const std::string& filepath)
{
    std::string loadPath = "saves/" + filepath; // Prepend 'saves/' to the filepath

    std::ifstream file(loadPath);
    if (!file.is_open())
    {
        std::cout << "Failed to open file for loading: " << loadPath << std::endl;
        return;
    }

    json sceneJson;
    file >> sceneJson;
    file.close();

    // Clear existing models and lights
    models.clear();
    lights.clear();

    // Load Models
    if (sceneJson.contains("models"))
    {
        for (const auto& modelJson : sceneJson["models"])
        {
            std::string path = modelJson["path"];
            std::cout << "Loading model: " << path << std::endl; // Debugging line

            // Validate the model path ends with a supported extension
            bool valid = false;
            for (const auto& ext : supportedExtensions)
            {
                if (path.size() >= ext.size() &&
                    path.compare(path.size() - ext.size(), ext.size(), ext) == 0)
                {
                    valid = true;
                    break;
                }
            }

            if (!valid)
            {
                std::cout << "Invalid model file extension for path: " << path << ". Skipping this model." << std::endl;
                continue;
            }

            // Check if the file exists
            std::ifstream infile(path);
            if (!infile.good())
            {
                std::cout << "Model file does not exist: " << path << ". Skipping this model." << std::endl;
                continue;
            }

            try
            {
                Model model(path); // Ensure 'path' is the full model file path
                model.position = glm::vec3(modelJson["position"][0], modelJson["position"][1], modelJson["position"][2]);
                model.rotation = glm::vec3(modelJson["rotation"][0], modelJson["rotation"][1], modelJson["rotation"][2]);
                model.scaleFactor = glm::vec3(modelJson["scaleFactor"][0], modelJson["scaleFactor"][1], modelJson["scaleFactor"][2]);
                models.push_back(model);
            }
            catch (const std::exception& e)
            {
                std::cout << "Failed to load model at path: " << path << ". Error: " << e.what() << std::endl;
            }
        }
    }

    // Load Lights
    if (sceneJson.contains("lights"))
    {
        for (const auto& lightJson : sceneJson["lights"])
        {
            Light light;
            light.position = glm::vec3(lightJson["position"][0], lightJson["position"][1], lightJson["position"][2]);
            light.rotation = glm::vec3(lightJson["rotation"][0], lightJson["rotation"][1], lightJson["rotation"][2]);
            light.scale = glm::vec3(lightJson["scale"][0], lightJson["scale"][1], lightJson["scale"][2]);
            light.color = glm::vec3(lightJson["color"][0], lightJson["color"][1], lightJson["color"][2]);
            light.intensity = lightJson["intensity"];
            lights.push_back(light);
        }
    }

    std::cout << "Scene loaded from " << loadPath << std::endl;
}

void processInput(GLFWwindow* window)
{
    // Close window on ESC
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Toggle Cursor and Camera Mode on Spacebar Press
    bool spacePressedThisFrame = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    if (spacePressedThisFrame && !spacePressedLastFrame)
    {
        cursorDisabled = !cursorDisabled; // Toggle cursor state

        if (cursorDisabled)
        {
            // Switch to Camera Mode: Disable cursor and capture it
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true; // Prevent sudden camera jump
        }
        else
        {
            // Switch to UI Mode: Enable cursor
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    spacePressedLastFrame = spacePressedThisFrame;

    // Camera controls only when cursor is disabled (Camera Mode)
    if (cursorDisabled)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (!cursorDisabled)
        return; // Do not process camera movement when in UI Mode

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW\n";
        return -1;
    }
    // OpenGL version 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // For Mac OS X
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Mini Engine", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // Set callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Capture the mouse initially
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    cursorDisabled = true;

    // Initialize GLAD before calling any OpenGL function
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // Default depth function

    // Build and compile shader program
    Shader shader("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
    if (shader.ID == 0)
    {
        std::cout << "Failed to create shader program.\n";
        return -1;
    }

    // Build and compile skybox shader program
    Shader skyboxShader("shaders/skybox_vertex.glsl", "shaders/skybox_fragment.glsl");
    if (skyboxShader.ID == 0)
    {
        std::cout << "Failed to create skybox shader program.\n";
        return -1;
    }

    // Setup cube VAO and VBO (optional if you're not using the cube)
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texture coords attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Skybox VAO and VBO setup
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    // Load skybox textures
    std::vector<std::string> faces
    {
        "resources/textures/skybox/right.png",
        "resources/textures/skybox/left.png",
        "resources/textures/skybox/top.png",
        "resources/textures/skybox/bottom.png",
        "resources/textures/skybox/front.png",
        "resources/textures/skybox/back.png"
    };

    unsigned int cubemapTexture = loadCubemap(faces);
    if (cubemapTexture == 0)
    {
        std::cout << "Failed to load cubemap texture.\n";
        return -1;
    }

    // Shader configuration
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // Setup ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Lighting info
    // Add a default light
    Light defaultLight;
    defaultLight.position = glm::vec3(1.2f, 1.0f, 2.0f);
    defaultLight.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    defaultLight.scale = glm::vec3(1.0f);
    defaultLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
    defaultLight.intensity = 1.0f;
    lights.push_back(defaultLight);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui window for lights
        {
            ImGui::Begin("Lights");

            if (ImGui::Button("Add Light") && lights.size() < 10)
            {
                Light newLight;
                newLight.position = glm::vec3(0.0f);
                newLight.rotation = glm::vec3(0.0f);
                newLight.scale = glm::vec3(1.0f);
                newLight.color = glm::vec3(1.0f);
                newLight.intensity = 1.0f;
                lights.push_back(newLight);
            }

            for (size_t i = 0; i < lights.size(); ++i)
            {
                std::string header = "Light " + std::to_string(i + 1);
                if (ImGui::CollapsingHeader(header.c_str()))
                {
                    // Position
                    ImGui::DragFloat3(("Position##" + std::to_string(i)).c_str(), glm::value_ptr(lights[i].position), 0.1f);
                    // Rotation
                    ImGui::DragFloat3(("Rotation##" + std::to_string(i)).c_str(), glm::value_ptr(lights[i].rotation), 1.0f);
                    // Scale
                    ImGui::DragFloat3(("Scale##" + std::to_string(i)).c_str(), glm::value_ptr(lights[i].scale), 0.1f, 0.1f, 10.0f);
                    // Color
                    ImGui::ColorEdit3(("Color##" + std::to_string(i)).c_str(), glm::value_ptr(lights[i].color));
                    // Intensity
                    ImGui::DragFloat(("Intensity##" + std::to_string(i)).c_str(), &lights[i].intensity, 0.1f, 0.0f, 10.0f);
                    // Delete button
                    if (ImGui::Button(("Delete##" + std::to_string(i)).c_str()))
                    {
                        lights.erase(lights.begin() + i);
                        break;
                    }
                }
            }

            ImGui::End();
        }

        // ImGui window for models
        {
            ImGui::Begin("Model Importer");

            // Input for model file path
            static char modelPath[256] = "";
            ImGui::InputText("Model Path", modelPath, IM_ARRAYSIZE(modelPath));

            // Button to load model
            if (ImGui::Button("Load Model"))
            {
                std::string pathStr = std::string(modelPath);
                if (!pathStr.empty())
                {
                    // Validate the model path ends with a supported extension
                    bool valid = false;
                    for (const auto& ext : supportedExtensions)
                    {
                        if (pathStr.size() >= ext.size() &&
                            pathStr.compare(pathStr.size() - ext.size(), ext.size(), ext) == 0)
                        {
                            valid = true;
                            break;
                        }
                    }

                    if (valid)
                    {
                        // Check if the file exists
                        std::ifstream infile(pathStr);
                        if (infile.good())
                        {
                            try
                            {
                                models.emplace_back(pathStr);
                                std::cout << "Loaded model: " << pathStr << std::endl;
                                // Optionally, clear the input after loading
                                modelPath[0] = '\0';
                            }
                            catch (const std::exception& e)
                            {
                                std::cout << "Failed to load model: " << e.what() << std::endl;
                            }
                        }
                        else
                        {
                            std::cout << "Model file does not exist: " << pathStr << std::endl;
                        }
                    }
                    else
                    {
                        std::cout << "Invalid model file extension. Supported extensions are: .obj, .fbx, .dae, .3ds, .ply, .glb, .gltf" << std::endl;
                    }
                }
            }

            ImGui::Separator();

            // List of loaded models
            for (size_t i = 0; i < models.size(); ++i)
            {
                std::string modelName = "Model " + std::to_string(i + 1);
                if (ImGui::TreeNode(modelName.c_str()))
                {
                    // Display model path
                    ImGui::Text("Path: %s", models[i].path.c_str());

                    // Transformation controls
                    ImGui::DragFloat3(("Position##" + std::to_string(i)).c_str(), glm::value_ptr(models[i].position), 0.1f);
                    ImGui::DragFloat3(("Rotation##" + std::to_string(i)).c_str(), glm::value_ptr(models[i].rotation), 1.0f);
                    ImGui::DragFloat3(("Scale##" + std::to_string(i)).c_str(), glm::value_ptr(models[i].scaleFactor), 0.1f, 0.1f, 10.0f);

                    // Delete button
                    if (ImGui::Button(("Delete##" + std::to_string(i)).c_str()))
                    {
                        models.erase(models.begin() + i);
                        ImGui::TreePop();
                        break;
                    }

                    ImGui::TreePop();
                }
            }

            ImGui::End();
        }

        // ImGui window for Scene Save/Load
        {
            ImGui::Begin("Scene");

            // Input for scene file path
            static char scenePathInput[256] = "test.json"; // Renamed to avoid conflict with 'scenePath' variable
            ImGui::InputText("Scene File Path", scenePathInput, IM_ARRAYSIZE(scenePathInput));

            // Buttons for Save and Load
            if (ImGui::Button("Save Scene"))
            {
                std::string pathStr = std::string(scenePathInput);
                saveScene(pathStr);
            }

            if (ImGui::Button("Load Scene"))
            {
                std::string pathStr = std::string(scenePathInput);
                loadScene(pathStr);
            }

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate shader
        shader.use();

        // View/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        // Set view position
        shader.setVec3("viewPos", camera.Position);

        // Set lights
        shader.setInt("numLights", static_cast<int>(lights.size()));
        for (size_t i = 0; i < lights.size() && i < 10; ++i)
        {
            std::string base = "lights[" + std::to_string(i) + "].";
            shader.setVec3(base + "position", lights[i].position);
            shader.setVec3(base + "rotation", lights[i].rotation);
            shader.setVec3(base + "scale", lights[i].scale);
            shader.setVec3(base + "color", lights[i].color);
            shader.setFloat(base + "intensity", lights[i].intensity);
        }

        // Render all models
        for (auto& model : models)
        {
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, model.position);
            modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(model.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            modelMatrix = glm::scale(modelMatrix, model.scaleFactor);
            shader.setMat4("model", modelMatrix);
            model.Draw(shader);
        }

        // Draw skybox as last
        glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.GetViewMatrix())); // Remove translation from the view matrix
        skyboxShader.setMat4("view", skyboxView);
        skyboxShader.setMat4("projection", projection);
        // Skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // Set depth function back to default

        // Render ImGui on top
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup ImGui and GLFW
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
