#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
#include <array>

using namespace std;
using namespace glm;

// Globalis valtozok
const GLint windowWidth = 600;
const GLint windowHeight = 600;
GLchar windowTitle[] = "Beadando 1";
GLFWwindow* window = nullptr;
GLboolean keyboard[512] = { GL_FALSE };

// Shader programok
#define NUM_PROGRAMS 2
enum eProgram { CircleProgram = 0, LineProgram = 1 };
GLuint          program[NUM_PROGRAMS];

// VAO es VABO
#define NUM_VAOS 2
#define NUM_VBOS 2
GLuint VAO[NUM_VAOS];
GLuint VBO[NUM_VBOS];

// Kor adatok
static const array<vec2, 6> quadVertices = {
    vec2(-1.0f, -1.0f),
    vec2(1.0f, -1.0f),
    vec2(1.0f,  1.0f),
    vec2(-1.0f, -1.0f),
    vec2(1.0f,  1.0f),
    vec2(-1.0f,  1.0f)
};

// Kor allapot
const float RADIUS_PX = 50.0f; // sugar pixelben
const float RADIUS_NDC = RADIUS_PX / (windowWidth / 2.0f); // sugar NDC-ben

float circleCX = 0.0f; // kor X kozepe NDC-ben
float circleCY = 0.0f; // kor Y kozepe NDC-ben

// Mozgasi irany
const float ALPHA_DEG = 25.0f;
const float ALPHA_RAD = ALPHA_DEG * 3.14159265f / 180.0f;
float dirX = 1.0f;
float dirY = 0.0f;
bool diagonalMode = false;

// Sebesseg
const float SPEED = 0.005f;

// Szin csere
bool colorSwapped = false;

// Vonal
const float LINE_HALF_LEN_NDC = (windowWidth / 3.0f) / (windowWidth / 2.0f);
float lineY_NDC = 0.0f;
const float LINE_STEP_PX = 5.0f;
const float LINE_STEP_NDC = LINE_STEP_PX / (windowHeight / 2.0f);

// Szakasz csucsai
array<vec2, 2>  lineVertices = {
    vec2(-LINE_HALF_LEN_NDC, lineY_NDC),
    vec2(LINE_HALF_LEN_NDC, lineY_NDC)
};

GLint   locCircleCenter;
GLint   locCircleRadius;
GLint   locColorSwapped;
GLint   locWindowWidth;
GLint   locWindowHeight;
GLint   locLineY;
GLint   locLineHalfLen;

// Segedfuggvenyek
bool checkOpenGLError() {
    bool foundError = false;
    int  glErr = glGetError();
    while (glErr != GL_NO_ERROR) {
        cout << "GL Hiba: " << glErr << endl;
        foundError = true;
        glErr = glGetError();
    }
    return foundError;
}

void printShaderLog(GLuint shader) {
    int  length = 0;
    int  charsWritten = 0;
    char* log = nullptr;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if (length > 0) {
        log = (char*)malloc(length);
        glGetShaderInfoLog(shader, length, &charsWritten, log);
        cout << "Shader Info: " << log << endl;
        free(log);
    }
}

void printProgramLog(GLuint prog) {
    int  length = 0;
    int  charsWritten = 0;
    char* log = nullptr;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &length);
    if (length > 0) {
        log = (char*)malloc(length);
        glGetProgramInfoLog(prog, length, &charsWritten, log);
        free(log);
    }
}

string readShaderSource(const char* filePath) {
    ifstream fileStream(filePath, ios::in);
    if (!fileStream.is_open()) {
        cerr << "Nem lehet megnyitni a shader filet: " << filePath << endl;
        return "";
    }
    string content;
    string line;
    while (!fileStream.eof()) {
        getline(fileStream, line);
        content.append(line + "\n");
    }
    fileStream.close();
    return content;
}

GLuint createShaderProgram(const char* vertPath, const char* fragPath) {
    GLint vertCompiled, fragCompiled, linked;

    string vertShaderStr = readShaderSource(vertPath);
    string fragShaderStr = readShaderSource(fragPath);

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

    const char* vertSrc = vertShaderStr.c_str();
    const char* fragSrc = fragShaderStr.c_str();

    glShaderSource(vShader, 1, &vertSrc, NULL);
    glShaderSource(fShader, 1, &fragSrc, NULL);

    glCompileShader(vShader);
    glCompileShader(fShader);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vShader);
    glAttachShader(prog, fShader);
    glLinkProgram(prog);

    glDeleteShader(vShader);
    glDeleteShader(fShader);
    return prog;
}

bool circleIntersectsLine() {
    float clampedX = glm::clamp(circleCX, -LINE_HALF_LEN_NDC, LINE_HALF_LEN_NDC);
    float clampedY = lineY_NDC;
    float dx = circleCX - clampedX;
    float dy = circleCY - clampedY;
    float distSq = dx * dx + dy * dy;
    return (distSq <= RADIUS_NDC * RADIUS_NDC);
}

// Inicializalas
void init() {
    program[CircleProgram] = createShaderProgram(
        "./circleVertexShader.glsl",
        "./circleFragmentShader.glsl"
    );
    program[LineProgram] = createShaderProgram(
        "./lineVertexShader.glsl",
        "./lineFragmentShader.glsl"
    );

    glGenVertexArrays(NUM_VAOS, VAO);
    glGenBuffers(NUM_VBOS, VBO);

    glBindVertexArray(VAO[CircleProgram]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[CircleProgram]);
    glBufferData(GL_ARRAY_BUFFER,
        quadVertices.size() * sizeof(vec2),
        quadVertices.data(),
        GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAO[LineProgram]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[LineProgram]);
    glBufferData(GL_ARRAY_BUFFER,
        lineVertices.size() * sizeof(vec2),
        lineVertices.data(),
        GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    locCircleCenter = glGetUniformLocation(program[CircleProgram], "circleCenter");
    locCircleRadius = glGetUniformLocation(program[CircleProgram], "circleRadius");
    locColorSwapped = glGetUniformLocation(program[CircleProgram], "colorSwapped");
    locWindowWidth = glGetUniformLocation(program[CircleProgram], "windowWidth");
    locWindowHeight = glGetUniformLocation(program[CircleProgram], "windowHeight");
    locLineY = glGetUniformLocation(program[CircleProgram], "lineY");
    locLineHalfLen = glGetUniformLocation(program[CircleProgram], "lineHalfLen");

    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
}

// Visszapattanas
void updateCircle() {
    float newX = circleCX + dirX * SPEED;
    float newY = circleCY + dirY * SPEED;

    float wallRight = 1.0f - RADIUS_NDC;
    float wallLeft = -1.0f + RADIUS_NDC;
    float wallTop = 1.0f - RADIUS_NDC;
    float wallBottom = -1.0f + RADIUS_NDC;

    if (newX > wallRight) {
        float remaining = newX - wallRight;
        dirX = -dirX;
        newX = wallRight - remaining;
    }
    else if (newX < wallLeft) {
        float remaining = wallLeft - newX;
        dirX = -dirX;
        newX = wallLeft + remaining;
    }

    if (diagonalMode) {
        if (newY > wallTop) {
            float remaining = newY - wallTop;
            dirY = -dirY;
            newY = wallTop - remaining;
        }
        else if (newY < wallBottom) {
            float remaining = wallBottom - newY;
            dirY = -dirY;
            newY = wallBottom + remaining;
        }
    }

    circleCX = newX;
    circleCY = newY;
}

// Megjelenites
void display(GLFWwindow* window, double currentTime) {
    glClear(GL_COLOR_BUFFER_BIT);

    updateCircle();

    colorSwapped = circleIntersectsLine();

    glUseProgram(program[CircleProgram]);
    glBindVertexArray(VAO[CircleProgram]);

    glUniform2f(locCircleCenter, circleCX, circleCY);
    glUniform1f(locCircleRadius, RADIUS_NDC);
    glUniform1i(locColorSwapped, colorSwapped ? 1 : 0);
    glUniform1f(locWindowWidth, (float)windowWidth);
    glUniform1f(locWindowHeight, (float)windowHeight);
    glUniform1f(locLineY, lineY_NDC);
    glUniform1f(locLineHalfLen, LINE_HALF_LEN_NDC);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(program[LineProgram]);
    glBindVertexArray(VAO[LineProgram]);

    glLineWidth(3.0f);

    lineVertices[0] = vec2(-LINE_HALF_LEN_NDC, lineY_NDC);
    lineVertices[1] = vec2(LINE_HALF_LEN_NDC, lineY_NDC);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[LineProgram]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, lineVertices.size() * sizeof(vec2), lineVertices.data());

    glDrawArrays(GL_LINES, 0, 2);
}

// Ablak atmeretezes
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Gombok
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // ESC
    if ((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    // Lenyomas kezeles
    if (action == GLFW_PRESS)
        keyboard[key] = GL_TRUE;
    else if (action == GLFW_RELEASE)
        keyboard[key] = GL_FALSE;

    // FEL-nyil
    if ((action == GLFW_PRESS || action == GLFW_REPEAT) && (key == GLFW_KEY_UP)) {
        lineY_NDC += LINE_STEP_NDC;
        if (lineY_NDC > 1.0f - LINE_STEP_NDC) lineY_NDC = 1.0f - LINE_STEP_NDC;
    }

    // LE-nyil
    if ((action == GLFW_PRESS || action == GLFW_REPEAT) && (key == GLFW_KEY_DOWN)) {
        lineY_NDC -= LINE_STEP_NDC;
        if (lineY_NDC < -1.0f + LINE_STEP_NDC) lineY_NDC = -1.0f + LINE_STEP_NDC;
    }
    
    // S-gomb
    if ((action == GLFW_PRESS) && (key == GLFW_KEY_S)) {
        circleCX = 0.0f;
        circleCY = 0.0f;

        dirX = cos(ALPHA_RAD);
        dirY = sin(ALPHA_RAD);
        diagonalMode = true;
    }
}


int main(void) {
    if (!glfwInit()) exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, nullptr, nullptr);
    if (window == nullptr) {
        cerr << "GLFW ablkak keszitese sikertelen" << endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        cerr << "GLEW betoltese sikertelen" << endl;
    }

    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos(window, (mode->width - windowWidth) / 2, (mode->height - windowHeight) / 2);

    glfwSetWindowAspectRatio(window, 1, 1);

    // Inicializalas inditasa
    init();

    cout << "Elso Beadando" << endl << endl;
    cout << "Iranyitas:" << endl;
    cout << "ESC \t-> kilepes" << endl;
    cout << "S   \t-> atlos mozgas bekapcsolasa" << endl;
    cout << "Nyilak:" << endl;
    cout << "FEL \t-> vonal mozgatasa fel" << endl;
    cout << "LE \t-> vonal mozgatasa le";

    while (!glfwWindowShouldClose(window)) {
        display(window, glfwGetTime());

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    return EXIT_SUCCESS;
}