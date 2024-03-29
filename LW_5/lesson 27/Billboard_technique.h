#ifndef BILLBOARD_TECHNIQUE_H
#define	BILLBOARD_TECHNIQUE_H

#include "Technique.h"
#include "Math_3d.h"
#include "Util.h"

static const char* pVS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
layout (location = 0) in vec3 Position;                                             \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    gl_Position = vec4(Position, 1.0);                                              \n\
}                                                                                   \n\
";

static const char* pGS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
layout(points) in;                                                                  \n\
layout(triangle_strip) out;                                                         \n\
layout(max_vertices = 4) out;                                                       \n\
                                                                                    \n\
uniform mat4 gVP;                                                                   \n\
uniform vec3 gCameraPos;                                                            \n\
                                                                                    \n\
out vec2 TexCoord;                                                                  \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    vec3 Pos = gl_in[0].gl_Position.xyz;                                            \n\
    vec3 toCamera = normalize(gCameraPos - Pos);                                    \n\
    vec3 up = vec3(0.0, 1.0, 0.0);                                                  \n\
    vec3 right = cross(toCamera, up);                                               \n\
                                                                                    \n\
    Pos -= (right * 0.5);                                                           \n\
    gl_Position = gVP * vec4(Pos, 1.0);                                             \n\
    TexCoord = vec2(0.0, 0.0);                                                      \n\
    EmitVertex();                                                                   \n\
                                                                                    \n\
    Pos.y += 1.0;                                                                   \n\
    gl_Position = gVP * vec4(Pos, 1.0);                                             \n\
    TexCoord = vec2(0.0, 1.0);                                                      \n\
    EmitVertex();                                                                   \n\
                                                                                    \n\
    Pos.y -= 1.0;                                                                   \n\
    Pos += right;                                                                   \n\
    gl_Position = gVP * vec4(Pos, 1.0);                                             \n\
    TexCoord = vec2(1.0, 0.0);                                                      \n\
    EmitVertex();                                                                   \n\
                                                                                    \n\
    Pos.y += 1.0;                                                                   \n\
    gl_Position = gVP * vec4(Pos, 1.0);                                             \n\
    TexCoord = vec2(1.0, 1.0);                                                      \n\
    EmitVertex();                                                                   \n\
                                                                                    \n\
    EndPrimitive();                                                                 \n\
}                                                                                   \n\
";

static const char* pFS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
uniform sampler2D gColorMap;                                                        \n\
                                                                                    \n\
in vec2 TexCoord;                                                                   \n\
out vec4 FragColor;                                                                 \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    FragColor = texture2D(gColorMap, TexCoord);                                     \n\
                                                                                    \n\
    if (FragColor.r > 0.9f && FragColor.g > 0.9f && FragColor.b > 0.9f) {           \n\
        discard;                                                                    \n\
    }                                                                               \n\
}";

class BillboardTechnique : public Technique {
public:
    BillboardTechnique() {}

    virtual bool Init() {
        if (!Technique::Init())
            return false;
        if (!AddShader(GL_VERTEX_SHADER, pVS))
            return false;
        if (!AddShader(GL_GEOMETRY_SHADER, pGS))
            return false;
        if (!AddShader(GL_FRAGMENT_SHADER, pFS))
            return false;
        if (!Finalize())
            return false;

        m_VPLocation = GetUniformLocation("gVP");
        m_cameraPosLocation = GetUniformLocation("gCameraPos");
        m_colorMapLocation = GetUniformLocation("gColorMap");

        if (m_VPLocation == INVALID_UNIFORM_LOCATION ||
            m_cameraPosLocation == INVALID_UNIFORM_LOCATION ||
            m_colorMapLocation == INVALID_UNIFORM_LOCATION) {
            return false;
        }
        return true;
    }

    void SetVP(const Matrix4f& VP) {
        glUniformMatrix4fv(m_VPLocation, 1, GL_TRUE, (const GLfloat*)VP.m);
    }
    void SetCameraPosition(const Vector3f& Pos) {
        glUniform3f(m_cameraPosLocation, Pos.x, Pos.y, Pos.z);
    }
    void SetColorTextureUnit(unsigned int TextureUnit) {
        glUniform1i(m_colorMapLocation, TextureUnit);
    }

private:
    GLuint m_VPLocation;
    GLuint m_cameraPosLocation;
    GLuint m_colorMapLocation;
};
#endif