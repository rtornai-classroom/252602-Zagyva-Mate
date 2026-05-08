#include <vector>
#include <cmath>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const int VAOCount = 2;
const int BOCount = 4;
const int ProgramCount = 1;
const int TextureCount = 1;

#include "common.cpp"

GLchar windowTitle[] = "Zagyva Mate Jozsef";

// Kamera paraméterek
float cameraR = 9.0f;     // Sugár
float cameraAngle = 0.0f;
float cameraZ = 0.0f;
bool lightOn = true; 

GLfloat cubeVertices[] = {
    -0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,   0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,   0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
     0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,  -0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,  -0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
    -0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,   0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
     0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,  -0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,  -0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,  -0.5f, 0.5f,-0.5f, -1.0f, 0.0f, 0.0f,  -0.5f,-0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
    -0.5f,-0.5f,-0.5f, -1.0f, 0.0f, 0.0f,  -0.5f,-0.5f, 0.5f, -1.0f, 0.0f, 0.0f,  -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
     0.5f, 0.5f, 0.5f,  1.0f, 0.0f, 0.0f,   0.5f, 0.5f,-0.5f,  1.0f, 0.0f, 0.0f,   0.5f,-0.5f,-0.5f,  1.0f, 0.0f, 0.0f,
     0.5f,-0.5f,-0.5f,  1.0f, 0.0f, 0.0f,   0.5f,-0.5f, 0.5f,  1.0f, 0.0f, 0.0f,   0.5f, 0.5f, 0.5f,  1.0f, 0.0f, 0.0f,
    -0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,   0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,   0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,
     0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,  -0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,  -0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,
    -0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,   0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,   0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
     0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,  -0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,  -0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f
};

std::vector<glm::vec3> spherePos;
std::vector<glm::vec2> sphereUV;
std::vector<unsigned int> sphereIdx;
void genSphere(float r, int slices, int stacks) {
    for (int i = 0; i <= stacks; ++i) {
        float phi = M_PI * i / stacks;
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * M_PI * j / slices;
            spherePos.push_back(glm::vec3(r * sin(phi) * cos(theta), r * sin(phi) * sin(theta), r * cos(phi)));
            sphereUV.push_back(glm::vec2((float)j / slices, (float)i / stacks));
        }
    }
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            sphereIdx.push_back(i * (slices + 1) + j);
            sphereIdx.push_back((i + 1) * (slices + 1) + j);
            sphereIdx.push_back((i + 1) * (slices + 1) + j + 1);
            sphereIdx.push_back(i * (slices + 1) + j);
            sphereIdx.push_back((i + 1) * (slices + 1) + j + 1);
            sphereIdx.push_back(i * (slices + 1) + j + 1);
        }
    }
}

void initShaderProgram() {
    ShaderInfo shaders[] = {
        { GL_VERTEX_SHADER, "vertexShader.glsl", 0 },
        { GL_FRAGMENT_SHADER, "fragmentShader.glsl", 0 },
        { GL_NONE, nullptr, 0 }

    };
    program[0] = LoadShaders(shaders);

    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, BO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    genSphere(0.25f, 30, 30);
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, BO[1]);
    glBufferData(GL_ARRAY_BUFFER, spherePos.size() * sizeof(glm::vec3), spherePos.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, BO[2]);
    glBufferData(GL_ARRAY_BUFFER, sphereUV.size() * sizeof(glm::vec2), sphereUV.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BO[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIdx.size() * sizeof(unsigned int), sphereIdx.data(), GL_STATIC_DRAW);

    texture[0] = SOIL_load_OGL_texture("sun.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);

    glEnable(GL_DEPTH_TEST);

    glDisable(GL_CULL_FACE); // Kikapcsolja a belső lapok eldobását, így a kocka elejét fogjuk látni
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Sötétszürke háttér (az űr) beállítása

}

void display(GLFWwindow* window, double currentTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program[0]);

    glm::vec3 camPos = glm::vec3(cameraR * cos(cameraAngle), cameraR * sin(cameraAngle), cameraZ);
    matView = glm::lookAt(camPos, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1)); // kamera középre néz

    glUniformMatrix4fv(glGetUniformLocation(program[0], "matView"), 1, GL_FALSE, glm::value_ptr(matView));
    glUniformMatrix4fv(glGetUniformLocation(program[0], "matProjection"), 1, GL_FALSE, glm::value_ptr(matProjection));
    glUniform3fv(glGetUniformLocation(program[0], "cameraPosition"), 1, glm::value_ptr(camPos));
    glUniform1i(glGetUniformLocation(program[0], "useLighting"), lightOn);

    float lightR = 2.0f * cameraR;
    glm::vec3 lightPos = glm::vec3(lightR * cos(currentTime), lightR * sin(currentTime), 0.0f);
    glUniform3fv(glGetUniformLocation(program[0], "lightPosition"), 1, glm::value_ptr(lightPos));

    glUniform1i(glGetUniformLocation(program[0], "useTexture"), 0);
    glUniform3f(glGetUniformLocation(program[0], "objectColor"), 1.0f, 1.0f, 1.0f);
    glBindVertexArray(VAO[0]);

    float zOffsets[] = { 0.0f, 2.0f, -2.0f };
    for (int i = 0; i < 3; ++i) {
        matModel = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, zOffsets[i]));
        glUniformMatrix4fv(glGetUniformLocation(program[0], "matModel"), 1, GL_FALSE, glm::value_ptr(matModel));
        glm::mat3 invTrans = glm::mat3(glm::inverseTranspose(matModel));
        glUniformMatrix3fv(glGetUniformLocation(program[0], "inverseTransposeMatrix"), 1, GL_FALSE, glm::value_ptr(invTrans));
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // Nap
    glUniform1i(glGetUniformLocation(program[0], "useTexture"), 1);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    matModel = glm::translate(glm::mat4(1.0f), lightPos);
    glUniformMatrix4fv(glGetUniformLocation(program[0], "matModel"), 1, GL_FALSE, glm::value_ptr(matModel));
    glBindVertexArray(VAO[1]);
    glDrawElements(GL_TRIANGLES, sphereIdx.size(), GL_UNSIGNED_INT, 0);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        // Kamera mozgatása nyilakkal
        if (key == GLFW_KEY_LEFT) cameraAngle += 0.05f;
        if (key == GLFW_KEY_RIGHT) cameraAngle -= 0.05f;
        if (key == GLFW_KEY_UP) cameraZ += 0.1f;
        if (key == GLFW_KEY_DOWN) cameraZ -= 0.1f;

        // Világítás kapcsoló
        if (key == GLFW_KEY_L && action == GLFW_PRESS) lightOn = !lightOn;

        // Kilépés ESC gombbal 
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

// Üres Callbackek a Linker hibák elkerüléséhez
void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {}
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    windowWidth = width; windowHeight = height;
    glViewport(0, 0, width, height);
    matProjection = glm::perspective(glm::radians(55.0f), (float)width / height, 0.1f, 100.0f); // 55 fok
}

int main() {
    init(3, 3, GLFW_OPENGL_CORE_PROFILE);

    std::cout << "Bal - Jobb    :  Kamera forgatasa Z-tengely korul" << std::endl;
    std::cout << "Fel - Le      :  Kamera mozgatasa a Z-tengely menten" << std::endl;
    std::cout << "L             :  Vilagitas ki/be kapcsolasa" << std::endl;
    std::cout << "ESC           :  Kilepes" << std::endl;

    initShaderProgram();
    framebufferSizeCallback(window, windowWidth, windowHeight);
    while (!glfwWindowShouldClose(window)) {
        display(window, glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    cleanUpScene(0);
    return 0;
}
