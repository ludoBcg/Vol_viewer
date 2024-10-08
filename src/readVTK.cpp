#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <cstdio>
#include <cstring>

#include "readVTK.h"

namespace ReadVTK
{
    bool isVTKFile(const std::string &filename)
    {
        std::ifstream f(filename, std::ios::binary);
        if (!f.is_open()) {
            errorLog() << "Could not open " << filename;
            return false;
        }

        std::string checkvtk;
        f >> checkvtk; // #
        f >> checkvtk; // vtk
        f.close();
        if (!(checkvtk == "vtk" || checkvtk == "VTK")) {
            return false;
        }

        return true;
    }

    // Extract data format (ASCII or BINARY) from header strings
    bool extractFormat(const std::vector<std::string> &headerLines, VTKHeader *header)
    {
        for (auto it = headerLines.begin(); it != headerLines.end(); ++it) {
            if (it->substr(0, 6) == "BINARY") {
                header->binary = true;
                return true;
            }
            else if (it->substr(0, 5) == "ASCII") {
                header->binary = false;
                return true;
            }
        }
        return false;
    }

    // Extract volume dimensions (width, height, depth) from header
    // strings
    bool extractDimensions(const std::vector<std::string> &headerLines, VTKHeader *header)
    {
        for (auto it = headerLines.begin(); it != headerLines.end(); ++it) {
            if (it->substr(0, 10) == "DIMENSIONS") {
                int width, height, depth;
                sscanf(it->c_str(), "%*s %d %d %d", &width, &height, &depth);
                header->dimensions = glm::ivec3(width, height, depth);
                return true;
            }
        }
        return false;
    }

    // Extract volume origin (ox, oy, oz) from header strings
    bool extractOrigin(const std::vector<std::string> &headerLines, VTKHeader *header)
    {
        for (auto it = headerLines.begin(); it != headerLines.end(); ++it) {
            if (it->substr(0, 6) == "ORIGIN") {
                float ox, oy, oz;
                sscanf(it->c_str(), "%*s %f %f %f", &ox, &oy, &oz);
                header->origin = glm::vec3(ox, oy, oz);
                return true;
            }
        }
        return false;
    }

    // Extract voxel spacing (sx, sy, sz) from header strings
    bool extractSpacing(const std::vector<std::string> &headerLines, VTKHeader *header)
    {
        for (auto it = headerLines.begin(); it != headerLines.end(); ++it) {
            if (it->substr(0, 7) == "SPACING") {
                float sx, sy, sz;
                sscanf(it->c_str(), "%*s %f %f %f", &sx, &sy, &sz);
                header->spacing = glm::vec3(sx, sy, sz);
                return true;
            }
        }
        return false;
    }

    // Extract voxel data type from header strings
    bool extractDataType(const std::vector<std::string> &headerLines, VTKHeader *header)
    {
        for (auto it = headerLines.begin(); it != headerLines.end(); ++it) {
            if (it->substr(0, 7) == "SCALARS") {
                char buffer[20];
                sscanf(it->c_str(), "%*s %*s %s", buffer);
                std::string typestring(buffer);
                header->datatype = typestring;

                if (typestring == "unsigned_char") {
                    header->datatype = "uint8";
                }
                else if (typestring == "unsigned_short") {
                    header->datatype = "uint16";
                }
                else if (typestring == "short") {
                    header->datatype = "int16";
                }
                else if (typestring == "unsigned_int") {
                    header->datatype = "uint32";
                }
                else if (typestring == "float") {
                    header->datatype = "float32";
                }
                else { // unsupported or invalid data type
                    return false;
                }
                return true;
            }
        }
        return false;
    }

    // Check endianness of target architecture
    bool isLittleEndian()
    {
        union {
            unsigned long l;
            unsigned char c[sizeof(unsigned long)];
        } data;
        data.l = 1;
        return data.c[0] == 1;
    }

    // Swap byte order of 2-byte element
    void swap2Bytes(unsigned char* &ptr)
    {
        unsigned char tmp;
        tmp = ptr[0]; ptr[0] = ptr[1]; ptr[1] = tmp;
    }

    // Swap byte order of 4-byte element
    void swap4Bytes(unsigned char* &ptr)
    {
        unsigned char tmp;
        tmp = ptr[0]; ptr[0] = ptr[3]; ptr[3] = tmp;
        tmp = ptr[1]; ptr[1] = ptr[2]; ptr[2] = tmp;
    }

    // Read the header part (the first ten lines) of the file
    bool readHeader(const std::string filename, VTKHeader *header)
    {
        std::ifstream VTKFile(filename, std::ios::binary);
        if (!VTKFile.is_open()) {
            errorLog() << "ReadVTK::readHeader(): could not open " << filename;
            return false;
        }

        // Read header
        int numHeaderLines = 10;
        std::vector<std::string> headerLines;
        for (int i = 0; i < numHeaderLines; i++) {
            std::string line;
            std::getline(VTKFile, line);
            if (line.empty()) {
                VTKFile.close();
                return false;
            }
            else {
                headerLines.push_back(line);
            }
        }

        // Extract header information
        if (!extractFormat(headerLines, header)) {
            return false;
        }
        if (!extractDimensions(headerLines, header)) {
            return false;
        }
        if (!extractOrigin(headerLines, header)) {
            return false;
        }
        if (!extractSpacing(headerLines, header)) {
            return false;
        }
        if (!extractDataType(headerLines, header)) {
            return false;
        }

        return true;
    }


} // end namespace ReadVTK