#ifndef BILLBOARD_LIST_H
#define	BILLBOARD_LIST_H

#include <string>
#include "Texture.h"
#include "Billboard_technique.h"
#include "Util.h"
#include "Engine_common.h"

#define NUM_ROWS 10
#define NUM_COLUMNS 10

class BillboardList {
public:
    BillboardList() {
        m_pTexture = NULL;
        m_VB = INVALID_OGL_VALUE;
    }

    ~BillboardList() {
        SAFE_DELETE(m_pTexture);
        if (m_VB != INVALID_OGL_VALUE)
            glDeleteBuffers(1, &m_VB);
    }

    bool Init(const std::string& TexFilename) {
        m_pTexture = new Texture(GL_TEXTURE_2D, TexFilename.c_str());
        if (!m_pTexture->Load())
            return false;

        CreatePositionBuffer();
        if (!m_technique.Init())
            return false;

        return true;
    }

    void Render(const Matrix4f& VP, const Vector3f& CameraPos) {
        m_technique.Enable();
        m_technique.SetVP(VP);
        m_technique.SetCameraPosition(CameraPos);
        m_pTexture->Bind(COLOR_TEXTURE_UNIT);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, m_VB);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glDrawArrays(GL_POINTS, 0, NUM_ROWS * NUM_COLUMNS);
        glDisableVertexAttribArray(0);
    }

private:
    void CreatePositionBuffer() {
        Vector3f Positions[NUM_ROWS * NUM_COLUMNS];
        for (unsigned int j = 0; j < NUM_ROWS; j++) {
            for (unsigned int i = 0; i < NUM_COLUMNS; i++) {
                Vector3f Pos((float)i - 5.0f, 0.0f, (float)j - 5.0f);
                Positions[j * NUM_COLUMNS + i] = Pos;
            }
        }
        glGenBuffers(1, &m_VB);
        glBindBuffer(GL_ARRAY_BUFFER, m_VB);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Positions), &Positions[0], GL_STATIC_DRAW);
    }

    GLuint m_VB;
    Texture* m_pTexture;
    BillboardTechnique m_technique;
};
#endif