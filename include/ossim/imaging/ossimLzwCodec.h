//---
// License: MIT
// Description: class declaration for LZW codec.
//---

#ifndef OSSIM_LZW_CODEC_H
#define OSSIM_LZW_CODEC_H 1

#include <ossim/imaging/ossimCodecBase.h>
#include <vector>

class OSSIM_DLL ossimLzwCodec : public ossimCodecBase
{
public:
   ossimLzwCodec();
   virtual ~ossimLzwCodec();
   
   // ossimCodecBase interface
   virtual ossimString getCodecType() const override;
   virtual bool encode(const ossimRefPtr<ossimImageData>& in,
                       std::vector<ossim_uint8>& out) const override;
   virtual bool decode(const std::vector<ossim_uint8>& in,
                       ossimRefPtr<ossimImageData>& out) const override;
   virtual const std::string& getExtension() const override;
   
private:
   // Internal non-STL LZW implementation helpers
   bool encodeLzw(const std::vector<ossim_uint8>& input,
                  std::vector<ossim_uint8>& output) const;
   bool decodeLzw(const std::vector<ossim_uint8>& input,
                  std::vector<ossim_uint8>& output) const;
   
   static const std::string EXTENSION;
};
#endif
