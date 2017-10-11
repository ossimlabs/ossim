#ifndef ossimTiffStreamAdaptor_HEADER
#define ossimTiffStreamAdaptor_HEADER
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimIoStream.h>
#include <ossim/base/ossimStreamFactoryRegistry.h>
#include <tiffio.h>

namespace ossim
{
   class OSSIM_DLL TiffIStreamAdaptor
   {
   public:
      TiffIStreamAdaptor()
      {
      }
      TiffIStreamAdaptor(const ossimString& connectionString)
      {
         openStream(connectionString);
      }
      TiffIStreamAdaptor(std::shared_ptr<ossim::istream> inputStream,
                         const ossimString& connectionString="")
      {
         setStream(inputStream, connectionString);
      }

      ~TiffIStreamAdaptor()
      {
         close();
      }

      bool openStream(const ossimString& connectionString)
      {
         setStream(ossim::StreamFactoryRegistry::instance()->createIstream(connectionString));
      
         return (m_tiffStream != nullptr);
      }

      void setStream(std::shared_ptr<ossim::istream> inputStream, 
                     const ossimString& connectionString="")
      {
         m_tiffStream       = inputStream;
         m_connectionString = connectionString;
      }

      const ossimString& getConnectionString()const{return m_connectionString;}

      std::shared_ptr<ossim::istream> getStream()
      {
         return m_tiffStream;
      }
      
      void close()
      {
         m_tiffStream.reset();
      }

      static tsize_t tiffRead(thandle_t st,tdata_t buffer,tsize_t size);
      static tsize_t tiffWrite(thandle_t st,tdata_t buffer,tsize_t size);
      static int tiffClose(thandle_t st);
      static toff_t tiffSeek(thandle_t st,toff_t pos, int whence);
      static toff_t tiffSize(thandle_t st);
      static int tiffMap(thandle_t, tdata_t*, toff_t*);
      static void tiffUnmap(thandle_t, tdata_t, toff_t);

   private:
      std::shared_ptr<ossim::istream> m_tiffStream;
      ossimString                     m_connectionString;
   };   
}

#endif