/*********************************************************************************************************************
 *
 * utils.h
 *
 * Collection of helpers functions
 * 
 * Vol_viewer
 * Ludovic Blache
 *
 *********************************************************************************************************************/


#ifndef UTILS_H
#define UTILS_H

#include "volumeBase.h"

#define QT_NO_OPENGL_ES_2
#include <GL/glew.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>
#define NOMINMAX // avoid min*max macros to interfer with std::min/max from <windows.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>

#include "GLtools.h"


        /*------------------------------------------------------------------------------------------------------------+
        |                                             MISC. CLASSES                                                   |
        +------------------------------------------------------------------------------------------------------------*/

struct Gbuffer
{
    GLuint colTex = 0;      /*!< G-buffer color screen-textures */
    GLuint normTex = 0;     /*!< G-buffer normal screen-texture */
    GLuint posTex = 0;      /*!< G-buffer position screen-texture */
};

struct RayCasting
{          
    GLuint frontPosTex = 0; /*!< Front face bounding geometry position screen-texture */
    GLuint backPosTex = 0;  /*!< Back face bounding geometry position screen-texture */
    GLuint volTex = 0;      /*!< Volume 3D texture */
};

struct MVPmatrices
{
    glm::mat4 modelMat = glm::mat4(1.0);
    glm::mat4 viewMat = glm::mat4(1.0);
    glm::mat4 projMat = glm::mat4(1.0);
};



        /*------------------------------------------------------------------------------------------------------------+
        |                                            MISC. FUNCTIONS                                                  |
        +------------------------------------------------------------------------------------------------------------*/


namespace
{

    /*!
    * \fn sphericalToEuclidean
    * \brief Spherical coordinates to Euclidean coordinates
    * \param _spherical : spherical 3D coords
    * \return 3D Euclidean coords
    */
    glm::vec3 sphericalToEuclidean(glm::vec3 _spherical)
    {
        return glm::vec3(sin(_spherical.x) * cos(_spherical.y),
                         sin(_spherical.y),
                         cos(_spherical.x) * cos(_spherical.y)) * _spherical.z;
    }



    /*!
    * \fn readShaderSource
    * \brief read shader program and copy it in a string
    * \param _filename : shader file name
    * \return string containing shader program
    */
    std::string readShaderSource(const std::string& _filename)
    {
        std::ifstream file(_filename);
        std::stringstream stream;
        stream << file.rdbuf();

        return stream.str();
    }



    /*!
    * \fn showShaderInfoLog
    * \brief print out shader info log (i.e. compilation errors)
    * \param _shader : shader
    */
    void showShaderInfoLog(GLuint _shader)
    {
        GLint logInfoLength = 0;
        glGetShaderiv(_shader, GL_INFO_LOG_LENGTH, &logInfoLength);
        std::vector<char> logInfo(logInfoLength);
        glGetShaderInfoLog(_shader, logInfoLength, &logInfoLength, &logInfo[0]);
        std::string logInfoStr(logInfo.begin(), logInfo.end());
        std::cerr << "[SHADER INFOLOG] " << logInfoStr << std::endl;
    }



    /*!
    * \fn showProgramInfoLog
    * \brief print out program info log (i.e. linking errors)
    * \param _program : program
    */
    void showProgramInfoLog(GLuint _program)
    {
        GLint logInfoLength = 0;
        glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &logInfoLength);
        std::vector<char> logInfo(logInfoLength);
        glGetProgramInfoLog(_program, logInfoLength, &logInfoLength, &logInfo[0]);
        std::string logInfoStr(logInfo.begin(), logInfo.end());
        std::cerr << "[PROGRAM INFOLOG] " << logInfoStr << std::endl;
    }



    /*!
    * \fn loadShaderProgram
    * \brief load shader program from shader files
    * \param _vertShaderFilename : vertex shader filename
    * \param _fragShaderFilename : fragment shader filename
    */
    GLuint loadShaderProgram(const std::string& _vertShaderFilename, const std::string& _fragShaderFilename, const std::string& _vertHeader = "", const std::string& _fragHeader = "")
    {
        // read headers
        std::string vertHeaderSource, fragHeaderSource;
        vertHeaderSource = readShaderSource(_vertHeader);
        fragHeaderSource = readShaderSource(_fragHeader);


        // Load and compile vertex shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        std::string vertexShaderSource = readShaderSource(_vertShaderFilename);
        if (!_vertHeader.empty())
        {
            // if headers are provided, add them to the shader
            const char* vertSources[2] = { vertHeaderSource.c_str(), vertexShaderSource.c_str() };
            glShaderSource(vertexShader, 2, vertSources, nullptr);
        }
        else
        {
            // if no header provided, the shader is contained in a single file
            const char* vertexShaderSourcePtr = vertexShaderSource.c_str();
            glShaderSource(vertexShader, 1, &vertexShaderSourcePtr, nullptr);
        }
        glCompileShader(vertexShader);
        GLint success = 0;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            errorLog() << "loadShaderProgram(): Vertex shader compilation failed:";
            showShaderInfoLog(vertexShader);
            glDeleteShader(vertexShader);
            return 0;
        }


        // Load and compile fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        std::string fragmentShaderSource = readShaderSource(_fragShaderFilename);
        if (!_fragHeader.empty())
        {
            // if headers are provided, add them to the shader
            const char* fragSources[2] = { fragHeaderSource.c_str(), fragmentShaderSource.c_str() };
            glShaderSource(fragmentShader, 2, fragSources, nullptr);
        }
        else
        {
            // if no header provided, the shader is contained in a single file
            const char* fragmentShaderSourcePtr = fragmentShaderSource.c_str();
            glShaderSource(fragmentShader, 1, &fragmentShaderSourcePtr, nullptr);
        }
        glCompileShader(fragmentShader);
        success = 0;
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            errorLog() << "loadShaderProgram(): Fragment shader compilation failed:";
            showShaderInfoLog(fragmentShader);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return 0;
        }


        // Create program object
        GLuint program = glCreateProgram();

        // Attach shaders to the program
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);


        // Link program
        glLinkProgram(program);

        // Check linking status
        success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            errorLog() << "[ERROR] loadShaderProgram(): Linking failed:";
            showProgramInfoLog(program);
            glDeleteProgram(program);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return 0;
        }

        // Clean up
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);

        errorLog().lastGLerror();

        return program;
    }




    /*!
    * \fn buildScreenFBOandTex
    * \brief Generate a FBO and attach a texture to its color output (used for various screen texture generation)
    * \param _screenFBO : reference to id of FBO to generate
    * \param _screenTex : reference to id of texture to generate
    * \param _texWidth : texture width
    * \param _texHeight : texture height
    */
    void buildScreenFBOandTex(GLuint& _screenFBO, GLuint& _screenTex, unsigned int _texWidth, unsigned int _texHeight)
    {

        // generate FBO 
        glGenFramebuffers(1, &_screenFBO);

        // generate texture
        glGenTextures(1, &_screenTex);
        glBindTexture(GL_TEXTURE_2D, _screenTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // bind FBO
        glBindFramebuffer(GL_FRAMEBUFFER, _screenFBO);

        // attach textures to color output of the FBO
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _screenTex, 0);


        // create and attach depth buffer (renderbuffer) to handle polygon occlusion properly (for 3D geometry rendering)
        unsigned int rboScreen;
        glGenRenderbuffers(1, &rboScreen);
        glBindRenderbuffer(GL_RENDERBUFFER, rboScreen);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _texWidth, _texHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboScreen);


        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            errorLog() << "buildScreenFBOandTex(): screen FBO incomplete";
        }

        // Bind default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        errorLog().lastGLerror();
    }


    /*!
    * \fn build1DTex
    * \brief Creates a 1D texture 
    * This texture is a lookup table / palette for 8b volume rendering, therefore the width is always 256
    * \param _1dTex : reference to id of texture to generate
    */
    void build1DTex(GLuint& _1dTex)
    {
        // create array of 256 values,
        std::vector<glm::vec4> values;
        for (unsigned int i = 0; i < 256; i++)
        {
            if (i > 200)     // implants
                values.push_back(glm::vec4(0.8, 0.8, 0.8, 1.0));
            else if (i > 82) // bone
                values.push_back(glm::vec4(0.97, 0.93, 0.78, 1.0));
            else if (i > 70) // cartilage & others
                values.push_back(glm::vec4(0.7, 0.68, 0.5, 1.0));
            else if (i > 61) // soft tissue
                values.push_back(glm::vec4(0.8, 0.09, 0.0, 1.0));
            else if (i > 13) // skin
                values.push_back(glm::vec4(0.97, 0.82, 0.7, 1.0));
            else if (i > 3) // fabric
                values.push_back(glm::vec4(0.8, 0.8, 0.8, 1.0));
            else             // other
                values.push_back(glm::vec4(0.9, 0.9, 0.9, 0.0));
        }

        // generate 1D texture
        glGenTextures(1, &_1dTex);
        glBindTexture(GL_TEXTURE_1D, _1dTex);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA16F, 256, 0, GL_RGBA, GL_FLOAT, &values[0]);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_1D, 0);

        errorLog().lastGLerror();
    }


    /*!
    * \fn build3DTex
    * \brief Create a 3D texture and copy volume data into it.
    * \param _volTex : reference to id of texture to generate
    * \param _vol : 3D image data (i.e., volume)
    * \param _useNearest : flag to indicate if texture uses GL_NEAREST param (if not, uses GL_LINEAR by default)
    */
    void build3DTex(GLuint& _volTex, VolumeBase<std::uint8_t>* _vol, bool _useNearest = false)
    {
        GLint param;
        _useNearest ? param = GL_NEAREST : param = GL_LINEAR;
        //_useNearest ? param = GL_NEAREST_MIPMAP_NEAREST : param = GL_LINEAR_MIPMAP_NEAREST;

        // generate 3D texture
        glGenTextures(1, &_volTex);
        glBindTexture(GL_TEXTURE_3D, _volTex);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, _vol->getDimensions().x, _vol->getDimensions().y, _vol->getDimensions().z, 0, GL_RED, GL_UNSIGNED_BYTE, _vol->getFront());
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, param);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, param);

        //glGenerateMipmap(GL_TEXTURE_3D);

        glBindTexture(GL_TEXTURE_3D, 0);

        errorLog().lastGLerror();
    }


    /*!
    * \fn update3DTex
    * \brief Update the content of  a 3D texture.
    * \param _volTex : pointer to id of texture
    * \param _vol : new 3D image data (i.e., volume)
    */
    void update3DTex(GLuint* _volTex, VolumeBase<std::uint8_t>* _vol)
    {

        glBindTexture(GL_TEXTURE_3D, *_volTex);

        glTexSubImage3D(GL_TEXTURE_3D, // target
            0, // level
            0, // x offset
            0, // y offset
            0, // z offset
            _vol->getDimensions().x,
            _vol->getDimensions().y,
            _vol->getDimensions().z,
            GL_RED, // format
            GL_UNSIGNED_BYTE, // type
            _vol->getFront()); // zeroed memory

        //glGenerateMipmap(GL_TEXTURE_3D);

        glBindTexture(GL_TEXTURE_3D, 0);

        errorLog().lastGLerror();
    }


    /*!
    * \fn buildScreenFBOandTex
    * \brief Generate a FBO and attach a texture to its color output (used for various screen texture generation)
    *        - can use a renderbuffer to handle depth buffering (for 3D geometry rendering)
             - can initialize texture with null alpha value (for TSD texture generation)
    * \param _screenFBO : pointer to id of FBO to generate
    * \param _screenTex : pointer to id of texture to generate
    * \param _texWidth : texture width
    * \param _texHeight : texture height
    * \param _useRenderBuffer : use a renderbuffer or not
    * \param _nullAlpha : initialize texture with null alpha or not
    */
    void buildScreenFBOandTex(GLuint* _screenFBO, GLuint* _screenTex, unsigned int _texWidth, unsigned int _texHeight, bool _useRenderBuffer, bool _nullAlpha)
    {

        // generate FBO 
        glGenFramebuffers(1, _screenFBO);

        // generate texture
        glGenTextures(1, _screenTex);
        glBindTexture(GL_TEXTURE_2D, *_screenTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // bind FBO
        glBindFramebuffer(GL_FRAMEBUFFER, *_screenFBO);

        // attach textures to color output of the FBO
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *_screenTex, 0);

        if (_nullAlpha)
        {
            glDrawBuffer(GL_COLOR_ATTACHMENT0); //Only need to do this once.
            // make sure that alphae channel is set to zero everywhere (for gaussian blur mask in TSD)
            GLuint clearColor[4] = { 0, 0, 0, 0 };
            glClearBufferuiv(GL_COLOR, 0, clearColor);
        }

        if (_useRenderBuffer)
        {
            // create and attach depth buffer (renderbuffer) to handle polygon occlusion properly (for 3D geometry rendering)
            unsigned int rboScreen;
            glGenRenderbuffers(1, &rboScreen);
            glBindRenderbuffer(GL_RENDERBUFFER, rboScreen);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _texWidth, _texHeight);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboScreen);
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            errorLog() << "buildScreenFBOandTex(): screen FBO incomplete";
        }

        // Bind default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        errorLog().lastGLerror();
    }


    /*!
    * \fn buildGbuffFBOandTex
    * \brief Generate a FBO and attach textures to 2 color outputs (used for G-buffer textures generation)
    *        Use a renderbuffer to handle depth buffering
    * \param _gFBO : reference to id of FBO to generate
    * \param _gBufferTex : reference to G-buffer (i.e., 3 color outputs for position, normal, and color)
    * \param _texWidth : texture width
    * \param _texHeight : texture height
    */
    void buildGbuffFBOandTex(GLuint& _gFBO, Gbuffer& _gBufferTex, unsigned int _texWidth, unsigned int _texHeight)
    {
        // generate FBO 
        glGenFramebuffers(1, &_gFBO);

        // 1st texture (position buffer)
        glGenTextures(1, &_gBufferTex.posTex);
        glBindTexture(GL_TEXTURE_2D, _gBufferTex.posTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // 2nd texture (normal buffer)
        glGenTextures(1, &_gBufferTex.normTex);
        glBindTexture(GL_TEXTURE_2D, _gBufferTex.normTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // 3rd texture (color buffer)
        glGenTextures(1, &_gBufferTex.colTex);
        glBindTexture(GL_TEXTURE_2D, _gBufferTex.colTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


        // bind FBO
        glBindFramebuffer(GL_FRAMEBUFFER, _gFBO);

        // attach textures to different color outputs of the FBO
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _gBufferTex.posTex, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, _gBufferTex.normTex, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, _gBufferTex.colTex, 0);

        // handle multiple color attachments
        GLenum attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, attachments);

        // create and attach depth buffer (renderbuffer)
        unsigned int rboGbuff;
        glGenRenderbuffers(1, &rboGbuff);
        glBindRenderbuffer(GL_RENDERBUFFER, rboGbuff);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _texWidth, _texHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboGbuff);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            errorLog() << "buildGbuffFBOandTex(): G-buffer FBO incomplete";
        }

        // Bind default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        errorLog().lastGLerror();
    }


    float lerp(float a, float b, float f)
    {
        return a + f * (b - a);
    }


    /*!
    * \fn buildRandKernel
    * \brief Generate a sample kernel in tangent-space hemisphere
    * \param _kernel : randomly sampled positions
    */
    void buildRandKernel(std::vector<glm::vec3>& _kernel)
    {
        // generate sample kernel 
        std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 and 1.0
        std::default_random_engine generator;
        _kernel.clear();

        int i = 0;
        while (i < 256)
        {
            glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator));
            if (glm::length(sample) <= 1.0)
            {
                sample = glm::normalize(sample);
                sample *= randomFloats(generator);
                float scale = (float)i / 64.0f;

                // scale samples s.t. they're more aligned to center of kernel
                scale = lerp(0.1f, 1.0f, scale * scale);
                _kernel.push_back(sample);
                i++;
            }
        }
    }


    /*!
    * \fn buildRandKernel
    * \brief Generate random rotation vectors, to be stored in a texture
    * \param _noiseTex : 2D texture to containing the output random vectors
    */
    void buildKernelRot(GLuint& _noiseTex)
    {
        std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 and 1.0
        std::default_random_engine generator;

        //create a 4x4 array of random rotation vectors oriented around the tangent-space surface normal
        std::vector<glm::vec3> ssaoNoise;
        for (unsigned int i = 0; i < 256; i++)
        {
            glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                0.0f);

            ssaoNoise.push_back(noise);
        }

        glGenTextures(1, &_noiseTex);
        glBindTexture(GL_TEXTURE_2D, _noiseTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 16, 16, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    /*!
    * \fn buildPerlinTex
    * \brief Generate pseudo random Perlin noise, to be stored in a texture
    * \param _noiseTex : 2D texture to containing the output random values
    */
    void buildPerlinTex(GLuint& _perlinTex)
    {
        std::vector<float> noise;
        for (unsigned int i = 0; i < 128; i++)
        {
            for (unsigned int j = 0; j < 128; j++)
            {
                float val = glm::perlin(glm::vec2((float)i / 128.0 * 30.0, (float)j / 128.0 * 30.0));
                // change range from [-1;1] to [0;1]
                val = (val + 1.0f) * 0.5f;

                noise.push_back( val );
            }
        }

        glGenTextures(1, &_perlinTex);
        glBindTexture(GL_TEXTURE_2D, _perlinTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 128, 128, 0, GL_RED, GL_FLOAT, &noise[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

}

#endif // UTILS_H