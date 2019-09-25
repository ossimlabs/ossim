//---
// File: ossimNitfRsmecbTag.h
//---
#ifndef ossimNitfRsmecaTag_HEADER
#define ossimNitfRsmecaTag_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <vector>

class OSSIM_DLL ossimNitfRsmecbTag : public ossimNitfRegisteredTag
{
public:
   typedef std::vector<ossim_float64> DoubleArray;
   typedef std::vector<ossimString> StringArray;
   enum
   {
      FLOAT21_SIZE = 21,
      IID_SIZE = 80,
      EDITION_SIZE = 40,
      TID_SIZE = 40,
      INCLIC_SIZE = 1,
      INCLUC_SIZE = 1,
      NPARO_SIZE = 2,
      IGN_SIZE = 2,
      CVDATE_SIZE = 8,
      NPAR_SIZE = 2,
      APTYP_SIZE = 1,
      LOCTYP_SIZE = 1,
      APBASE_SIZE = 1,
      NISAP_SIZE = 2,
      NISAPR_SIZE = 2,
      // Used for both row and column adjustments
      XPW_SIZE = 1,
      YPW_SIZE = 1,
      ZPW_SIZE = 1,

      NISAPC_SIZE = 2,
      XPWRC_SIZE = 1,
      YPWRC_SIZE = 1,
      ZPWRC_SIZE = 1,
      NGSAP_SIZE = 2,
      GSAPID_SIZE = 4,
      NBASIS_SIZE = 2,
      NUMOPG_SIZE = 2,
      TCDF_SIZE = 1,
      ACSMC_SIZE = 1,
      NCSEG_SIZE = 1,
      UACSMC_SIZE = 1,
      UNCSR_SIZE = 1,
      UNCSC_SIZE = 1

   };
   class OSSIM_DLL ImageSpaceAdjustableParameter
   {
   public:
      ImageSpaceAdjustableParameter();
      void parseStream(std::istream &in);
      void writeStream(std::ostream &out);
      void clearFields();
      std::ostream &print(std::ostream &out,
                          const std::string &prefix = std::string()) const;

   protected:
      char m_parameterPowerX[XPW_SIZE + 1];
      char m_parameterPowerY[YPW_SIZE + 1];
      char m_parameterPowerZ[ZPW_SIZE + 1];
   };
   class OSSIM_DLL CorrelationSegment
   {
   public:
      CorrelationSegment();
      void parseStream(std::istream &in);
      void writeStream(std::ostream &out);
      void clearFields();
      std::ostream &print(std::ostream &out,
                          const std::string &prefix = std::string()) const;

   protected:
      char m_segmentCorrelationValue[FLOAT21_SIZE + 1];
      char m_segmentTauValue[FLOAT21_SIZE + 1];
   };

   /**
    * independent adjustable parameter (error) 
    * subgroups associated with the original 
    * adjustable parameters of the associated image.
    */
   class OSSIM_DLL IGNEntry
   {
      public:
         IGNEntry();
         void parseStream(std::istream &in);
         void writeStream(std::ostream &out);
         void clearFields();
         std::ostream &print(std::ostream &out,
                             const std::string &prefix = std::string()) const;

         ossim_int64 getNumberOfOriginalAdjustableParametersInSubgroup() const;
         bool getCSMCorrelationOptionFlag() const;
         ossim_int64 getNumberOfCorrelationSegments() const;

      protected:
         /**
          * 
          * NUMOPG (Bytes 2). Conditional INCLIC == Y
          * 
          * Number of Original Adjustable Parameters in Subgroup. This field contains the number 
          * of contiguous original adjustable parameters in this independent error subgroup. 
          * (Independent error subgroups are contiguous as well.)
          * 
          */
         char m_numopg[NUMOPG_SIZE + 1];

         /**
          * ERRCVG ( Bytes 21 each BCS-A). Conditional INCLIC == Y
          * 
          * Original Error Covariance Element. This field contains an original adjustable
          * parameter error covariance element corresponding to the independent error subgroup. 
          * The elements correspond to the upper triangular portion of the errorccovariance. 
          * They are in row major order.
          * 
          * Values:
          * 
          * +-9.99999999999999E+-99
          */
         DoubleArray m_errorCovarianceElement;

         /**
          * 
          * TCDF (1 byte BCS-N) Conditional INCLIC == Y
          * 
          * Time Correlation Domain Flag. This field defines the type of original 
          * adjustable parameter error, and hence, the corresponding correlation 
          * function domain, for this independent error subgroup. If this field is 0, 
          * the time correlation applies to all time intervals, both within and 
          * between images. The associated errors in the original adjustable parameters 
          * are “image element errors”.  If this field is 1, the time correlation 
          * applies to time intervals between images only. Time correlation for time 
          * intervals within an image is defined 100% positively correlated. The 
          * associated errors in the original adjustable parameters are “image errors”.  
          * If this field is 2, the time correlation applies to time intervals within an 
          * image only.  Time correlation for time intervals between images is defined 
          * as zero. The associated errors in the original adjustable parameters are 
          * “restricted image element errors”.
          * 
          * Values: 0,1,2
          */
         char m_tcdf[TCDF_SIZE + 1];

         /**
          * ACSMC (1 byte BCS-A).  Conditional INCLIC == Y
          * 
          * CSM Correlation Option. This field indicates whether a CSM correlation 
          * functional form is to be used (ACSMC=Y). instead of the baseline piece-wise 
          * linear correlation functional form (ACSMC=N) in order to represent the correlation 
          * of adjustable parameter error.
          * 
          * Values: Y or N
          */
         char m_acsmc[ACSMC_SIZE + 1];

         /**
          * NCSEG (1 Byte BCS-N). Conditinal ACSMC == N and INCLIC == Y
          * 
          * Number of Correlation Segments. This field contains the number of piece-wise 
          * linear correlation segments that make up the correlation function for this 
          * independent error subgroup.
          * 
          * Values: 2 through 9
          */
         char m_ncseg[NCSEG_SIZE + 1];

         /**
          * Conditional ACSMC==N and INCLIC == Y
          * 
          * Array defined by NCSEG
          */
         std::vector<CorrelationSegment> m_correlationSegmentArray;

         /**
          * AC (21 byte BCS-A). Conditional ACSMC==Y and INCLIC == Y
          * 
          * This field provides the CSM correlation function A parameter
          * 
          * Values:
          * 
          * +0.00000000000001E-99 to +1.00000000000000E+00 (greater than 0 and less than or equal to +1)
          */
         char m_ac[FLOAT21_SIZE + 1];

         /**
          * ALPC (21 bytes BCS-A). Conditional INCLIC == Y
          * 
          * This field provides the CSM correlation function alpha parameter.
          * 
          * Values:
          * 
          * +0.00000000000001E-99 to +1.00000000000000E+00 (greater than 0 and less than or equal to +1)
          */
         char m_alpc[FLOAT21_SIZE + 1];

         /**
          * BETC (21 Bytes BCS-A). Conditional INCLIC == Y
          * 
          * This field provides the CSM correlation function alpha parameter
          * 
          * Values: 
          * 
          * +0.00000000000000E+00 to +1.000000000000000E+01 (0 to +10)
          */
         char m_betc[FLOAT21_SIZE + 1];

         /**
          *  TC (21 Bytes BCS-A). Conditional INCLIC == Y
          *  Units: seconds
          * 
          *  This field provides the CSM correlation function T parameter.
          * 
          * Values:
          * 
          * +0.00000000000001E-99 to +9.99999999999999E+99 Positive value
          */
         char m_tc[FLOAT21_SIZE + 1];
   };
   typedef std::vector<IGNEntry> IGNEntryArray;
   typedef std::vector<ImageSpaceAdjustableParameter> AdjustableParameterArray;

   ossimNitfRsmecbTag();

   virtual void parseStream(std::istream &in);

   virtual void writeStream(std::ostream &out);

   virtual void clearFields();


   virtual std::ostream &print(std::ostream &out,
                               const std::string &prefix = std::string()) const;

   bool getInclicFlag() const;
   bool getInclucFlag() const;
   ossimString getLocalCoordinateSystemType() const;
   ossimString getAdjustableParameterType() const;
   ossim_int64 getNumberOfImageSpaceAdjParametersRow() const;
   ossim_int64 getNumberOfImageSpaceAdjParametersCol() const;
   ossim_int64 getNumberOfGroundSpaceAdjParameters() const;
   bool getAdjustableParameterBasisOption()const;
   ossim_int64 getNumberOfBasisAdjustableParameters()const;
   ossim_int64 getNumberOfActiveAdjustableParameters() const;
   ossim_int64 getNumberOfIndependentSubgroups() const;
   bool getUnmodeledCSMCorrelationOptionFlag()const;
   ossim_int64 getUncsr() const;
   ossim_int64 getUncsc() const;

       protected :
       /**
   * TAG IID (80 bytes BCS-A). Required
   *
   * Image Identifier. This field contains a character string that uniquely 
   * identifies the original full image that corresponds to the associated image. 
   * This is not to be confused with the identification of an image derived by filtering, 
   * chipping, re-sampling, or other such image-to-image transformations. The image 
   * identifier is left justified with trailing spaces
   */
       char m_iid[IID_SIZE + 1];

   /**
    * TAG EDITION (40 bytes BCS-A). Required
    * 
    * RSM Image Support Data Edition. This field contains a character string 
    * that uniquely identifies the RSM support data for the associated 
    * original full image. It is to consist of an identifier of up to 20 
    * characters for the processor that generated the RSM support data, 
    * to which is appended up to 20 characters that are unique among RSM TRE sets
    */
   char m_edition[EDITION_SIZE + 1];

   /**
    * TAG TID (40 byte BCS-A). Required
    *
    * Triangulation ID. This field contains an identifier that is unique 
    * to the most recent process after RSM support data generation that led to 
    * the adjustments and/or error covariance in this RSM support data edition.
    */
   char m_tid[TID_SIZE + 1];

   /**
    * TAG INCLIC (1 byte BCS-A). Required
    *
    * Include Indirect Error Covariance Flag. If the value of this field is Y, 
    * the indirect error covariance information is included in this TRE.
    * 
    * Values: Y or N
    */
   char m_inclic[INCLIC_SIZE + 1];

   /**
    * TAG INCLUC (1 byte BCS-A). Required.
    * 
    * Include Unmodeled Error Covariance Flag. If the value of this field is Y, 
    * the unmodeled error covariance information is included in this TRE.
    * 
    * Values: Y or N
    */
   char m_incluc[INCLUC_SIZE + 1];

   /**
    * TAG NOPARO (2 bytes BCS-N). Conditional INCLIC == Y
    * 
    * Number of Original Adjustable Parameters. This field 
    * contains the number of original adjustable parameters of 
    * the associated image. It is both the row and column 
    * dimensions of the (unmapped) original image error covariance 
    * and the original image error cross-covariance.  The maximum 
    * allowed number of original adjustable parameters is 53.
    * 
    * Values 01 to 53
    */
   char m_nparo[NPAR_SIZE + 1];

   /**
    * TAG IGN (2 bytes BCS-N). Conditional INCLIC == Y
    * 
    * Number of Independent Subgroups. This field contains the number 
    * of independent adjustable parameter (error) subgroups associated 
    * with the original adjustable parameters of the associated image.
    * 
    * Values: 01 to 36
    */
   char m_ign[IGN_SIZE+1];

   /**
    * TAG CVDATE (8 bytes BCS-A ). Conditional INCLIC == Y
    * 
    * Version Date of the Original Image Error Covariance. Date representing 
    * the version of the error model applicable to the original image error 
    * covariance. If populated, and two images are from the same sequence of 
    * images from the same sensor, and if the values of CVDATE are different 
    * in the two RSMECB TREs, all original adjustable parameter (errors) are 
    * assumed uncorrelated between the images.
    * 
    * Values:
    * 
    * YYYYMMDD
    * 
    *   (YYYY=four digit year,
    *    MM=two digit month;
    *    DD=two digit day)
    */
   char m_cvdate[CVDATE_SIZE + 1];

   /**
    * TAG NPAR ( 2  bytes BCS-N ). Required
    * 
    * Number of Active RSM Adjustable Parameters. This field contains the total 
    * number of active RSM adjustable parameters for the associated image.
    * The value of this field is the row dimension of any RSM cross-covariance (block) 
    * for the associated image, as well as the row dimension of the associated 
    * image's mapping matrix (field MAP). It is also both the row and the column 
    * dimension of any RSM (auto) covariance (block) for the associated image. NPAR's 
    * maximum value of 36 constrains an RSM covariance block to be reasonable size.
    * (If the "basis" option is off (APBASE=N), NPAR=NISAP if APTYP=I, and 
    * NPAR=NGSAP if APTYP=G. If the basis option is on (APBASE=Y), NPAR corresponds 
    * to the number of (new) active adjustable parameters and the number of row in 
    * the matrix A, as described for field APBASE.)
    * 
    * Values: 01 to 36
    * 
    */
   char m_npar[NPAR_SIZE + 1];

   /**
    * TAG APTYP (1 Byte BCS-A). Conditional INCLIC == Y
    * 
    * Adjustable Parameter Type. This field identifies whether RSM adjustable parameters 
    * are image-space (APTYP=I) or ground-space (APTYP=G) adjustable parameters.
    * 
    * Value: I or G
    */
   char m_aptype[APTYP_SIZE + 1];

   /**
    * TAG LOCTYP (1 byte BCS-A). Conditional INCLIC == Y
    * 
    * Local Coordinate System Identifier. The field identifies whether the Local 
    * coordinate system references rectangular ground coordinates (LOCTYP=R) or 
    * non-rectangular (image row/image column/geodetic height) coordinates (LOCTYP=N).
    * If RSM adjustable parameters are specified as ground-space (APTYP=G), the only 
    * valid value is LOCTYP=R.
    * 
    * Values:
    * 
    *   R or N
    */
   char m_loctype[LOCTYP_SIZE + 1];

   /**
    * TAG NSFX ( 21 byte BCS-A). Conditional INCLIC == Y
    * 
    * Normalization Scale Factor for X. This field contains the normalization scale factor 
    * for the x component of the Local coordinate system. Units are meters if field LOCTYP=R 
    * or pixels if LOCTYP=N.
    * 
    * Values:
    * +-9.99999999999999E+99
    */
   char m_nsfx[FLOAT21_SIZE+1];

   /**
    * TAG NSFX ( 21 byte BCS-A). Conditional INCLIC == Y
    * 
    * Normalization Scale Factor for Y. This field contains the normalization scale factor 
    * for the y component of the Local coordinate system. Units are meters if field LOCTYP=R 
    * or pixels if LOCTYP=N.
    * 
    * Values:
    * +-9.99999999999999E+99
    */
   char m_nsfy[FLOAT21_SIZE + 1];

   /**
    * TAG NSFX ( 21 byte BCS-A). Conditional INCLIC == Y
    * 
    * Normalization Scale Factor for Z. This field contains the normalization scale factor 
    * for the Z component of the Local coordinate system. Units are meters.
    * 
    * Values:
    * +-9.99999999999999E+99
    */
   char m_nsfz[FLOAT21_SIZE + 1];

   /**
    * TAG NOFFX ( 21 byte BCS-A). Conditional INCLIC == Y
    * 
    * Normalization Offset for X. This field contains the normalization offset for the x component 
    * of the Local coordinate system. Units are meters if field LOCTYP=R or pixels if LOCTYP=N.
    * 
    * Values:
    * +-9.99999999999999E+99
    */
   char m_noffx[FLOAT21_SIZE + 1];

   /**
    * TAG NOFFY ( 21 byte BCS-A). Conditional INCLIC == Y
    * 
    * Normalization Offset for Y. This field contains the normalization offset for the x component 
    * of the Local coordinate system. Units are meters if field LOCTYP=R or pixels if LOCTYP=N.
    * 
    * Values:
    * +-9.99999999999999E+99
    */
   char m_noffy[FLOAT21_SIZE + 1];

   /**
    * TAG NOFFZ ( 21 byte BCS-A). Conditional INCLIC == Y
    * 
    * Normalization Offset for Z. This field contains the normalization offset for the Z component 
    * of the Local coordinate system. Units are meters.
    * 
    * Values:
    * +-9.99999999999999E+99
    */
   char m_noffz[FLOAT21_SIZE + 1];

   /**
    * TAG XUOL (21 byte BCS-A). Conditional LOCTYPE==R and INCLIC == Y
    * Units: meters
    * 
    * Local Coordinate Origin (XUOL). This field provides the WGS 84 X coordinate of the origin 
    * of the Local (rectangular) coordinate system. This coordinate system is part of the 
    * RSM adjustable parameters definition for the image.
    * 
    * Values:
    * +-9.99999999999999E+99
    */
   char m_xuol[FLOAT21_SIZE + 1];

   /**
    * TAG YUOL (21 byte BCS-A). Conditional LOCTYPE==R and INCLIC == Y
    * Units: meters
    * 
    * Local Coordinate Origin (YUOL). This field provides the WGS 84 Y coordinate of the origin 
    * of the Local (rectangular) coordinate system. This coordinate system is part of the 
    * RSM adjustable parameters definition for the image.
    * 
    * Values:
    * +-9.99999999999999E+99
    */
   char m_yuol[FLOAT21_SIZE + 1];

   /**
    * TAG ZUOL (21 byte BCS-A). Conditional LOCTYPE==R and INCLIC == Y
    * Units: meters
    * 
    * Local Coordinate Origin (ZUOL). This field provides the WGS 84 Z coordinate of the origin 
    * of the Local (rectangular) coordinate system. This coordinate system is part of the 
    * RSM adjustable parameters definition for the image.
    * 
    * Values:
    * +-9.99999999999999E+99
    */
   char m_zuol[FLOAT21_SIZE + 1];

   /**
    * TAG XUXL (21 byte BCS-A). Conditional LOCTYPE==R and INCLIC == Y
    * 
    * Local Coordinate Unit Vector (XUXL). This field provides the WGS 84 X component 
    * of the unit vector defining the X-axis of the Local (rectangular) coordinate system. 
    * This coordinate system is part of the RSM adjustable parameters definition for the image.
    * 
    * Values:
    * 
    *   -1.00000000000000E+00 to +1.00000000000000E+00 (-1 to +1)
    *   Value consistent with fields XUXL through ZUZL forming an 
    *   orthogonal matrix
    */
   char m_xuxl[FLOAT21_SIZE + 1];

   /**
    * TAG XUYL (21 byte BCS-A). Conditional LOCTYPE==R and INCLIC == Y
    * 
    * Local Coordinate Unit Vector (XUYL). This field provides the WGS 84 Y component 
    * of the unit vector defining the X-axis of the Local (rectangular) coordinate system. 
    * This coordinate system is part of the RSM adjustable parameters definition for the image.
    * 
    * Values:
    * 
    *   -1.00000000000000E+00 to +1.00000000000000E+00 (-1 to +1)
    *   Value consistent with fields XUXL through ZUZL forming an 
    *   orthogonal matrix
    */
   char m_xuyl[FLOAT21_SIZE + 1];

   /**
    * TAG XUZL (21 byte BCS-A). Conditional LOCTYPE==R and INCLIC == Y
    * 
    * Local Coordinate Unit Vector (XUZL). This field provides the WGS 84 Z component 
    * of the unit vector defining the X-axis of the Local (rectangular) coordinate system. 
    * This coordinate system is part of the RSM adjustable parameters definition for the image.
    * 
    * Values:
    * 
    *   -1.00000000000000E+00 to +1.00000000000000E+00 (-1 to +1)
    *   Value consistent with fields XUXL through ZUZL forming an 
    *   orthogonal matrix
    */
   char m_xuzl[FLOAT21_SIZE + 1];

   /**
    * TAG YUXL (21 byte BCS-A). Conditional LOCTYPE==R and INCLIC == Y
    * 
    * Local Coordinate Unit Vector (YUXL). This field provides the WGS 84 X component 
    * of the unit vector defining the Y-axis of the Local (rectangular) coordinate system. 
    * This coordinate system is part of the RSM adjustable parameters definition for the image.
    * 
    * Values:
    * 
    *   -1.00000000000000E+00 to +1.00000000000000E+00 (-1 to +1)
    *   Value consistent with fields XUXL through ZUZL forming an 
    *   orthogonal matrix
    */
   char m_yuxl[FLOAT21_SIZE + 1];

   /**
    * TAG YUYL (21 byte BCS-A). Conditional LOCTYPE==R and INCLIC == Y
    * 
    * Local Coordinate Unit Vector (YUYL). This field provides the WGS 84 Y component 
    * of the unit vector defining the Y-axis of the Local (rectangular) coordinate system. 
    * This coordinate system is part of the RSM adjustable parameters definition for the image.
    * 
    * Values:
    * 
    *   -1.00000000000000E+00 to +1.00000000000000E+00 (-1 to +1)
    *   Value consistent with fields XUXL through ZUZL forming an 
    *   orthogonal matrix
    */
   char m_yuyl[FLOAT21_SIZE + 1];

   /**
    * TAG YUZL (21 byte BCS-A). Conditional LOCTYPE==R and INCLIC == Y
    * 
    * Local Coordinate Unit Vector (YUZL). This field provides the WGS 84 Z component 
    * of the unit vector defining the Y-axis of the Local (rectangular) coordinate system. 
    * This coordinate system is part of the RSM adjustable parameters definition for the image.
    * 
    * Values:
    * 
    *   -1.00000000000000E+00 to +1.00000000000000E+00 (-1 to +1)
    *   Value consistent with fields XUXL through ZUZL forming an 
    *   orthogonal matrix
    */
   char m_yuzl[FLOAT21_SIZE + 1];

   /**
    * TAG ZUXL (21 byte BCS-A). Conditional LOCTYPE==R and INCLIC == Y
    * 
    * Local Coordinate Unit Vector (ZUXL). This field provides the WGS 84 X component 
    * of the unit vector defining the Z-axis of the Local (rectangular) coordinate system. 
    * This coordinate system is part of the RSM adjustable parameters definition for the image.
    * 
    * Values:
    * 
    *   -1.00000000000000E+00 to +1.00000000000000E+00 (-1 to +1)
    *   Value consistent with fields XUXL through ZUZL forming an 
    *   orthogonal matrix
    */
   char m_zuxl[FLOAT21_SIZE + 1];

   /**
    * TAG ZUYL (21 byte BCS-A). Conditional LOCTYPE==R and INCLIC == Y
    * 
    * Local Coordinate Unit Vector (ZUYL). This field provides the WGS 84 Y component 
    * of the unit vector defining the Z-axis of the Local (rectangular) coordinate system. 
    * This coordinate system is part of the RSM adjustable parameters definition for the image.
    * 
    * Values:
    * 
    *   -1.00000000000000E+00 to +1.00000000000000E+00 (-1 to +1)
    *   Value consistent with fields XUXL through ZUZL forming an 
    *   orthogonal matrix
    */
   char m_zuyl[FLOAT21_SIZE + 1];

   /**
    * TAG ZUZL (21 byte BCS-A). Conditional LOCTYPE==R and INCLIC == Y
    * 
    * Local Coordinate Unit Vector (YUZL). This field provides the WGS 84 Z component 
    * of the unit vector defining the Z-axis of the Local (rectangular) coordinate system. 
    * This coordinate system is part of the RSM adjustable parameters definition for the image.
    * 
    * Values:
    * 
    *   -1.00000000000000E+00 to +1.00000000000000E+00 (-1 to +1)
    *   Value consistent with fields XUXL through ZUZL forming an 
    *   orthogonal matrix
    */
   char m_zuzl[FLOAT21_SIZE + 1];

   /**
    * TAG APBASE (1 byte BCS-A). Conditional INCLIC == Y
    * 
    * Basis Option. This field indicates whether the RSM adjustable parameters "basis"
    * option is on (APBASE=Y).  If this option is off (APBASE=N), the RSM adjustable 
    * parameters specified in the following fields are the active RSM adjustable 
    * parameters. The order (component number) of an active RSM adjustable parameter 
    * is the order in which it is specified.  If this option is on, the RSM adjustable
    * parameters specified in the following fields are the basis set of RSM adjustable
    * parameters. The order (component number) of a basis RSM adjustable parameter 
    * is the order in which it is specified. If this option is on, the active RSM 
    * adjustable parameters are a linear combination of the basis set of RSM adjustable 
    * parameters. The matrix A (fieldxAEL) maps the basis set to the active set of
    * RSM adjustable parameters. In addition, the pseudo-inverse of the matrix A is equal
    * to the matrix A transpose. It maps the active set to the basis set of RSM
    * adjustable parameters. The matrix A is mxn, where m  n and the rank of A equals m. 
    * The number of adjustable parameters (n) in the basis set is specified in field NBASIS. 
    * The number of active adjustable parameters (m) is specified in the field NPAR.
    * The RSM image error covariance is always with respect to the active RSM adjustable
    * parameters. For example, the second active RSM adjustable parameter corresponds to row 2 
    * and column 2 of the image (auto) covariance, and corresponds to row 2 and column k of 
    * x the crosscovariance of the associated image with image k.
    * 
    * Values:
    * 
    *  Y or N
    */
   char m_apbase[APBASE_SIZE + 1];

   /**
    * TAG NISAP (2 bytes BCS-A). Conditional APTYP == I and INCLIC == Y
    * 
    * Number of Image-Space Adjustable Parameters. This field contains the total
    * number of image-space adjustable parameters. If the basis option is off (APBASE=N),
    * specified image-space adjustable parameters are the active RSM adjustable 
    * parameters. The total number of imagespace adjustable parameters is constrained 
    * as follows: (0<NPAR=NISAP=(NISAPR + NISAPC)<37). NISAPR is the number of
    * image-space adjustable parameters that affect the image row-coordinate, and NISAPC 
    * the number that affect the image column-coordinate. If the basis option is 
    * on (APBASE=Y), specified image-space adjustable parameters are the basis RSM 
    * adjustable parameters. The total number of imagespace adjustable parameters making up the
    * basis set is constrained as follows: (0<NBASIS=NISAP=(NISAPR + NISAPC)<100).
    * 
    * Values:
    * 
    * 1-36 (if APBASE=N)
    * 1-99 (if APBASE=Y)
    */
   char m_nisap[NISAP_SIZE + 1];

   /**
    * TAG NISAPR (2 bytes BCS-A). Conditional APTYP == I and INCLIC == Y
    * 
    * Number of Image-Space Adjustable Parameters for Image Row Coordinate. This 
    * field provides the total number of image-space adjustable parameters that adjust 
    * the image row coordinate. The general form for the row coordinate adjustment (Delta r ) 
    * corresponding to an adjustable parameter (ap) is as follows:
    * 
    * <PLEASE See RS_NITF_TRE_v1.0.pdf for formula>
    * 
    * i,j,k are the corresponding powers of normalized Local coordinates x,y,z, respectively. 
    * Each adjustable parameter has units of pixels.
    */
   char m_nisapr[NISAPR_SIZE + 1];

   /**
    * XPWRR (Parameter array) Conditional (APTYP=I)
    * 
    * Row Parameter Power of X. The power (exponent) of x 
    * associated with this imagespace adjustable parameter 
    * for image row adjustment. This power along with the 
    * following two powers (fields) uniquely specify the adjustable parameter.
    * x is a normalized Local rectangular ground coordinate 
    * (LOCTYP=R) or a normalized image row coordinate (LOCTYP=N)
    * 
    * YPWRR (Parameter array) Conditional (APTYP=I)
    * 
    * Row Parameter Power of Y. The power (exponent) of y 
    * associated with this imagespace adjustable parameter for 
    * image row adjustment. y is a normalized Local rectangular 
    * ground coordinate (LOCTYP=R) or a normalized image column 
    * coordinate (LOCTYP=N)
    * 
    * ZPWRR (Parameter array) Conditional (APTYP=I)
    * 
    * Row Parameter Power of Z. The power (exponent) of z 
    * associated with this imagespace adjustable parameter for 
    * image row adjustment. z is a normalized Local rectangular 
    * ground coordinate (LOCTYP=R) or a normalized geodetic height
    * coordinate (LOCTYP=N)
    * 
    * 
    */
   AdjustableParameterArray m_imageSpaceAdjParamRowArray;

   /**
    * TAG NISAPC ( 2 bytes BCS-A). Conditional APTYP == I and INCLIC == Y
    * 
    * Number of Image-Space Adjustable Parameters for Image Column Coordinate. This field provides 
    * the total number of image-space adjustable parameters that adjust the image column coordinate.
    * The general form for the column coordinate adjustment (deltac ) corresponding to an adjustable 
    * parameter (ap) is as follows
    * 
    * <PLEASE See RS_NITF_TRE_v1.0.pdf for formula>
    * 
    * the corresponding powers of normalized Local coordinates x,y,z, respectively. Each adjustable 
    * parameter has units of pixels.
    */
   char m_nisapc[NISAPC_SIZE + 1];

   /**
    * XPWRC Array Conditional (APTYP=I)
    * 
    * Column Parameter Power of X. The power (exponent) of x 
    * associated with this imagespace adjustable parameter for 
    * image column adjustment. This power along with the following
    * two powers (fields) uniquely specify the adjustable parameter.
    * x is a normalized Local rectangular ground coordinate (LOCTYP=R) 
    * or a normalized image row coordinate (LOCTYP=N)
    * 
    * 
    */
   AdjustableParameterArray m_imageSpaceAdjParamColArray;

   /**
    * TAG NGSAP ( 2 Byte BCS-A). Conditional  APTYP==G and INCLIC == Y
    * 
    * Number of Ground-Space Adjustable Parameters This field provides the total number 
    * of ground-space adjustable parameters. Each ground-space adjustable parameter is 
    * either associated with a "seven parameter" adjustment or is a first order "rate" 
    * term.  The general form for the seven parameter adjustment is:
    * 
    * <PLEASE See RS_NITF_TRE_v1.0.pdf for formula>
    * 
    * where the vector on the left side of the equation is the ground-space adjustment in
    * Local rectangular ground coordinates(meters), the vector on the far right side of 
    * the equation is the ground point location in normalized Local rectangular ground coordinates.
    * 
    * The seven parameters
    * 
    * dx,dy,dz,dalpha,dbeta, dkappa, ds are termed x-offset, y-offset, z-offset, rotation angle 
    * alpha, rotation angle beta, rotation angle kappa, and scale. For identification purposes 
    * in the field below, these seven parameters are assigned 4 character identifications 
    * "OFFX", "OFFY" ,"OFFZ", "ROTX", "ROTY", "ROTZ", "SCAL", respectively. Each has units of meters.
    * There a total of 9 possible rate terms xx xy zz ap ,ap ,.., ap , termed "XRTX", "XRTY", 
    * "XRTZ", "YRTX", "YRTY", "YRTZ", "ZRTZ", "ZRTY, "ZRTZ, respectively. Their effect is 
    * illustrated as follows for the adjustable parameter "XRTY" (xy ap ) and corresponding adjustment 
    *
    *  <PLEASE See RS_NITF_TRE_v1.0.pdf for formula>
    * 
    * If the basis option is off (APBASE=N), specified ground-space adjustable parameters 
    * are the active RSM adjustable parameters. If the basis option is on (APBASE=Y), specified 
    * ground-space adjustable parameters are the basis RSM adjustable parameters. The total 
    * number of ground-space adjustable parameters (NGSAP) is constrained to be between 1 and 16
    * regardless the value of APBASE, i.e., regardless if the basis option is on or off. If 
    * the basis option if off, NPAR=NGSAP. If the basis option if on, NBASE=NGSAP
    */
   char m_ngsap[NGSAP_SIZE + 1];

   /**
    * GSAPID (4 Bytes BCS-A). Conditional APTYP==G and INCLIC == Y
    * 
    * Ground-space Adjustable Parameter ID. This field identifies a ground-space adjustable parameter.
    */
   StringArray m_gsapidArray;

       /**
    * NBASIS (2 Bytes). Conditional APBASE==Y and INCLIC == Y
    * 
    * Number of Basis Adjustable Parameters. This field contains the number of RSM adjustable parameters 
    * in the basis set. It is equal to the number of columns in the matrix A. NBASIS=NISAP or NGSAP, 
    * depending on whether the previously identified active adjustable parameters were image-space or 
    * ground-space active adjustable parameters. The number of columns must be no less than the number 
    * of rows in the matrix A, i.e., NBASIS >= NPAR. The size of the matrix A is also constrained
    * such that NPAR*NBASIS<=1296.
    * 
    * Values 1-99
    */
       char m_nbasis[NBASIS_SIZE + 1];

   /**
    * AEL ( 21 bytes each BCS-A). Conditional APBASE==Y and INCLIC == Y
    * Matrix A Element. This field contains an element of the matrix A. 
    * The elements are stored in row major order.
    * 
    * NPAR*NBASIS entries
    */
   DoubleArray m_aelArray;


   IGNEntryArray m_ignEntryArray;

       /**
    * MAP (List of 21 Bytes BCS-A). Conditional INCLIC == Y
    * 
    * Mapping Matrix Element. This field contains the value of the next 
    * mapping matrix element, stored in row major order. The mapping matrix 
    * is used to map the associated image’s original error covariance to 
    * RSM error covariance. The mapping matrix has NPAR rows and NPARO columns.
    * 
    * Values:
    * 
    *   +9.99999999999999E+99
    */
       StringArray m_map;

   /**
    * URR (21 Byte BCS-A). Conditional INCLUC == Y
    * Units: Pixels^2
    * 
    * Unmodeled Row Variance. This field provides the variance of unmodeled 
    * error represented as an image row error.
    * 
    * Values:
    * 
    *    +0.00000000000000E+00 to
    *    +9.99999999999999E+99 Non-negative value
    */
   char m_urr[FLOAT21_SIZE+1];

   /**
    * URC (21 Byte BCS-A). Conditional INCLUC == Y
    * Units: Pixels^2
    * 
    * Unmodeled Row-Column Covariance. This field provides the covariance between 
    * the unmodeled error represented as an image row error and unmodeled 
    * error represented as an image column error.
    * 
    * Values:
    * 
    *  +9.99999999999999E+99 
    * 
    * Collectively, URR, URC, and UCC values must correspond to a positive 
    * semi-definite (2x2) error covariance matrix
    */
   char m_urc[FLOAT21_SIZE + 1];

   /**
    * UCC (21 Byte BCS-A). Conditional INCLUC == Y
    * Units: Pixels^2
    * 
    * Unmodeled Column Variance. This field provides the variance of unmodeled 
    * error represented as an image column error.
    * 
    * Values:
    * 
    *     +0.00000000000000E+00 to +9.99999999999999E+99 Non-negative value
    */
   char m_ucc[FLOAT21_SIZE+1];

   /**
    * UACSMC (1 Byte BCS-A). Conditional INCLUC == Y
    * 
    * Unmodeled CSM Correlation Option. This field indicates whether a CSM 
    * correlation functional form is to be used (UACSMC=Y). instead of 
    * the baseline piece-wise linear correlation functional form (UACSMC=N) 
    * in order to represent the correlation of unmodeled error.
    * 
    * Values:
    *    Y or N
    */
   char m_uacsmc[UACSMC_SIZE+1];

   /**
    * UNCSR (1 Byte BCS-N). Conditional INCLUC == Y and UACSMC == N
    * 
    * Number of Correlation Segments for independent variable ROW distance. 
    * This field contains the number of piece-wise linear correlation 
    * segments that make up the correlation function for unmodeled error 
    * with independent variable image row distance.
    * 
    * Values:
    * 
    *    2 through 9
    */
   char m_uncsr[UNCSR_SIZE+1];

   /**
    * This will include two tag VALUES in and array:
    * 
    * UCORSR (21 Byte BCS-A) Conditional INCLUC == Y and UACSMC == N
    * UTAUSR (21 Byte BCS-A) Conditional INCLUC == Y and UACSMC == N
    * 
    * UCORSR:
    * Segment Correlation Value. This field contains the correlation 
    * value applicable at the beginning of the segment. Note that the 
    * value is defined as one for the first segment (correlation segment=1), 
    * and defined as zero for the last segment (correlation segment=UNCSR). 
    * It is a nonnegative number for all segments, decreasing in value from 
    * one segment to the next.
    * 
    * Values:
    * +0.00000000000000E+00 to +1.00000000000000E+00 (0 to +1)
    * Value consistent with a non-negative, convex, piece-wise linear 
    * correlation function defined by UNCSR entries of field UCORSR and 
    * field UTAUSR
    * 
    * UTAUSR:
    * Segment Tau Value. This field contains the correlation row distance (tau) 
    * applicable at the beginning of the segment. Note that the value is defined 
    * as zero for the first segment (correlation segment=1). It is a positive 
    * number for all other segments, increasing in value from one segment to the 
    * next. Note that the values of the fields UCORSR and UTAUSR for all the 
    * segments are further constrained such that the corresponding piece-wise 
    * linear correlation function is convex (non-negative and increasing slope 
    * from one segment to the next). Also, the last segment is defined equal to 
    * zero for all tau greater than the last segment’s UTAUSR value.
    * 
    * Values:
    * 
    * +0.00000000000000E+00 to +9.99999999999999E+99 Non-negative value. 
    * Value consistent with a non-negative, convex, piece-wise linear 
    * correlation function defined by UNCSR entries of field UCORSR and
    * field UTAUSR
    */
   std::vector<CorrelationSegment> m_uncsrEntries;

   /**
    * UNCSR (1 Byte BCS-N). Conditional INCLUC == Y and UACSMC == N
    * 
    * Number of Correlation Segments for independent variable Column distance. 
    * This field contains the number of piece-wise linear correlation 
    * segments that make up the correlation function for unmodeled error 
    * with independent variable image col distance.
    * 
    * Values:
    * 
    *    2 through 9
    */
   char m_uncsc[UNCSC_SIZE+1];

   /**
    * This will include two tag VALUES in and array:
    * 
    * UCORSC (21 Byte BCS-A) Conditional INCLUC == Y and UACSMC == N
    * UTAUSC (21 Byte BCS-A) Conditional INCLUC == Y and UACSMC == N
    * 
    * UCORSC:
    * Segment Correlation Value. This field contains the correlation 
    * value applicable at the beginning of the segment. Note that the 
    * value is defined as one for the first segment (correlation segment=1), 
    * and defined as zero for the last segment (correlation segment=UNCSC). 
    * It is a nonnegative number for all segments, decreasing in value from 
    * one segment to the next.
    * 
    * Values:
    * +0.00000000000000E+00 to +1.00000000000000E+00 (0 to +1)
    * Value consistent with a non-negative, convex, piece-wise linear 
    * correlation function defined by UNCSC entries of field UCORSC and 
    * field UTAUSC
    * 
    * UTAUSC:
    * Segment Tau Value. This field contains the correlation row distance (tau) 
    * applicable at the beginning of the segment. Note that the value is defined 
    * as zero for the first segment (correlation segment=1). It is a positive 
    * number for all other segments, increasing in value from one segment to the 
    * next. Note that the values of the fields UCORSC and UTAUSC for all the 
    * segments are further constrained such that the corresponding piece-wise 
    * linear correlation function is convex (non-negative and increasing slope 
    * from one segment to the next). Also, the last segment is defined equal to 
    * zero for all tau greater than the last segment’s UTAUSC value.
    * 
    * Values:
    * 
    * +0.00000000000000E+00 to +9.99999999999999E+99 Non-negative value. 
    * Value consistent with a non-negative, convex, piece-wise linear 
    * correlation function defined by UNCSC entries of field UCORSC and
    * field UTAUSC
    */
   std::vector<CorrelationSegment> m_uncscEntries;

   /**
    * UACR (21 Byte BCS-A). Conditional UACSMC == Y and INCLUC = Y
    * 
    * This field provides the CSM correlation function A parameter 
    * for unmodeled error with independent variable row distance.
    * 
    * Values:
    *    +0.00000000000001E-99 to +1.00000000000000E+00 
    *    (greater than 0 and less than or equal to +1)
    */
   char m_uacr[FLOAT21_SIZE+1];

   /**
    * UALPCR (21 Byte BCS-A). Conditional UACSMC == Y and INCLUC = Y
    * 
    * This field provides the CSM correlation function alpha parameter 
    * for unmodeled error with independent variable row distance. 
    * 
    * Values:
    *    +0.00000000000000E+00 to +9.99999999999999E-01
    *    (greater than or equal to 0 and less than +1)
    */
   char m_ualpcr[FLOAT21_SIZE+1];

   /**
    * UBETCR (21 Byte BCS-A). Conditional UACSMC == Y and INCLUC = Y
    * 
    * This field provides the CSM correlation function beta parameter 
    * for unmodeled error with independent variable row distance.
    * 
    * Values:
    *    +0.00000000000000E+00 to +1.000000000000000E+01
    *    (0 to +10)
    */
   char m_ubetcr[FLOAT21_SIZE+1];

   /**
    * UTCR (21 Byte BCS-A). Conditional UACSMC == Y and INCLUC = Y
    * Units: Secons
    * This field provides the CSM correlation function T parameter 
    * for unmodeled error with independent variable row distance.
    * 
    * Values:
    *    +0.00000000000001E-99 to +9.99999999999999E+99
    *    Positive value
    */
   char m_utcr[FLOAT21_SIZE + 1];

   /**
    * UACC (21 Byte BCS-A). Conditional UACSMC == Y and INCLUC = Y
    * 
    * This field provides the CSM correlation function A parameter 
    * for unmodeled error with independent variable column distance.
    * 
    * Values:
    *    +0.00000000000001E-99 to +1.00000000000000E+00
    *    (greater than 0 and less than or equal to +1
    */
   char m_uacc[FLOAT21_SIZE + 1];

   /**
    * UALPCC (21 Byte BCS-A). Conditional UACSMC == Y and INCLUC = Y
    * 
    * This field provides the CSM correlation function alpha 
    * parameter for unmodeled error with independent variable 
    * column distance.
    * 
    * +0.00000000000000E+00 to +1.00000000000000E+00
    * (greater than or equal to 0 and less than +1)
    */
   char m_ualpcc[FLOAT21_SIZE + 1];

   /**
    * UBETCC (21 Byte BCS-A). Conditional UACSMC == Y and INCLUC = Y
    * 
    * This field provides the CSM correlation function beta parameter 
    * for unmodeled error with independent variable column distance.
    * 
    * Values:
    *    +0.00000000000000E+00 to +1.000000000000000E+01] 
    *    (0 to +10)
    */
   char m_ubetcc[FLOAT21_SIZE + 1];

   /**
    * UTCC (21 Byte BCS-A). Conditional UACSMC == Y and INCLUC = Y
    * Units: seconds
    * 
    * This field provides the CSM correlation function T parameter 
    * for unmodeled error with independent variable column distance
    * 
    * Values:
    *    +0.00000000000001E-99 to +9.99999999999999E+99
    *    Positive value
    */
   char m_utcc[FLOAT21_SIZE + 1];

   TYPE_DATA

}; // End: class ossimNitfRsmecbTag

#endif /* #ifndef ossimNitfRsmecaTag_HEADER */
