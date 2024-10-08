#ifndef READVTK_H
#define READVTK_H

#include "volumeBase.h"


namespace ReadVTK
{
    
    // Struct for VTK header info
    struct VTKHeader {
        bool binary;
        glm::ivec3 dimensions;
        glm::vec3 origin;
        glm::vec3 spacing;
        std::string datatype;

        VTKHeader() :
            binary(true),
            dimensions(glm::ivec3(0, 0, 0)),
            origin(glm::vec3(0.0f, 0.0f, 0.0f)),
            spacing(glm::vec3(0.0f, 0.0f, 0.0f)),
            datatype("")
        {}
    };

    bool isVTKFile(const std::string &filename);

    // Extract data format (ASCII or BINARY) from header strings
    bool extractFormat(const std::vector<std::string> &headerLines, VTKHeader *header);

    // Extract volume dimensions (width, height, depth) from header
    // strings
    bool extractDimensions(const std::vector<std::string> &headerLines, VTKHeader *header);

    // Extract volume origin (ox, oy, oz) from header strings
    bool extractOrigin(const std::vector<std::string> &headerLines, VTKHeader *header);

    // Extract voxel spacing (sx, sy, sz) from header strings
    bool extractSpacing(const std::vector<std::string> &headerLines, VTKHeader *header);

    // Extract voxel data type from header strings
    bool extractDataType(const std::vector<std::string> &headerLines, VTKHeader *header);

    // Check endianness of target architecture
    bool isLittleEndian();

    // Swap byte order of 2-byte element
    void swap2Bytes(unsigned char* &ptr);

    // Swap byte order of 4-byte element
    void swap4Bytes(unsigned char* &ptr);

    // Swap byte order of image data elements
    template<typename T>
    void swapByteOrder(std::vector<T> *imageData);

    // Read image data in binary format
    template<typename T>
    void readVTKBinary(std::ifstream &is, std::vector<T> *imageData, int n);

    // Read image data in ASCII format
    template<typename T>
    void readVTKASCII(std::ifstream &is, std::vector<T> *imageData, int n);

    // Read the header part (the first ten lines) of the file
    bool readHeader(const std::string filename, VTKHeader *header);

    // Read the data part of the file
    template<typename VoxelType>
    bool readData(const std::string filename, const VTKHeader &header, std::vector<VoxelType> *imageData);

    
    // Swap byte order of image data elements
    template<typename T>
    void swapByteOrder(std::vector<T> *imageData)
    {
        int numElements = (int)imageData->size();
        int elementSizeInBytes = sizeof(T);
        unsigned char *elementPointer = reinterpret_cast<unsigned char *>(&(*imageData)[0]);
        switch (elementSizeInBytes) {
        case 2: // uint16, int16
            for (int i = 0; i < numElements; i++, elementPointer += elementSizeInBytes) {
                swap2Bytes(elementPointer);
            }
            break;
        case 4: // uint32, float32
            for (int i = 0; i < numElements; i++, elementPointer += elementSizeInBytes) {
                swap4Bytes(elementPointer);
            }
            break;
        default:
            break;
        }
    }

    // Read image data in binary format
    template<typename T>
    void readVTKBinary(std::ifstream &is, std::vector<T> *imageData, int n)
    {
        if (isLittleEndian()) {
            is.read(reinterpret_cast<char *>(&(*imageData)[0]), sizeof(T) * n);
            swapByteOrder(imageData);
        }
        else {
            is.read(reinterpret_cast<char *>(&(*imageData)[0]), sizeof(T) * n);
        }
    }

    // Read image data in ASCII format
    template<typename T>
    void readVTKASCII(std::ifstream &is, std::vector<T> *imageData, int n)
    {
        T value;
        for(int i = 0; i < n; i++) {
            is >> value;
            imageData->push_back(value);
        }
    }

    // Read the data part of the file
    template<typename VoxelType>
    bool readData(const std::string filename, const VTKHeader &header, std::vector<VoxelType> *imageData)
    {
        std::ifstream VTKFile(filename, std::ios::binary);
        if (!VTKFile.is_open()) {
            errorLog() << "Could not open " << filename;
            return false;
        }

        // Jump to data
        int numHeaderLines = 10;
        for (int i = 0; i < numHeaderLines; i++) {
            std::string line;
            std::getline(VTKFile, line);
            if (line.empty()) {
                VTKFile.close();
                return false;
            }
        }

        // Read data
        glm::ivec3 dimensions = header.dimensions;
        int numElements = dimensions[0] * dimensions[1] * dimensions[2];
        imageData->clear();
        imageData->reserve(numElements);
        if (header.binary) {
            imageData->resize(numElements);
            readVTKBinary(VTKFile, imageData, numElements);
        }
        else {
            readVTKASCII(VTKFile, imageData, numElements);
        }

        return true;
    }

}

#endif // READVTK_H