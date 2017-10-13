#include <ossim/support_data/TiffStreamAdaptor.h>


tsize_t ossim::TiffIStreamAdaptor::tiffRead(thandle_t st,tdata_t buffer,tsize_t size)
{
   TiffIStreamAdaptor* streamAdaptor = static_cast<TiffIStreamAdaptor*>(st);
   tsize_t result = -1;
   std::shared_ptr<ossim::istream> is = streamAdaptor->getStream();
   if(is)
   {
      if(!is->good()) is->clear();
      is->read((char*)buffer, size);

      result = is->gcount();
   }

   return result;
};

tsize_t ossim::TiffIStreamAdaptor::tiffWrite(thandle_t st, tdata_t buffer,tsize_t size)
{
   return -1;
};

int ossim::TiffIStreamAdaptor::tiffClose(thandle_t st)
{
   TiffIStreamAdaptor* streamAdaptor = static_cast<TiffIStreamAdaptor*>(st);

   streamAdaptor->close();
   
   return 0;
};

toff_t ossim::TiffIStreamAdaptor::tiffSeek(thandle_t st,toff_t pos, int whence)
{
   TiffIStreamAdaptor* streamAdaptor = static_cast<TiffIStreamAdaptor*>(st);
   toff_t result = -1;
   std::ios_base::seekdir seekDir = std::ios::beg;
   std::shared_ptr<ossim::istream> is = streamAdaptor->getStream();

   
   if(is)
   {
      // Because we are adapting, on each seek we need to clear our previous error so 
      // we can continue on.  Do not want the stream to stay in a failed state.
      //
      if(!is->good()) is->clear();

      switch(whence)
      {
         case 0: // SEEK_SET
         {
          seekDir = std::ios::beg;
          break;
         }
         case 1: // SEEK_CUR
         {
            seekDir = std::ios::cur;
            break;
         }
         case 2: // SEEK_END
         {
           seekDir = std::ios::end;
            break;
         }
      }

      is->seekg(pos, seekDir);
      result = is->tellg();
   }

   return result;
};

toff_t ossim::TiffIStreamAdaptor::tiffSize(thandle_t st)
{
   toff_t result = -1;
   TiffIStreamAdaptor* streamAdaptor = static_cast<TiffIStreamAdaptor*>(st);
   std::shared_ptr<ossim::istream> is = streamAdaptor->getStream();
   if (is)
   {
      if(!is->good()) is->clear();
      ossim_int64 currentOffset = is->tellg();
      is->seekg(0, std::ios::end);
      result = is->tellg();
      is->seekg(currentOffset);
   }
   return result;
};

int ossim::TiffIStreamAdaptor::tiffMap(thandle_t, tdata_t*, toff_t*)
{
    return 0;
};

void ossim::TiffIStreamAdaptor::tiffUnmap(thandle_t, tdata_t, toff_t)
{
    return;
};
