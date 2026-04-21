#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>

const int WIDTH = 800;
const int HEIGHT = 600;

std::vector<float> controlPoints;
int selectedPoint = -1;
bool isDragging = false;
const float CLICK_THRESHOLD = 0.05f;

GLuint curveProgram, pointProgram, lineProgram;
GLuint curveVAO, curveVBO;
GLuint pointVAO, pointVBO;
GLuint lineVAO, lineVBO;

std::string loadShaderSource(const char* filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Shader file not found: " << filepath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compile error:\n" << infoLog << std::endl;
    }
    return shader;
}

GLuint createCurveProgram()
{
    std::string vertSrc = loadShaderSource("shaders/curve.vert");
    std::string tescSrc = loadShaderSource("shaders/curve.tesc");
    std::string teseSrc = loadShaderSource("shaders/curve.tese");
    std::string fragSrc = loadShaderSource("shaders/curve.frag");

    GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc.c_str());
    GLuint tesc = compileShader(GL_TESS_CONTROL_SHADER, tescSrc.c_str());
    GLuint tese = compileShader(GL_TESS_EVALUATION_SHADER, teseSrc.c_str());
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc.c_str());

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, tesc);
    glAttachShader(program, tese);
    glAttachShader(program, frag);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Program link error:\n" << infoLog << std::endl;
    }

    glDeleteShader(vert);
    glDeleteShader(tesc);
    glDeleteShader(tese);
    glDeleteShader(frag);
    return program;
}

GLuint createSimpleProgram(const char* vertPath, const char* fragPath)
{
    std::string vertSrc = loadShaderSource(vertPath);
    std::string fragSrc = loadShaderSource(fragPath);

    GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc.c_str());
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc.c_str());

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    glDeleteShader(vert);
    glDeleteShader(frag);
    return program;
}

void initDefaultPoints()
{
    controlPoints = {
        -0.6f, -0.4f,
        -0.2f,  0.6f,
         0.2f, -0.6f,
         0.6f,  0.4f
    };
}

void setupBuffers()
{
    glGenVertexArrays(1, &curveVAO);
    glGenBuffers(1, &curveVBO);
    glGenVertexArrays(1, &pointVAO);
    glGenBuffers(1, &pointVBO);
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
}

void updateAllBuffers()
{
    glBindVertexArray(curveVAO);
    glBindBuffer(GL_ARRAY_BUFFER, curveVBO);
    glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(float), controlPoints.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(pointVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
    glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(float), controlPoints.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(float), controlPoints.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void screenToNDC(double sx, double sy, float& nx, float& ny)
{
    nx = (float)(2.0 * sx / WIDTH - 1.0);
    ny = (float)(1.0 - 2.0 * sy / HEIGHT);
}

int getNearestPoint(float x, float y)
{
    int count = controlPoints.size() / 2;
    for (int i = 0; i < count; i++)
    {
        float px = controlPoints[i * 2];
        float py = controlPoints[i * 2 + 1];
        float dist = hypot(x - px, y - py);
        if (dist < CLICK_THRESHOLD) return i;
    }
    return -1;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    float nx, ny;
    screenToNDC(mx, my, nx, ny);

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            selectedPoint = getNearestPoint(nx, ny);
            if (selectedPoint != -1)
                isDragging = true;
            else
            {
                controlPoints.push_back(nx);
                controlPoints.push_back(ny);
                updateAllBuffers();
            }
        }
        else if (action == GLFW_RELEASE)
        {
            isDragging = false;
            selectedPoint = -1;
        }
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        int idx = getNearestPoint(nx, ny);
        if (idx != -1 && controlPoints.size() > 4)
        {
            controlPoints.erase(controlPoints.begin() + idx * 2, controlPoints.begin() + idx * 2 + 2);
            updateAllBuffers();
        }
    }
}

void cursorPosCallback(GLFWwindow* window, double x, double y)
{
    if (isDragging && selectedPoint != -1)
    {
        float nx, ny;
        screenToNDC(x, y, nx, ny);
        controlPoints[selectedPoint * 2] = nx;
        controlPoints[selectedPoint * 2 + 1] = ny;
        updateAllBuffers();
    }
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    int pointCount = controlPoints.size() / 2;
    if (pointCount < 2) return;

    glUseProgram(lineProgram);
    glBindVertexArray(lineVAO);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINE_STRIP, 0, pointCount);

    glUseProgram(curveProgram);
    glUniform1i(glGetUniformLocation(curveProgram, "numCP"), pointCount);
    glUniform2fv(glGetUniformLocation(curveProgram, "cp"), pointCount, controlPoints.data());

    glBindVertexArray(curveVAO);
    glPatchParameteri(GL_PATCH_VERTICES, pointCount);
    glDrawArrays(GL_PATCHES, 0, pointCount);

    glUseProgram(pointProgram);
    glBindVertexArray(pointVAO);
    glPointSize(10.0f);
    glDrawArrays(GL_POINTS, 0, pointCount);
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "GLFW init failed" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Bezier Curve Tessellation", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "GLEW init failed" << std::endl;
        return -1;
    }

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    curveProgram = createCurveProgram();
    pointProgram = createSimpleProgram("shaders/point.vert", "shaders/point.frag");
    lineProgram = createSimpleProgram("shaders/line.vert", "shaders/line.frag");

    initDefaultPoints();
    setupBuffers();
    updateAllBuffers();

    while (!glfwWindowShouldClose(window))
    {
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &curveVAO);
    glDeleteVertexArrays(1, &pointVAO);
    glDeleteVertexArrays(1, &lineVAO);
    glDeleteBuffers(1, &curveVBO);
    glDeleteBuffers(1, &pointVBO);
    glDeleteBuffers(1, &lineVBO);
    glDeleteProgram(curveProgram);
    glDeleteProgram(pointProgram);
    glDeleteProgram(lineProgram);

    glfwTerminate();
    return 0;
}
