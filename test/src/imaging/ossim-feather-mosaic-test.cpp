#include <ossim/base/ossimArgumentParser.h>
#include <ossim/init/ossimInit.h>
#include <ossim/imaging/ossimFeatherMosaic.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/imaging/ossimMemoryImageSource.h>

#include <cmath>
#include <iostream>

namespace
{
class ClippedImageSource : public ossimImageSource
{
public:
   ClippedImageSource(ossimScalarType scalarType,
                      double value,
                      const ossimIrect& rect)
   {
      m_image = ossimImageDataFactory::instance()->create(
         this, scalarType, 1, rect.width(), rect.height());
      m_image->setImageRectangle(rect);
      m_image->initialize();
      m_image->fill(value);
      m_image->validate();
   }

   virtual ossimRefPtr<ossimImageData> getTile(
      const ossimIrect& rect, ossim_uint32 = 0)
   {
      return rect.intersects(m_image->getImageRectangle()) ? m_image : 0;
   }
   virtual ossimIrect getBoundingRect(ossim_uint32 = 0) const
   {
      return m_image->getImageRectangle();
   }
   virtual ossimScalarType getOutputScalarType() const
   {
      return m_image->getScalarType();
   }
   virtual ossim_uint32 getNumberOfInputBands() const { return 1; }
   virtual ossim_uint32 getNumberOfOutputBands() const { return 1; }
   virtual void initialize() {}
   virtual double getNullPixelValue(ossim_uint32 = 0) const
   {
      return m_image->getNullPix(0);
   }
   virtual double getMinPixelValue(ossim_uint32 = 0) const
   {
      return m_image->getMinPix(0);
   }
   virtual double getMaxPixelValue(ossim_uint32 = 0) const
   {
      return m_image->getMaxPix(0);
   }
   virtual bool canConnectMyInputTo(
      ossim_int32, const ossimConnectableObject*) const { return false; }

private:
   ossimRefPtr<ossimImageData> m_image;
};

ossimRefPtr<ossimMemoryImageSource> makeSource(ossimScalarType scalarType,
                                               double value,
                                               ossim_uint32 size = 32)
{
   ossimRefPtr<ossimImageData> image =
      ossimImageDataFactory::instance()->create(
         0, scalarType, 1, size, size);
   image->initialize();
   image->fill(value);
   image->validate();

   ossimRefPtr<ossimMemoryImageSource> source = new ossimMemoryImageSource;
   source->setImage(image);
   source->setRect(0, 0, size, size);
   return source;
}
}

int main(int argc, char* argv[])
{
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->initialize(ap);

   ossimRefPtr<ossimMemoryImageSource> eightBit =
      makeSource(OSSIM_UINT8, 128.0);
   ossimRefPtr<ossimMemoryImageSource> elevenBit =
      makeSource(OSSIM_USHORT11, 1024.0);
   ossimRefPtr<ossimMemoryImageSource> sixteenBit =
      makeSource(OSSIM_UINT16, 32768.0);

   ossimRefPtr<ossimFeatherMosaic> mosaic = new ossimFeatherMosaic;
   mosaic->connectMyInputTo(0, eightBit.get());
   mosaic->connectMyInputTo(1, elevenBit.get());
   mosaic->connectMyInputTo(2, sixteenBit.get());
   mosaic->initialize();

   const ossimIrect rect(0, 0, 31, 31);
   ossimRefPtr<ossimImageData> result = mosaic->getTile(rect);
   if(!result.valid() || result->getScalarType() != OSSIM_UINT16)
   {
      std::cerr << "Feather mosaic did not promote mixed inputs to 16-bit output; got "
                << (result.valid() ? result->getScalarType() : OSSIM_SCALAR_UNKNOWN)
                << ".\n";
      return 1;
   }

   if((result->getMinPix(0) != 1.0) ||
      (result->getMaxPix(0) != 65535.0))
   {
      std::cerr << "Promoted output range was " << result->getMinPix(0)
                << " to " << result->getMaxPix(0)
                << ", expected 1 to 65535.\n";
      return 1;
   }

   const ossim_uint16* pixels = result->getUshortBuf();
   const ossim_uint16 center = pixels[16*32 + 16];
   if((center <= 1) || (center >= 65535) ||
      (std::abs(static_cast<int>(center) - 32768) > 2))
   {
      std::cerr << "Normalized feather center was " << center
                << ", expected approximately 32768.\n";
      return 1;
   }

   ossimRefPtr<ossimFeatherMosaic> collapsed = new ossimFeatherMosaic;
   collapsed->connectMyInputTo(0, makeSource(OSSIM_UINT8, 128.0, 1).get());
   collapsed->connectMyInputTo(1, makeSource(OSSIM_USHORT11, 1024.0, 1).get());
   collapsed->initialize();
   result = collapsed->getTile(ossimIrect(0, 0, 0, 0));
   if(!result.valid() || !result->getBuf() || result->isNull(0))
   {
      std::cerr << "Collapsed Feather footprint produced a null display pixel.\n";
      return 1;
   }

   ossimRefPtr<ClippedImageSource> top = new ClippedImageSource(
      OSSIM_UINT8, 128.0, ossimIrect(0, 0, 7, 3));
   ossimRefPtr<ClippedImageSource> bottom = new ClippedImageSource(
      OSSIM_USHORT11, 1024.0, ossimIrect(0, 4, 7, 7));
   ossimRefPtr<ossimFeatherMosaic> clipped = new ossimFeatherMosaic;
   clipped->connectMyInputTo(0, top.get());
   clipped->connectMyInputTo(1, bottom.get());
   clipped->initialize();
   result = clipped->getTile(ossimIrect(0, 0, 7, 7));
   if(!result.valid() || result->isNull(1*8 + 3) || result->isNull(5*8 + 3))
   {
      std::cerr << "Clipped Feather inputs were not mapped by image origin.\n";
      return 1;
   }

   return 0;
}
