#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Pipeline.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

GLuint VBO;
GLuint IBO;
GLuint gWVPLocation;

static const char* pVS = "                          \n\
#version 330                                        \n\
                                                    \n\
layout (location = 0) in vec3 Position;             \n\
uniform mat4 gWVP;                                  \n\
out vec4 Color;                                     \n\
                                                    \n\
void main() {                                       \n\
    gl_Position = gWVP * vec4(Position, 1.0);       \n\
    Color = vec4(clamp(Position, 0.5, 1.0), 1.0);   \n\
}";

static const char* pFS = "                          \n\
#version 330                                        \n\
                                                    \n\
in vec4 Color;                                      \n\
out vec4 FragColor;                                 \n\
                                                    \n\
void main() {                                       \n\
    FragColor = Color;                              \n\
}";

static void RenderSceneCB() {
    glClear(GL_COLOR_BUFFER_BIT);

    static float Scale = 0.0f;
    Scale += 0.0001f;

    Pipeline p; // Изменение свойств сцены
    p.Scale(0.33f, 0.33f, 0.33f);
    p.Rotate(0.0f, 0.0f, 0.0f);
    p.WorldPos(0.0f, 0.0f, 0.0f);

    // Изменение свойств камеры
    Vector3f CameraPos(sinf(Scale), -2.0f * sinf(Scale), -2.0f * fabs(cosf(Scale)));
    Vector3f CameraTarget( 0.0f, sinf(Scale), fabs(cosf(Scale)));
    Vector3f CameraUp(1.0f, 0.0f, 0.0f);
    p.SetCamera(CameraPos, CameraTarget, CameraUp);
    p.SetPerspectiveProj(60.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f, 100.0f);

    glUniformMatrix4fv(gWVPLocation, 1, GL_TRUE, (const GLfloat*)p.GetTrans());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);
    glutSwapBuffers();
}


static void InitializeGlutCallbacks() {
    glutDisplayFunc(RenderSceneCB);
    glutIdleFunc(RenderSceneCB);
}

static void CreateVertexBuffer() {
    Vector3f Vertices[8]; // Вершины куба
    Vertices[0] = Vector3f(-1.0f, -1.0f, -1.0f); 
    Vertices[1] = Vector3f(-1.0f, -1.0f, 1.0f);
    Vertices[2] = Vector3f(1.0f, -1.0f, 1.0f);
    Vertices[3] = Vector3f(1.0f, -1.0f, -1.0f);
    Vertices[4] = Vector3f(-1.0f, 1.0f, -1.0f);
    Vertices[5] = Vector3f(-1.0f, 1.0f, 1.0f);
    Vertices[6] = Vector3f(1.0f, 1.0f, 1.0f);
    Vertices[7] = Vector3f(1.0f, 1.0f, -1.0f);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static void CreateIndexBuffer() {
    unsigned int Indices[] = { 0, 1, 2, 0, 2, 3, // Создание плоскостей куба
                               0, 4, 7, 0, 7, 3, // Путем последовательного соединения
                               0, 4, 5, 0, 5, 1, // Вершин в заданном порядке
                               3, 7, 6, 3, 6, 2,
                               1, 2, 6, 1, 6, 5,
                               4, 5, 6, 4, 6, 7};
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType) {
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }

    const GLchar* p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0] = strlen(pShaderText);
    glShaderSource(ShaderObj, 1, p, Lengths);
    glCompileShader(ShaderObj);
    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }
    glAttachShader(ShaderProgram, ShaderObj);
}

static void CompileShaders() {
    GLuint ShaderProgram = glCreateProgram();
    if (ShaderProgram == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    AddShader(ShaderProgram, pVS, GL_VERTEX_SHADER);
    AddShader(ShaderProgram, pFS, GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };
    glLinkProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(ShaderProgram);
    gWVPLocation = glGetUniformLocation(ShaderProgram, "gWVP");
    assert(gWVPLocation != 0xFFFFFFFF);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Camera transform");

    InitializeGlutCallbacks();
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    glClearColor(0.8f, 0.3f, 0.5f, 0.0f);
    CreateVertexBuffer();
    CreateIndexBuffer();
    CompileShaders();
    glutMainLoop();
    return 0;
}