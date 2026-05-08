#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

using namespace std;
using namespace glm;


GLFWwindow* window;

GLboolean   keyboard[512] = { GL_FALSE };
int         windowWidth = 800;
int         windowHeight = 800;
char        windowTitle[] = "Kockak";

// A sugár értéke 8 és 10 közé esik, kezdőértéke 9 (feladatkövetelmény: 8 <= r <= 10)
const float CAM_R_MIN = 8.0f;
const float CAM_R_MAX = 10.0f;
float       camR = 9.0f;

// A kamera szöge a Z-tengely körül radiánban; phi=0 => pozíció (r, 0, 0)
float       camPhi = 0.0f;

// A kamera magassága a Z-tengely mentén
float       camZ = 0.0f;

// Mozgási sebességek
const float CAM_PHI_SPEED = 1.2f;  // szögsebesseg rad/s  (Bal/Jobb nyíl)
const float CAM_Z_SPEED = 3.0f;  // magassági sebesség egység/s (Fel/Le nyíl)

bool        lightingEnabled = true;     // Világítás állapota (L-lel váltható)
float       lightAngle = 0.0f;     // A fény aktuális szöghelyzete a pályán
const float LIGHT_ORBIT_R = 2.0f * 9.0f; // A pályarádisuz = 2*r

// Shader uniform helyek (GPU-n lévő változók azonosítói)
GLuint modelLoc, viewLoc, projectionLoc;
GLuint inverseTransposeMatrixLoc;
GLuint lightPositionLoc, cameraPositionLoc;
GLuint lightColorLoc, lightingEnabledLoc;

// Vetítési mátrix (csak ablakméret-változáskor számítjuk újra)
mat4 projection;

// Shader program azonosítója
GLuint renderingProgram;

// Vertex buffer és vertex array objektumok
#define numVBOs 1
#define numVAOs 1
GLuint VBO[numVBOs];
GLuint VAO[numVAOs];

double currentTime, deltaTime, lastTime = 0.0;

float cubeVertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

     -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
      0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
      0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
      0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

     -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
      0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
};
const int CUBE_VERTEX_COUNT = 36;

vec3 cubePositions[3] = {
    vec3(0.0f,  0.0f,  0.0f),   // középső kocka
    vec3(0.0f,  0.0f,  2.0f),   // felső kocka
    vec3(0.0f,  0.0f, -2.0f),   // alsó kocka
};


bool checkOpenGLError() {
    bool found = false;
    int err = glGetError();
    while (err != GL_NO_ERROR) {
        cerr << "glError: " << err << endl;
        found = true;
        err = glGetError();
    }
    return found;
}

void printShaderLog(GLuint shader) {
    int len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
        vector<char> log(len);
        glGetShaderInfoLog(shader, len, nullptr, log.data());
        cerr << "Shader log: " << log.data() << endl;
    }
}

void printProgramLog(GLuint prog) {
    int len = 0;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
        vector<char> log(len);
        glGetProgramInfoLog(prog, len, nullptr, log.data());
        cerr << "Program log: " << log.data() << endl;
    }
}

// Shader forrásfájl beolvasása
string readShaderSource(const char* path) {
    ifstream f(path, ios::in);
    if (!f.is_open()) {
        cerr << "Nem sikerult megnyitni a shader fajlt: " << path << endl;
        return "";
    }
    string content, line;
    while (getline(f, line)) content += line + "\n";
    return content;
}

GLuint createShaderProgram() {
    string vsStr = readShaderSource("vertexShader.glsl");
    string fsStr = readShaderSource("fragmentShader.glsl");
    const char* vsSrc = vsStr.c_str();
    const char* fsSrc = fsStr.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vs, 1, &vsSrc, nullptr);
    glShaderSource(fs, 1, &fsSrc, nullptr);

    GLint status;
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if (!status) { cerr << "Vertex shader fordítási hiba!" << endl; printShaderLog(vs); }

    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if (!status) { cerr << "Fragment shader fordítási hiba!" << endl; printShaderLog(fs); }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (!status) { cerr << "Shader linkelési hiba!" << endl; printProgramLog(prog); }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

void init(GLFWwindow* win) {
    renderingProgram = createShaderProgram();

    // VBO és VAO létrehozása
    glGenBuffers(numVBOs, VBO);
    glGenVertexArrays(numVAOs, VAO);

    // Kocka adatok feltöltése a GPU-ra
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glBindVertexArray(VAO[0]);

    // 0-s attribútum
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // 1-es attribútum
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glUseProgram(renderingProgram);

    // Shader uniform változók helyeinek lekérdezése
    modelLoc = glGetUniformLocation(renderingProgram, "matModel");
    viewLoc = glGetUniformLocation(renderingProgram, "matView");
    projectionLoc = glGetUniformLocation(renderingProgram, "matProjection");
    inverseTransposeMatrixLoc = glGetUniformLocation(renderingProgram, "inverseTransposeMatrix");
    lightPositionLoc = glGetUniformLocation(renderingProgram, "lightPosition");
    cameraPositionLoc = glGetUniformLocation(renderingProgram, "cameraPosition");
    lightColorLoc = glGetUniformLocation(renderingProgram, "lightColor");
    lightingEnabledLoc = glGetUniformLocation(renderingProgram, "lightingEnabled");

    // Perspektív vetítési mátrix beállítása 55 fokos látószöggel 
    projection = perspective(
        radians(55.0f),                           // látószög: 55 fok
        (float)windowWidth / (float)windowHeight,
        0.1f,
        200.0f
    );
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, value_ptr(projection));

    // Fehértől eltérő diffúz fényszín
    vec3 lightColor(1.0f, 0.6f, 0.1f);
    glUniform3fv(lightColorLoc, 1, value_ptr(lightColor));

    // Sötétkék háttér
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);

    // Mélységteszt engedélyezése
    glEnable(GL_DEPTH_TEST);
}

void drawCube(vec3 position) {
    mat4 model = translate(mat4(1.0f), position);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model));

    mat3 itm = mat3(inverseTranspose(model));
    glUniformMatrix3fv(inverseTransposeMatrixLoc, 1, GL_FALSE, value_ptr(itm));

    glDrawArrays(GL_TRIANGLES, 0, CUBE_VERTEX_COUNT);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    float dt = (float)deltaTime;


    // Bal nyíl
    if (keyboard[GLFW_KEY_LEFT])
        camPhi -= CAM_PHI_SPEED * dt;

    // Jobb nyíl
    if (keyboard[GLFW_KEY_RIGHT])
        camPhi += CAM_PHI_SPEED * dt;

    // Fel nyíl
    if (keyboard[GLFW_KEY_UP])
        camZ += CAM_Z_SPEED * dt;

    // Le nyíl
    if (keyboard[GLFW_KEY_DOWN])
        camZ -= CAM_Z_SPEED * dt;

    vec3 cameraPosition(
        camR * cos(camPhi),
        camR * sin(camPhi),
        camZ
    );

    // A kamera mindig az origóba néz
    vec3 cameraTarget(0.0f, 0.0f, 0.0f);

    // UP vektor: (0, 0, 1) — a Z-tengely pozitív iránya
    vec3 cameraUp(0.0f, 0.0f, 1.0f);

    mat4 view = lookAt(cameraPosition, cameraTarget, cameraUp);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));
    glUniform3fv(cameraPositionLoc, 1, value_ptr(cameraPosition));


    lightAngle += 0.5f * dt;  // szögsebesseg
    float lr = 2.0f * camR;   // pályarádisuz = 2*r
    vec3 lightPosition(
        lr * cos(lightAngle),
        lr * sin(lightAngle),
        0.0f                  // Z=0 magasságon kering
    );
    glUniform3fv(lightPositionLoc, 1, value_ptr(lightPosition));

    glUniform1i(lightingEnabledLoc, lightingEnabled ? 1 : 0);

    glBindVertexArray(VAO[0]);
    for (int i = 0; i < 3; i++) {
        drawCube(cubePositions[i]);
    }
}

void cleanUpScene() {
    glfwDestroyWindow(window);
    glDeleteVertexArrays(numVAOs, VAO);
    glDeleteBuffers(numVBOs, VBO);
    glDeleteProgram(renderingProgram);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
    
void framebufferSizeCallback(GLFWwindow* win, int w, int h) {
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
    
    projection = perspective(radians(55.0f), (float)w / (float)h, 0.1f, 200.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, value_ptr(projection));
}

void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
    // ESC
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
        cleanUpScene();

    // L gomb
    if (action == GLFW_PRESS && key == GLFW_KEY_L) {
        lightingEnabled = !lightingEnabled;
    }

    if (action == GLFW_PRESS)
        keyboard[key] = GL_TRUE;
    else if (action == GLFW_RELEASE)
        keyboard[key] = GL_FALSE;
}

void cursorPosCallback(GLFWwindow* win, double x, double y) {}
void mouseButtonCallback(GLFWwindow* win, int button, int action, int mods) {}

void printControls() {
    cout << "  Kockas beadando" << endl;
    cout << "  Bal nyil       : Forgatas a Z-tengely korul (balra)" << endl;
    cout << "  Jobb nyil      : Forgatas a Z-tengely korul (jobbra)" << endl;
    cout << "  Fel nyil       : Emelkedes a Z-tengely menten" << endl;
    cout << "  Le nyil        : Süllyedes a Z-tengely menten" << endl;
    cout << "  L              : Vilagitas ki/bekapcsolasa" << endl;
    cout << "  ESC            : Kilepes" << endl;
    cout << endl;
}

int main(void) {
    printControls();

    if (!glfwInit()) {
        cerr << "GLFW inicializalasi hiba!" << endl;
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, nullptr, nullptr);
    if (!window) {
        cerr << "Ablak letrehozasi hiba!" << endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    if (glewInit() != GLEW_OK) {
        cerr << "GLEW inicializalasi hiba!" << endl;
        return EXIT_FAILURE;
    }

    glfwSwapInterval(1);

     init(window);

     while (!glfwWindowShouldClose(window)) {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanUpScene();
    return EXIT_SUCCESS;
}