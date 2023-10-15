/*********************************************************************************************************************
 *
 * volumeImg.h
 *
 * 8b 3D image
 * 
 * Vol_viewer
 * Ludovic Blache
 *
 *********************************************************************************************************************/


#ifndef VOLUMEIMG_H
#define VOLUMEIMG_H

#define NOMINMAX // avoid min*max macros to interfer with std::min/max


#include "volumeBase.h"

#include <fstream>


/*!
* \class VolumeImg
* \brief Represents a 3D image with 8b data
* Uses beamformer to update its content from ultrasound data
*/
class VolumeImg : public VolumeBase<uint8_t>
{

    public:

        VolumeImg() : VolumeBase<uint8_t>() {}

        virtual ~VolumeImg() {m_data.clear();}

        void volumeInit();

        void volumeLoad(const std::string& filename);

        

    protected:

        bool volumeLoadVTK(const std::string& filename);
        bool volumeLoadRAW(const std::string& filename);

        std::uint8_t Int16ToUint8(std::int16_t _imageVal);
        void convertInt16ToUint8(std::vector<int16_t>* _imageData);


};

#endif // VOLUMEIMG_H