#ifndef RANDOM_TEXTURE_H
#define	RANDOM_TEXTURE_H

#include <GL/glew.h>
#include <ctime>

#include "Math_3d.h"
#include "Util.h"

float RandomFloat() {
    return (float)(rand()) / (float)(rand());
}

class RandomTexture {
public:
    RandomTexture() {
        m_textureObj = 0;
    }

    ~RandomTexture() {
        if (m_textureObj != 0)
            glDeleteTextures(1, &m_textureObj);
    }

    bool InitRandomTexture(unsigned int Size) {
        srand(time(nullptr));
        Vector3f* pRandomData = new Vector3f[Size];
        for (unsigned int i = 0; i < Size; i++) {
            pRandomData[i].x = RandomFloat();
            pRandomData[i].y = RandomFloat();
            pRandomData[i].z = RandomFloat();
        }

        glGenTextures(1, &m_textureObj);
        glBindTexture(GL_TEXTURE_1D, m_textureObj);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, Size, 0.0f, GL_RGB, GL_FLOAT, pRandomData);
        glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);

        delete[] pRandomData;

        return GLCheckError();
    }

    void Bind(GLenum TextureUnit) {
        glActiveTexture(TextureUnit);
        glBindTexture(GL_TEXTURE_1D, m_textureObj);
    }

private:
    GLuint m_textureObj;
};
#endif