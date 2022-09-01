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

void setUpLights();

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



DirLight dirLight;
SpotLight spotLight;
std::vector<PointLight> pointLights;

std::vector<Cube*> cubes;

float cubesSpeed = 1.5f;

float xPandaPosition = 0.0f;

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

    setUpLights();

    /*Shaderi */

    Shader baseShader("resources/shaders/base.vs", "resources/shaders/base.fs");
    Shader lightShader ("resources/shaders/light.vs","resources/shaders/light.fs");
  //  Shader pointShader("resources/shaders/light.vs","resources/shaders/point.fs");
    Shader cubeShader ("resources/shaders/cube.vs","resources/shaders/cube.fs");
    Shader blendShader ("resources/shaders/blending.vs","resources/shaders/blending.fs");
    Shader modelShader ("resources/shaders/model.vs","resources/shaders/model.fs");

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

    glm::vec3 pointLightPositions[] = {
            glm::vec3(0.7f,5.9f,-8.0f),
            glm::vec3(0.5f,10.7f,-4.0f),
            glm::vec3(1.0f,18.8f,-1.0f)
           // glm::vec3(0.0f,7.0f,-4.0f),
           // glm::vec3 (1.0f, 6.0f, -4.0f )


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

    unsigned int lightCubeVAO;
    glGenVertexArrays(1,&lightCubeVAO);

    glBindVertexArray(lightCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE, 8*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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

    /* Generisanje teksture */

    //stbi_set_flip_vertically_on_load(true);

    unsigned int planeTexture = loadTexture("resources/textures/grass2.jpg",true);
    unsigned int cubeTexture = loadTexture("resources/textures/brick.jpg",false);
    unsigned int vegetationTexture = loadTexture("resources/textures/grass.png",false);
    unsigned int goldTexture = loadTexture("resources/textures/gold.jpg",false);

    /* Pravimo kolekciju prepreka i poena*/

    Cube* initialBrick = new Cube(false);
    Cube* initialPoint = new Cube(true);
    cubes.push_back(initialBrick);
    cubes.push_back(initialPoint);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

        /* Point svetlo */
        lightShader.use();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);

        glBindVertexArray(lightCubeVAO);

        for (unsigned int i = 0; i < 2; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            lightShader.setMat4("model",model);
            glDrawArrays(GL_TRIANGLES,0,36);
        }

        glBindVertexArray(cubeVAO);

        cubeShader.use();
        cubeShader.setMat4("projection", projection);
        cubeShader.setMat4("view", view);
        cubeShader.setInt("cubeTexture", 0);
        cubeShader.setInt("goldTexture",1);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cubeTexture);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, goldTexture);

        setUpShaderLights(cubeShader);


        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CW);

        float nearestZ = 0.0f;
        float xPosition;

        /* Poeni i prepreke */
        for(auto it = cubes.begin(); it != cubes.end(); ) {
            xPosition = (*it)->getXCoord();
            float zPosition = (*it)->getZCoord();

            float zNewPosition = zPosition + deltaTime * cubesSpeed;

            if(nearestZ  >= zPosition ) {
                nearestZ = zPosition;
            }

            if(zPosition  >= 0.9f){
                delete *it;
                it = cubes.erase(it);
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

        if(nearestZ > -5.0f) {
            Cube* newCube = new Cube(false);
            cubes.push_back(newCube);
            Cube* newPointCube = new Cube(true);
            cubes.push_back(newPointCube);
        }

        /* Renderovanje modela */
        modelShader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(xPandaPosition, 0.6f, 0.9f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        model = glm::scale(model, glm::vec3(0.007f));

        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        modelShader.setMat4("model", model);
        setUpShaderLights(modelShader);

        pandaModel.Draw(modelShader);



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){

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

void setUpLights(){

    glm::vec3 pointLightPositions[] = {
            glm::vec3(0.7f,5.9f,-8.0f),
            glm::vec3(0.5f,10.7f,-4.0f),
            glm::vec3(1.0f,18.8f,-1.0f)
            //glm::vec3(1.0f,8.8f,-1.0f)
           // glm::vec3(0.0f,7.0f,-4.0f),
           // glm::vec3 (1.0f, 6.0f, -4.0f )


    };

    spotLight.position = glm::vec3(0.0f,1.0f,9.0f);
    spotLight.direction = glm::vec3(0.2f, 0.7f, -3.0f);
    spotLight.ambient = glm::vec3( 0.08f, 0.08f, 0.08f);
    spotLight.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    spotLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    spotLight.constant = 1.0f;
    spotLight.linear = 0.09;
    spotLight.quadratic = 0.032;
    spotLight.cutOff = glm::cos(glm::radians(15.0f));
    spotLight.outerCutOff = glm::cos(glm::radians(30.0f));

    for (int i = 0; i < 3; ++i) {
        PointLight pointLight;
        pointLight.position =pointLightPositions[i];
        pointLight.ambient = glm::vec3(0.05f,0.05f,0.05f);
        pointLight.diffuse = glm::vec3(0.8f,0.8f,0.8f);
        pointLight.specular = glm::vec3(1.0f,1.0f,1.0f);
        pointLight.constant = 1.0f;
        pointLight.linear = 0.09f;
        pointLight.quadratic = 0.032f;


        pointLights.push_back(pointLight);
    }

    dirLight.direction = glm::vec3(0.0f, -10.0f, 0.0f);
    dirLight.ambient = glm::vec3(0.5f, 0.5f, 0.5f);
    dirLight.diffuse = glm::vec3(0.2f, 0.2f, 0.2f);
    dirLight.specular = glm::vec3(0.6f, 0.6f, 0.6f);

}

void setUpShaderLights(Shader shader){
    shader.setVec3("dirLight.direction", dirLight.direction);
    shader.setVec3("dirLight.ambient", dirLight.ambient);
    shader.setVec3("dirLight.diffuse", dirLight.diffuse);
    shader.setVec3("dirLight.specular", dirLight.specular);

    shader.setVec3("spotLight.position", spotLight.position);
    shader.setVec3("spotLight.direction", spotLight.direction);
    shader.setVec3("spotLight.ambient", spotLight.ambient);
    shader.setVec3("spotLight.diffuse", spotLight.diffuse);
    shader.setVec3("spotLight.specular", spotLight.specular);
    shader.setFloat("spotLight.constant", spotLight.constant);
    shader.setFloat("spotLight.linear", spotLight.linear);
    shader.setFloat("spotLight.quadratic", spotLight.quadratic);
    shader.setFloat("spotLight.cutOff", spotLight.cutOff);
    shader.setFloat("spotLight.outerCutOff", spotLight.outerCutOff);

    for (int i = 0; i < 4; i++) {
        shader.setVec3("pointLights[" + std::to_string(i) + "]" + ".position", pointLights[i].position);
        shader.setVec3("pointLights[" + std::to_string(i) + "]" + ".ambient", pointLights[i].ambient);
        shader.setVec3("pointLights[" + std::to_string(i) + "]" + ".diffuse", pointLights[i].diffuse);
        shader.setVec3("pointLights[" + std::to_string(i) + "]" + ".specular", pointLights[i].specular);
        shader.setFloat("pointLights[" + std::to_string(i) + "]" + ".constant", pointLights[i].constant);
        shader.setFloat("pointLights[" + std::to_string(i) + "]" + ".linear", pointLights[i].linear);
        shader.setFloat("pointLights[" + std::to_string(i) + "]" + ".quadratic", pointLights[i].quadratic);



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