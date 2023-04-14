#ifndef LIGHTINGTECHNIQUE_H
#define LIGHTINGTECHNIQUE_H

#include "Technique.h"
#include "Math_3d.h"

static const char* pVS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
layout (location = 0) in vec3 Position;                                             \n\
layout (location = 1) in vec2 TexCoord;                                             \n\
layout (location = 2) in vec3 Normal;                                               \n\
                                                                                    \n\
uniform mat4 gWVP;                                                                  \n\
uniform mat4 gWorld;                                                                \n\
                                                                                    \n\
out vec2 TexCoord0;                                                                 \n\
out vec3 Normal0;                                                                   \n\
out vec3 WorldPos0;                                                                 \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    gl_Position = gWVP * vec4(Position, 1.0);                                       \n\
    TexCoord0 = TexCoord;                                                           \n\
    Normal0   = (gWorld * vec4(Normal, 0.0)).xyz;                                   \n\
    WorldPos0 = (gWorld * vec4(Position, 1.0)).xyz;                                 \n\
}";

static const char* pFS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
in vec2 TexCoord0;                                                                  \n\
in vec3 Normal0;                                                                    \n\
in vec3 WorldPos0;                                                                  \n\
                                                                                    \n\
out vec4 FragColor;                                                                 \n\
                                                                                    \n\
struct DirectionalLight                                                             \n\
{                                                                                   \n\
    vec3 Color;                                                                     \n\
    float AmbientIntensity;                                                         \n\
    vec3 Direction;                                                                 \n\
    float DiffuseIntensity;                                                         \n\
};                                                                                  \n\
                                                                                    \n\
uniform DirectionalLight gDirectionalLight;                                         \n\
uniform sampler2D gSampler;                                                         \n\
                                                                                    \n\
uniform vec3 gEyeWorldPos;                                                          \n\
uniform float gMatSpecularIntensity;                                                \n\
uniform float gSpecularPower;                                                       \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    vec4 AmbientColor = vec4(gDirectionalLight.Color, 1.0f) *                       \n\
                        gDirectionalLight.AmbientIntensity;                         \n\
                                                                                    \n\
    vec3 LightDirection = -gDirectionalLight.Direction;                             \n\
    vec3 Normal = normalize(Normal0);                                               \n\
                                                                                    \n\
    float DiffuseFactor = dot(Normal, LightDirection);                              \n\
                                                                                    \n\
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);                                          \n\
    vec4 SpecularColor = vec4(0, 0, 0, 0);                                          \n\
                                                                                    \n\
    if (DiffuseFactor > 0){                                                         \n\
        DiffuseColor = vec4(gDirectionalLight.Color, 1.0f) *                        \n\
                       gDirectionalLight.DiffuseIntensity *                         \n\
                       DiffuseFactor;                                               \n\
                                                                                    \n\
        vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos0);                     \n\
        vec3 LightReflect = normalize(reflect(gDirectionalLight.Direction, Normal));\n\
        float SpecularFactor = dot(VertexToEye, LightReflect);                      \n\
        SpecularFactor = pow(SpecularFactor, gSpecularPower);                       \n\
                                                                                    \n\
        if (SpecularFactor > 0){                                                    \n\
            SpecularColor = vec4(gDirectionalLight.Color, 1.0f) *                   \n\
                            gMatSpecularIntensity *                                 \n\
                            SpecularFactor;                                         \n\
        }                                                                           \n\
    }                                                                               \n\
                                                                                    \n\
    FragColor = texture2D(gSampler, TexCoord0.xy) *                                 \n\
                (AmbientColor + DiffuseColor + SpecularColor);                      \n\
}";

struct DirectionLight {
    Vector3f Color;
    float AmbientIntensity;
    Vector3f Direction;
    float DiffuseIntensity;
};

class LightingTechnique : public Technique {
public:
    LightingTechnique() {}
    virtual bool Init() {
        if (!Technique::Init())
            return false;
        if (!AddShader(GL_VERTEX_SHADER, pVS))
            return false;
        if (!AddShader(GL_FRAGMENT_SHADER, pFS))
            return false;
        if (!Finalize())
            return false;

        m_WVPLocation = GetUniformLocation("gWVP");
        m_WorldMatrixLocation = GetUniformLocation("gWorld");
        m_samplerLocation = GetUniformLocation("gSampler");

        m_eyeWorldPosition = GetUniformLocation("gEyeWorldPos");
        m_matSpecularIntensityLocation = GetUniformLocation("gMatSpecularIntensity");
        m_matSpecularPowerLocation = GetUniformLocation("gSpecularPower");

        m_dirLightLocation.Color = GetUniformLocation("gDirectionalLight.Color");
        m_dirLightLocation.AmbientIntensity = GetUniformLocation("gDirectionalLight.AmbientIntensity");
        m_dirLightLocation.Direction = GetUniformLocation("gDirectionalLight.Direction");
        m_dirLightLocation.DiffuseIntensity = GetUniformLocation("gDirectionalLight.DiffuseIntensity");
        if (m_dirLightLocation.AmbientIntensity == 0xFFFFFFFF ||
            m_WVPLocation == 0xFFFFFFFF ||
            m_WorldMatrixLocation == 0xFFFFFFFF ||
            m_samplerLocation == 0xFFFFFFFF ||
            m_dirLightLocation.Color == 0xFFFFFFFF ||
            m_dirLightLocation.DiffuseIntensity == 0xFFFFFFFF ||
            m_dirLightLocation.Direction == 0xFFFFFFFF)
            return false;

        return true;
    }

    void SetWVP(const Matrix4f& WVP) {
        glUniformMatrix4fv(m_WVPLocation, 1, GL_TRUE, (const GLfloat*)WVP.m);
    }
    void SetWorldMatrix(const Matrix4f& WorldInverse) {
        glUniformMatrix4fv(m_WorldMatrixLocation, 1, GL_TRUE, (const GLfloat*)WorldInverse.m);
    }
    void SetTextureUnit(unsigned int TextureUnit) {
        glUniform1i(m_samplerLocation, TextureUnit);
    }
    void SetDirectionalLight(const DirectionLight& Light) {
        glUniform3f(m_dirLightLocation.Color, Light.Color.x, Light.Color.y, Light.Color.z);
        glUniform1f(m_dirLightLocation.AmbientIntensity, Light.AmbientIntensity);
        Vector3f Direction = Light.Direction;
        Direction.Normalize();
        glUniform3f(m_dirLightLocation.Direction, Direction.x, Direction.y, Direction.z);
        glUniform1f(m_dirLightLocation.DiffuseIntensity, Light.DiffuseIntensity);
    }

    void SetEyeWorldPos(const Vector3f& EyeWorldPos) {
        glUniform3f(m_eyeWorldPosition, EyeWorldPos.x, EyeWorldPos.y, EyeWorldPos.z);
    }
    void SetMatSpecularIntensity(float Intensity) {
        glUniform1f(m_matSpecularIntensityLocation, Intensity);
    }
    void SetMatSpecularPower(float Power) {
        glUniform1f(m_matSpecularPowerLocation, Power);
    }
        
private:
    GLuint m_WVPLocation;
    GLuint m_WorldMatrixLocation;
    GLuint m_samplerLocation;

    GLuint m_eyeWorldPosition;
    GLuint m_matSpecularIntensityLocation;
    GLuint m_matSpecularPowerLocation;

    struct {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint Direction;
        GLuint DiffuseIntensity;
    } m_dirLightLocation;
};
#endif

/*
static void RenderSceneCB() {
    pGameCamera->OnRender();

    glClear(GL_COLOR_BUFFER_BIT);

    static float Scale = 0.0f;
    Scale += 0.0001f;

    Pipeline p; // Изменение свойств сцены
    p.Scale(0.33f, 0.33f, 0.33f);
    p.Rotate(0.0f, Scale, 0.0f);
    p.WorldPos(0.0f, 0.0f, 0.0f);

    // Изменение свойств камеры
    p.SetCamera(pGameCamera->GetPos(), pGameCamera->GetTarget(), pGameCamera->GetUp());
    p.SetPerspectiveProj(60.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f, 100.0f);

    glUniformMatrix4fv(gWVPLocation, 1, GL_TRUE, (const GLfloat*)p.GetTrans());

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    pTexture->Bind(GL_TEXTURE0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glutSwapBuffers();
}

static void SpecialKeyboardCB(int Key, int x, int y) {
    pGameCamera->OnKeyboard(Key);
}

static void KeyboardCB(unsigned char Key, int x, int y) {
    switch (Key) {
    case 'q': exit(0);
    }
}

static void PassiveMouseCB(int x, int y) {
    pGameCamera->OnMouse(x, y);
}


static void InitializeGlutCallbacks() {
    glutDestroyWindow(1);
    glutDisplayFunc(RenderSceneCB);
    glutIdleFunc(RenderSceneCB);
    glutSpecialFunc(SpecialKeyboardCB);
    glutPassiveMotionFunc(PassiveMouseCB);
    glutKeyboardFunc(KeyboardCB);
}

static void CreateVertexBuffer() {
    /*
    Vertex Vertices[8] = { Vertex(Vector3f(1.0f, 1.0f, 1.0f),Vector2f(0.0f, 0.0f)),
                           Vertex(Vector3f(-1.0f, 1.0f, -1.0f), Vector2f(0.0f, 0.1f)),
                           Vertex(Vector3f(-1.0f, 1.0f, 1.0f),  Vector2f(1.0f, 0.0f)),
                           Vertex(Vector3f(1.0f, -1.0f, -1.0f), Vector2f(1.0f, 1.0f)),
                           Vertex(Vector3f(-1.0f, -1.0f, -1.0f), Vector2f(0.0f, 0.0f)),
                           Vertex(Vector3f(1.0f, 1.0f, -1.0f),  Vector2f(0.0f, 1.0f)),
                           Vertex(Vector3f(1.0f, -1.0f, 1.0f),   Vector2f(1.0f, 0.0f)),
                           Vertex(Vector3f(-1.0f, -1.0f, 1.0f),  Vector2f(1.0f, 1.0f)) };
                           

    Vertex Vertices[4] = { Vertex(Vector3f(-1.0f, -1.0f, 0.5f),Vector2f(0.0f, 0.0f)),
                           Vertex(Vector3f(0.0f, -1.0f, -1.0f), Vector2f(0.5f, 0.0f)),
                           Vertex(Vector3f(1.0f, -1.0f, 0.5f),  Vector2f(1.0f, 0.0f)),
                           Vertex(Vector3f(0.0f, 1.0f, 0.0f), Vector2f(0.5f, 1.0f)) };

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static void CreateIndexBuffer() {
    /*
    unsigned int Indices[] = { 0, 1, 2, 1, 3, 4,
                              5, 6, 3, 7, 3, 6,
                              2, 4, 7, 0, 7, 6,
                              0, 5, 1, 1, 5, 3,
                              5, 0, 6, 7, 4, 3,
                              2, 1, 4, 0, 2, 7 };
    
    unsigned int Indices[] = { 0, 3, 1,
                               1, 3, 2,
                               2, 3, 0,
                               1, 2, 0 };
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
*/