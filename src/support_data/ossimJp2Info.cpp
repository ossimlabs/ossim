//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: JP2 Info object.
// 
//----------------------------------------------------------------------------
// $Id: ossimJp2Info.cpp 23222 2015-04-05 15:44:57Z dburken $

#include <ossim/support_data/ossimJp2Info.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimEndian.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimTrace.h>
// #include <ossim/support_data/ossimJ2kCommon.h>
#include <ossim/support_data/ossimTiffInfo.h>
#include <fstream>
#include <istream>
#include <iostream>

static ossimTrace traceDebug("ossimJp2Info:debug");

ossimJp2Info::ossimJp2Info()
   : ossimJ2kInfo()
{
}

ossimJp2Info::~ossimJp2Info()
{
}

bool ossimJp2Info::open(const ossimFilename& file)
{
   bool result = false;

   std::ifstream str;
   str.open( file.c_str(), std::ios_base::in | std::ios_base::binary);

   if ( str.is_open() )
   {
      const ossim_uint8 J2K_SIGNATURE_BOX[SIGNATURE_BOX_SIZE] = 
         {0x00,0x00,0x00,0x0c,0x6a,0x50,0x20,0x20,0x0d,0x0a,0x87,0x0a};
      
      ossim_uint8 box[SIGNATURE_BOX_SIZE];
      
      // Read in the box.
      str.read((char*)box, SIGNATURE_BOX_SIZE);
      
      result = true;
      for (ossim_uint32 i = 0; i < SIGNATURE_BOX_SIZE; ++i)
      {
         if (box[i] != J2K_SIGNATURE_BOX[i])
         {
            result = false;
            break;
         }
      }
      
      str.close();
   }

   if ( result )
   {
      m_file = file; // Capture file name.
   }
   
   return result;
}

std::ostream& ossimJp2Info::print(std::ostream& out) const
{
   if ( m_file.size() )
   {
      // Open the file.
      std::ifstream str( m_file.c_str(), std::ios_base::in | std::ios_base::binary );
      if (str.good())
      {
         //---
         // ISO/IEC 15444-1:2004 (E)
         // LBox, Box Length. This field specifies the length of the box, stored
         // as a 4-byte big endian unsigned integer. This value includes all of
         // the fields of the box, including the length and type. If the value
         // of this field is 1, then the XLBox field shall exist and the value
         // of that field shall be the actual length of the box. If the value
         // of this field is 0, then the length of the box was not known when
         // the LBox field was written. In this case, this box contains all
         // bytes up to the end of the file. If a box of length 0 is contained
         // within another box (its superbox), then the length of that superbox
         // shall also be 0.  This means that this box is the last box in the
         // file. The values 2-7 are reserved for ISO use.
         //---
         ossim_uint32 lbox  = 0;
         ossim_uint32 tbox  = 0;
         ossim_uint64 xlbox = 0;

         const ossim_uint32 UUID_TYPE = 0x75756964;
         const ossim_uint8 GEOTIFF_UUID[GEOTIFF_UUID_SIZE] = 
            {
               0xb1, 0x4b, 0xf8, 0xbd,
               0x08, 0x3d, 0x4b, 0x43,
               0xa5, 0xae, 0x8c, 0xd7,
               0xd5, 0xa6, 0xce, 0x03
            };
         
         while ( str.good() )
         {
            std::streamoff boxPos = str.tellg();
            
            readUInt32( lbox, str );
            readUInt32( tbox, str );
            
            if ( 1 == lbox )
            {
               readUInt64( xlbox, str );
            }

            if ( traceDebug() )
            {
               ossimNotify(ossimNotifyLevel_DEBUG)
                  << "jp2.lbox: " << std::dec << lbox << "\n"
                  << "jp2.tbox: " << std::hex << tbox << std::dec << "\n";
               printTboxType( tbox, ossimNotify(ossimNotifyLevel_DEBUG)  );

               if ( 1 == lbox )
               {
                  ossimNotify(ossimNotifyLevel_DEBUG) << "jp2.xlbox: " << xlbox << std::endl;
               }
            }            
            
            if (tbox == UUID_TYPE)
            {
               // Check for GeoTIFF box.
               
               ossim_uint8 geotiffUuid[GEOTIFF_UUID_SIZE];
               
               // Read in the id.
               str.read((char*)geotiffUuid, GEOTIFF_UUID_SIZE);

#if 0
               bool foundIt = true;
               for (ossim_uint32 i = 0; i < GEOTIFF_UUID_SIZE; ++i)
               {
                  if (geotiffUuid[i] != GEOTIFF_UUID[i])
                  {
                     foundIt = false;
                     break;
                  }
               }
#endif          

               if( memcmp( (char*)geotiffUuid, GEOTIFF_UUID, GEOTIFF_UUID_SIZE) == 0)
               {
                  //---
                  // Feed the stream to the tiff info object to get a keyword
                  // list with geometry/projection info.
                  //---
                  ossimTiffInfo info;
                  
                  //---
                  // Have geotiff boxes with badly terminated geotiffs. So to keep
                  // the tag parser from walking past the first image file directory
                  // (IFD) into garbage we will set the process overview flag to false.
                  //
                  // Note if we ever get multiple entries we will need to take this out.
                  //---
                  info.setProcessOverviewFlag(false);

                  // Do a print to a memory stream.
                  std::ostringstream os;
                  info.print(str, os);

                  // Open an input stream to pass to the keyword list.
                  std::istringstream in( os.str() );
                  
                  ossimKeywordlist kwl;
                  if ( kwl.parseStream(in) )      
                  {
                     kwl.addPrefixToAll( ossimString("jp2.") );

                     // Print it:
                     out << kwl << std::endl;
                  }
                  
                  // Get out of while loop.
                  // break;
               }
            }

            if (lbox == 0) // last box?
            {
               break;
            }
            
            // Seek to the next box.  If xlbox is 1
            if (lbox == 1)
            {
               boxPos += (std::streamoff)xlbox;
            }
            else
            {
               boxPos += (std::streamoff)lbox;
            }

            // Go to next box:
            str.seekg(boxPos, std::ios_base::beg);
            
         } // matches: while ( str.good() )
         
         // Close the stream.
         str.close();
         
      } // matches: if ( str.is_open() )
      
   } // matches: if ( m_file.size() )
   
   return out; 
}

void ossimJp2Info::readUInt32(ossim_uint32& i, std::ifstream& str) const
{
   str.read((char*)&i, 4);
   if (m_endian)
   {
      m_endian->swap(i);
   }
}

void ossimJp2Info::readUInt64(ossim_uint64& i, std::ifstream& str) const
{
   str.read((char*)&i, 8);
   if (m_endian)
   {
      m_endian->swap(i);
   }
}

std::streamoff ossimJp2Info::findBoxData( const ossim_uint32& type,
                                          std::ifstream& str,
                                          ossim_uint32& length ) const
{
   std::streamoff boxPos = 0;
   std::streamoff dataPosOfType = 0;

   if ( str.good() )
   {
      ossim_uint32   lbox   = 0;
      ossim_uint32   tbox   = 0;
      ossim_uint64   xlbox  = 0;
      std::streamoff offsetToDbox = 0;

      while ( str.good() )
      {
         boxPos = str.tellg();

         readUInt32( lbox, str );
         readUInt32( tbox, str );

         if ( lbox == 1 )
         {
            readUInt64( xlbox, str );
            offsetToDbox = 16;
         }
         else
         {
            offsetToDbox = 8;
         }

         if ( traceDebug() )
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "jp2.lbox: " << std::dec << lbox << "\n"
               << "jp2.tbox: " << std::hex << tbox << std::dec << "\n";
            printTboxType( tbox, ossimNotify(ossimNotifyLevel_DEBUG)  );

            if ( lbox == 1 )
            {
               ossimNotify(ossimNotifyLevel_DEBUG) << "jp2.xlbox: " << xlbox << std::endl;
            }
         }            

         if ( tbox == type )
         {
            dataPosOfType = boxPos + offsetToDbox; // set up return value
            length = lbox;
            break; // done...
         }

         if ( lbox == 0 ) // last box?
         {
            break;
         }

         // Seek to the next box.
         std::streamoff nextBoxPos = boxPos;
         if (lbox == 1)
         {
            nextBoxPos+= (std::streamoff)xlbox;
         }
         else
         {
            nextBoxPos+= (std::streamoff)lbox;
         }

         // Go to next box:
         str.seekg(nextBoxPos, std::ios_base::beg);

      } // matches: while ( str.good() )

   } // matches: if ( str.good() )

   return dataPosOfType;
}

std::streamoff ossimJp2Info::getBox( const ossim_uint32 type,
                                     bool includeAll,
                                     std::vector<ossim_uint8>& box ) const
{
   std::streamoff boxPos = 0;

   if ( m_file.size() )
   {
      // Open the file.
      std::ifstream str( m_file.c_str(), std::ios_base::in | std::ios_base::binary );
      if (str.good())
      {
         boxPos = getBox( type, includeAll, str, box ) ;

      } // matches: if ( str.is_open() )

   } // matches: if ( m_file.size() ) 

   return boxPos;

} // End: ossimJp2Info::getBox( type, includeAll, box ) const

std::streamoff ossimJp2Info::getBox( const ossim_uint32& type,
                                     bool includeAll,
                                     std::ifstream& str,
                                     std::vector<ossim_uint8>& box ) const
{
   std::streamoff boxPos = 0;
   std::streamoff boxPosOfType = 0;
   box.clear();
   
   if ( str.good() )
   {
      ossim_uint32   lbox   = 0;
      ossim_uint32   tbox   = 0;
      ossim_uint64   xlbox  = 0;
      std::streamoff offsetToDbox = 0;
      
      while ( str.good() )
      {
         boxPos = str.tellg();

         readUInt32( lbox, str );
         readUInt32( tbox, str );
            
         if ( lbox == 1 )
         {
            readUInt64( xlbox, str );
            offsetToDbox = 16;
         }

         if ( !includeAll )
         {
            if ( lbox == 1 )
            {
               offsetToDbox = 16;
            }
            else
            {
               offsetToDbox = 8;
            }
         }

         if ( traceDebug() )
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "jp2.lbox: " << std::dec << lbox << "\n"
               << "jp2.tbox: " << std::hex << tbox << std::dec << "\n";
            printTboxType( tbox, ossimNotify(ossimNotifyLevel_DEBUG)  );
            
            if ( 1 == lbox )
            {
               ossimNotify(ossimNotifyLevel_DEBUG) << "jp2.xlbox: " << xlbox << std::endl;
            }
         }            

            
         if ( tbox == type )
         {
            //---
            // This is our box, copy it to box.
            // Not copying the lbox,tbox or xlbox
            std::streamoff boxSize = 0;
            if ( lbox == 0 )
            {
               // Last box, goes to end of file.
               str.seekg( 0, std::ios_base::end );
               boxSize = str.tellg() - boxPos;
            }
            else if ( lbox == 1 )
            {
               boxSize = xlbox;
            }
            else
            {
               boxSize = lbox;
            }
            boxSize -= offsetToDbox;
               
            // Seek to box data start
            str.seekg( boxPos + offsetToDbox, std::ios_base::beg );
               
            // std::vector::resize can throw a std::bad_alloc so wrap it...
            try
            {
               box.resize( boxSize );
               str.read( (char*)&box.front(), boxSize );
            }
            catch( std::exception& e )
            {
               std::ostringstream errMsg;
               errMsg << "ossimJp2Info::getBox caught exception on resize:\n"
                      << "Buffer size in bytes: " << boxSize
                      << "\n" << e.what() << std::endl;
               throw ossimException( errMsg.str() );
            }

            boxPosOfType = boxPos; // set up return value 
            break; // done...
         }
            
         if (lbox == 0) // last box?
         {
            break;
         }
            
         // Seek to the next box.
         std::streamoff nextBoxPos = boxPos;
         if (lbox == 1)
         {
            nextBoxPos+= (std::streamoff)xlbox;
         }
         else
         {
            nextBoxPos+= (std::streamoff)lbox;
         }

         // Go to next box:
         str.seekg(nextBoxPos, std::ios_base::beg);
            
      } // matches: while ( str.good() )
         
   } // matches: if ( str.good() )
   
   return boxPosOfType;
   
} // End: ossimJp2Info::getBox( type, includeAll, str, box ) const

std::streamoff ossimJp2Info::getGeotiffBox(
   std::ifstream& str, std::vector<ossim_uint8>& box ) const
{
   std::streamoff boxPos = 0;
   if ( str.good() )
   {
      const ossim_uint32 UUID_TYPE = 0x75756964;
      
      boxPos = getBox( UUID_TYPE, false, str, box );
      if ( boxPos && ( box.size() >= GEOTIFF_UUID_SIZE) )
      {
         const ossim_uint8 GEOTIFF_UUID[GEOTIFF_UUID_SIZE] = 
            {
               0xb1, 0x4b, 0xf8, 0xbd,
               0x08, 0x3d, 0x4b, 0x43,
               0xa5, 0xae, 0x8c, 0xd7,
               0xd5, 0xa6, 0xce, 0x03
            };
         
         if( memcmp( (char*)&box.front(), GEOTIFF_UUID, GEOTIFF_UUID_SIZE) != 0)
         {
            boxPos = 0;
            box.clear();
         }
      }
   }
   return boxPos;
}

std::streamoff ossimJp2Info::getGmlBox(
   std::ifstream& str, std::vector<ossim_uint8>& box ) const
{
   std::streamoff boxPos = 0;
   if ( str.good() )
   {
      const ossim_uint32 ASOC_TBOX_ID = 0x61736F63;
      const ossim_uint32 LBL_TBOX_ID  = 0x6C626C20;
      const ossim_uint32 XML_TBOX_ID  = 0x786D6C20;
      
      ossim_uint32 asoc0_data_length = 0;
      std::streamoff dataPosAsoc0 = findBoxData( ASOC_TBOX_ID, str, asoc0_data_length );

      if ( dataPosAsoc0 > 0 && asoc0_data_length > 0 )
      {
         ossim_uint32 lbl0_data_length = 0;
         std::streamoff dataPosLbl0 = findBoxData( LBL_TBOX_ID, str, lbl0_data_length );

         if ( dataPosLbl0 > 0 && lbl0_data_length > 0 )
         {
            std::string gmlDataStr;
            gmlDataStr.resize( lbl0_data_length );

            // Removed c++ 11 call for portablility...
            // str.read( (char*)&gmlDataStr.front(), lbl0_data_length );
            str.read( (char*)&gmlDataStr[0], lbl0_data_length );            

            // Do we have a GML data asoc ?
            if ( gmlDataStr.compare( 0, 8, "gml.data" ) == 0 )
            {
               // Seek to asoc 0 start of data
               str.seekg( dataPosAsoc0, std::ios_base::beg );

               ossim_uint32 asoc1_data_length = 0;
               std::streamoff dataPosAsoc1 = findBoxData( ASOC_TBOX_ID, str, asoc1_data_length );

               if ( dataPosAsoc1 > 0 && asoc1_data_length > 0 )
               {
                  ossim_uint32 lbl1_data_length = 0;
                  std::streamoff dataPosLbl1 = findBoxData( LBL_TBOX_ID, str, lbl1_data_length );

                  if ( dataPosLbl1 > 0 && lbl1_data_length > 0 )
                  {
                     std::string gmlRootInstanceStr;
                     gmlRootInstanceStr.resize( lbl1_data_length );
                     // Removed c++ 11 call for portablility...
                     // str.read( (char*)&gmlRootInstanceStr.front(), lbl1_data_length );
                     str.read( (char*)&gmlRootInstanceStr[0], lbl1_data_length );

                     // Do we have a GML root-instance asoc ?
                     if ( gmlRootInstanceStr.compare( 0, 17, "gml.root-instance" ) == 0 )
                     {
                        // Seek to asoc 1 start of data
                        str.seekg( dataPosAsoc1, std::ios_base::beg );

                        boxPos = getBox( XML_TBOX_ID, false, str, box );
                     }
                  }
               }
            }
         }
      }
   }
   return boxPos;
}

void ossimJp2Info::printTboxType( ossim_uint32 tbox, std::ostream& out ) const
{
   std::string boxType;

   const ossim_uint32 JP2_SIG_TYPE   = 0x6A502020;
   const ossim_uint32 FILE_TYPE      = 0x66747970;
   const ossim_uint32 JP2_HDR_TYPE   = 0x6A703268;
   const ossim_uint32 IMG_HDR_TYPE   = 0x69686472;
   const ossim_uint32 BPCC_TYPE      = 0x62706363;
   const ossim_uint32 COLR_TYPE      = 0x636F6C72;
   const ossim_uint32 PCLR_TYPE      = 0x70636C72;
   const ossim_uint32 CMAP_TYPE      = 0x636D6170;
   const ossim_uint32 CDEF_TYPE      = 0x63646566;
   const ossim_uint32 RES_TYPE       = 0x72657320;
   const ossim_uint32 RESC_TYPE      = 0x72657363;
   const ossim_uint32 RESD_TYPE      = 0x72657364;
   const ossim_uint32 JP2C_TYPE      = 0x6A703263;
   const ossim_uint32 JP2I_TYPE      = 0x6A703269;
   const ossim_uint32 XML_TYPE       = 0x786D6C20;
   const ossim_uint32 UUID_TYPE      = 0x75756964;

   if ( tbox == JP2_SIG_TYPE )
   {
      boxType = "jp2_sig";
   }
   else if ( tbox == FILE_TYPE )
   {
      boxType = "ftyp";
   }
   else if ( tbox == JP2_HDR_TYPE )
   {
      boxType = "jp2h";
   }
   else if ( tbox == IMG_HDR_TYPE )
   {
      boxType = "ihdr";
   }
   else if ( tbox == BPCC_TYPE )
   {
      boxType = "bpcc";
   }
   else if ( tbox == COLR_TYPE )
   {
      boxType = "colr";
   }
   else if ( tbox == PCLR_TYPE )
   {
      boxType = "pclr";
   }
   else if ( tbox == CMAP_TYPE )
   {
      boxType = "cmap";
   }
   else if ( tbox == CDEF_TYPE )
   {
      boxType = "cdef";
   }
   else if ( tbox == RES_TYPE )
   {
      boxType = "res";
   }
   else if ( tbox == RESC_TYPE )
   {
      boxType = "resc";
   }
   else if ( tbox == RESD_TYPE )
   {
      boxType = "resd";
   }
   else if ( tbox == JP2C_TYPE )
   {
      boxType = "jp2c";
   }
   else if ( tbox == JP2I_TYPE )
   {
      boxType = "jp2i";
   }
   else if ( tbox == XML_TYPE )
   {
      boxType = "xml";
   }
   else if ( tbox == UUID_TYPE )
   {
      boxType = "uuid";
   }
   else
   {
      boxType = "unhandled";
   }

   out << "jp2.box_type: " << boxType << "\n";
}
