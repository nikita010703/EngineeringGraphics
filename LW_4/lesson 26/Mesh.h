#ifndef MESH_H
#define	MESH_H

#include <assert.h>
#include <map>
#include <vector>
#include <GL/glew.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "engine_common.h"
#include "util.h"
#include "math_3d.h"
#include "texture.h"

struct Vertex {
    Vector3f m_pos;
    Vector2f m_tex;
    Vector3f m_normal;
    Vector3f m_tangent;

    Vertex() {}

    Vertex(const Vector3f& pos, const Vector2f& tex, const Vector3f& normal, const Vector3f& Tangent) {
        m_pos = pos;
        m_tex = tex;
        m_normal = normal;
        m_tangent = Tangent;

    }
};

class Mesh {
public:
    Mesh() {}
    ~Mesh() {
        Clear();
    }

    bool LoadMesh(const std::string& Filename) {
        // Release the previously loaded mesh (if it exists)
        Clear();

        bool Ret = false;
        Assimp::Importer Importer;  
        const aiScene* pScene = Importer.ReadFile(Filename.c_str(), aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace);
        if (pScene)
            Ret = InitFromScene(pScene, Filename);
        else
            printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
        return Ret;
    }

    void Render() {
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);

        for (unsigned int i = 0; i < m_Entries.size(); i++) {
            glBindBuffer(GL_ARRAY_BUFFER, m_Entries[i].VB);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);                 // position
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12); // texture coordinate
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20); // normal
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)32); // tangent

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Entries[i].IB);

            const unsigned int MaterialIndex = m_Entries[i].MaterialIndex;

            if (MaterialIndex < m_Textures.size() && m_Textures[MaterialIndex]) {
                m_Textures[MaterialIndex]->Bind(COLOR_TEXTURE_UNIT);
            }

            glDrawElements(GL_TRIANGLES, m_Entries[i].NumIndices, GL_UNSIGNED_INT, 0);
        }

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(3);
    }

private:
    bool InitFromScene(const aiScene* pScene, const std::string& Filename) {
        m_Entries.resize(pScene->mNumMeshes);
        m_Textures.resize(pScene->mNumMaterials);

        // Initialize the meshes in the scene one by one
        for (unsigned int i = 0; i < m_Entries.size(); i++) {
            const aiMesh* paiMesh = pScene->mMeshes[i];
            InitMesh(i, paiMesh);
        }
        return InitMaterials(pScene, Filename);
    }

    void InitMesh(unsigned int Index, const aiMesh* paiMesh) {
        m_Entries[Index].MaterialIndex = paiMesh->mMaterialIndex;

        std::vector<Vertex> Vertices;
        std::vector<unsigned int> Indices;

        const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

        for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
            const aiVector3D* pPos = &(paiMesh->mVertices[i]);
            const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
            const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;
            const aiVector3D* pTangent = &(paiMesh->mTangents[i]);

            Vertex v(Vector3f(pPos->x, pPos->y, pPos->z),
                Vector2f(pTexCoord->x, pTexCoord->y),
                Vector3f(pNormal->x, pNormal->y, pNormal->z),
                Vector3f(pTangent->x, pTangent->y, pTangent->z));

            Vertices.push_back(v);
        }

        for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
            const aiFace& Face = paiMesh->mFaces[i];
            assert(Face.mNumIndices == 3);
            Indices.push_back(Face.mIndices[0]);
            Indices.push_back(Face.mIndices[1]);
            Indices.push_back(Face.mIndices[2]);
        }

        m_Entries[Index].Init(Vertices, Indices);
    }
    bool InitMaterials(const aiScene* pScene, const std::string& Filename) {
        // Extract the directory part from the file name
        std::string::size_type SlashIndex = Filename.find_last_of("/");
        std::string Dir;

        if (SlashIndex == std::string::npos)
            Dir = ".";
        else if (SlashIndex == 0)
            Dir = "/";
        else
            Dir = Filename.substr(0, SlashIndex);

        bool Ret = true;
        // Initialize the materials
        for (unsigned int i = 0; i < pScene->mNumMaterials; i++) {
            const aiMaterial* pMaterial = pScene->mMaterials[i];

            m_Textures[i] = NULL;
            if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                aiString Path;
                if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                    std::string FullPath = Dir + "/" + Path.data;
                    m_Textures[i] = new Texture(GL_TEXTURE_2D, FullPath.c_str());

                    if (!m_Textures[i]->Load()) {
                        printf("Error loading texture '%s'\n", FullPath.c_str());
                        delete m_Textures[i];
                        m_Textures[i] = NULL;
                        Ret = false;
                    }
                    else
                        printf("Loaded texture '%s'\n", FullPath.c_str());
                }
            }
        }
        return Ret;
    }

    void Clear() {
        for (unsigned int i = 0; i < m_Textures.size(); i++)
            SAFE_DELETE(m_Textures[i]);
    }

#define INVALID_MATERIAL 0xFFFFFFFF

    struct MeshEntry {
        MeshEntry() {
            VB = INVALID_OGL_VALUE;
            IB = INVALID_OGL_VALUE;
            NumIndices = 0;
            MaterialIndex = INVALID_MATERIAL;
        };

        ~MeshEntry() {
            if (VB != INVALID_OGL_VALUE)
                glDeleteBuffers(1, &VB);
            if (IB != INVALID_OGL_VALUE)
                glDeleteBuffers(1, &IB);
        }

        bool Init(const std::vector<Vertex>& Vertices,
            const std::vector<unsigned int>& Indices) {
            NumIndices = Indices.size();

            glGenBuffers(1, &VB);
            glBindBuffer(GL_ARRAY_BUFFER, VB);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

            glGenBuffers(1, &IB);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * NumIndices, &Indices[0], GL_STATIC_DRAW);
            return true;
        }

        GLuint VB;
        GLuint IB;
        unsigned int NumIndices;
        unsigned int MaterialIndex;
    };

    std::vector<MeshEntry> m_Entries;
    std::vector<Texture*> m_Textures;
};
#endif	/* MESH_H */