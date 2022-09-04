#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "rg/Cube.h"

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>



#include <vector>
#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

void drawImGui();

void resetGame();

void renderQuad();

unsigned int loadCubemap(std::vector<std::string> faces);

void setUpShaderLights(Shader shader);

unsigned int loadTexture(char const* path, bool gammaCorrection);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float cutOff;
    float outerCutOff;
};

//global light
struct DirLight {
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 specular;
    glm::vec3 diffuse;
};

struct PointLight {
    glm::vec3 position;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;

};

/* Podesavanja ProgramState-a */

struct ProgramState {
    bool ImguiEnabled = false;
    DirLight dirLight;
    SpotLight spotLight;
    std::vector<PointLight> pointLights;
    int numOfPointLights = 3;

    bool bloom = true;
    bool bloomKeyPressed = false;
    float exposure = 1.0f;
    float cubesSpeed = 1.5f;
    int score = 0;
    int highScore;

    void setUpLights();

    void SaveToFile(std::string filename);
    void LoadFromFile(std::string filename);


};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << ImguiEnabled << '\n'
        << dirLight.direction.x << '\n'
        << dirLight.direction.y << '\n'
        << dirLight.direction.z << '\n'
        << dirLight.diffuse.x << '\n'
        << dirLight.diffuse.y << '\n'
        << dirLight.diffuse.z << '\n'
         << dirLight.ambient.x << '\n'
         << dirLight.ambient.y << '\n'
         << dirLight.ambient.z << '\n'
         << dirLight.specular.x << '\n'
         << dirLight.specular.y << '\n'
         << dirLight.specular.z << '\n'
         <<spotLight.position.x << '\n'
         <<spotLight.position.y << '\n'
         <<spotLight.position.z << '\n'
          << spotLight.direction.x << '\n'
            << spotLight.direction.y << '\n'
            << spotLight.direction.z << '\n'
            << spotLight.diffuse.x << '\n'
            << spotLight.diffuse.y << '\n'
            << spotLight.diffuse.z << '\n'
            << spotLight.ambient.x << '\n'
            << spotLight.ambient.y << '\n'
            << spotLight.ambient.z << '\n'
            << spotLight.specular.x << '\n'
            << spotLight.specular.y << '\n'
            << spotLight.specular.z << '\n';
    for (int i = 0; i < numOfPointLights; i++) {
        out << pointLights[i].position.x << '\n'
            <<pointLights[i].position.y << '\n'
            <<pointLights[i].position.z << '\n'
            << pointLights[i].diffuse.x << '\n'
            << pointLights[i].diffuse.y << '\n'
            << pointLights[i].diffuse.z << '\n'
            << pointLights[i].ambient.x << '\n'
            << pointLights[i].ambient.y << '\n'
            << pointLights[i].ambient.z << '\n'
            << pointLights[i].specular.x << '\n'
            << pointLights[i].specular.y << '\n'
            << pointLights[i].specular.z << '\n';
    }
    out << bloom << '\n'
        << exposure << '\n'
        << cubesSpeed << '\n'
        << highScore << '\n';

}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream  in(filename);
    if (in) {
        in >>  ImguiEnabled
            >> dirLight.direction.x
            >> dirLight.direction.y
            >> dirLight.direction.z
            >> dirLight.diffuse.x
            >> dirLight.diffuse.y
            >> dirLight.diffuse.z
            >> dirLight.ambient.x
            >> dirLight.ambient.y
            >> dirLight.ambient.z
            >> dirLight.specular.x
            >> dirLight.specular.y
            >> dirLight.specular.z
            >> spotLight.position.x
            >> spotLight.position.y
            >> spotLight.position.z
            >> spotLight.direction.x
            >> spotLight.direction.y
            >> spotLight.direction.z
            >> spotLight.diffuse.x
            >> spotLight.diffuse.y
            >> spotLight.diffuse.z
            >> spotLight.ambient.x
            >> spotLight.ambient.y
            >> spotLight.ambient.z
            >> spotLight.specular.x
            >> spotLight.specular.y
            >> spotLight.specular.z ;
        for (int i = 0; i < numOfPointLights; i++) {
            in >> pointLights[i].position.x
                >> pointLights[i].position.y
                >> pointLights[i].position.z
                >> pointLights[i].diffuse.x
                >> pointLights[i].diffuse.y
                >> pointLights[i].diffuse.z
                >> pointLights[i].ambient.x
                >> pointLights[i].ambient.y
                >> pointLights[i].ambient.z
                >> pointLights[i].specular.x
                >> pointLights[i].specular.y
                >> pointLights[i].specular.z;
        }
        in >> bloom
            >> exposure
            >> cubesSpeed
            >> highScore ;

    }
}

void ProgramState::setUpLights() {

    glm::vec3 pointLightPositions[] = {
            glm::vec3(0.0f,8.0f,-8.0f),
            glm::vec3(0.0f,8.0f,-4.0f),
            glm::vec3(0.0f,8.0f,0.0f)

    };

    spotLight.position = glm::vec3(0.0f,1.5f,1.5f);
    spotLight.direction = glm::vec3(0.2f, 0.7f, -3.0f);
    spotLight.ambient = glm::vec3( 0.08f, 0.08f, 0.08f);
    spotLight.diffuse = glm::vec3(0.2f, 0.2f, 0.2f);
    spotLight.specular = glm::vec3(0.5f, 0.5f, 0.5f);
    spotLight.constant = 1.0f;
    spotLight.linear = 0.09;
    spotLight.quadratic = 0.032;
    spotLight.cutOff = glm::cos(glm::radians(15.0f));
    spotLight.outerCutOff = glm::cos(glm::radians(30.0f));

    for (int i = 0; i < numOfPointLights; ++i) {
        PointLight pointLight;
        pointLight.position =pointLightPositions[i];
        pointLight.ambient = glm::vec3(0.05f,0.05f,0.05f);
        pointLight.diffuse = glm::vec3(0.2f,0.2f,0.2f);
        pointLight.specular = glm::vec3(0.5f,0.5f,0.5f);
        pointLight.constant = 1.0f;
        pointLight.linear = 0.09f;
        pointLight.quadratic = 0.032f;


        pointLights.push_back(pointLight);
    }

    dirLight.direction = glm::vec3(0.0f, -1.0f, 0.0f);
    dirLight.ambient = glm::vec3(0.5f, 0.5f, 0.5f);
    dirLight.diffuse = glm::vec3(0.6f, 0.6f, 0.6f);
    dirLight.specular = glm::vec3(0.2f, 0.2f, 0.2f);

}


ProgramState* programState;
std::vector<Cube*> cubes;


float xPandaPosition = 0.0f;

bool isGameOver = false;


int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Go, Panda, go!", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }



    programState = new ProgramState();
    programState->setUpLights();
    programState ->LoadFromFile("resources/program_state.txt");
    if (programState->ImguiEnabled) {
        glfwSetInputMode(window,GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    ImGui_ImplGlfw_InitForOpenGL(window,true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    /*Shaderi */

    Shader baseShader("resources/shaders/base.vs", "resources/shaders/base.fs");
    Shader cubeShader ("resources/shaders/cube.vs","resources/shaders/cube.fs");
    Shader blendShader ("resources/shaders/blending.vs","resources/shaders/blending.fs");
    Shader modelShader ("resources/shaders/model.vs","resources/shaders/model.fs");
    Shader skyboxShader("resources/shaders/skybox.vs","resources/shaders/skybox.fs");
    Shader blurShader("resources/shaders/blur.vs","resources/shaders/blur.fs");
    Shader finalShader ("resources/shaders/final.vs","resources/shaders/final.fs");


    // configure (floating point) framebuffers
    // ---------------------------------------
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebufer
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }


    /*Modeli*/

    Model pandaModel("resources/objects/panda/scene.gltf");

    float planeVertices[] = {
            //positions - 3f                   //normals - 3f                      //texture coords - 2f
            1.0f,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f,       1.0f, 0.0f,
            1.0f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
            -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,0.0f, 1.0f,
            -1.0f, 0.0f,1.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    };

    unsigned int planeIndices[] = {
            0, 1, 3,
            1, 2, 3
    };


    glEnable(GL_DEPTH_TEST);

    float cubeVertices[] = {
            //back face
            // positions                       // normals                         // texture coords
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

            //front face
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,

            //left face
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,

            //right face
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            //bottom face
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,

            //top face
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };


    float transparentVertices[] = {
            // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };

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

    /*Saljemo podatke o vertexima na graficku */
    unsigned int planeVAO, planeVBO, planeEBO;

    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glGenBuffers(1, &planeEBO);

    glBindVertexArray(planeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndices), planeIndices, GL_STATIC_DRAW);

    /* Znacenje atributa */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    /* Deakiviramo bafer */
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int cubeVBO, cubeVAO;

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData (GL_ARRAY_BUFFER, sizeof(cubeVertices),cubeVertices,GL_STATIC_DRAW);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);


    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    std::vector<std::string> faces
            {
                    "resources/textures/skybox/right.jpg",
                    "resources/textures/skybox/left.jpg",
                   "resources/textures/skybox/top.jpg",
                    "resources/textures/skybox/bottom.jpg",
                    "resources/textures/skybox/front.jpg",
                    "resources/textures/skybox/back.jpg"
            };

    /* Generisanje teksture */



    unsigned int planeTexture = loadTexture("resources/textures/grass2.jpg",true);
    unsigned int cubeTexture = loadTexture("resources/textures/brick.jpg",false);
    unsigned int vegetationTexture = loadTexture("resources/textures/grass.png",false);
    unsigned int cubemapTexture = loadCubemap(faces);




    /* Pravimo kolekciju prepreka i poena*/

    Cube* initialBrick = new Cube(false);
    Cube* initialPoint = new Cube(initialBrick->getXCoord(), true);
    cubes.push_back(initialBrick);
    cubes.push_back(initialPoint);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);
    blurShader.use();
    blurShader.setInt("image", 0);
    finalShader.use();
    finalShader.setInt("scene", 0);
    finalShader.setInt("bloomBlur", 1);


       // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.0f, 0.7f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* 1.RENDER U FB*/
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        baseShader.use();
        baseShader.setInt("planeTexture", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, planeTexture);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH/(float)SCR_HEIGHT, 0.1f, 100.0f);

        baseShader.setMat4("projection", projection);
        baseShader.setMat4("view", view);
        setUpShaderLights(baseShader);

        glBindVertexArray(planeVAO);
        for(unsigned int i = 0; i< 10; i++){
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f,0.0f,-2.0f * i));
            baseShader.setMat4("model", model);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }


        glBindVertexArray(cubeVAO);

        cubeShader.use();
        cubeShader.setMat4("projection", projection);
        cubeShader.setMat4("view", view);
        cubeShader.setInt("cubeTexture", 0);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cubeTexture);


        setUpShaderLights(cubeShader);


        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CW);

        float nearestZ = 0.0f;
        float xPosition;

        /* Poeni i prepreke */
        for(auto it = cubes.begin(); it != cubes.end(); ) {
            xPosition = (*it)->getXCoord();
            float zPosition = (*it)->getZCoord();

            float zNewPosition = zPosition + deltaTime * programState->cubesSpeed;

            if(nearestZ  >= zPosition ) {
                nearestZ = zPosition;
            }

            if((zNewPosition  >= 0.9f && !(*it)->isPoint()) || (zNewPosition  >= 1.0f && (*it)->isPoint()) ){
                delete *it;
                it = cubes.erase(it);
                continue;
            }

            /* Detekcija kolizije */

            // naisli na prepreku
            if(zNewPosition >= 0.6 && xPandaPosition == xPosition && !(*it)->isPoint()){
                cubes.clear();
                isGameOver = true;
                if (programState->highScore < programState->score)
                    programState->highScore = programState->score;
                programState->score = 0;
                break;
            }

            // naisli na poen
            if(zNewPosition >= 0.65 && xPandaPosition == xPosition  && (*it)->isPoint()){
                delete *it;
                it = cubes.erase(it);
                programState->score++;
                std::cerr << "Score " << programState->score <<std::endl;
                continue;
            }

            glm::mat4 model = (*it)->translateCube(xPosition, 0.5f, zNewPosition);

            cubeShader.setMat4("model", model);
            cubeShader.setBool("isPoint",(*it)->isPoint());
            glDrawArrays(GL_TRIANGLES, 0, 36);
            ++it;

        }

        glDisable(GL_CULL_FACE);

        /* Vegetacija */

        glBindVertexArray(transparentVAO);

        blendShader.use();
        blendShader.setMat4("view",view);
        blendShader.setMat4("projection",projection);
        blendShader.setInt("texture1",0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, vegetationTexture);

        setUpShaderLights(blendShader);

        for(auto it = cubes.begin(); it != cubes.end(); it++ ) {
            Cube* cube = *it;
            if (!cube->isPoint()) {
                float xPos = cube->getXCoord();
                float yPos = cube->getYCoord();
                float zPos = cube->getZCoord() + 0.2f;

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(xPos - 0.4, yPos, zPos));
                model = glm::scale(model, glm::vec3(0.6));
                blendShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }

        if(nearestZ > -5.0f && !isGameOver) {
            Cube* newCube = new Cube(false);
            cubes.push_back(newCube);
            Cube* newPointCube = new Cube(newCube->getXCoord(), true);
            cubes.push_back(newPointCube);
        }

        /* Renderovanje modela */

        modelShader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(xPandaPosition, 0.6f, 0.7f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        model = glm::scale(model, glm::vec3(0.007f));

        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        modelShader.setMat4("model", model);
        setUpShaderLights(modelShader);

        pandaModel.Draw(modelShader);


        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);

        skyboxShader.use();

        //eliminisemo translaciju da bi kocka izgledala beskonacno daleko
        skyboxShader.setMat4("view",glm::mat4(glm::mat3(view)));

        skyboxShader.setMat4("projection",projection);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES,0,36);

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        /* 2. Blurujemo bright fragmente */

        bool horizontal = true, first_iteration = true;
        unsigned int amount = 50;
        blurShader.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            blurShader.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
            renderQuad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* 3. Spajamo sve */

        finalShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        finalShader.setInt("bloom", programState->bloom);
        finalShader.setFloat("exposure", programState->exposure);
        renderQuad();

        if(programState->ImguiEnabled){
            drawImGui();
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    programState->SaveToFile("resources/program_state.txt");
    cubes.clear();
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
    if(key == GLFW_KEY_LEFT && action == GLFW_PRESS && xPandaPosition > -0.75f){
        xPandaPosition -= 0.25f;
    }

    if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS && xPandaPosition < 0.75f){
        xPandaPosition += 0.25f;
    }

    if(key == GLFW_KEY_R && action == GLFW_PRESS){
        resetGame();
    }

    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        programState->bloom = !programState->bloom;
    }

    if (key == GLFW_KEY_F5 && action == GLFW_PRESS) {
        programState->ImguiEnabled = !programState->ImguiEnabled;
    }



}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;
}


void resetGame(){
    cubes.clear();
    isGameOver = false;
    programState->score = 0;
}



void setUpShaderLights(Shader shader){

    shader.setInt("numOfPointLights",programState->numOfPointLights);

    shader.setVec3("dirLight.direction", programState->dirLight.direction);
    shader.setVec3("dirLight.ambient", programState->dirLight.ambient);
    shader.setVec3("dirLight.diffuse",programState-> dirLight.diffuse);
    shader.setVec3("dirLight.specular", programState->dirLight.specular);

    shader.setVec3("spotLight.position", programState->spotLight.position);
    shader.setVec3("spotLight.direction", programState->spotLight.direction);
    shader.setVec3("spotLight.ambient", programState->spotLight.ambient);
    shader.setVec3("spotLight.diffuse", programState->spotLight.diffuse);
    shader.setVec3("spotLight.specular", programState->spotLight.specular);
    shader.setFloat("spotLight.constant", programState->spotLight.constant);
    shader.setFloat("spotLight.linear", programState->spotLight.linear);
    shader.setFloat("spotLight.quadratic", programState->spotLight.quadratic);
    shader.setFloat("spotLight.cutOff", programState->spotLight.cutOff);
    shader.setFloat("spotLight.outerCutOff", programState->spotLight.outerCutOff);

    for (int i = 0; i < programState->numOfPointLights; i++) {
        shader.setVec3("pointLights[" + std::to_string(i) + "]" + ".position", programState->pointLights[i].position);
        shader.setVec3("pointLights[" + std::to_string(i) + "]" + ".ambient", programState->pointLights[i].ambient);
        shader.setVec3("pointLights[" + std::to_string(i) + "]" + ".diffuse", programState->pointLights[i].diffuse);
        shader.setVec3("pointLights[" + std::to_string(i) + "]" + ".specular",programState-> pointLights[i].specular);
        shader.setFloat("pointLights[" + std::to_string(i) + "]" + ".constant", programState->pointLights[i].constant);
        shader.setFloat("pointLights[" + std::to_string(i) + "]" + ".linear",programState-> pointLights[i].linear);
        shader.setFloat("pointLights[" + std::to_string(i) + "]" + ".quadratic",programState-> pointLights[i].quadratic);



    }

    shader.setVec3("viewPos", camera.Position);
}

/* funkcija za ucitavanje teksture */
unsigned int loadTexture(char const* path, bool gammaCorrection)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;

    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if(data)
    {
        GLenum internalFormat;
        GLenum dataFormat;

        if(nrChannels == 1)
        {
            internalFormat = dataFormat = GL_RED;
        }
        else if(nrChannels == 3)
        {
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if(nrChannels == 4)
        {
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cerr << "ERROR::TEXTURE failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

/* ucitavanje skyboxa */
unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width,height,nrChannels;
    unsigned char* data;

    for (int i = 0; i < faces.size(); ++i) {
        data = stbi_load(faces[i].c_str(),&width,&height,&nrChannels, 0);
        if (data) {

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X +i,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,data);


        } else {
            std::cerr << "Failed to load cube map texture" << std::endl;
            return -1;
        }

        stbi_image_free(data);
    }

    //podesavamo parametre za cubemap
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);

    return textureID;

}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void drawImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("Settings ");
        ImGui::DragFloat("Exposure", (float *) &programState->exposure, 0.1, 0.1, 10);
        ImGui::Checkbox("Enable Bloom", (bool *) &programState->bloom);
        ImGui::DragFloat("Game level", (float* ) &programState->cubesSpeed,0.1,1.5f,7.0);
        ImGui::Text("Score: %d", programState->score);
        ImGui::Text("Highest score: %d", programState->highScore);
        ImGui::End();
    }
    {
        ImGui::Begin("Direction Light");
        ImGui::DragFloat3("direction", (float *) &(programState->dirLight.direction));
        ImGui::DragFloat3("ambient", (float *) &(programState->dirLight.ambient), 0.02, 0.0);
        ImGui::DragFloat3("diffuse", (float *) &(programState->dirLight.diffuse), 0.02, 0.0);
        ImGui::DragFloat3("specular", (float *) &(programState->dirLight.specular), 0.02, 0.0);
        ImGui::End();
    }
    {
        ImGui::Begin("SpotLight");
        ImGui::DragFloat3("position", (float *) &(programState->spotLight.position));
        ImGui::DragFloat3("direction", (float *) &(programState->spotLight.direction));
        ImGui::DragFloat3("ambient", (float *) &(programState->spotLight.ambient), 0.02, 0.0);
        ImGui::DragFloat3("diffuse", (float *) &(programState->spotLight.diffuse), 0.02, 0.0);
        ImGui::DragFloat3("specular", (float *) &(programState->spotLight.specular), 0.02, 0.0);
        ImGui::DragFloat("constant", (float *) &programState->spotLight.constant, 0.02, 0.0);
        ImGui::DragFloat("linear", (float *) &programState->spotLight.linear, 0.02, 0.0, 1.0);
        ImGui::DragFloat("quadratic", (float *) &programState->spotLight.quadratic, 0.02, 0.0, 1.0);
        ImGui::End();
    }


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}