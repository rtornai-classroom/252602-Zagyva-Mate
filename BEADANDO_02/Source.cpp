#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;
using namespace glm;

// Ablak beallitasok
int         windowWidth  = 800;
int         windowHeight = 800;
GLFWwindow* window       = nullptr;

// Shader programok
GLuint curveProgram;
GLuint simpleProgram;
GLuint circleProgram;

// Uniform poziciok
GLint uCurveModelView, uCurveProjection, uCurveType, uCurveControlPointsNum;
GLint uSimpleModelView, uSimpleProjection, uSimpleColor;
GLint uCircleModelView, uCircleProjection, uCircleColor;

// GPU objektumok
GLuint vaoPoints, vboPoints;
GLuint vaoCircle, vboCircle;
GLuint vaoLine,   vboLine;

// Allapotok
struct CP { float x, y; };
vector<CP> controlPoints;

int  dragIndex  = -1;   // mozgatott pont indexe
bool leftDown   = false;

// Szinek
const vec3 COL_CURVE   = vec3(0.0f, 1.0f, 0.3f); // zold
const vec3 COL_POLYGON  = vec3(1.0f, 0.85f, 0.0f); // sarga
const vec3 COL_POINT    = vec3(0.0f, 0.8f, 1.0f); // cyan

// Pontokkivalasztasi foka NDC-ben
static const float PICK_RADIUS = 0.03f;
// Kirajzolasi fok NDC-ben
static const float DRAW_RADIUS = 0.03f;
// Kor felbontas
static const int   CIRCLE_SEGS = 64;

// Segedfuggvenyek
string readFile(const char* path) {
    ifstream f(path);
    if (!f.is_open()) {
        cerr << "Cannot open: " << path << endl;
        return "";
    }
    return string(istreambuf_iterator<char>(f), istreambuf_iterator<char>());
}

GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024]; glGetShaderInfoLog(s, 1024, nullptr, log);
        cerr << "Shader compile error:\n" << log << endl;
    }
    return s;
}

GLuint linkProgram(vector<GLuint> shaders) {
    GLuint p = glCreateProgram();
    for (auto s : shaders) glAttachShader(p, s);
    glLinkProgram(p);
    GLint ok; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024]; glGetProgramInfoLog(p, 1024, nullptr, log);
        cerr << "Program link error:\n" << log << endl;
    }
    for (auto s : shaders) glDeleteShader(s);
    return p;
}

vec2 pixelToNDC(double px, double py) {
    return vec2(
         2.0f * (float)px / (float)windowWidth  - 1.0f,
        -2.0f * (float)py / (float)windowHeight + 1.0f
    );
}

int findClosestCP(vec2 pos) {
    for (int i = 0; i < (int)controlPoints.size(); ++i) {
        vec2 cp(controlPoints[i].x, controlPoints[i].y);
        if (length(pos - cp) <= PICK_RADIUS)
            return i;
    }
    return -1;
}

void buildCircleVBO() {
    vector<float> verts;
    verts.push_back(0.f); verts.push_back(0.f); // kozep
    for (int i = 0; i <= CIRCLE_SEGS; ++i) {
        float a = 2.f * 3.14159265359 * i / CIRCLE_SEGS;
        verts.push_back(cosf(a));
        verts.push_back(sinf(a));
    }
    glBindVertexArray(vaoCircle);
    glBindBuffer(GL_ARRAY_BUFFER, vboCircle);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
}

void initGL() {
    string vsStr  = readFile("BezierVertShader.glsl");
    string tcStr  = readFile("BezierTessContShader.glsl");
    string teStr  = readFile("BezierTessEvalShader.glsl");
    string fsStr  = readFile("BezierFragShader.glsl");

    GLuint vs  = compileShader(GL_VERTEX_SHADER,          vsStr.c_str());
    GLuint tcs = compileShader(GL_TESS_CONTROL_SHADER,    tcStr.c_str());
    GLuint tes = compileShader(GL_TESS_EVALUATION_SHADER, teStr.c_str());
    GLuint fs  = compileShader(GL_FRAGMENT_SHADER,        fsStr.c_str());
    curveProgram = linkProgram({vs, tcs, tes, fs});

    uCurveModelView        = glGetUniformLocation(curveProgram, "matModelView");
    uCurveProjection       = glGetUniformLocation(curveProgram, "matProjection");
    uCurveType             = glGetUniformLocation(curveProgram, "curveType");
    uCurveControlPointsNum = glGetUniformLocation(curveProgram, "controlPointsNumber");

    string svStr = readFile("SimpleVertShader.glsl");
    string sfStr = readFile("SimpleFragShader.glsl");

    GLuint sv = compileShader(GL_VERTEX_SHADER,   svStr.c_str());
    GLuint sf = compileShader(GL_FRAGMENT_SHADER, sfStr.c_str());
    simpleProgram = linkProgram({sv, sf});
    circleProgram = simpleProgram;

    uSimpleModelView  = glGetUniformLocation(simpleProgram, "matModelView");
    uSimpleProjection = glGetUniformLocation(simpleProgram, "matProjection");
    uSimpleColor      = glGetUniformLocation(simpleProgram, "uColor");
    uCircleModelView  = uSimpleModelView;
    uCircleProjection = uSimpleProjection;
    uCircleColor      = uSimpleColor;

    // VAOK-k/VABO-k
    GLuint vaos[3], vbos[3];
    glGenVertexArrays(3, vaos);
    glGenBuffers     (3, vbos);
    vaoPoints = vaos[0]; vboPoints = vbos[0];
    vaoCircle = vaos[1]; vboCircle = vbos[1];
    vaoLine   = vaos[2]; vboLine   = vbos[2];

    // circle geometry (static)
    buildCircleVBO();

    // vonal VAO
    glBindVertexArray(vaoLine);
    glBindBuffer(GL_ARRAY_BUFFER, vboLine);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // pont VAO
    glBindVertexArray(vaoPoints);
    glBindBuffer(GL_ARRAY_BUFFER, vboPoints);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);

    glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
    glPointSize(6.0f);
    glLineWidth(2.0f);
}

// Kontroll pontok GPU-ba toltese
void uploadControlPoints() {
    glBindBuffer(GL_ARRAY_BUFFER, vboPoints);
    if (controlPoints.empty()) {
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    } else {
        glBufferData(GL_ARRAY_BUFFER,
                     controlPoints.size() * 2 * sizeof(float),
                     controlPoints.data(),
                     GL_DYNAMIC_DRAW);
    }
}

// Metrikak beallitasa
mat4 identityMV  = mat4(1.0f);
mat4 identityP   = mat4(1.0f);

void setSimpleUniforms(vec3 color) {
    glUniformMatrix4fv(uSimpleModelView,  1, GL_FALSE, value_ptr(identityMV));
    glUniformMatrix4fv(uSimpleProjection, 1, GL_FALSE, value_ptr(identityP));
    glUniform3fv      (uSimpleColor,      1, value_ptr(color));
}

void drawCircle(float cx, float cy, float r, vec3 color) {
    mat4 model = translate(mat4(1.0f), vec3(cx, cy, 0.0f))
               * scale(mat4(1.0f), vec3(r, r, 1.0f));

    glUseProgram(circleProgram);
    glUniformMatrix4fv(uCircleModelView,  1, GL_FALSE, value_ptr(model));
    glUniformMatrix4fv(uCircleProjection, 1, GL_FALSE, value_ptr(identityP));
    glUniform3fv      (uCircleColor,      1, value_ptr(color));

    glBindVertexArray(vaoCircle);
    glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_SEGS + 2);
}

void drawLine(float x0, float y0, float x1, float y1, vec3 color) {
    float verts[4] = {x0, y0, x1, y1};
    glBindBuffer(GL_ARRAY_BUFFER, vboLine);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    glUseProgram(simpleProgram);
    setSimpleUniforms(color);
    glBindVertexArray(vaoLine);
    glDrawArrays(GL_LINES, 0, 2);
}

// Megjelenites
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    int n = (int)controlPoints.size();

    // Sarga gorbe
    if (n >= 2) {
        for (int i = 0; i < n - 1; ++i) {
            drawLine(controlPoints[i].x,   controlPoints[i].y,
                     controlPoints[i+1].x, controlPoints[i+1].y,
                     COL_POLYGON);
        }
    }

    // Bezier gorbe
    if (n >= 2) {
        uploadControlPoints();

        glUseProgram(curveProgram);
        glUniformMatrix4fv(uCurveModelView,        1, GL_FALSE, value_ptr(identityMV));
        glUniformMatrix4fv(uCurveProjection,       1, GL_FALSE, value_ptr(identityP));
        glUniform1i       (uCurveType,             3);
        glUniform1i       (uCurveControlPointsNum, n);

        glPatchParameteri(GL_PATCH_VERTICES, n);

        glBindVertexArray(vaoPoints);
        glDrawArrays(GL_PATCHES, 0, n);
    }

    // Cyan szinu pontok
    for (int i = 0; i < n; ++i) {
        drawCircle(controlPoints[i].x, controlPoints[i].y, DRAW_RADIUS, COL_POINT);
    }

    glfwSwapBuffers(window);
}

void framebufferSizeCallback(GLFWwindow*, int w, int h) {
    windowWidth  = std::max(w, 1);
    windowHeight = std::max(h, 1);
    glViewport(0, 0, windowWidth, windowHeight);
}

void keyCallback(GLFWwindow* win, int key, int, int action, int) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(win, GLFW_TRUE);
}

void mouseButtonCallback(GLFWwindow* win, int button, int action, int) {
    double px, py;
    glfwGetCursorPos(win, &px, &py);
    vec2 pos = pixelToNDC(px, py);

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            leftDown  = true;
            dragIndex = findClosestCP(pos);
            if (dragIndex == -1) {
                // Uj pont hozzaadasa
                controlPoints.push_back({pos.x, pos.y});
            }
        } else {
            leftDown  = false;
            dragIndex = -1;
        }
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        int idx = findClosestCP(pos);
        if (idx != -1) {
            controlPoints.erase(controlPoints.begin() + idx);
        }
    }
}

void cursorPosCallback(GLFWwindow*, double px, double py) {
    if (leftDown && dragIndex != -1) {
        vec2 pos = pixelToNDC(px, py);
        controlPoints[dragIndex].x = pos.x;
        controlPoints[dragIndex].y = pos.y;
    }
}

// main fuggveny
int main() {
    if (!glfwInit()) return EXIT_FAILURE;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(windowWidth, windowHeight, "Bezier Curve Editor", nullptr, nullptr);
    if (!window) { glfwTerminate(); return EXIT_FAILURE; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback            (window, keyCallback);
    glfwSetMouseButtonCallback    (window, mouseButtonCallback);
    glfwSetCursorPosCallback      (window, cursorPosCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { glfwTerminate(); return EXIT_FAILURE; }

    initGL();

    cout << "Bezier Curve Editor" << endl;
    cout << "  Left click  : add control point" << endl;
    cout << "  Left drag   : move control point" << endl;
    cout << "  Right click : remove control point" << endl;
    cout << "  ESC         : exit" << endl;

    while (!glfwWindowShouldClose(window)) {
        display();
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
