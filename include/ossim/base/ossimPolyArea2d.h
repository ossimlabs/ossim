//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//*******************************************************************
//$Id: ossimPolyArea2d.h 23608 2015-10-28 13:51:35Z gpotts $
#ifndef ossimPolyArea2d_HEADER
#define ossimPolyArea2d_HEADER 1
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimPolygon.h>
#include <ossim/base/ossimReferenced.h>
#include <vector>

class ossimPolyArea2dPrivate;

class OSSIM_DLL ossimPolyArea2d
{
public:
   friend OSSIM_DLL std::ostream &operator<<(std::ostream &out, const ossimPolyArea2d &data);
   friend class ossimPolyArea2dPrivate;
   ossimPolyArea2d();
   ossimPolyArea2d(const std::vector<ossimGpt> &polygon);
   ossimPolyArea2d(const std::vector<ossimDpt> &polygon);
   ossimPolyArea2d(const ossimPolygon &shell, const std::vector<ossimPolygon> &holes);

   ossimPolyArea2d(const ossimDpt &p1,
                   const ossimDpt &p2,
                   const ossimDpt &p3,
                   const ossimDpt &p4);
   ossimPolyArea2d(const ossimPolyArea2d &rhs);

   ossimPolyArea2d(const ossimIrect &rect);
   ossimPolyArea2d(const ossimDrect &rect);
   ossimPolyArea2d(const ossimPolygon &polygon);
   ~ossimPolyArea2d();

   void clear()
   {
      clearPolygons();
   }
   const ossimPolyArea2d &operator=(const ossimPolyArea2d &rhs);
   const ossimPolyArea2d &operator=(const ossimPolygon &rhs);
   const ossimPolyArea2d &operator=(const ossimIrect &rect);
   const ossimPolyArea2d &operator=(const ossimDrect &rect);
   const ossimPolyArea2d &operator=(const std::vector<ossimGpt> &polygon);
   const ossimPolyArea2d &operator=(const std::vector<ossimDpt> &polygon);
   const ossimPolyArea2d &operator&=(const ossimPolyArea2d &rhs);
   ossimPolyArea2d operator&(const ossimPolyArea2d &rhs) const;
   ossimPolyArea2d operator+(const ossimPolyArea2d &rhs) const;
   const ossimPolyArea2d &operator+=(const ossimPolyArea2d &rhs);
   ossimPolyArea2d operator-(const ossimPolyArea2d &rhs) const;
   const ossimPolyArea2d &operator-=(const ossimPolyArea2d &rhs);

   ossim_float64 getArea()const;
   bool isEmpty() const;
   void makeValid();
   bool isValid(bool displayValidationError = false) const;
   bool isPointWithin(const ossimDpt &point) const;
   bool isPointWithin(double x, double y) const;
   void getBoundingRect(ossimDrect &rect) const;

   bool intersects(const ossimPolyArea2d &rhs) const;
   void add(const ossimPolyArea2d &rhs);
   bool getVisiblePolygons(std::vector<ossimPolygon> &polyList) const;
   bool getPolygonHoles(std::vector<ossimPolygon> &polyList) const;

   ossimPolyArea2d &toMultiPolygon();

   /**
   * Returns the Well Known Text string
   */
   std::string toString() const;
   bool setFromWkt(const std::string &s);

   bool saveState(ossimKeywordlist &kwl,
                  const char *prefix = 0) const;
   bool loadState(const ossimKeywordlist &kwl,
                  const char *prefix = 0);

protected:
   ossimPolyArea2dPrivate *m_privateData;

   void clearPolygons();
};

#endif /* #ifndef ossimPolyArea2d_HEADER */
