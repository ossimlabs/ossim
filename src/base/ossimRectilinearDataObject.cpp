//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// Contributor: David A. Horner (DAH) - http://dave.thehorners.com
//
//*************************************************************************
// $Id: ossimRectilinearDataObject.cpp 22828 2014-07-11 15:56:19Z dburken $

#include <ossim/base/ossimRectilinearDataObject.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimScalarTypeLut.h>
#include <ossim/base/ossimKeywordlist.h>
#include <sstream>

RTTI_DEF1(ossimRectilinearDataObject, "ossimRectilinearDataObject", ossimDataObject);

using namespace std;

ossimRectilinearDataObject::ossimRectilinearDataObject()
   : ossimDataObject(),
     m_numberOfDataComponents(0),
     m_scalarType(),
     m_dataBuffer(),
     m_spatialExtents()
{
}

ossimRectilinearDataObject::ossimRectilinearDataObject(
   const ossimRectilinearDataObject& rhs)
   : ossimDataObject(rhs),
     m_numberOfDataComponents(rhs.m_numberOfDataComponents),
     m_scalarType(rhs.m_scalarType),
     m_dataBuffer(rhs.m_dataBuffer),
     m_spatialExtents(rhs.m_spatialExtents)
{
}

ossimRectilinearDataObject::ossimRectilinearDataObject(
    ossim_uint64 numberOfSpatialComponents,
    ossimSource *owner,
    ossim_uint64 numberOfDataComponents,
    ossimScalarType scalarType,
    ossimDataObjectStatus /* status */)
    : ossimDataObject(owner, OSSIM_NULL),
      m_numberOfDataComponents(numberOfDataComponents),
      m_scalarType(scalarType),
      m_dataBuffer(0),
      m_spatialExtents(numberOfSpatialComponents)
{
}

ossimRectilinearDataObject::ossimRectilinearDataObject(
    ossimSource *owner,
    ossim_uint64 numberOfDataComponents,
    ossim_uint64 length,
    ossimScalarType scalarType,
    ossimDataObjectStatus /* status */)
    : ossimDataObject(owner, OSSIM_NULL),
      m_numberOfDataComponents(numberOfDataComponents),
      m_scalarType(scalarType),
      m_dataBuffer(0),
      m_spatialExtents(1)
{
   m_spatialExtents[0] = length;
}

ossimRectilinearDataObject::ossimRectilinearDataObject(
    ossimSource *owner,
    ossim_uint64 numberOfDataComponents,
    ossim_uint64 width,
    ossim_uint64 height,
    ossimScalarType scalarType,
    ossimDataObjectStatus /* status */)
    : ossimDataObject(owner, OSSIM_NULL),
      m_numberOfDataComponents(numberOfDataComponents),
      m_scalarType(scalarType),
      m_dataBuffer(0),
      m_spatialExtents(2)
{
   m_spatialExtents[0] = width;
   m_spatialExtents[1] = height;
}

ossimRectilinearDataObject::ossimRectilinearDataObject(
    ossimSource *owner,
    ossim_uint64 numberOfDataComponents,
    ossim_uint64 width,
    ossim_uint64 height,
    ossim_uint64 depth,
    ossimScalarType scalarType,
    ossimDataObjectStatus /* status */)
    : ossimDataObject(owner, OSSIM_NULL),
      m_numberOfDataComponents(numberOfDataComponents),
      m_scalarType(scalarType),
      m_dataBuffer(0),
      m_spatialExtents(3)
{
   m_spatialExtents[0] = width;
   m_spatialExtents[1] = height;
   m_spatialExtents[2] = depth;
}

ossimRectilinearDataObject::~ossimRectilinearDataObject()
{
}

ossim_uint64 ossimRectilinearDataObject::computeSpatialProduct() const
{
   ossim_uint64 spatialProduct = 1;
   for (ossim_uint64 index = 0; index < m_spatialExtents.size(); ++index)
   {
      spatialProduct *= m_spatialExtents[index];
   }
   return spatialProduct;
}

void ossimRectilinearDataObject::setNumberOfDataComponents(ossim_uint64 n)
{
   m_numberOfDataComponents = n;
}

void ossimRectilinearDataObject::setSpatialExtents(ossim_uint64 *extents,
                                                   ossim_uint64 size)
{
   if (extents)
   {
      m_spatialExtents.resize(size);
      for (ossim_uint64 i = 0; i < size; ++i)
      {
         m_spatialExtents[i] = extents[i];
      }
   }
}

void ossimRectilinearDataObject::setScalarType(ossimScalarType type)
{
   m_scalarType = type;
}

ossim_uint64 ossimRectilinearDataObject::getNumberOfDataComponents() const
{
   return m_numberOfDataComponents;
}

ossim_uint64 ossimRectilinearDataObject::getNumberOfSpatialComponents() const
{
   return (ossim_uint64)m_spatialExtents.size();
}

const ossim_uint64 *ossimRectilinearDataObject::getSpatialExtents() const
{
   return &(m_spatialExtents.front());
}

ossimScalarType ossimRectilinearDataObject::getScalarType() const
{
   return m_scalarType;
}

ossim_uint64 ossimRectilinearDataObject::getScalarSizeInBytes() const
{
   return ossim::scalarSizeInBytes(getScalarType());
}

void* ossimRectilinearDataObject::getBuf()
{
   if (m_dataBuffer.size() > 0)
   {
      return static_cast<void*>(&m_dataBuffer.front());
   }
   return NULL;
}

const void* ossimRectilinearDataObject::getBuf()const
{
   if (m_dataBuffer.size() > 0)
   {
      return static_cast<const void*>(&m_dataBuffer.front());
   }
   return NULL;
}

void ossimRectilinearDataObject::assign(const ossimRectilinearDataObject* data)
{
   if(data)
   {
      if (this != data)
      {
         ossimDataObject::assign(data);
         
         m_numberOfDataComponents    = data->m_numberOfDataComponents;
         m_scalarType                = data->m_scalarType;
         m_dataBuffer                = data->m_dataBuffer;
         m_spatialExtents            = data->m_spatialExtents;
      }
   }
}

void ossimRectilinearDataObject::initialize()
{
   if (m_dataBuffer.size() != getDataSizeInBytes())
   {
      // std::vector::resize can throw a std::bad_alloc so wrap it...
      try
      {
         m_dataBuffer.resize( getDataSizeInBytes() );
      }
      catch( std::exception& e )
      {
         std::ostringstream errMsg;
         errMsg << "ossimRectilinearDataObject::initialize caught exception on resize:\n"
                << "Buffer size in bytes: " << getDataSizeInBytes()
                << "\n" << e.what() << std::endl;
         throw ossimException( errMsg.str() );
      }
      
      setDataObjectStatus(OSSIM_STATUS_UNKNOWN);
   }
}

ossim_uint64 ossimRectilinearDataObject::getDataSizeInBytes() const
{
   return (ossim_uint64)(getScalarSizeInBytes() *
                         computeSpatialProduct() *
                         m_numberOfDataComponents);
}

std::ostream& ossimRectilinearDataObject::print(std::ostream& out) const
{
   out << "ossimRectilinearDataObject::print:"
       << "\nm_numberOfDataComponents:     " << m_numberOfDataComponents
       << "\ntheNumberOfSpatialComponents:  " << m_spatialExtents.size()
       << "\nm_scalarType:                 "
       << (ossimScalarTypeLut::instance()->getEntryString(m_scalarType))
       << endl;
   
   return ossimDataObject::print(out);
}

const ossimRectilinearDataObject& ossimRectilinearDataObject::operator=(
   const ossimRectilinearDataObject& rhs)
{
   if (this != &rhs)
   {
      // ossimDataObject initialization:
      ossimDataObject::operator=(rhs);

      // ossimRectilinearDataObject (this) initialization:
      m_numberOfDataComponents    = rhs.m_numberOfDataComponents;
      m_scalarType                = rhs.m_scalarType;
      m_dataBuffer                = rhs.m_dataBuffer;
      m_spatialExtents            = rhs.m_spatialExtents;
   }
   return *this;
}

bool ossimRectilinearDataObject::saveState(ossimKeywordlist& kwl, const char* prefix)const
{
   ossimString byteEncoded;
   ossim::toSimpleStringList(byteEncoded, m_dataBuffer);
   kwl.add(prefix, "data_buffer", byteEncoded, true);
   ossim::toSimpleStringList(byteEncoded, m_spatialExtents);
   kwl.add(prefix, "spatial_extents", byteEncoded, true);
   kwl.add(prefix, ossimKeywordNames::SCALAR_TYPE_KW, ossimScalarTypeLut::instance()->getEntryString(m_scalarType));
   
   return ossimDataObject::saveState(kwl, prefix);
}

bool ossimRectilinearDataObject::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   if(!ossimDataObject::loadState(kwl, prefix)) return false;
   
   const char* spatial_extents = kwl.find(prefix, "spatial_extents");
   const char* data_buffer = kwl.find(prefix, "data_buffer");
   const char* scalar_type = kwl.find(prefix, ossimKeywordNames::SCALAR_TYPE_KW);
   m_spatialExtents.clear();
   m_dataBuffer.clear();
                                 
   if(spatial_extents)
   {
      if(!ossim::toSimpleVector(m_spatialExtents, ossimString(spatial_extents)))
      {
         return false;
      }
   }
   if(data_buffer)
   {
      if(!ossim::toSimpleVector(m_dataBuffer, ossimString(kwl.find(prefix, "data_buffer"))))
      {
         return false;
      }
   }
   if(scalar_type)
   {
      ossimScalarTypeLut::instance()->getScalarTypeFromString(scalar_type);
   }
   else 
   {
      m_scalarType = OSSIM_SCALAR_UNKNOWN;
   }

   m_numberOfDataComponents = (ossim_uint64)m_spatialExtents.size();

   return true;
   
}                     
