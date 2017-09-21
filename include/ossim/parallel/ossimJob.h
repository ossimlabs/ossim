//**************************************************************************************************
//                          OSSIM -- Open Source Software Image Map
//
// LICENSE: See top level LICENSE.txt file.
//
//**************************************************************************************************
//  $Id$
#ifndef ossimJob_HEADER
#define ossimJob_HEADER
#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimString.h>
#include <list>
#include <mutex>
#include <memory>
class ossimJob;

/**
* This is the job callback interface
* It allows one to attach and listen for different states of the job
* and if properties have changed
*
* @code
* #include <ossim/base/Thread.h>
* #include <ossim/parallel/ossimJob.h>
* // Put your includes here:
* 
* // System includes:
* #include <memory>
* #include <iostream>
* 
* class TestJob : public ossimJob
* {
* public:
*    TestJob(){}
* protected:
*    virtual void run()
*    {
*       std:cout << "Running Job\n";
*       ossim::Thread::sleepInSeconds(2);
*       std::cout << "Finished Running Job\n";
*    }
* };
* class MyCallback : public ossimJobCallback
* {
* public:
*    MyCallback(){}
*    virtual void started(std::shared_ptr<ossimJob> job)  
*    {
*       std::cout << "Started job\n";
*       ossimJobCallback::started(job);
*    }
*    virtual void finished(std::shared_ptr<ossimJob> job) 
*    {
*       std::cout << "Finished job\n";
*       ossimJobCallback::finished(job);
*    }
* };
* int main(int argc, char *argv[])
* {
*    std::shared_ptr<TestJob> job = std::make_shared<TestJob>();
*    job->setCallback(std::make_shared<MyCallback>());
*    job->start();
*
*    return 0;
* }
* @endcode
*/
class OSSIM_DLL ossimJobCallback
{
public:
   ossimJobCallback(std::shared_ptr<ossimJobCallback> nextCallback=0):m_nextCallback(nextCallback){}

   virtual void ready(std::shared_ptr<ossimJob> job)    {if(m_nextCallback) m_nextCallback->ready(job);   }
   virtual void started(std::shared_ptr<ossimJob> job)  {if(m_nextCallback) m_nextCallback->started(job); }
   virtual void finished(std::shared_ptr<ossimJob> job) {if(m_nextCallback) m_nextCallback->finished(job);}
   virtual void canceled(std::shared_ptr<ossimJob> job) {if(m_nextCallback) m_nextCallback->canceled(job);}

   virtual void nameChanged(const ossimString& name, std::shared_ptr<ossimJob> job)
   {if(m_nextCallback) m_nextCallback->nameChanged(name, job);}
   
   virtual void descriptionChanged(const ossimString& description, std::shared_ptr<ossimJob> job)
   {if(m_nextCallback) m_nextCallback->descriptionChanged(description, job);}

   virtual void idChanged(const ossimString& id, std::shared_ptr<ossimJob> job)
   {if(m_nextCallback) m_nextCallback->idChanged(id, job);}

   virtual void percentCompleteChanged(double percentValue, std::shared_ptr<ossimJob> job)
   {if(m_nextCallback) m_nextCallback->percentCompleteChanged(percentValue, job);}

   void setCallback(std::shared_ptr<ossimJobCallback> c){m_nextCallback = c;}
   std::shared_ptr<ossimJobCallback> callback(){return m_nextCallback;}

protected:
   std::shared_ptr<ossimJobCallback> m_nextCallback;
};


/**
* This is the job callback interface
* It allows one to attach and listen for different states of the job
* and if properties have changed
*
* @code
* #include <ossim/base/Thread.h>
* #include <ossim/parallel/ossimJob.h>
* // Put your includes here:
* 
* // System includes:
* #include <memory>
* #include <iostream>
* 
* class TestJob : public ossimJob
* {
* public:
*    TestJob(){}
* protected:
*    virtual void run()
*    {
*       std:cout << "Running Job\n";
*       ossim::Thread::sleepInSeconds(2);
*       std::cout << "Finished Running Job\n";
*    }
* };
* class MyCallback : public ossimJobCallback
* {
* public:
*    MyCallback(){}
*    virtual void started(std::shared_ptr<ossimJob> job)  
*    {
*       std::cout << "Started job\n";
*       ossimJobCallback::started(job);
*    }
*    virtual void finished(std::shared_ptr<ossimJob> job) 
*    {
*       std::cout << "Finished job\n";
*       ossimJobCallback::finished(job);
*    }
* };
* int main(int argc, char *argv[])
* {
*    std::shared_ptr<TestJob> job = std::make_shared<TestJob>();
*    job->setCallback(std::make_shared<MyCallback>());
*    job->start();
*
*    return 0;
* }
* @endcode
*/
class OSSIM_DLL ossimJob : public std::enable_shared_from_this<ossimJob>
{
public:
   typedef std::list<std::shared_ptr<ossimJob> > List;

   /** 
   * This is a Bit vector.  The only value that can be assigned as both active is FINISHED and CANCEL.
   * CANCELED job may not yet be finished.  Once the job is finished the Cancel is complete
   */ 
   enum State
   {
      ossimJob_NONE     = 0,
      ossimJob_READY    = 1,
      ossimJob_RUNNING  = 2,
      ossimJob_CANCEL   = 4,
      ossimJob_FINISHED = 8,
      ossimJob_ALL = (ossimJob_READY|ossimJob_RUNNING|ossimJob_CANCEL|ossimJob_FINISHED)
   };
   
   ossimJob() : m_state(ossimJob_READY),  m_priority(0.0) {}

   /**
   * Main entry point to the job.  It will set the state as running and then
   * call the pure virtual method run. Once completed the job is marked finished
   * only if the job was not canceled.
   *
   * Classes must override the run method.  @see run.
   */
   virtual void start();

   /**
   * This is a convenience method to get the shared representation 
   * of this pointer
   *
   * @return the shared pointer
   */
   std::shared_ptr<ossimJob> getSharedFromThis(){
      return shared_from_this();
   }

   /**
   * This is a convenience method to get the shared representation 
   * of this pointer
   *
   * @return the shared pointer
   */
   std::shared_ptr<const ossimJob> getSharedFromThis()const{
      return shared_from_this();
   }

   /**
   * When the pernet complete is set for the job it will call any callbacks 
   * and nofity percentCompleteChanged.
   *
   * @value percent complete
   */   
   void setPercentComplete(double value)
   {
      std::lock_guard<std::mutex> lock(m_jobMutex);
      if(m_callback)
      {
         m_callback->percentCompleteChanged(value, getSharedFromThis());
      }
   }

   /**
   * sets the priority of the job
   *
   * @param value priority value
   */
   void setPriority(double value)
   {
      std::lock_guard<std::mutex> lock(m_jobMutex);
      m_priority = value;
   }

   /*
   * @return the priotiy of the job
   */
   double priority()const
   {
      return m_priority;
   }

   /**
   * If derived interfaces implement a block this will allow one to release.
   * Derived classes must override.
   */
   virtual void release(){}

   /**
   * @return the state of the object
   */
   State state()const
   {
      std::lock_guard<std::mutex> lock(m_jobMutex);
      return m_state;
   }

   /**
   * Will clear out the state and the call setState
   *
   * @param value is the state you wish to reset to
   */
   virtual void resetState(int value)
   {
      m_jobMutex.lock();
      if(value != m_state)
      {
         m_state = ossimJob_NONE;
         m_jobMutex.unlock();
         setState(value);
      }
      else 
      {
         m_jobMutex.unlock();
      }

   }

   /**
   * Will allow you to set the state of the job
   *
   * @param value is the state you wish to set to
   * @param on will turn the value on if on is true
   *        and turn it off otherwise.
   */
   virtual void setState(int value, bool on=true);

   /**
   * @return true if the job is in a cancel state and false
   *         otherwise
   */
   bool isCanceled()const
   {
      std::lock_guard<std::mutex> lock(m_jobMutex);
      return (m_state & ossimJob_CANCEL);
   }

   /**
   * Sets the state if the object as cancelled
   */
   virtual void cancel()
   {
      // append the cancel flag to current state
      setState(ossimJob_CANCEL);
   }

   /**
   * Sets the state if the object as ready
   */
   virtual void ready()
   {
      resetState(ossimJob_READY);
   }

   /**
   * Sets the state if the object as running
   */
   virtual void running()
   {
      resetState(ossimJob_RUNNING);
   }

   /**
   * Sets the state if the object as finished
   */
   virtual void finished()
   {
      int newState = 0;
      {
         // maintain the cancel flag so we can indicate the job has now finished
         std::lock_guard<std::mutex> lock(m_jobMutex);
         newState = ((m_state & ossimJob_CANCEL) | 
            (ossimJob_FINISHED));
      }
      // now reset to the new state
      resetState(newState);
   }

   /**
   * @return true if the state of the object is in a ready state.
   */
   bool isReady()const
   {
      std::lock_guard<std::mutex> lock(m_jobMutex);
      return m_state & ossimJob_READY;
   }

   /**
   * @return true if the state of the object is in some kind of stopped state.
   */
   bool isStopped()const
   {
      std::lock_guard<std::mutex> lock(m_jobMutex);
      return (m_state & ossimJob_FINISHED);
   }

   /**
   * @return true if the state of the object is in a finished state.
   */
   bool isFinished()const
   {
      std::lock_guard<std::mutex> lock(m_jobMutex);
      return (m_state & ossimJob_FINISHED);
   }

   /**
   * @return true if the state of the object is in a running state.
   */
   bool isRunning()const
   {
      std::lock_guard<std::mutex> lock(m_jobMutex);
      return (m_state & ossimJob_RUNNING);
   }

   /**
   * @param callback callback used to call different state of a job
   */
   void setCallback(std::shared_ptr<ossimJobCallback> callback)
   {
      std::lock_guard<std::mutex> lock(m_jobMutex);
      m_callback = callback;
   }

   /**
   * Sets the name of a job
   *
   * @param value the name of the job
   */
   void setName(const ossimString& value)
   {
      bool changed = false;
      std::shared_ptr<ossimJobCallback> callback;
      {
         std::lock_guard<std::mutex> lock(m_jobMutex);
         changed = value!=m_name;
         m_name = value;
         callback = m_callback;
      }
      if(changed&&callback)
      {
         callback->nameChanged(value, getSharedFromThis());
      }
   }

   /**
   * @return the name of the job
   */
   const ossimString& name()const
   {
      std::lock_guard<std::mutex> lock(m_jobMutex);
      return m_name;
   }

   /*
   * sets the ID
   *
   * @param value the id to set the job to
   */
   void setId(const ossimString& value)
   {
      bool changed = false;
      std::shared_ptr<ossimJobCallback> callback;
      {
         std::lock_guard<std::mutex> lock(m_jobMutex);
         changed = value!=m_id;
         m_id = value;
         callback = m_callback;
      }
      if(changed&&callback)
      {
         callback->idChanged(value, getSharedFromThis());
      }
   }

   /*
   * @return id of the job
   */
   const ossimString& id()const
   {
      std::lock_guard<std::mutex> lock(m_jobMutex);
      return m_id;
   }

   /*
   * @param value the description to set on the job
   */
   void setDescription(const ossimString& value)
   {
      bool changed = false;
      std::shared_ptr<ossimJobCallback> callback;
      {
         std::lock_guard<std::mutex> lock(m_jobMutex);
         changed = value!=m_description;
         m_description = value;
         callback = m_callback;
      }
      if(changed&&callback)
      {
         callback->descriptionChanged(value, getSharedFromThis());
      }
   }

   /**
   * @return the desciption of the job
   */
   const ossimString& description()const
   {
      std::lock_guard<std::mutex> lock(m_jobMutex);
      return m_description;
   }

   /**
   * @return the callback
   */
   std::shared_ptr<ossimJobCallback> callback() {return m_callback;}

protected:
   mutable std::mutex m_jobMutex;
   ossimString m_name;
   ossimString m_description;
   ossimString m_id;
   State       m_state;
   double      m_priority;
   std::shared_ptr<ossimJobCallback> m_callback;

   /**
   * Abstract method and must be overriden by the base class.  The base ossimJob
   * will call run from the start method after setting some variables.
   */
   virtual void run()=0;
};

#endif
