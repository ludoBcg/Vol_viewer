/*********************************************************************************************************************
 *
 * drawablemesh.cpp
 * 
 * Vol_viewer
 * Ludovic Blache
 *
 *********************************************************************************************************************/


#include "drawablemesh.h"



DrawableMesh::DrawableMesh()
{
    setUseGammaCorrecFlag(false);
    setModeVR(1);

    m_vertexProvided = false;
    m_normalProvided = false;
    m_colorProvided = false;
    m_uvProvided = false;
    m_tex3dProvided = false;
    m_indexProvided = false;

    m_useAO = false;
    m_useShadow = false;
}


DrawableMesh::~DrawableMesh()
{
    glDeleteBuffers(1, &(m_vertexVBO));
    glDeleteBuffers(1, &(m_normalVBO));
    glDeleteBuffers(1, &(m_colorVBO));
    glDeleteBuffers(1, &(m_uvVBO));
    glDeleteBuffers(1, &(m_tex3dVBO));
    glDeleteBuffers(1, &(m_indexVBO));
    glDeleteVertexArrays(1, &(m_meshVAO));
}


void DrawableMesh::createScreenQuadVAO()
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;

    // generate a quad in front of the camera
    vertices = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f) };
    normals = { glm::vec3(1.0f, 1.0f,  1.0f), glm::vec3(1.0f, 1.0f,  1.0f), glm::vec3(1.0f, 1.0f,  1.0f), glm::vec3(1.0f, 1.0f,  1.0f) };

    // add UV coords so we can map textures on the screen quad
    std::vector<glm::vec2> texcoords{ glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f) };
    std::vector<uint32_t> indices {0, 1, 2, 2, 3, 0 };


    // Generates and populates a VBO for vertex coords
    glGenBuffers(1, &(m_vertexVBO));
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
    size_t verticesNBytes = vertices.size() * sizeof(vertices[0]);
    glBufferData(GL_ARRAY_BUFFER, verticesNBytes, vertices.data(), GL_STATIC_DRAW);

    // Generates and populates a VBO for vertex normals
    glGenBuffers(1, &(m_normalVBO));
    glBindBuffer(GL_ARRAY_BUFFER, m_normalVBO);
    size_t normalsNBytes = normals.size() * sizeof(normals[0]);
    glBufferData(GL_ARRAY_BUFFER, normalsNBytes, normals.data(), GL_STATIC_DRAW);

    // Generates and populates a VBO for the element indices
    glGenBuffers(1, &(m_indexVBO));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
    size_t indicesNBytes = indices.size() * sizeof(indices[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesNBytes, indices.data(), GL_STATIC_DRAW);

    // Generates and populates a VBO for UV coords
    glGenBuffers(1, &(m_uvVBO));
    glBindBuffer(GL_ARRAY_BUFFER, m_uvVBO);
    size_t texcoordsNBytes = texcoords.size() * sizeof(texcoords[0]);
    glBufferData(GL_ARRAY_BUFFER, texcoordsNBytes, texcoords.data(), GL_STATIC_DRAW);


    // Creates a vertex array object (VAO) for drawing the mesh
    glGenVertexArrays(1, &(m_meshVAO));
    glBindVertexArray(m_meshVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
    glEnableVertexAttribArray(POSITION);
    glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, m_normalVBO);
    glEnableVertexAttribArray(NORMAL);
    glVertexAttribPointer(NORMAL, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, m_uvVBO);
    glEnableVertexAttribArray(UV);
    glVertexAttribPointer(UV, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
    glBindVertexArray(m_defaultVAO); // unbinds the VAO

    // Additional information required by draw calls
    m_numVertices = (unsigned int)vertices.size();
    m_numIndices = (unsigned int)indices.size();

    // Clear temporary vectors
    vertices.clear();
    normals.clear();
    indices.clear();
}


void DrawableMesh::createUnitCubeVAO()
{

    std::vector<glm::vec3> vertices = { glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3( 0.0f,  0.0f,  0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 1.0f,  1.0f,  0.0f), 
                                        glm::vec3( 0.0f,  1.0f,  1.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3( 1.0f,  0.0f,  1.0f), glm::vec3( 1.0f,  1.0f,  1.0f) };

    std::vector<glm::vec3> normals = { glm::vec3(-0.33f,  0.33f, -0.33f), glm::vec3(-0.33f, -0.33f, -0.33f), glm::vec3( 0.33f, -0.33f, -0.33f), glm::vec3( 0.33f,  0.33f, -0.33f),
                                       glm::vec3(-0.33f,  0.33f,  0.33f), glm::vec3(-0.33f, -0.33f,  0.33f), glm::vec3( 0.33f, -0.33f,  0.33f), glm::vec3( 0.33f,  0.33f,  0.33f) };


    std::vector<uint32_t> indices{ 2, 1, 0, 0, 3, 2,   // back face
                                   5, 6, 7, 7, 4, 5,   // front face
                                   1, 5, 4, 4, 0, 1,   // left face
                                   6, 2, 3, 3, 7, 6,   // right face
                                   3, 0, 4, 4, 7, 3,   // top face
                                   6, 5, 1, 1, 2, 6 }; // bottom face
    

    // Generates and populates a VBO for vertex coords
    glGenBuffers(1, &(m_vertexVBO));
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
    size_t verticesNBytes = vertices.size() * sizeof(vertices[0]);
    glBufferData(GL_ARRAY_BUFFER, verticesNBytes, vertices.data(), GL_STATIC_DRAW);

    // Generates and populates a VBO for vertex normals
    glGenBuffers(1, &(m_normalVBO));
    glBindBuffer(GL_ARRAY_BUFFER, m_normalVBO);
    size_t normalsNBytes = normals.size() * sizeof(normals[0]);
    glBufferData(GL_ARRAY_BUFFER, normalsNBytes, normals.data(), GL_STATIC_DRAW);

    // Generates and populates a VBO for the element indices
    glGenBuffers(1, &(m_indexVBO));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
    size_t indicesNBytes = indices.size() * sizeof(indices[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesNBytes, indices.data(), GL_STATIC_DRAW);


    // Creates a vertex array object (VAO) for drawing the mesh
    glGenVertexArrays(1, &(m_meshVAO));
    glBindVertexArray(m_meshVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
    glEnableVertexAttribArray(POSITION);
    glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, m_normalVBO);
    glEnableVertexAttribArray(NORMAL);
    glVertexAttribPointer(NORMAL, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
    glBindVertexArray(m_defaultVAO); // unbinds the VAO

    // Additional information required by draw calls
    m_numVertices = (unsigned int)vertices.size();
    m_numIndices = (unsigned int)indices.size();

    // Clear temporary vectors
    vertices.clear();
    normals.clear();
    indices.clear();
}


void DrawableMesh::createSliceVAO(unsigned int _orientation)
{

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> texcoords;

    if (_orientation == 0)
    {
        // AXIAL = Z
        vertices = { glm::vec3(0.5f,  0.5f,  0.0f), glm::vec3(-0.5f,  0.5f,  0.0f), glm::vec3(-0.5f, -0.5f,  0.0f), glm::vec3(0.5f, -0.5f,  0.0f) };
        texcoords = { glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(1.0f, 0.0f, 0.0f),   glm::vec3(1.0f, 1.0f, 0.0f),   glm::vec3(0.0f, 1.0f, 0.0f) };
    }
    else if (_orientation == 1)
    {
        // CORONAL = Y
        vertices = { glm::vec3(0.5f,  0.0f,  0.5f), glm::vec3(-0.5f,  0.0f,  0.5f), glm::vec3(-0.5f,  0.0f, -0.5f), glm::vec3(0.5f,  0.0f, -0.5f) };
        texcoords = { glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(1.0f, 0.0f, 0.0f),   glm::vec3(1.0f, 0.0f, 1.0f),   glm::vec3(0.0f, 0.0f, 1.0f) };

    }
    else if (_orientation == 2)
    {
        // SAGITAL = X
        vertices = { glm::vec3(0.0f,  0.5f,  0.5f), glm::vec3(0.0f,  0.5f, -0.5f), glm::vec3(0.0f, -0.5f, -0.5f), glm::vec3(0.0f, -0.5f,  0.5f) };
        texcoords = { glm::vec3(0.0f, 0.0f, 0.0f),   glm::vec3(0.0f, 0.0f, 1.0f),   glm::vec3(0.0f, 1.0f, 1.0f),   glm::vec3(0.0f, 1.0f, 0.0f) };
    }
    else
        std::cerr << "DrawableMesh::createSliceVAO(): Invalide Slice orientation" << std::endl;

    std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };
    


    // Generates and populates a VBO for vertex coords
    glGenBuffers(1, &(m_vertexVBO));
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
    size_t verticesNBytes = vertices.size() * sizeof(vertices[0]);
    glBufferData(GL_ARRAY_BUFFER, verticesNBytes, vertices.data(), GL_STATIC_DRAW);

    // Generates and populates a VBO for 3D Tex coords
    glGenBuffers(1, &(m_tex3dVBO));
    glBindBuffer(GL_ARRAY_BUFFER, m_tex3dVBO);
    size_t texcoordsNBytes = texcoords.size() * sizeof(texcoords[0]);
    glBufferData(GL_ARRAY_BUFFER, texcoordsNBytes, texcoords.data(), GL_STATIC_DRAW);

    // Generates and populates a VBO for the element indices
    glGenBuffers(1, &(m_indexVBO));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
    size_t indicesNBytes = indices.size() * sizeof(indices[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesNBytes, indices.data(), GL_STATIC_DRAW);


    // Creates a vertex array object (VAO) for drawing the mesh
    glGenVertexArrays(1, &(m_meshVAO));
    glBindVertexArray(m_meshVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
    glEnableVertexAttribArray(POSITION);
    glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, m_tex3dVBO);
    glEnableVertexAttribArray(TEX3D);
    glVertexAttribPointer(TEX3D, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
    glBindVertexArray(m_defaultVAO); // unbinds the VAO

    // Additional information required by draw calls
    m_numVertices = (unsigned int)vertices.size();
    m_numIndices = (unsigned int)indices.size();

    // Clear temporary vectors
    vertices.clear();
    texcoords.clear();
    indices.clear();
}


void DrawableMesh::drawBoundingGeom(GLuint _program, MVPmatrices _mvpMatrices)
{
    // Activate program
    glUseProgram(_program);


    // Pass uniforms
    glUniformMatrix4fv(glGetUniformLocation(_program, "u_matM"), 1, GL_FALSE, &_mvpMatrices.modelMat[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(_program, "u_matV"), 1, GL_FALSE, &_mvpMatrices.viewMat[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(_program, "u_matP"), 1, GL_FALSE, &_mvpMatrices.projMat[0][0]);

    // Draw!
    glBindVertexArray(m_meshVAO);                       // bind the VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);  // do not forget to bind the index buffer AFTER !

    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(m_defaultVAO);

    glUseProgram(0);
}


void DrawableMesh::drawScreenQuad(GLuint _program, GLuint _tex, bool _isBlurOn, bool _isGaussH, int _filterWidth)
{

    // Activate program
    glUseProgram(_program);

    // bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _tex);

    GLint ShadowMapUniform = glGetUniformLocation(_program, "u_screenTex");
    if (ShadowMapUniform == -1) {
        fprintf(stderr, "[ERROR] DrawableMesh::drawScreenQuad(): Could not bind screen quad texture\n");
        exit(-1);
    }
    glUniform1i(ShadowMapUniform, 0);


        
    glBindVertexArray(m_meshVAO);                       // bind the VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);  // do not forget to bind the index buffer AFTER !

    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(m_defaultVAO);


    glUseProgram(0);
}


void DrawableMesh::drawDeferred(GLuint _program, Gbuffer _gBufferTex, MVPmatrices _mvpMatrices, glm::vec2 _screenDims)
{

    // Activate program
    glUseProgram(_program);

    // bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _gBufferTex.colTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _gBufferTex.normTex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _gBufferTex.posTex);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_noiseTex);

    glUniform1i(glGetUniformLocation(_program, "u_colorTex"), 0);
    glUniform1i(glGetUniformLocation(_program, "u_normalTex"), 1);
    glUniform1i(glGetUniformLocation(_program, "u_positionTex"), 2);
    for (unsigned int i = 0; i < 64; ++i)
    {
        std::string str = "u_samples[" + std::to_string(i) + "]";
        glm::vec3 myVec = m_randKernel[i];
        glUniform3fv(glGetUniformLocation(_program, str.c_str()), 1, &myVec[0]);
    }
    glUniform1i(glGetUniformLocation(_program, "u_noiseTex"), 3);
    glUniformMatrix4fv(glGetUniformLocation(_program, "u_matM"), 1, GL_FALSE, &_mvpMatrices.modelMat[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(_program, "u_matV"), 1, GL_FALSE, &_mvpMatrices.viewMat[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(_program, "u_matP"), 1, GL_FALSE, &_mvpMatrices.projMat[0][0]);
    glUniform1i(glGetUniformLocation(_program, "u_useAO"), m_useAO);
    glUniform2fv(glGetUniformLocation(_program, "u_screenDims"), 1, &_screenDims[0]);


    glBindVertexArray(m_meshVAO);                       // bind the VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);  // do not forget to bind the index buffer AFTER !

    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(m_defaultVAO);


    glUseProgram(0);
}


void DrawableMesh::drawRayCast(GLuint _program, GLuint _3dTex, GLuint _frontTex, GLuint _backTex, GLuint _1dTex, glm::mat4 _mvpMat)
{
    glUseProgram(_program);

    // bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, _3dTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _frontTex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _backTex);

    // set uniforms
    glUniform1i(glGetUniformLocation(_program, "u_volumeTexture"), 0);
    glUniform1i(glGetUniformLocation(_program, "u_frontFaceTexture"), 1);
    glUniform1i(glGetUniformLocation(_program, "u_backFaceTexture"), 2);
    glUniform1i(glGetUniformLocation(_program, "u_useGammaCorrec"), m_useGammaCorrec);
    glUniform1i(glGetUniformLocation(_program, "u_modeVR"), m_modeVR);
    glUniform1i(glGetUniformLocation(_program, "u_maxSteps"), m_maxSteps);
    glUniformMatrix4fv(glGetUniformLocation(_program, "u_matMVP"), 1, GL_FALSE, &_mvpMat[0][0]);


    // Draw!
    glBindVertexArray(m_meshVAO);                       // bind the VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);  // do not forget to bind the index buffer AFTER !

    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(m_defaultVAO);

    glUseProgram(0);
}


void DrawableMesh::drawIsoSurf(GLuint _program, GLuint _3dTex, GLuint _frontTex, GLuint _backTex, GLuint _1dTex, 
                               GLuint _isoValue, MVPmatrices _mvpMatrices, glm::vec3 _lightDir, glm::vec2 _screenDims)
{
    glUseProgram(_program);

    // bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, _3dTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _frontTex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _backTex);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_noiseTex);

    // set uniforms
    glUniform1i(glGetUniformLocation(_program, "u_volumeTexture"), 0);
    glUniform1i(glGetUniformLocation(_program, "u_frontFaceTexture"), 1);
    glUniform1i(glGetUniformLocation(_program, "u_backFaceTexture"), 2);
    glUniform1i(glGetUniformLocation(_program, "u_noiseTex"), 3);
    glUniform1i(glGetUniformLocation(_program, "u_useGammaCorrec"), m_useGammaCorrec);
    glUniform1i(glGetUniformLocation(_program, "u_maxSteps"), m_maxSteps);
    glUniform1f(glGetUniformLocation(_program, "u_isoValue"), (float)_isoValue / 255.0f);
    glUniformMatrix4fv(glGetUniformLocation(_program, "u_matM"), 1, GL_FALSE, &_mvpMatrices.modelMat[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(_program, "u_matV"), 1, GL_FALSE, &_mvpMatrices.viewMat[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(_program, "u_matP"), 1, GL_FALSE, &_mvpMatrices.projMat[0][0]);
    glUniform3fv(glGetUniformLocation(_program, "u_lightDir"), 1, &_lightDir[0]);
    glUniform1i(glGetUniformLocation(_program, "u_useShadow"), m_useShadow);
    glUniform2fv(glGetUniformLocation(_program, "u_screenDims"), 1, &_screenDims[0]);


    // Draw!
    glBindVertexArray(m_meshVAO);                       // bind the VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);  // do not forget to bind the index buffer AFTER !

    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(m_defaultVAO);

    glUseProgram(0);
}


void DrawableMesh::drawSlice(GLuint _program, MVPmatrices _mvpMatrices, glm::mat4 _tex3dMat, GLuint _3dTex, GLuint _1dTex)
{
    // Activate program
    glUseProgram(_program);

    // bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, _3dTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, _1dTex);


    // Pass uniforms
    glUniformMatrix4fv(glGetUniformLocation(_program, "u_matM"), 1, GL_FALSE, &_mvpMatrices.modelMat[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(_program, "u_matV"), 1, GL_FALSE, &_mvpMatrices.viewMat[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(_program, "u_matP"), 1, GL_FALSE, &_mvpMatrices.projMat[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(_program, "u_matTex"), 1, GL_FALSE, &_tex3dMat[0][0]);
    glUniform1i(glGetUniformLocation(_program, "u_volumeTexture"), 0);
    glUniform1i(glGetUniformLocation(_program, "u_lookupTexture"), 1);
    glUniform1i(glGetUniformLocation(_program, "u_useGammaCorrec"), m_useGammaCorrec);

    // Draw!
    glBindVertexArray(m_meshVAO);                       // bind the VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);  // do not forget to bind the index buffer AFTER !

    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(m_defaultVAO);


    glUseProgram(0);
}

