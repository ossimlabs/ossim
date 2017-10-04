//---
//
// License: MIT
// 
// Description: class declaration for base codec(encoder/decoder).
// 
//---
// $Id$

#include <ossim/imaging/ossimJpegCodec.h>

// we need to get rid of the jpedlib include in this header
#include <ossim/imaging/ossimJpegMemSrc.h>
// same for here
#include <ossim/imaging/ossimJpegMemDest.h>

#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimNumericProperty.h>

#include <ossim/imaging/ossimU8ImageData.h>
#include <csetjmp>     /** for jmp_buf */
#include <jpeglib.h>   /** for jpeg stuff */

/** @brief Extended error handler struct. */
struct ossimJpegErrorMgr
{
   struct jpeg_error_mgr pub;	/* "public" fields */
   jmp_buf setjmp_buffer;	/* for return to caller */
};
typedef struct ossimJpegErrorMgr* ossimJpegErrorPtr;
   
void ossimJpegErrorExit (jpeg_common_struct* cinfo)
{
   /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
   ossimJpegErrorPtr myerr = (ossimJpegErrorPtr) cinfo->err;
   
   /* Always display the message. */
   /* We could postpone this until after returning, if we chose. */
   (*cinfo->err->output_message) (cinfo);
   
   /* Return control to the setjmp point */
   longjmp(myerr->setjmp_buffer, 1);
}

ossimJpegCodec::ossimJpegCodec()
   :m_quality(100),
    m_ext("jpg")
{
}

ossimJpegCodec::~ossimJpegCodec()
{
}

ossimString ossimJpegCodec::getCodecType()const
{
   return ossimString("jpeg");
}

const std::string& ossimJpegCodec::getExtension() const
{
   return m_ext; // "jpg"
}

bool ossimJpegCodec::encode( const ossimRefPtr<ossimImageData>& in,
                             std::vector<ossim_uint8>& out )const
{
   bool result = false;
   if ( in.valid() && (in->getDataObjectStatus() != OSSIM_NULL) )
   {
      if ( in->getScalarType() == OSSIM_UINT8 )
      {
         // Open a memory stream up to put the jpeg image in memory:
         std::stringstream jpegStreamBuf;
         
         //---
         // Initialize JPEG compression library:
         // NOTE: JDIMENSION is an "unsigned int"
         //---
         struct jpeg_compress_struct cinfo;
         struct jpeg_error_mgr jerr;
         cinfo.err = jpeg_std_error( &jerr );
         jpeg_create_compress(&cinfo);
      
         // Define a custom stream destination manager for jpeglib to write compressed block:
         jpeg_cpp_stream_dest(&cinfo, jpegStreamBuf);
      
         /* Setting the parameters of the output file here */
         cinfo.image_width = in->getWidth();
         cinfo.image_height = in->getHeight();
   
         // Bands must be one or three for this writer.
         const ossim_uint32 INPUT_BANDS = in->getNumberOfBands();
         if ( (INPUT_BANDS == 1) || (INPUT_BANDS == 3) )
         {
            cinfo.input_components = INPUT_BANDS;
         }
         else
         {
            if ( INPUT_BANDS < 3 )
            {
               cinfo.input_components = 1; // Use first band.
            }
            else
            {
               cinfo.input_components = 3; // Use the first 3 bands.
            }
         }
      
         // colorspace of input image 
         if ( cinfo.input_components == 3)
         {
            cinfo.in_color_space = JCS_RGB;
         }
         else
         {
            cinfo.in_color_space = JCS_GRAYSCALE;
         }
      
         // Default compression parameters, we shouldn't be worried about these.
         jpeg_set_defaults( &cinfo );
      
         jpeg_set_quality(&cinfo, m_quality, TRUE); //limit to baseline-JPEG values
      
         // Now do the compression...
         jpeg_start_compress( &cinfo, TRUE );
      
         // Line buffer:
         ossim_uint32 buf_size = cinfo.input_components*cinfo.image_width;
         std::vector<ossim_uint8> buf(buf_size);
      
         // Compress the tile on line at a time:
      
         JSAMPROW row_pointer[1]; // Pointer to a single row.
         row_pointer[0] = (JSAMPLE*)&buf.front();

         // Get pointers to the input data:
         std::vector<const ossim_uint8*> inBuf(cinfo.input_components);
         for ( ossim_int32 band = 0; band < cinfo.input_components; ++band )
         {
            inBuf[band] = in->getUcharBuf(band);
         }

         ossim_uint32 inIdx = 0;
         for (ossim_uint32 line=0; line< cinfo.image_height; ++line)
         {
            // Convert from band sequential to band interleaved by pixel.
            ossim_uint32 outIdx = 0;
            for ( ossim_uint32 p = 0; p < cinfo.image_width; ++p )
            {
               for ( ossim_int32 band = 0; band < cinfo.input_components; ++band )
               {
                  buf[outIdx++] = inBuf[band][inIdx];
               }
               ++inIdx;
            }

            // Write it...
            jpeg_write_scanlines( &cinfo, row_pointer, 1 );
         }
      
         // Similar to read file, clean up after we're done compressing.
         jpeg_finish_compress( &cinfo );
         jpeg_destroy_compress( &cinfo );

         // Copy the memory stream to output vector.
         out.resize(jpegStreamBuf.str().size());
         jpegStreamBuf.seekg(0, std::ios_base::beg);
         jpegStreamBuf.read((char*)&out.front(), jpegStreamBuf.str().size());

         result = true;
      }
      else // Scalar type check...
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimCodecFactory::encodeJpeg ERROR:"
            << "\nPassing non-eight bit data to eight bit encoder!" << std::endl;
      }
      
   } // Matches: if ( in.valid() ... )
   
   return result;

}

bool ossimJpegCodec::decode( const std::vector<ossim_uint8>& in,
                             ossimRefPtr<ossimImageData>& out ) const
{
   bool result = false;

   // Check for jpeg signature:
   if ( in.size() > 3 )
   {
      if ( (in[0] == 0xff) &&
           (in[1] == 0xd8) &&
           (in[2] == 0xff) )
      {
         if (in[3] == 0xe0)
         {
            result = decodeJpeg( in, out );
         }
         else if (in[3] == 0xdb)
         {
            result = decodeJpegToRgb( in, out );
         }
      }
   }
   
   return result;	
}

bool ossimJpegCodec::decodeJpeg( const std::vector<ossim_uint8>& in,
                                    ossimRefPtr<ossimImageData>& out ) const
{
   bool result = false;

   /* This struct contains the JPEG decompression parameters and pointers
    * to working space (which is allocated as needed by the JPEG library).
    */
   jpeg_decompress_struct cinfo;
   
   /* We use our private extension JPEG error handler.
    * Note that this struct must live as long as the main JPEG parameter
    * struct, to avoid dangling-pointer problems.
    */
   ossimJpegErrorMgr jerr;
   
   /* Step 1: allocate and initialize JPEG decompression object */
   
   /* We set up the normal JPEG error routines, then override error_exit. */
   cinfo.err = jpeg_std_error(&jerr.pub);
   
   jerr.pub.error_exit = ossimJpegErrorExit;
   
   /* Establish the setjmp return context for my_error_exit to use. */
   if (setjmp(jerr.setjmp_buffer) == 0)
   {
      result = true;
      
      /* Now we can initialize the JPEG decompression object. */
      jpeg_CreateDecompress(&cinfo, JPEG_LIB_VERSION, sizeof(cinfo));
      
      //---
      // Step 2: specify data source.  In this case we will uncompress from
      // memory so we will use "ossimJpegMemorySrc" in place of " jpeg_stdio_src".
      //---
      ossimJpegMemorySrc ( &cinfo, &(in.front()), (size_t)(in.size()) );
      
      /* Step 3: read file parameters with jpeg_read_header() */
      jpeg_read_header(&cinfo, TRUE);
      
      /* Step 4: set parameters for decompression */
      
      /* Step 5: Start decompressor */
      jpeg_start_decompress(&cinfo);

      const ossim_uint32 SAMPLES = cinfo.output_width;
      const ossim_uint32 LINES   = cinfo.output_height;
      const ossim_uint32 BANDS   = cinfo.output_components;

#if 0       /* Please leave for debug. (drb) */
      if ( traceDebug() )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "jpeg cinfo.output_width:  " << cinfo.output_width
            << "\njpeg cinfo.output_height: " << cinfo.output_height
            << "\njpeg cinfo.out_color_space: " << cinfo.out_color_space
            << "\n";
      }
#endif

      if ( out.valid() )
      {
         // This will resize tile if not correct.
         out->setImageRectangleAndBands(
            ossimIrect(0,0,(ossim_int32)SAMPLES-1,(ossim_int32)LINES-1), BANDS );
      }
      else
      {
         out = new ossimU8ImageData( 0, BANDS, SAMPLES, LINES );
         out->initialize();
      }
      
      // Get pointers to the cache tile buffers.
      std::vector<ossim_uint8*> destinationBuffer(BANDS);
      for (ossim_uint32 band = 0; band < BANDS; ++band)
      {
         destinationBuffer[band] = out->getUcharBuf(band);
      }
      
      std::vector<ossim_uint8> lineBuffer(SAMPLES * cinfo.output_components);
      JSAMPROW jbuf[1];
      jbuf[0] = (JSAMPROW) &(lineBuffer.front());
      
      while (cinfo.output_scanline < LINES)
      {
         // Read a line from the jpeg file.
         jpeg_read_scanlines(&cinfo, jbuf, 1);
         
         //---
         // Copy the line which if band interleaved by pixel the the band
         // separate buffers.
         //---
         ossim_uint32 index = 0;
         for (ossim_uint32 sample = 0; sample < SAMPLES; ++sample)         
         {
            for (ossim_uint32 band = 0; band < BANDS; ++band)
            {
               destinationBuffer[band][sample] = lineBuffer[index];
               ++index;
            }
         }
         
         for (ossim_uint32 band = 0; band < BANDS; ++band)
         {
            destinationBuffer[band] += SAMPLES;
         }
      }

      // Set the tile status:
      out->validate();

      // clean up...
      
      jpeg_finish_decompress(&cinfo);
      
   } // Matches: if (setjmp(jerr.setjmp_buffer) == 0)

   jpeg_destroy_decompress(&cinfo);
   
   return result;
}

bool ossimJpegCodec::decodeJpegToRgb(const std::vector<ossim_uint8>& in,
                                     ossimRefPtr<ossimImageData>& out ) const
   
{   
   bool result = false;
   
   ossim_int32 jpegColorSpace = getColorSpace( in );

   if ( jpegColorSpace == JCS_CMYK )
   {
      ossimRefPtr<ossimImageData> cmykTile = 0;

      result = decodeJpeg( in, cmykTile ); // Decode to CMYK tile.
      if ( result )
      {
         if ( cmykTile.valid() )
         {
            const ossim_uint32 INPUT_BANDS = cmykTile->getNumberOfBands();
            if ( INPUT_BANDS == 4 )
            {
               const ossim_uint32 OUTPUT_BANDS = 3;
               const ossimIrect   RECT         = cmykTile->getImageRectangle();
               const ossim_uint32 LINES        = RECT.height();
               const ossim_uint32 SAMPLES      = RECT.width();
               ossim_uint32 band = 0;
               
               // Set or create output tile:
               if ( out.valid() )
               {
                  // This will resize tile if not correct.
                  out->setImageRectangleAndBands( RECT, OUTPUT_BANDS );
               }
               else
               {
                  out = new ossimU8ImageData( 0, OUTPUT_BANDS, SAMPLES, LINES );
                  out->initialize();
               }
               
               // Assign pointers to bands.
               std::vector<const ossim_uint8*> inBands(INPUT_BANDS);
               for ( band = 0; band < INPUT_BANDS; ++band )
               {
                  inBands[band] = cmykTile->getUcharBuf( band );
               }
               std::vector<ossim_uint8*> outBands(INPUT_BANDS);
               for ( band = 0; band < OUTPUT_BANDS; ++band )
               {
                  outBands[band] = out->getUcharBuf( band );
               }
               
               const ossim_uint8 NP   = 0;   // null pixel
               const ossim_uint8 MAXP = 255; // max pixel
               
               std::vector<ossim_float32> cmyk(INPUT_BANDS, 0.0);
               std::vector<ossim_float32> rgb(OUTPUT_BANDS, 0.0);
               
               for ( ossim_uint32 line = 0; line < LINES; ++line )
               {
                  for (ossim_uint32 sample = 0; sample < SAMPLES; ++sample)
                  {
                     //---
                     // NOTE:
                     // This current does NOT work, colors come out wrong, with
                     // the one dataset that I have:
                     // "2015_05_05_Whitehorse_3857.gpkg"
                     // (drb - 03 June 2015)
                     //---
                     
                     cmyk[0] = inBands[0][sample]; // C
                     cmyk[1] = inBands[1][sample]; // M
                     cmyk[2] = inBands[2][sample]; // Y
                     cmyk[3] = inBands[3][sample]; // K

                     //---
                     // The red (R) color is calculated from the cyan (C) and black (K) colors.
                     // The green color (G) is calculated from the magenta (M) and black (K) colors.
                     // The blue color (B) is calculated from the yellow (Y) and black (K) colors.
                     //---
                     // rgb[0] = (255.0-cmyk[0]) * 255.0-cmyk[3];
                     // rgb[1] = (255.0-cmyk[1]) * 255.0-cmyk[3];
                     // rgb[2] = (255.0-cmyk[2]) * 255.0-cmyk[3];
                     rgb[0] = (cmyk[0]) * cmyk[3]/255.0;
                     rgb[1] = (cmyk[1]) * cmyk[3]/255.0;
                     rgb[2] = (cmyk[2]) * cmyk[3]/255.0;
                     
                     outBands[0][sample] =
                        ( (rgb[0] >= 0.0) ? ( (rgb[0] <= 255.0) ?
                                              (ossim_uint8)rgb[0] : MAXP ) : NP );
                     outBands[1][sample] =
                        ( (rgb[1] >= 0.0) ? ( (rgb[1] <= 255.0) ?
                                              (ossim_uint8)rgb[1] : MAXP ) : NP );
                     outBands[2][sample] =
                        ( (rgb[2] >= 0.0) ? ( (rgb[2] <= 255.0) ?
                                              (ossim_uint8)rgb[2] : MAXP ) : NP );
                     
                  } // End sample loop.
                  
                  // Increment pointers.
                  for (ossim_uint32 band = 0; band < OUTPUT_BANDS; ++band)
                  {
                     inBands[band]  += SAMPLES;
                     outBands[band] += SAMPLES;
                  }
                  inBands[3] += SAMPLES; // Last band of input.
                  
               } // End of line loop.
               
               // Set the tile status:
               out->validate();
               
            } // Matches sanity check: if ( INPUT_BANDS == 4 )

         }  // Matches: if ( cmykTile.valid() )
         
      } // Matches: if ( decodeJpeg( in, cmykTile ) )
      
   } // Matches: if ( jpegColorSpace == JCS_CMYK )
   else
   {
      ossimNotify(ossimNotifyLevel_WARN)
            << "ossimJpegCodec::decodeJpegRgb: WARNING: "
            << "Unhandled jpeg output color space!" << std::endl;
   }

   return result;
   
} // End: ossimJpegCodec::decodeJpegRgb( ... )

ossim_int32 ossimJpegCodec::getColorSpace( const std::vector<ossim_uint8>& in ) const
{
   J_COLOR_SPACE result = JCS_UNKNOWN;

   if ( in.size() )
   {
      jpeg_decompress_struct cinfo;
      ossimJpegErrorMgr jerr;
      cinfo.err = jpeg_std_error(&jerr.pub);
      jerr.pub.error_exit = ossimJpegErrorExit;
      
      /* Establish the setjmp return context for my_error_exit to use. */
      if (setjmp(jerr.setjmp_buffer) == 0)
      {
         jpeg_CreateDecompress(&cinfo, JPEG_LIB_VERSION, sizeof(cinfo));
         ossimJpegMemorySrc ( &cinfo, &(in.front()), (size_t)(in.size()) );
         jpeg_read_header(&cinfo, TRUE);
         result = cinfo.out_color_space;
         jpeg_destroy_decompress(&cinfo);
      }
   }
   return result;
}

void ossimJpegCodec::setProperty(ossimRefPtr<ossimProperty> property)
{
   if(property->getName() == ossimKeywordNames::QUALITY_KW)
   {
      m_quality = property->valueToString().toUInt32();
   }
}

ossimRefPtr<ossimProperty> ossimJpegCodec::getProperty(const ossimString& name)const
{
   ossimRefPtr<ossimProperty> result;
   
   if(name == ossimKeywordNames::QUALITY_KW)
   {
      result = new ossimNumericProperty(ossimKeywordNames::QUALITY_KW,
                                        ossimString::toString(m_quality),
                                        0,
                                        100);
   }
   else
   {
      result = ossimCodecBase::getProperty(name);
   }
   return result;
}

void ossimJpegCodec::getPropertyNames(std::vector<ossimString>& propertyNames)const
{
   ossimCodecBase::getPropertyNames(propertyNames);
   propertyNames.push_back(ossimKeywordNames::QUALITY_KW);
}

bool ossimJpegCodec::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   const char* quality = kwl.find(prefix, ossimKeywordNames::QUALITY_KW);
   
   if(quality)
   {
      m_quality = ossimString(quality).toUInt32();
   }
   
   return ossimCodecBase::loadState(kwl, prefix);
}

bool ossimJpegCodec::saveState(ossimKeywordlist& kwl, const char* prefix)const
{
   kwl.add(prefix, ossimKeywordNames::QUALITY_KW, m_quality);
   
   return ossimCodecBase::saveState(kwl, prefix);
}
