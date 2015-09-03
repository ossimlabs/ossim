//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: ossimAdjustableParameterInterface.h 22892 2014-09-22 15:17:33Z okramer $
#ifndef ossimAdjustableParameterInterface_HEADER
#define ossimAdjustableParameterInterface_HEADER
#include <ossim/base/ossimRtti.h>
#include <vector>
#include <ossim/base/ossimAdjustmentInfo.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimObject.h>

class OSSIMDLLEXPORT ossimAdjustableParameterInterface
{
public:
   ossimAdjustableParameterInterface();
   ossimAdjustableParameterInterface(const ossimAdjustableParameterInterface& rhs);
   virtual ~ossimAdjustableParameterInterface(){}
   void newAdjustment(ossim_uint32 numberOfParameters=0);
   void setAdjustmentDescription(const ossimString& description);
   ossimString getAdjustmentDescription()const;

   //! Returns adjustmentDescription of specific adjustmentInfo
   ossimString getAdjustmentDescription(ossim_uint32 adjustmentIdx)const;

   void setCurrentAdjustment(ossim_uint32 adjustmentIndex, bool notify=false);

   //! Sets the current adjustment to the adjustment with a matching description. 
   //! Returns true if successfully matched.
   bool setCurrentAdjustment(const ossimString& description, bool notify=false);

   void eraseAdjustment(bool notify);
   void eraseAdjustment(ossim_uint32 idx, bool notify);
   virtual void initAdjustableParameters();
   void resetAdjustableParameters(bool notify=false);
   void copyAdjustment(ossim_uint32 idx, bool notify);
   void copyAdjustment(bool notify = false);

   virtual ossimObject* getBaseObject()=0;
   virtual const ossimObject* getBaseObject()const=0;
   
   /*!
    * Will copy the adjustment but will set the new center to the
    * applied current center plus the application of the adjustment
    *
    */
   void keepAdjustment(ossim_uint32 idx, bool createCopy);
   virtual void keepAdjustment(bool createCopy=true);
   

   const ossimAdjustableParameterInterface& operator = (const ossimAdjustableParameterInterface& rhs);
   void removeAllAdjustments();
   ossim_uint32 getNumberOfAdjustableParameters()const;
   double       getAdjustableParameter(ossim_uint32 idx)const;
   virtual void setAdjustableParameter(ossim_uint32 idx, double value,
                                       bool notify=false);
   virtual void setAdjustableParameter(ossim_uint32 idx,
                                       double value,
                                       double sigma,
                                       bool notify=false);
   double       getParameterSigma(ossim_uint32 idx)const;
   void         setParameterSigma(ossim_uint32 idx,
                                  double value,
                                  bool notify=false);
   ossimUnitType getParameterUnit(ossim_uint32 idx)const;
   void          setParameterUnit(ossim_uint32 idx, ossimUnitType unit);
   void          setParameterUnit(ossim_uint32 idx, const ossimString& unit);

   void           setParameterCenter(ossim_uint32 idx,
                                     double center,
                                     bool notify = false);
   double        getParameterCenter(ossim_uint32 idx)const;
   double        computeParameterOffset(ossim_uint32 idx)const;
   void          setParameterOffset(ossim_uint32 idx,
                                    ossim_float64 value,
                                    bool notify = false);
   
   ossimString   getParameterDescription(ossim_uint32 idx)const;
   void          setParameterDescription(ossim_uint32 idx,
                                         const ossimString& descrption);

   ossim_int32 findParameterIdxGivenDescription(ossim_uint32 adjustmentIdx,
                                                const ossimString& name)const;
   ossim_int32 findParameterIdxContainingDescription(ossim_uint32 adjustmentIdx,
                                                     const ossimString& name)const;
   
   bool isParameterLocked(ossim_uint32 idx)const;

   void setParameterLockFlag(ossim_uint32 idxParam, bool flag);
   bool getParameterLockFlag(ossim_uint32 idx)const;

   void lockAllParametersCurrentAdjustment();
   void unlockAllParametersCurrentAdjustment();

   void lockAllParameters(ossim_uint32 idxAdjustment);
   void unlockAllParameters(ossim_uint32 idxAdjustment);
   
   void resizeAdjustableParameterArray(ossim_uint32 numberOfParameters);

   void setAdjustment(const ossimAdjustmentInfo& adj, bool notify=false);
   void setAdjustment(ossim_uint32 idx, const ossimAdjustmentInfo& adj, bool notify=false);
   
   void addAdjustment(const ossimAdjustmentInfo& adj, bool notify);
   void getAdjustment(ossimAdjustmentInfo& adj) const;
   void getAdjustment(ossim_uint32 idx, ossimAdjustmentInfo& adj) const;
   
   ossim_uint32 getNumberOfAdjustments()const;
   ossim_uint32 getCurrentAdjustmentIdx()const;
   
   void setDirtyFlag(bool flag=true);
   void setAllDirtyFlag(bool flag = true);
   bool hasDirtyAdjustments()const;
   
   //! Saves the current active adjustment to the KWL file. If an adjustment of the same 
   //! description already exists in the KWL, it is replaced by the current adjustment.
   //! The object is synced up with contents of KWL.
   virtual void saveCurrentAdjustmentOnly(ossimKeywordlist &kwl, 
                                          const ossimString& prefix=ossimString(""));

   bool saveAdjustments(ossimKeywordlist& kwl,
                        const ossimString& prefix=ossimString(""))const;
   bool loadAdjustments(const ossimKeywordlist& kwl,
                        const ossimString& prefix=ossimString(""));

   //! Provides the derived adjustable class an opportunity to update its state after a param change.
   virtual void updateModel() {}

   //!  Dumps the currently active adjustment to ostream.
   std::ostream& print(std::ostream& out) const;

protected:
   //! Returns true if specified parameter has been modified since last setAllChangeFlag(false) call.
   bool paramChanged(ossim_uint32 param_idx) const { return theChangeFlags[param_idx]; }

   //! Sets all the change flags to the boolean indicated to indicate parameters are changed (TRUE)
   //! or not (FALSE).
   void setAllChangeFlags(bool areChanged);

   //! Initializes the change flags to TRUE.
   void initChangeFlags();
   
private:
   std::vector<bool> theChangeFlags;
   std::vector<ossimAdjustmentInfo> theAdjustmentList;
   ossim_uint32                     theCurrentAdjustment;
   
public:
   virtual void adjustableParametersChanged();
   
TYPE_DATA
};

#endif
