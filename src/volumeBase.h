/*********************************************************************************************************************
 *
 * volumeBase.h
 *
 * Template voxel grid
 * 
 * Vol_viewer
 * Ludovic Blache
 *
 *********************************************************************************************************************/


#ifndef VOLUMEBASE_H
#define VOLUMEBASE_H


#include <iostream>

#include <vector>
#include <string>
#include <cstdint>
#include <cstdlib>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GLtools.h"


/*!
* \class VolumeBase
* \brief Represents a 3D grid of voxels
* Parents class for all types of discrete volumes
*/
template <typename VoxelType>
class VolumeBase
{
    public:

        /*------------------------------------------------------------------------------------------------------------+
        |                                        CONSTRUCTORS / DESTRUCTORS                                           |
        +-------------------------------------------------------------------------------------------------------------*/

        VolumeBase() : m_dimensions(0,0,0), m_origin(0,0,0), m_datatype("") { m_data.assign(0, 0); }

        virtual ~VolumeBase() { m_data.clear(); }


        /*------------------------------------------------------------------------------------------------------------+
        |                                       VOXEL VALUES GETTERS/SETTERS                                          |
        +-------------------------------------------------------------------------------------------------------------*/

        VoxelType getValue1ui(unsigned int _id) 
        {
            if( _id < 0 || _id >= m_data.size() )
                errorLog() << "VolumeBase::getValue1ui(): out of bound: " << _id;

            return m_data[_id]; 
        }

        VoxelType getValue3ui(unsigned int _i, unsigned int _j, unsigned int _k)
        {
            int index = 0; 
            if (_i >= 0 && _i < m_dimensions.x && _j >= 0 && _j < m_dimensions.y && _k >= 0 && _k < m_dimensions.z)
                index = (m_dimensions.x * m_dimensions.y * _k) + (m_dimensions.x * _j) + _i;
            else
            {
                errorLog() << "VolumeBase::getValue3ui(): out of bound: " << _i << " " << _j << " " << _k;
                errorLog() << "VolumeBase::getValue3ui(): out of bound: " << m_dimensions.x << " " << m_dimensions.y << " " << m_dimensions.z;
            }

            return m_data[index];
        }

        VoxelType getValue3uiBound(unsigned int _i, unsigned int _j, unsigned int _k)
        {
            int index = 0; 
            if (_i >= 0 && _i < m_dimensions.x && _j >= 0 && _j < m_dimensions.y && _k >= 0 && _k < m_dimensions.z)
                index = (m_dimensions.x * m_dimensions.y * _k) + (m_dimensions.x * _j) + _i;
            else
                return 0;

            return m_data[index];
        }

        VoxelType getValue3ui(glm::ivec3 _uiCoords){ return getValue3ui(_uiCoords.x, _uiCoords.y, _uiCoords.z); }

        unsigned int getIdfromCoords(unsigned int _i, unsigned int _j, unsigned int _k) 
        { 
            if (_i < 0 || _i >= m_dimensions.x || _j < 0 || _j >= m_dimensions.y || _k < 0 || _k >= m_dimensions.z)
                errorLog() << "VolumeBase::getIdfromCoords(): out of bound: " << _i << " " << _j << " " << _k;

            return (m_dimensions.x * m_dimensions.y * _k) + (m_dimensions.x * _j) + _i; 
        }

        void setValue3ui(unsigned int _i, unsigned int _j, unsigned int _k, VoxelType _val) { setValue1ui( getIdfromCoords(_i, _j, _k), _val); }
        void setValue3ui(glm::ivec3 _uiCoords, VoxelType _val) { setValue3ui(_uiCoords.x, _uiCoords.y, _uiCoords.z, _val); }

        inline void setValue1ui(unsigned int _id, VoxelType _val) { m_data[_id] = _val; }

        VoxelType getValue3f(float _x, float _y, float _z)
        {
            int i = 0;
            int j = 0;
            int k = 0;
            if (_x >= m_origin.x && _y >= m_origin.y && _z >= m_origin.z)
            {
                float x2 = _x - m_origin.x;
                float y2 = _y - m_origin.y;
                float z2 = _z - m_origin.z;

                x2 /= m_spacing.x;
                y2 /= m_spacing.y;
                z2 /= m_spacing.z;

                i = (int)x2;
                j = (int)y2;
                k = (int)z2;
            }
            else
            {
                errorLog() << "VolumeBase::getValue3f(): out of bound: " << _x << " " << _y << " " << _z;
            }

            return (*this).getValue3ui(i, j, k);
        }


        /*------------------------------------------------------------------------------------------------------------+
        |                                        ATTRIBUTES GETTERS/SETTERS                                           |
        +-------------------------------------------------------------------------------------------------------------*/

        inline glm::ivec3 getDimensions() { return m_dimensions; }
        inline glm::vec3 getOrigin() { return m_origin; }
        inline glm::vec3 getSpacing() { return m_spacing; }
        inline std::string getDatatype() { return m_datatype; }

        inline void setDimensions(glm::ivec3 _dimensions) { m_dimensions = _dimensions; }
        inline void setOrigin(glm::vec3 _origin) { m_origin = _origin; }
        inline void setSpacing(glm::vec3 _spacing) { m_spacing = _spacing; }
        inline void setDatatype(std::string _datatype) { m_datatype = _datatype; }


        /*------------------------------------------------------------------------------------------------------------+
        |                                                   MISC                                                      |
        +-------------------------------------------------------------------------------------------------------------*/

        // Computes the model matrix for the volume image. This matrix can be
        // used during rendering to scale a unit cube to the size of
        // the volume image. Assumes that the min corner of the cube is centered at origin.
        glm::mat4 volumeComputeModelMatrix()
        {
            // volume size
            glm::vec3 extent = glm::vec3(m_dimensions) * m_spacing;
            // scale to a diagonal of size 1 (so it fits in the screen)
            extent = glm::normalize(extent);

            //cube mesh size
            glm::vec3 extentC(1.0f, 1.0f, 1.0f);
            // compute factor to scale the cube to the appropriate volume size
            glm::vec3 scale = extent / extentC;

            // Assume cube min corner is centered on origin.
            // First translate center of cube on origin
            glm::mat4 modelMat = glm::scale(glm::mat4(1.0), scale);
            //glm::mat4 modelMat = glm::translate(glm::mat4(1.0), glm::vec3(-0.5f, -0.5f, -0.5f));
            // Then scale to appropriate dimensions 
            //return glm::scale(modelMat, scale);
            return glm::translate(modelMat, glm::vec3(-0.5f, -0.5f, -0.5f));
        }

        glm::mat4 volumeComputeModelMatrixSlices()
        {
            // volume size
            glm::vec3 extent = glm::vec3(m_dimensions) * m_spacing;
            // scale to a diagonal of size 1 (so it fits in the screen)
            extent = glm::normalize(extent);

            //cube mesh size
            glm::vec3 extentC(1.0f, 1.0f, 1.0f);
            // compute factor to scale the cube to the appropriate volume size
            glm::vec3 scale = extent / extentC;

            // Assume cube min corner is centered on origin.
            // First translate center of cube on origin
            //glm::mat4 modelMat = glm::scale(glm::mat4(1.0), scale);
            //glm::mat4 modelMat = glm::translate(glm::mat4(1.0), glm::vec3(-0.5f, -0.5f, -0.5f));
            // Then scale to appropriate dimensions 
            //return glm::scale(modelMat, scale);
            return glm::scale(glm::mat4(1.0), scale);
        }

        inline VoxelType* getFront() { return &m_data[0]; }

        void clear() { if(m_data.size() != 0) m_data.clear(); }

        void assign(unsigned int _nbElem, VoxelType _val) 
        {
            if( _nbElem > m_dimensions.x*m_dimensions.y*m_dimensions.z)
                errorLog() << "VolumeBase::assign(): assign more than grid dimensions: " << _nbElem;

            m_data.assign(_nbElem, _val); 
        }

        void copyData(VolumeBase<VoxelType>* _newVol) 
        {
            if( m_data.size() != _newVol->m_data.size() )
                errorLog() << "VolumeBase::copyData(): new array size larger than grid dimensions: " << _newVol->m_data.size();

            m_data = _newVol->m_data; 
        }

        glm::ivec3 coord3fto3i(glm::vec3 _3fCoords)
        {
            int i = 0;
            int j = 0;
            int k = 0;
            if (_3fCoords.x >= m_origin.x && _3fCoords.y >= m_origin.y && _3fCoords.z >= m_origin.z)
            {
                float x2 = _3fCoords.x - m_origin.x;
                float y2 = _3fCoords.y - m_origin.y;
                float z2 = _3fCoords.z - m_origin.z;

                x2 /= m_spacing.x;
                y2 /= m_spacing.y;
                z2 /= m_spacing.z;

                i = (int)x2;
                j = (int)y2;
                k = (int)z2;
            }
            else
            {
                errorLog() << "VolumeBase::coord3fto3i(): out of bound: " << _3fCoords.x << " " << _3fCoords.y << " " << _3fCoords.z;
            }

            return glm::ivec3(i, j, k);
        }

        bool is3fInData(glm::vec3 _3fCoords)
        {
            if (_3fCoords.x > m_origin.x && _3fCoords.y > m_origin.y && _3fCoords.z > m_origin.z
                && _3fCoords.x < m_origin.x + double(m_dimensions.x)*m_spacing.x && _3fCoords.y < m_origin.y + double(m_dimensions.y)*m_spacing.y && _3fCoords.z < m_origin.z + double(m_dimensions.z)*m_spacing.z )
            {
                return true;
            }
            return false;
        }

        template <typename VoxelType>
        glm::vec3 getGradient3ui(unsigned int _i, unsigned int _j, unsigned int _k)
        {
            // 3D sobel filter
            double sobelX[27] = { -1.0f, 0.0f, 1.0f,
                                  -1.0f, 0.0f, 1.0f,
                                  -1.0f, 0.0f, 1.0f,

                                  -1.0f, 0.0f, 1.0f,
                                  -2.0f, 0.0f, 2.0f,
                                  -1.0f, 0.0f, 1.0f,

                                  -1.0f, 0.0f, 1.0f,
                                  -1.0f, 0.0f, 1.0f,
                                  -1.0f, 0.0f, 1.0f };

            double sobelY[27] = { -1.0f, -1.0f, -1.0f,
                                   0.0f,  0.0f,  0.0f,
                                   1.0f,  1.0f,  1.0f,

                                  -1.0f, -2.0f, -1.0f,
                                   0.0f,  0.0f,  0.0f,
                                   1.0f,  2.0f,  1.0f,

                                  -1.0f, -1.0f, -1.0f,
                                   0.0f,  0.0f,  0.0f,
                                   1.0f,  1.0f,  1.0f };

            double sobelZ[27] = { -1.0f, -1.0f, -1.0f,
                                  -1.0f, -2.0f, -1.0f,
                                  -1.0f, -1.0f, -1.0f,

                                   0.0f,  0.0f,  0.0f,
                                   0.0f,  0.0f,  0.0f,
                                   0.0f,  0.0f,  0.0f,

                                   1.0f,  1.0f,  1.0f,
                                   1.0f,  2.0f,  1.0f,
                                   1.0f,  1.0f,  1.0f };


            double Gx, Gy, Gz;
            Gx = Gy = Gz = 0.0f;

            int cpt = 0;

            // scan  3x3x3 neighborhood
            for (unsigned int z = _k - 1; z <= _k + 1; z++)
                for (unsigned int y = _j - 1; y <= _j + 1; y++)
                    for (unsigned int x = _i - 1; x <= _i + 1; x++)
                    {
                        // get value of current voxel
                        VoxelType valImg;

                        //border checking
                        if (x >= 0 && x < m_dimensions.x && y >= 0 && y < m_dimensions.y && z >= 0 && z < m_dimensions.z)
                        {
                            valImg = getValue3ui(x, y, z);
                        }
                        else
                        {
                            valImg = getValue3ui(_i, _j, _k);
                        }

                        // apply sobel filters
                        Gx += sobelX[cpt] * valImg;
                        Gy += sobelY[cpt] * valImg;
                        Gz += sobelZ[cpt] * valImg;

                        cpt++;
                    }

            // compute final gradient vector
            return glm::vec3(Gx, Gy, Gz);
        }

    protected:

        /*------------------------------------------------------------------------------------------------------------+
        |                                                ATTRIBUTES                                                   |
        +-------------------------------------------------------------------------------------------------------------*/

        glm::ivec3 m_dimensions = { 0, 0, 0 };      /*!< volume dimensions (i.e. resolution) */
        glm::vec3 m_origin = { 0.0, 0.0, 0.0 };     /*!< volume origin (i.e. real coords of bottom corner in space) */
        glm::vec3 m_spacing = { 0.0, 0.0, 0.0 };    /*!< voxel spacing (i.e. real distance between two voxels along each axis) */
        std::string m_datatype = "";                /*!< voxel data type string */
        std::vector<VoxelType> m_data;              /*!< voxel data (i.e. voxel grid) */


        /*------------------------------------------------------------------------------------------------------------+
        |                                                   MISC                                                      |
        +-------------------------------------------------------------------------------------------------------------*/

        void resize(unsigned int _size)
        {
            if( _size > m_dimensions.x*m_dimensions.y*m_dimensions.z)
                errorLog() << "VolumeBase::resize(): new array size larger than grid dimensions: " << _size;

            m_data.resize(_size);
        }
        

};
#endif // VOLUMEBASE_H