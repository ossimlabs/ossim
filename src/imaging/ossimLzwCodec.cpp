//---
// License: MIT
// Description: Class definition for LZW codec.
//---
#include <ossim/imaging/ossimLzwCodec.h>

const std::string ossimLzwCodec::EXTENSION = "lzw";

static const uint16_t CLEAR_CODE = 256;
static const uint16_t EOI_CODE   = 257;
static const uint16_t FIRST_CODE = 258;
static const uint16_t MAX_CODE   = 4095;
static const uint8_t  MIN_BITS   = 9;
static const uint8_t  MAX_BITS   = 12;

namespace ossim
{
   class BitWriter
   {
   public:
      void write(uint16_t code, uint8_t bits)
      {
         buffer |= (uint32_t(code) << bitCount);
         bitCount += bits;
         
         while (bitCount >= 8)
         {
            output.push_back(uint8_t(buffer & 0xFF));
            buffer >>= 8;
            bitCount -= 8;
         }
      }
      void flush()
      {
         if (bitCount)
            output.push_back(uint8_t(buffer & 0xFF));
      }
      std::vector<ossim_uint8> output;
      
   private:
      uint32_t buffer = 0;
      uint8_t bitCount = 0;
   };
   
   class BitReader
   {
   public:
      BitReader(const std::vector<ossim_uint8>& data)
         : input(data) {}
      bool read(uint16_t& code, uint8_t bits)
      {
         while (bitCount < bits)
         {
            if (idx >= input.size())
               return false;
            buffer |= (uint32_t(input[idx++]) << bitCount);
            bitCount += 8;
         }
         code = uint16_t(buffer & ((1 << bits) - 1));
         buffer >>= bits;
         bitCount -= bits;
         return true;
      }
      
   private:
      const std::vector<ossim_uint8>& input;
      size_t idx = 0;
      uint32_t buffer = 0;
      uint8_t bitCount = 0;
   };
}

ossimLzwCodec::ossimLzwCodec() {}
ossimLzwCodec::~ossimLzwCodec() {}

ossimString ossimLzwCodec::getCodecType() const
{
    return "tiff_lzw";
}

const std::string& ossimLzwCodec::getExtension() const
{
    return EXTENSION;
}

// Main encode entrypoint
bool ossimLzwCodec::encode(const ossimRefPtr<ossimImageData>& in,
                           std::vector<ossim_uint8>& out) const
{
   if (!in.valid()) return false;
   
   return encodeLzw(in->getDataBuffer(), out);
}

// Main decode entrypoint
bool ossimLzwCodec::decode(const std::vector<ossim_uint8>& in,
                           ossimRefPtr<ossimImageData>& out) const
{
   std::vector<ossim_uint8> raw;
   if (!decodeLzw(in, raw))
      return false;
   
   // Wrap raw decoded data into an ossimImageData object
#if 0 /* tmp drb */
   ossimRefPtr<ossimImageData> imgData =
      new ossimImageData();
   imgData->setDataObjectStatus(ossimImageData::OK);
   imgData->setBuf(raw.data());
   imgData->setSize(raw.size());
   out = imgData;
#endif
   return true;
}

bool ossimLzwCodec::encodeLzw(const std::vector<ossim_uint8>& input,
                              std::vector<ossim_uint8>& output) const
{
   if (input.empty())
      return true;
   
   uint16_t prefix[4096];
   uint8_t  suffix[4096];
   int32_t  hash[8192];
   
   std::memset(hash, -1, sizeof(hash));
   
   auto hashIndex = [&](uint16_t p, uint8_t s)
   {
      return (((p << 8) ^ s) & 0x1FFF);
   };
   
   uint16_t nextCode = FIRST_CODE;
   uint8_t codeSize = MIN_BITS;
   
   ossim::BitWriter writer;
   writer.write(CLEAR_CODE, codeSize);
   
   uint16_t w = input[0];
   
   for (size_t i = 1; i < input.size(); ++i)
   {
      uint8_t k = input[i];
      uint32_t h = hashIndex(w, k);
      
      while (true)
      {
         int32_t ent = hash[h];
         if (ent == -1)
            break;
         if (prefix[ent] == w && suffix[ent] == k)
         {
            w = ent;
            goto continue_loop;
         }
         h = (h + 1) & 0x1FFF;
      }
      
      writer.write(w, codeSize);
      
      if (nextCode <= MAX_CODE)
      {
         prefix[nextCode] = w;
         suffix[nextCode] = k;
         hash[h] = nextCode++;
         if (nextCode == (1u << codeSize) && codeSize < MAX_BITS)
            ++codeSize;
      }
      else
      {
         writer.write(CLEAR_CODE, codeSize);
         std::memset(hash, -1, sizeof(hash));
         nextCode = FIRST_CODE;
         codeSize = MIN_BITS;
      }
      
      w = k;
      
   continue_loop: ;
   }

   writer.write(w, codeSize);
   writer.write(EOI_CODE, codeSize);
   writer.flush();
   
   output.swap(writer.output);
   return true;
}

bool ossimLzwCodec::decodeLzw(const std::vector<ossim_uint8>& input,
                              std::vector<ossim_uint8>& output) const
{
   if (input.empty())
      return true;
   
   uint16_t prefix[4096];
   uint8_t  suffix[4096];
   uint8_t  stack[4096];
   
   ossim::BitReader reader(input);
   uint16_t nextCode = FIRST_CODE;
   uint8_t codeSize = MIN_BITS;
   
   uint16_t code = 0;
   if (!reader.read(code, codeSize) || code != CLEAR_CODE)
      return false;
   
   if (!reader.read(code, codeSize))
      return false;
   
   uint16_t oldCode = code;
   output.push_back(uint8_t(code));
   
   while (reader.read(code, codeSize))
   {
      if (code == EOI_CODE)
         break;
      if (code == CLEAR_CODE)
      {
         nextCode = FIRST_CODE;
         codeSize = MIN_BITS;
         if (!reader.read(code, codeSize))
            break;
         oldCode = code;
         output.push_back(uint8_t(code));
         continue;
      }
      
      uint16_t inCode = code;
      int top = 0;
      
      if (code >= nextCode)
      {
         stack[top++] = suffix[oldCode];
         code = oldCode;
      }
      
      while (code >= 256)
      {
         stack[top++] = suffix[code];
         code = prefix[code];
      }
      
      uint8_t first = uint8_t(code);
      stack[top++] = first;
      
      while (top)
         output.push_back(stack[--top]);
      
      if (nextCode <= MAX_CODE)
      {
         prefix[nextCode] = oldCode;
         suffix[nextCode] = first;
         ++nextCode;
         if (nextCode == (1u << codeSize) && codeSize < MAX_BITS)
            ++codeSize;
      }
      oldCode = inCode;
   }
   
   return true;
}



