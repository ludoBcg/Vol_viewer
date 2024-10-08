/*********************************************************************************************************************
 *
 * volumeImg.cpp
 *
 * Vol_viewer
 * Ludovic Blache
 *
 *********************************************************************************************************************/

#define _USE_MATH_DEFINES
#include <math.h>
#define NOMINMAX // avoid min*max macros to interfer with std::min/max

#include <algorithm>

#include "volumeImg.h"
#include "readVTK.h"


void VolumeImg::volumeInit()
{
    // setup volume param
    m_dimensions = glm::ivec3(100 , 100, 100);
    
    m_origin = glm::vec3(0, 0, 0);
    m_spacing = glm::vec3(0.2f, 0.2f, 0.2f);
    m_datatype = "uint8";

    m_data.resize(100 * 100 * 100);

}

void VolumeImg::volumeLoad(const std::string& filename)
{
    if (filename.find(".vtk") != std::string::npos)
        volumeLoadVTK(filename);
    else if (filename.find(".raw") != std::string::npos)
        volumeLoadRAW(filename);
    else
        errorLog() << "VolumeImg::volumeLoad(): file " << filename << " format no supported";
}



bool VolumeImg::volumeLoadRAW(const std::string& filename)
{
    m_dimensions = glm::ivec3(208 , 224 , 208);

    m_origin = glm::vec3(0, 0, 0);
    m_spacing = glm::vec3(0.1f, 0.1f, 0.1f);
    m_datatype = "uint8";

    // Note: files contain [0,255] values encoded as short integers
    const int nbVoxels = 208 * 224 * 208;

    std::vector<short> dataShort;
    dataShort.resize(nbVoxels);
    FILE* file;
    errno_t err = fopen_s(&file, filename.c_str(), "rb");
    if (err != 0)
    {
        throw std::runtime_error{ "Could not open file!" };
    }
    // read short int values and copy them in a vector
    fread(dataShort.data(), sizeof(short), nbVoxels, file);

    m_data.resize(nbVoxels);

    // cast each element from short to uChar and copy them into a new vector
    std::transform(dataShort.begin(), dataShort.end(), m_data.begin(), [](auto ptr) { return static_cast<unsigned char>(ptr); });

    fclose(file);

    return true;
}



// Reads a volume image in the legacy VTK StructuredPoints format
// from a file. Returns true on success, false otherwise. Possible
// datatypes are: "uint8", "uint16", "int16", "uint32", and "float32".
// Assumes that the file starts with a ten line header
// section followed by a data section in ASCII or binary format,
// i.e.:
//
// # vtk DataFile Version x.x\n
// Some information about the file\n
// BINARY\n
// DATASET STRUCTURED_POINTS\n
// DIMENSIONS 128 128 128\n
// ORIGIN 0.0 0.0 0.0\n
// SPACING 1.0 1.0 1.0\n
// POINT_DATA 2097152\n
// SCALARS image_data unsigned_char\n
// LOOKUP_TABLE default\n
// raw data........\n
bool VolumeImg::volumeLoadVTK(const std::string& filename)
{
    if (!ReadVTK::isVTKFile(filename)) {
        return false;
    }

    // Read header
    ReadVTK::VTKHeader header;
    if (!ReadVTK::readHeader(filename, &header)) {
        return false;
    }

    m_dimensions = header.dimensions;
    m_origin = header.origin;
    m_spacing = header.spacing;
    m_datatype = header.datatype;

    std::cout << "[INFO] VolumeImage::volumeLoadVTK(): load " << filename << std::endl;
    std::cout << std::endl << "    HEADER INFO:" << std::endl;
    std::cout << "    dimension (x,y,z) :" << header.dimensions.x << " " << header.dimensions.y << " " << header.dimensions.z << std::endl;
    std::cout << "    spacing (x,y,z) :" << header.spacing.x << " " << header.spacing.y << " " << header.spacing.z << std::endl;
    std::cout << "    origin (x,y,z) :" << header.origin.x << " " << header.origin.y << " " << header.origin.z << std::endl;
    std::cout << "    datatype :" << header.datatype << std::endl << std::endl;
    std::cout << "    volume box size: " << header.dimensions.x * header.spacing.x << " " <<
              header.dimensions.y * header.spacing.y << " " << header.dimensions.z * header.spacing.z << std::endl;

    // Read data
    if (header.datatype == "uint8") {
        std::vector<uint8_t> imageData;
        if (!ReadVTK::readData(filename, header, &imageData)) {
            return false;
        }
        size_t nBytes = imageData.size() * sizeof(imageData[0]);
        m_data.resize(nBytes);
        std::memcpy(&m_data[0], &imageData[0], nBytes);
    }
    else if (header.datatype == "uint16") {
        errorLog() << "VolumeImage::volumeLoadVTK(): uint16 datatype not supported";
        std::vector<uint16_t> imageData;
        if (!ReadVTK::readData(filename, header, &imageData)) {
            return false;
        }
        size_t nBytes = imageData.size() * sizeof(imageData[0]);
        m_data.resize(nBytes);
        std::memcpy(&m_data[0], &imageData[0], nBytes);
    }
    else if (header.datatype == "int16") {
        warningLog() << "VolumeImage::volumeLoadVTK(): int16 datatype cast to uint8";
        std::vector<int16_t> imageData;
        if (!ReadVTK::readData(filename, header, &imageData)) {
            return false;
        }
        size_t nBytes = imageData.size() * sizeof(imageData[0]);
        m_data.resize(nBytes);

        // convert 16 to 8 bits
        convertInt16ToUint8(&imageData);

        // call the function Int16ToUint8 for each element of imageData and store the result in volume->data
        //std::transform(imageData.begin(), imageData.end(), m_data.begin(), Int16ToUint8 );

        //std::memcpy(&volume->data[0], &imageData[0], nBytes);
    }
    else if (header.datatype == "uint32") {
        errorLog() << "VolumeImage::volumeLoadVTK(): uint32 datatype not supported";
        std::vector<uint32_t> imageData;
        if (!ReadVTK::readData(filename, header, &imageData)) {
            return false;
        }
        size_t nBytes = imageData.size() * sizeof(imageData[0]);
        m_data.resize(nBytes);
        std::memcpy(&m_data[0], &imageData[0], nBytes);
    }
    else if (header.datatype == "float32") {
        errorLog() << "VolumeImage::volumeLoadVTK(): float32 datatype not supported";
        std::vector<float> imageData;
        if (!ReadVTK::readData(filename, header, &imageData)) {
            return false;
        }
        size_t nBytes = imageData.size() * sizeof(imageData[0]);
        m_data.resize(nBytes);
        std::memcpy(&m_data[0], &imageData[0], nBytes);
    }
    else {
        return false;
    }

    return true;
}


std::uint8_t VolumeImg::Int16ToUint8(std::int16_t _imageVal)
{
    // first ensure that the value is in [-1024 ; 3071] (apply threshold if not)
    _imageVal = std::max((std::int16_t )-1024, std::min((std::int16_t)3071, _imageVal));
    // cross multiplication to convert value into [0 ; 255]
    _imageVal = static_cast<int>(255.0f * ((float(_imageVal) + 1024.0f) / 4095.0f));
    return std::uint8_t(_imageVal);
}


void VolumeImg::convertInt16ToUint8(std::vector<int16_t>* _imageData)
{
    for (int id = 0; id < m_dimensions.x * m_dimensions.y * m_dimensions.z; id++)
    {
        float value = (*_imageData)[id];
        // int16bit data are encoded on 4095 values contained in [-1024 ; 3071]

        // first ensure that the value is in [-1024 ; 3071] (apply threshold if not)
        value = std::max(-1024.0f, std::min(3071.0f, value));
        // cross multiplication to convert value into [0 ; 255]
        value = 255.0f * ((value + 1024.0f) / 4095.0f);
        m_data[id] = static_cast<int>(value);
    }
}
