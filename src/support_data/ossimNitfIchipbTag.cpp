//---
//
// License: MIT
//
// Author:  David Burken
//
// Description: ICHIPB tag class declaration.
//
// See document STDI-0002 Table 5-2 for more info.
// 
// http://164.214.2.51/ntb/baseline/docs/stdi0002/final.pdf
//
//---
// $Id$

#include <ossim/support_data/ossimNitfIchipbTag.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossim2dBilinearTransform.h>
#include <ossim/imaging/ossimImageGeometry.h>

#include <cstring>
#include <iomanip>
#include <iostream>
#include <cstdio>

static const ossimString XFRM_FLAG_KW = "XFRM_FLAG";
static const ossimString SCALE_FACTOR_KW = "SCALE_FACTOR";
static const ossimString ANAMRPH_CORR_KW = "ANAMRPH_CORR";
static const ossimString SCANBLK_NUM_KW = "SCANBLK_NUM";
static const ossimString OP_ROW_11_KW = "OP_ROW_11";
static const ossimString OP_COL_11_KW = "OP_COL_11";
static const ossimString OP_ROW_12_KW = "OP_ROW_12";
static const ossimString OP_COL_12_KW = "OP_COL_12";
static const ossimString OP_ROW_21_KW = "OP_ROW_21";
static const ossimString OP_COL_21_KW = "OP_COL_21";
static const ossimString OP_ROW_22_KW = "OP_ROW_22";
static const ossimString OP_COL_22_KW = "OP_COL_22";
static const ossimString FI_ROW_11_KW = "FI_ROW_11";
static const ossimString FI_COL_11_KW = "FI_COL_11";
static const ossimString FI_ROW_12_KW = "FI_ROW_12";
static const ossimString FI_COL_12_KW = "FI_COL_12";
static const ossimString FI_ROW_21_KW = "FI_ROW_21";
static const ossimString FI_COL_21_KW = "FI_COL_21";
static const ossimString FI_ROW_22_KW = "FI_ROW_22";
static const ossimString FI_COL_22_KW = "FI_COL_22";
static const ossimString FI_ROW_KW = "FI_ROW";
static const ossimString FI_COL_KW = "FI_COL";

RTTI_DEF1(ossimNitfIchipbTag, "ossimNitfIchipbTag", ossimNitfRegisteredTag);

ossimNitfIchipbTag::ossimNitfIchipbTag()
   : ossimNitfRegisteredTag(std::string("ICHIPB"), 224)
{
   clearFields();
}

void ossimNitfIchipbTag::parseStream(std::istream& in)
{
   clearFields();

   in.read(theXfrmFlag,     XFRM_FLAG_SIZE);
   in.read(theScaleFactor,  SCALE_FACTOR_SIZE);
   in.read(theAnamrphCorr,  ANAMRPH_CORR_SIZE);
   in.read(theScanBlock,    SCANBLK_NUM_SIZE);
   in.read(theOpRow11,      OP_ROW_11_SIZE);
   in.read(theOpCol11,      OP_COL_11_SIZE);
   in.read(theOpRow12,      OP_ROW_12_SIZE);
   in.read(theOpCol12,      OP_COL_12_SIZE);
   in.read(theOpRow21,      OP_ROW_21_SIZE);
   in.read(theOpCol21,      OP_COL_21_SIZE);
   in.read(theOpRow22,      OP_ROW_22_SIZE);
   in.read(theOpCol22,      OP_COL_22_SIZE);
   in.read(theFiRow11,      FI_ROW_11_SIZE);
   in.read(theFiCol11,      FI_COL_11_SIZE);
   in.read(theFiRow12,      FI_ROW_12_SIZE);
   in.read(theFiCol12,      FI_COL_12_SIZE);
   in.read(theFiRow21,      FI_ROW_21_SIZE);
   in.read(theFiCol21,      FI_COL_21_SIZE);
   in.read(theFiRow22,      FI_ROW_22_SIZE);
   in.read(theFiCol22,      FI_COL_22_SIZE);
   in.read(theFullImageRow, FI_ROW_SIZE);
   in.read(theFullImageCol, FI_COL_SIZE);
}

void ossimNitfIchipbTag::writeStream(std::ostream& out)
{
   out.write(theXfrmFlag,     XFRM_FLAG_SIZE);
   out.write(theScaleFactor,  SCALE_FACTOR_SIZE);
   out.write(theAnamrphCorr,  ANAMRPH_CORR_SIZE);
   out.write(theScanBlock,    SCANBLK_NUM_SIZE);
   out.write(theOpRow11,      OP_ROW_11_SIZE);
   out.write(theOpCol11,      OP_COL_11_SIZE);
   out.write(theOpRow12,      OP_ROW_12_SIZE);
   out.write(theOpCol12,      OP_COL_12_SIZE);
   out.write(theOpRow21,      OP_ROW_21_SIZE);
   out.write(theOpCol21,      OP_COL_21_SIZE);
   out.write(theOpRow22,      OP_ROW_22_SIZE);
   out.write(theOpCol22,      OP_COL_22_SIZE);
   out.write(theFiRow11,      FI_ROW_11_SIZE);
   out.write(theFiCol11,      FI_COL_11_SIZE);
   out.write(theFiRow12,      FI_ROW_12_SIZE);
   out.write(theFiCol12,      FI_COL_12_SIZE);
   out.write(theFiRow21,      FI_ROW_21_SIZE);
   out.write(theFiCol21,      FI_COL_21_SIZE);
   out.write(theFiRow22,      FI_ROW_22_SIZE);
   out.write(theFiCol22,      FI_COL_22_SIZE);
   out.write(theFullImageRow, FI_ROW_SIZE);
   out.write(theFullImageCol, FI_COL_SIZE);
}

void ossimNitfIchipbTag::clearFields()
{
   memset(theXfrmFlag,     '0', XFRM_FLAG_SIZE);
   memset(theScaleFactor,  '0', SCALE_FACTOR_SIZE);
   memset(theAnamrphCorr,  '0', ANAMRPH_CORR_SIZE);
   memset(theScanBlock,    '0', SCANBLK_NUM_SIZE);
   memset(theOpRow11,      '0', OP_ROW_11_SIZE);
   memset(theOpCol11,      '0', OP_COL_11_SIZE);
   memset(theOpRow12,      '0', OP_ROW_12_SIZE);
   memset(theOpCol12,      '0', OP_COL_12_SIZE);
   memset(theOpRow21,      '0', OP_ROW_21_SIZE);
   memset(theOpCol21,      '0', OP_COL_21_SIZE);
   memset(theOpRow22,      '0', OP_ROW_22_SIZE);
   memset(theOpCol22,      '0', OP_COL_22_SIZE);
   memset(theFiRow11,      '0', FI_ROW_11_SIZE);
   memset(theFiCol11,      '0', FI_COL_11_SIZE);
   memset(theFiRow12,      '0', FI_ROW_12_SIZE);
   memset(theFiCol12,      '0', FI_COL_12_SIZE);
   memset(theFiRow21,      '0', FI_ROW_21_SIZE);
   memset(theFiCol21,      '0', FI_COL_21_SIZE);
   memset(theFiRow22,      '0', FI_ROW_22_SIZE);
   memset(theFiCol22,      '0', FI_COL_22_SIZE);
   memset(theFullImageRow, '0', FI_ROW_SIZE);
   memset(theFullImageCol, '0', FI_COL_SIZE);

   theXfrmFlag[XFRM_FLAG_SIZE]       = '\0';
   theScaleFactor[SCALE_FACTOR_SIZE] = '\0';
   theAnamrphCorr[ANAMRPH_CORR_SIZE] = '\0';
   theScanBlock[SCANBLK_NUM_SIZE]    = '\0';
   theOpRow11[OP_ROW_11_SIZE]        = '\0';
   theOpCol11[OP_COL_11_SIZE]        = '\0';
   theOpRow12[OP_ROW_12_SIZE]        = '\0';
   theOpCol12[OP_COL_12_SIZE]        = '\0';
   theOpRow21[OP_ROW_21_SIZE]        = '\0';
   theOpCol21[OP_COL_21_SIZE]        = '\0';
   theOpRow22[OP_ROW_22_SIZE]        = '\0';
   theOpCol22[OP_COL_22_SIZE]        = '\0';
   theFiRow11[FI_ROW_11_SIZE]        = '\0';
   theFiCol11[FI_COL_11_SIZE]        = '\0';
   theFiRow12[FI_ROW_12_SIZE]        = '\0';
   theFiCol12[FI_COL_12_SIZE]        = '\0';
   theFiRow21[FI_ROW_21_SIZE]        = '\0';
   theFiCol21[FI_COL_21_SIZE]        = '\0';
   theFiRow22[FI_ROW_22_SIZE]        = '\0';
   theFiCol22[FI_COL_22_SIZE]        = '\0';
   theFullImageRow[FI_ROW_SIZE]      = '\0';
   theFullImageCol[FI_COL_SIZE]      = '\0';
}

std::ostream& ossimNitfIchipbTag::print(std::ostream& out,
                                        const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";

   out << std::setiosflags(std::ios::left)
       << pfx << std::setw(24) << "CETAG:"
       << getTagName() << "\n"
       << pfx << std::setw(24) << "CEL:"
       << getTagLength() << "\n"
       << pfx << std::setw(24) << "XFRM_FLAG:"     << theXfrmFlag << "\n"
       << pfx << std::setw(24) << "SCALE_FACTOR:"  << theScaleFactor << "\n"
       << pfx << std::setw(24) << "ANAMRPH_CORR:"  << theAnamrphCorr << "\n"
       << pfx << std::setw(24) << "SCANBLK_NUM:"   << theScanBlock << "\n"
       << pfx << std::setw(24) << "OP_ROW_11:"     << theOpRow11 << "\n"
       << pfx << std::setw(24) << "OP_COL_11:"     << theOpCol11 << "\n"
       << pfx << std::setw(24) << "OP_ROW_12:"     << theOpRow12 << "\n"
       << pfx << std::setw(24) << "OP_COL_12:"     << theOpCol12 << "\n"
       << pfx << std::setw(24) << "OP_ROW_21:"     << theOpRow21 << "\n"
       << pfx << std::setw(24) << "OP_COL_21:"     << theOpCol21 << "\n"
       << pfx << std::setw(24) << "OP_ROW_22:"     << theOpRow22 << "\n"
       << pfx << std::setw(24) << "OP_COL_22:"     << theOpCol22 << "\n"
       << pfx << std::setw(24) << "FI_ROW_11:"     << theFiRow11 << "\n"
       << pfx << std::setw(24) << "FI_COL_11:"     << theFiCol11 << "\n"
       << pfx << std::setw(24) << "FI_ROW_12:"     << theFiRow12 << "\n"
       << pfx << std::setw(24) << "FI_COL_12:"     << theFiCol12 << "\n"
       << pfx << std::setw(24) << "FI_ROW_21:"     << theFiRow21 << "\n"
       << pfx << std::setw(24) << "FI_COL_21:"     << theFiCol21 << "\n"
       << pfx << std::setw(24) << "FI_ROW_22:"     << theFiRow22 << "\n"
       << pfx << std::setw(24) << "FI_COL_22:"     << theFiCol22 << "\n"
       << pfx << std::setw(24) << "FI_ROW:"        << theFullImageRow << "\n"
       << pfx << std::setw(24) << "FI_COL:"        << theFullImageCol
       << std::endl;

   return out;
}

bool ossimNitfIchipbTag::initialize( const ossimDrect& opRect,
                                     const ossimDrect& fiRect )
{
   bool result = true;
   if ( opRect.hasNans() || fiRect.hasNans() )
   {
      result = false;
   }
   else
   {
      //---
      // Method converts from ossim coordinates from "Pixel is Point" to
      // "Pixel is Area".
      //---
      const ossim_float64 SHIFT = 0.5;
      
      setOpCol11( opRect.ul().x + SHIFT );
      setOpRow11( opRect.ul().y + SHIFT );

      setOpCol12( opRect.ur().x + SHIFT );
      setOpRow12( opRect.ur().y + SHIFT );

      setOpCol21( opRect.ll().x + SHIFT );
      setOpRow21( opRect.ll().y + SHIFT );

      setOpCol22( opRect.lr().x + SHIFT );
      setOpRow22( opRect.lr().y + SHIFT );

      setFiCol11( fiRect.ul().x + SHIFT );
      setFiRow11( fiRect.ul().y + SHIFT );

      setFiCol12( fiRect.ur().x + SHIFT );
      setFiRow12( fiRect.ur().y + SHIFT );

      setFiCol21( fiRect.ll().x + SHIFT );
      setFiRow21( fiRect.ll().y + SHIFT );

      setFiCol22( fiRect.lr().x + SHIFT );
      setFiRow22( fiRect.lr().y + SHIFT );
   }
   return result;
}

bool ossimNitfIchipbTag::getXfrmFlag() const
{
   return ossimString::toBool(theXfrmFlag);
}

void ossimNitfIchipbTag::setXfrmFlag( bool flag )
{
   if ( flag )
   {
      memcpy(theXfrmFlag, "01", XFRM_FLAG_SIZE);
   }
   else
   {
      memcpy(theXfrmFlag, "00", XFRM_FLAG_SIZE);
   }
}

ossim_float64 ossimNitfIchipbTag::getScaleFactor() const
{
   return ossimString::toFloat64(theScaleFactor);
}

void ossimNitfIchipbTag::setScaleFactor( ossim_float64 scale )
{
   ossimString os = ossimNitfCommon::convertToDoubleString(scale, 5, SCALE_FACTOR_SIZE);
   memcpy(theScaleFactor, os.c_str(), SCALE_FACTOR_SIZE);
}

bool ossimNitfIchipbTag::getAnamrphCorrFlag() const
{
   return ossimString::toBool(theAnamrphCorr);
}

void ossimNitfIchipbTag::setAnamrphCorrFlag( bool flag )
{
   if ( flag )
   {
      memcpy(theAnamrphCorr, "01", ANAMRPH_CORR_SIZE);
   }
   else
   {
      memcpy(theAnamrphCorr, "00", ANAMRPH_CORR_SIZE);
   }
}

ossim_uint32 ossimNitfIchipbTag::getScanBlock() const
{
   return ossimString::toUInt32(theScanBlock);
}

void ossimNitfIchipbTag::setScanBlock( ossim_uint32 block )
{
   // 00 - 99:
   if ( block < 100 )
   {
      ossimString os = ossimNitfCommon::convertToUIntString(block, SCANBLK_NUM_SIZE);
      memcpy(theScanBlock, os.c_str(), SCANBLK_NUM_SIZE);
   }
}

ossim_float64 ossimNitfIchipbTag::getOpRow11() const
{
   return ossimString::toFloat64(theOpRow11);
}

void ossimNitfIchipbTag::setOpRow11( ossim_float64 row )
{
   ossimString os = ossimNitfCommon::convertToDoubleString(row, 3, OP_ROW_11_SIZE);
   memcpy(theOpRow11, os.c_str(), OP_ROW_11_SIZE);
}

ossim_float64 ossimNitfIchipbTag::getOpCol11() const
{
   return ossimString::toFloat64(theOpCol11);
}

void ossimNitfIchipbTag::setOpCol11( ossim_float64 col )
{
   ossimString os = ossimNitfCommon::convertToDoubleString(col, 3, OP_COL_11_SIZE);
   memcpy(theOpCol11, os.c_str(), OP_COL_11_SIZE); 
}

ossim_float64 ossimNitfIchipbTag::getOpRow12() const
{
   return ossimString::toFloat64(theOpRow12);
}

void ossimNitfIchipbTag::setOpRow12( ossim_float64 row )
{
   ossimString os = ossimNitfCommon::convertToDoubleString(row, 3, OP_ROW_12_SIZE);
   memcpy(theOpRow12, os.c_str(), OP_ROW_12_SIZE);
}

ossim_float64 ossimNitfIchipbTag::getOpCol12() const
{
   return ossimString::toFloat64(theOpCol12);
}

void ossimNitfIchipbTag::setOpCol12( ossim_float64 col )
{
   ossimString os = ossimNitfCommon::convertToDoubleString(col, 3, OP_COL_12_SIZE);
   memcpy(theOpCol12, os.c_str(), OP_COL_12_SIZE); 
}

ossim_float64 ossimNitfIchipbTag::getOpRow21() const
{
   return ossimString::toFloat64(theOpRow21);
}

void ossimNitfIchipbTag::setOpRow21( ossim_float64 row )
{
   ossimString os = ossimNitfCommon::convertToDoubleString(row, 3, OP_ROW_21_SIZE);
   memcpy(theOpRow21, os.c_str(), OP_ROW_21_SIZE);
}

ossim_float64 ossimNitfIchipbTag::getOpCol21()
   const
{ return ossimString::toFloat64(theOpCol21);
}

void ossimNitfIchipbTag::setOpCol21( ossim_float64 col )
{
   ossimString os = ossimNitfCommon::convertToDoubleString(col, 3, OP_COL_21_SIZE);
   memcpy(theOpCol21, os.c_str(), OP_COL_21_SIZE); 
}

ossim_float64 ossimNitfIchipbTag::getOpRow22() const
{
   return ossimString::toFloat64(theOpRow22);
}

void ossimNitfIchipbTag::setOpRow22( ossim_float64 row )
{
   ossimString os = ossimNitfCommon::convertToDoubleString(row, 3, OP_ROW_22_SIZE);
   memcpy(theOpRow22, os.c_str(), OP_ROW_22_SIZE);
}

ossim_float64 ossimNitfIchipbTag::getOpCol22() const
{
   return ossimString::toFloat64(theOpCol22);
}

void ossimNitfIchipbTag::setOpCol22( ossim_float64 col )
{
   ossimString os = ossimNitfCommon::convertToDoubleString(col, 3, OP_COL_22_SIZE);
   memcpy(theOpCol22, os.c_str(), OP_COL_22_SIZE); 
}

ossim_float64 ossimNitfIchipbTag::getFiRow11() const
{
   return ossimString::toFloat64(theFiRow11);
}

void ossimNitfIchipbTag::setFiRow11( ossim_float64 row )
{
   ossimString os = ossimNitfCommon::convertToDoubleString(row, 3, FI_ROW_11_SIZE);
   memcpy(theFiRow11, os.c_str(), FI_ROW_11_SIZE);
}

ossim_float64 ossimNitfIchipbTag::getFiCol11() const
{
   return ossimString::toFloat64(theFiCol11);
}

void ossimNitfIchipbTag::setFiCol11( ossim_float64 col )
{
   ossimString os = ossimNitfCommon::convertToDoubleString(col, 3, FI_COL_11_SIZE);
   memcpy(theFiCol11, os.c_str(), FI_COL_11_SIZE); 
}

ossim_float64 ossimNitfIchipbTag::getFiRow12() const
{
   return ossimString::toFloat64(theFiRow12);
}

void ossimNitfIchipbTag::setFiRow12( ossim_float64 row )
{
   ossimString os = ossimNitfCommon::convertToDoubleString(row, 3, FI_ROW_12_SIZE);
   memcpy(theFiRow12, os.c_str(), OP_ROW_12_SIZE);
}

ossim_float64 ossimNitfIchipbTag::getFiCol12() const
{
   return ossimString::toFloat64(theFiCol12);
}

void ossimNitfIchipbTag::setFiCol12( ossim_float64 col )
{
   ossimString os = ossimNitfCommon::convertToDoubleString(col, 3, FI_COL_12_SIZE);
   memcpy(theFiCol12, os.c_str(), FI_COL_12_SIZE); 
}

ossim_float64 ossimNitfIchipbTag::getFiRow21() const
{
   return ossimString::toFloat64(theFiRow21);
}

void ossimNitfIchipbTag::setFiRow21( ossim_float64 row )
{
   ossimString os = ossimNitfCommon::convertToDoubleString(row, 3, FI_ROW_21_SIZE);
   memcpy(theFiRow21, os.c_str(), OP_ROW_21_SIZE);
}

ossim_float64 ossimNitfIchipbTag::getFiCol21() const
{
   return ossimString::toFloat64(theFiCol21);
}

void ossimNitfIchipbTag::setFiCol21( ossim_float64 col )
{
   ossimString os = ossimNitfCommon::convertToDoubleString(col, 3, FI_COL_21_SIZE);
   memcpy(theFiCol21, os.c_str(), FI_COL_21_SIZE); 
}

ossim_float64 ossimNitfIchipbTag::getFiRow22() const
{
   return ossimString::toFloat64(theFiRow22);
}

void ossimNitfIchipbTag::setFiRow22( ossim_float64 row )
{
   ossimString os = ossimNitfCommon::convertToDoubleString(row, 3, FI_ROW_22_SIZE);
   memcpy(theFiRow22, os.c_str(), OP_ROW_22_SIZE);
}

ossim_float64 ossimNitfIchipbTag::getFiCol22() const
{
   return ossimString::toFloat64(theFiCol22);
}

void ossimNitfIchipbTag::setFiCol22( ossim_float64 col )
{
   ossimString os = ossimNitfCommon::convertToDoubleString(col, 3, FI_COL_22_SIZE);
   memcpy(theFiCol22, os.c_str(), FI_COL_22_SIZE); 
}

ossim_uint32 ossimNitfIchipbTag::getFullImageRows() const
{
   return ossimString::toUInt32(theFullImageRow);
}

void ossimNitfIchipbTag::setFiRow( ossim_uint32 row )
{
   ossimString os = ossimNitfCommon::convertToUIntString(row, FI_ROW_SIZE);
   memcpy(theFullImageRow, os.c_str(), FI_ROW_SIZE);
}

ossim_uint32 ossimNitfIchipbTag::getFullImageCols() const
{
   return ossimString::toUInt32(theFullImageCol);
}

void ossimNitfIchipbTag::setFiCol( ossim_uint32 col )
{
   ossimString os = ossimNitfCommon::convertToUIntString(col, FI_COL_SIZE);
   memcpy(theFullImageCol, os.c_str(), FI_COL_SIZE);
}

void ossimNitfIchipbTag::getImageRect(ossimDrect& rect) const
{
   ossimDpt pt;

   pt.x = getOpCol11() - 0.5;
   pt.y = getOpRow11() - 0.5;
   rect.set_ul(pt);

   pt.x = getOpCol22() - 0.5;
   pt.y = getOpRow22() - 0.5;
   rect.set_lr(pt);
}

void ossimNitfIchipbTag::getFullImageRect(ossimDrect& rect) const
{
   ossimDpt pt;

   ossim_float64 minX = ossim::min(getFiCol11(), getFiCol22());
   ossim_float64 maxX = ossim::max(getFiCol11(), getFiCol22());
   ossim_float64 minY = ossim::min(getFiRow11(), getFiRow22());
   ossim_float64 maxY = ossim::max(getFiRow11(), getFiRow22());
   
   pt.x = minX - 0.5;
   pt.y = minY - 0.5;
   rect.set_ul(pt);

   pt.x = maxX - 0.5;
   pt.y = maxY - 0.5;
   rect.set_lr(pt);
}

#if 0
void ossimNitfIchipbTag::getSubImageOffset(ossimDpt& pt) const
{
   ossimDrect rect;
   
   getFullImageRect(rect);
   
   pt = rect.ul();
}
#endif

ossim2dTo2dTransform* ossimNitfIchipbTag::newTransform()const
{
   return new ossim2dBilinearTransform(ossimDpt(getOpCol11(), getOpRow11()),
                                       ossimDpt(getOpCol12(), getOpRow12()),
                                       ossimDpt(getOpCol21(), getOpRow21()),
                                       ossimDpt(getOpCol22(), getOpRow22()),
                                       ossimDpt(getFiCol11(), getFiRow11()),
                                       ossimDpt(getFiCol12(), getFiRow12()),
                                       ossimDpt(getFiCol21(), getFiRow21()),
                                       ossimDpt(getFiCol22(), getFiRow22()));
}

bool ossimNitfIchipbTag::initFromGeometry(const ossimImageGeometry* geom)
{
   if (!geom || !geom->hasTransform())
      return false;

   auto transform = geom->getTransform();
   if (!transform)
      return false;

   ossimDrect viewRect;
   geom->getBoundingRect(viewRect);

   if (dynamic_cast<const ossimMapProjection*>(geom->getProjection()))
      strcpy(theXfrmFlag, "01");
   else
      strcpy(theXfrmFlag, "00");

   ossimDpt ul, ur, ll, lr;

   transform->inverse(viewRect.ul(), ul);
   transform->inverse(viewRect.ur(), ur);
   transform->inverse(viewRect.ll(), ll);
   transform->inverse(viewRect.lr(), lr);

   ossimDrect imageRect(ul, ur, lr, ll);
   double imageArea = imageRect.area();
   if (imageArea <= FLT_EPSILON)
      return false;
   double scaleFactor = sqrt(viewRect.area() / imageArea);
   std::snprintf(theScaleFactor, sizeof(theScaleFactor), "%10.5f", scaleFactor);

   std::snprintf(theOpCol11, sizeof(theOpCol11), "%12.3f", viewRect.ul().x);
   std::snprintf(theOpRow11, sizeof(theOpRow11), "%12.3f", viewRect.ul().y);
   std::snprintf(theOpCol12, sizeof(theOpCol12), "%12.3f", viewRect.ur().x);
   std::snprintf(theOpRow12, sizeof(theOpRow12), "%12.3f", viewRect.ur().y);
   std::snprintf(theOpCol21, sizeof(theOpCol21), "%12.3f", viewRect.ll().x);
   std::snprintf(theOpRow21, sizeof(theOpRow21), "%12.3f", viewRect.ll().y);
   std::snprintf(theOpCol22, sizeof(theOpCol22), "%12.3f", viewRect.lr().x);
   std::snprintf(theOpRow22, sizeof(theOpRow22), "%12.3f", viewRect.lr().y);

   std::snprintf(theFiCol11, sizeof(theFiCol11), "%12.3f", ul.x);
   std::snprintf(theFiRow11, sizeof(theFiRow11), "%12.3f", ul.y);
   std::snprintf(theFiCol12, sizeof(theFiCol12), "%12.3f", ur.x);
   std::snprintf(theFiRow12, sizeof(theFiRow12), "%12.3f", ur.y);
   std::snprintf(theFiCol21, sizeof(theFiCol21), "%12.3f", ll.x);
   std::snprintf(theFiRow21, sizeof(theFiRow21), "%12.3f", ll.y);
   std::snprintf(theFiCol22, sizeof(theFiCol22), "%12.3f", lr.x);
   std::snprintf(theFiRow22, sizeof(theFiRow22), "%12.3f", lr.y);

   return true;
}

bool ossimNitfIchipbTag::isIdentity()const
{
   return ((ossim::almostEqual(getOpCol11(), getFiCol11()) 
           && ossim::almostEqual(getOpRow11(), getFiRow11()) 
           && ossim::almostEqual(getOpCol12(), getFiCol12()) 
           && ossim::almostEqual(getOpRow12(), getFiRow12())
           && ossim::almostEqual(getOpCol21(), getFiCol21()) 
           && ossim::almostEqual(getOpRow21(), getFiRow21()) 
           && ossim::almostEqual(getOpCol22(), getFiCol22())
           && ossim::almostEqual(getOpRow22(), getFiRow22())));
}

void ossimNitfIchipbTag::setProperty(ossimRefPtr<ossimProperty> property)
{
   ossimNitfRegisteredTag::setProperty(property);   
}

ossimRefPtr<ossimProperty> ossimNitfIchipbTag::getProperty(const ossimString& name)const
{
   ossimProperty* result = 0;

   if(name == XFRM_FLAG_KW)
   {
      result = new ossimStringProperty(name, theXfrmFlag);
   }
   else if(name == SCALE_FACTOR_KW)
   {
      result = new ossimStringProperty(name, theScaleFactor);
   }
   else if(name == ANAMRPH_CORR_KW)
   {
      result = new ossimStringProperty(name, theAnamrphCorr);
   }
   else if(name == SCANBLK_NUM_KW)
   {
      result = new ossimStringProperty(name, theScanBlock);
   }
   else if(name == OP_ROW_11_KW)
   {
      result = new ossimStringProperty(name, theOpRow11);
   }
   else if(name == OP_COL_11_KW)
   {
      result = new ossimStringProperty(name, theOpCol11);
   }
   else if(name == OP_ROW_12_KW)
   {
      result = new ossimStringProperty(name,theOpRow12 );
   }
   else if(name == OP_COL_12_KW)
   {
      result = new ossimStringProperty(name, theOpCol12);
   }
   else if(name == OP_ROW_21_KW)
   {
      result = new ossimStringProperty(name,theOpRow21);
   }
   else if(name == OP_COL_21_KW)
   {
      result = new ossimStringProperty(name, theOpCol21);
   }
   else if(name == OP_ROW_22_KW)
   {
      result = new ossimStringProperty(name, theOpRow22);
   }
   else if(name == OP_COL_22_KW)
   {
      result = new ossimStringProperty(name, theOpCol22);
   }
   else if(name == FI_ROW_11_KW)
   {
      result = new ossimStringProperty(name, theFiRow11);
   }
   else if(name == FI_COL_11_KW)
   {
      result = new ossimStringProperty(name, theFiCol11);
   }
   else if(name == FI_ROW_12_KW)
   {
      result = new ossimStringProperty(name, theFiRow12);
   }
   else if(name == FI_COL_12_KW)
   {
      result = new ossimStringProperty(name, theFiCol12);
   }
   else if(name == FI_ROW_21_KW)
   {
      result = new ossimStringProperty(name, theFiRow21);
   }
   else if(name == FI_COL_21_KW)
   {
      result = new ossimStringProperty(name, theFiCol21);
   }
   else if(name == FI_ROW_22_KW)
   {
      result = new ossimStringProperty(name, theFiRow22);
   }
   else if(name == FI_COL_22_KW)
   {
      result = new ossimStringProperty(name, theFiCol22);
   }
   else if(name == FI_ROW_KW)
   {
      result = new ossimStringProperty(name, theFullImageRow);
   }
   else if(name == FI_COL_KW)
   {
      result = new ossimStringProperty(name, theFullImageCol);
   }
   else
   {
      return ossimNitfRegisteredTag::getProperty(name);
   }

   return result;
}

void ossimNitfIchipbTag::getPropertyNames(std::vector<ossimString>& propertyNames)const
{
   ossimNitfRegisteredTag::getPropertyNames(propertyNames);

   propertyNames.push_back(XFRM_FLAG_KW);
   propertyNames.push_back(SCALE_FACTOR_KW);
   propertyNames.push_back(ANAMRPH_CORR_KW);
   propertyNames.push_back(SCANBLK_NUM_KW);
   propertyNames.push_back(OP_ROW_11_KW);
   propertyNames.push_back(OP_COL_11_KW);
   propertyNames.push_back(OP_ROW_12_KW);
   propertyNames.push_back(OP_COL_12_KW);
   propertyNames.push_back(OP_ROW_21_KW);
   propertyNames.push_back(OP_COL_21_KW);
   propertyNames.push_back(OP_ROW_22_KW);
   propertyNames.push_back(OP_COL_22_KW);
   propertyNames.push_back(FI_ROW_11_KW);
   propertyNames.push_back(FI_COL_11_KW);
   propertyNames.push_back(FI_ROW_12_KW);
   propertyNames.push_back(FI_COL_12_KW);
   propertyNames.push_back(FI_ROW_21_KW);
   propertyNames.push_back(FI_COL_21_KW);
   propertyNames.push_back(FI_ROW_22_KW);
   propertyNames.push_back(FI_COL_22_KW);
   propertyNames.push_back(FI_ROW_KW);
   propertyNames.push_back(FI_COL_KW);
}
  
bool ossimNitfIchipbTag::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   const char* lookup;
   lookup = kwl.find(prefix, XFRM_FLAG_KW);
   if(lookup)
   {
      strcpy(theXfrmFlag, lookup);
   }
   lookup = kwl.find(prefix, SCALE_FACTOR_KW);
   if(lookup)
   {
      strcpy(theScaleFactor, lookup);
   }
   lookup = kwl.find(prefix, ANAMRPH_CORR_KW);
   if(lookup)
   {
      strcpy(theAnamrphCorr, lookup);
   }
   lookup = kwl.find(prefix, SCANBLK_NUM_KW);
   if(lookup)
   {
      strcpy(theScanBlock, lookup);
   }
   lookup = kwl.find(prefix, OP_ROW_11_KW);
   if(lookup)
   {
      strcpy(theOpRow11, lookup);
   }
   lookup = kwl.find(prefix, OP_COL_11_KW);
   if(lookup)
   {
      strcpy(theOpCol11, lookup);
   }
   lookup = kwl.find(prefix, OP_ROW_12_KW);
   if(lookup)
   {
      strcpy(theOpRow12, lookup);
   }
   lookup = kwl.find(prefix, OP_COL_12_KW);
   if(lookup)
   {
      strcpy(theOpCol12, lookup);
   }
   lookup = kwl.find(prefix, OP_ROW_21_KW);
   if(lookup)
   {
      strcpy(theOpRow21, lookup);
   }
   lookup = kwl.find(prefix, OP_COL_21_KW);
   if(lookup)
   {
      strcpy(theOpCol21, lookup);
   }
   lookup = kwl.find(prefix, OP_ROW_22_KW);
   if(lookup)
   {
      strcpy(theOpRow22, lookup);
   }
   lookup = kwl.find(prefix, OP_COL_22_KW);
   if(lookup)
   {
      strcpy(theOpCol22, lookup);
   }
   lookup = kwl.find(prefix, FI_ROW_11_KW);
   if(lookup)
   {
      strcpy(theFiRow11, lookup);
   }
   lookup = kwl.find(prefix, FI_COL_11_KW);
   if(lookup)
   {
      strcpy(theFiCol11, lookup);
   }
   lookup = kwl.find(prefix, FI_ROW_12_KW);
   if(lookup)
   {
      strcpy(theFiRow12, lookup);
   }
   lookup = kwl.find(prefix, FI_COL_12_KW);
   if(lookup)
   {
      strcpy(theFiCol12, lookup);
   }
   lookup = kwl.find(prefix, FI_ROW_21_KW);
   if(lookup)
   {
      strcpy(theFiRow21, lookup);
   }
   lookup = kwl.find(prefix, FI_COL_21_KW);
   if(lookup)
   {
      strcpy(theFiCol21, lookup);
   }
   lookup = kwl.find(prefix, FI_ROW_22_KW);
   if(lookup)
   {
      strcpy(theFiRow22, lookup);
   }
   lookup = kwl.find(prefix, FI_COL_22_KW);
   if(lookup)
   {
      strcpy(theFiCol22, lookup);
   }
   lookup = kwl.find(prefix, FI_ROW_KW);
   if(lookup)
   {
      strcpy(theFullImageRow, lookup);
   }
   lookup = kwl.find(prefix, FI_COL_KW);
   if(lookup)
   {
      strcpy(theFullImageCol, lookup);
   }

   return true;
}
