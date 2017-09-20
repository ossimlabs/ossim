//**************************************************************************************************
//                          OSSIM -- Open Source Software Image Map
//
// LICENSE: See top level LICENSE.txt file.
//
//**************************************************************************************************
//  $Id$
#ifndef ossimJobQueue_HEADER
#define ossimJobQueue_HEADER

#include <ossim/parallel/ossimJob.h>
#include <ossim/base/Block.h>
#include <mutex>
#include <memory>
//*************************************************************************************************
//! Class for maintaining an ordered list of jobs to be processed. As the jobs are completed and
//! the product consumed, the jobs are removed from this list
//*************************************************************************************************
class OSSIM_DLL ossimJobQueue : public std::enable_shared_from_this<ossimJobQueue>
{
public:
   class OSSIM_DLL Callback
   {
   public:
      Callback(){}
      virtual void adding(ossimJobQueue* /*q*/, 
                          std::shared_ptr<ossimJob> /*job*/){}
      virtual void added(ossimJobQueue* /*q*/, std::shared_ptr<ossimJob> /*job*/){}
      virtual void removed(ossimJobQueue* /*q*/, std::shared_ptr<ossimJob>/*job*/){}
   };
   ossimJobQueue();
   
   std::shared_ptr<ossimJobQueue> getSharedFromThis(){
      return shared_from_this();
   }
   std::shared_ptr<const ossimJobQueue> getSharedFromThis()const{
      return shared_from_this();
   }
   virtual void add(std::shared_ptr<ossimJob> job, bool guaranteeUniqueFlag=true);
   virtual std::shared_ptr<ossimJob> removeByName(const ossimString& name);
   virtual std::shared_ptr<ossimJob> removeById(const ossimString& id);
   virtual void remove(const std::shared_ptr<ossimJob> Job);
   virtual void removeStoppedJobs();
   virtual void clear();
   virtual std::shared_ptr<ossimJob> nextJob(bool blockIfEmptyFlag=true);
   virtual void releaseBlock();
   bool isEmpty()const;
   ossim_uint32 size();
   void setCallback(std::shared_ptr<Callback> c);
   std::shared_ptr<Callback> callback();
   
protected:
   ossimJob::List::iterator findById(const ossimString& id);
   ossimJob::List::iterator findByName(const ossimString& name);
   ossimJob::List::iterator findByPointer(const std::shared_ptr<ossimJob> job);
   ossimJob::List::iterator findByNameOrPointer(const std::shared_ptr<ossimJob> job);
   bool hasJob(std::shared_ptr<ossimJob> job);
   
   mutable std::mutex m_jobQueueMutex;
   ossim::Block m_block;
   ossimJob::List m_jobQueue;
   std::shared_ptr<Callback> m_callback;
};

#endif
