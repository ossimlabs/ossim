/* -*-c++-*- libossim - Copyright (C) since 2004 Garrett Potts 
 * LICENSE: LGPL
 * Author: Garrett Potts
*/
#ifndef ossimReferenced_HEADER
#define ossimReferenced_HEADER
#include <ossim/base/ossimConstants.h>

#include <atomic>
#include <mutex>

/**
* ossimReferenced allows for shared object ref counting
* if the reference count ever gets to 0 or less it will
* delete 'this' object.
*
* Currently uses std::mutex to control the locking of the reference
* count variable.
*
* Eventually we would like to replace all of ossimReferenced and ossimRefPtr
* with C++11 std::shared_ptr
*/
class OSSIMDLLEXPORT ossimReferenced
{
 public:
   ossimReferenced()
   : m_refCount(0)
      {}
   
   ossimReferenced(const ossimReferenced&)
   : m_refCount(0)
   {}
   inline ossimReferenced& operator = (const ossimReferenced&) { return *this; }
   

   /** 
    * increment the reference count by one, indicating that 
    * this object has another pointer which is referencing it.
    */
   inline void ref() const;
   
   /**
    * decrement the reference count by one, indicating that 
    * a pointer to this object is referencing it.  If the
    * reference count goes to zero, it is assumed that this object
    * is no longer referenced and is automatically deleted.
    */
   inline void unref() const;
   
   /**
    * decrement the reference count by one, indicating that 
    * a pointer to this object is referencing it.  However, do
    * not delete it, even if ref count goes to 0.  Warning, unref_nodelete() 
    * should only be called if the user knows exactly who will
    * be resonsible for, one should prefer unref() over unref_nodelete() 
    * as the later can lead to memory leaks.
    */
   inline void unref_nodelete() const 
   { 
     // std::lock_guard<std::mutex> lock(theRefMutex); 
     --m_refCount;
   }
   
   /**
    * @return the number pointers currently referencing this object. 
    */
   inline int referenceCount() const { return m_refCount.load(); }
   
   
 protected:
   virtual ~ossimReferenced();

 private:
    mutable std::atomic_int      m_refCount;
};

inline void ossimReferenced::ref() const
{
  ++m_refCount;
}

inline void ossimReferenced::unref() const
{
  bool needDelete = false;
  {
    // fetch_sub should return the value before subtraction.
    // so if 1 was before the subtraction that means it should
    // be zero after the subtraction so we will test <=1
    needDelete = m_refCount.fetch_sub(1) <= 1;
  }

  if (needDelete)
  {
    delete this;
  }
}

#endif
