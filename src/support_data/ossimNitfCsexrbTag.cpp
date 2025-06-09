//----------------------------------------------------------------------------
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: CSEXRA tag class definition.
//
// Exploitation Reference Data TRE.
//
// See document STDI-0006-NCDRD Table 3.5-16 for more info.
//
//----------------------------------------------------------------------------
// $Id

#include <cstring>
#include <istream>
#include <iostream>
#include <iomanip>

#include <ossim/support_data/ossimNitfCsexrbTag.h>

using namespace std;

ossimNitfCsexraTag::ossimNitfCsexraTag()
   : ossimNitfRegisteredTag(std::string("CSEXRB"), 353)
{
   clearFields();
}

ossimNitfCsexraTag::~ossimNitfCsexraTag()
{
}

void ossimNitfRsmidaTag::parseStream(std::istream& in)
{
   clearFields();

   in.read(cetag, CETAG_SIZE);
   in.read(cel, CEL_SIZE);
   in.read(imageUuid, IMAGE_UUID_SIZE);
   in.read(numAssocDes, NUM_ASSOC_DES_SIZE);
   in.read(numAssocDes, NUM_ASSOC_DES_SIZE);
   in.read(assocDesUuidi, ASSOC_DES_UUIDi_SIZE);
   in.read(platformId, PLATFORM_ID_SIZE);
   in.read(payloadId, PAYLOAD_ID_SIZE);
   in.read(sensorId, SENSOR_ID_SIZE);
   in.read(sensorType, SENSOR_TYPE_SIZE);
   in.read(groundRefPointX, GROUND_REF_POINT_X_SIZE);
   in.read(groundRefPointY, GROUND_REF_POINT_Y_SIZE);
   in.read(groundRefPointZ, GROUND_REF_POINT_Z_SIZE);
   in.read(dayFirstLineImage, DAY_FIRST_LINE_IMAGE_SIZE);
   in.read(timeFirstLineImage, TIME_FIRST_LINE_IMAGE_SIZE);
   in.read(timeImageDuration, TIME_IMAGE_DURATION_SIZE);
   in.read(timeStampLoc, TIME_STAMP_LOC_SIZE);
   in.read(refrenceFrameNum, REFRENCE_FRAME_NUM_SIZE);
   in.read(baseTimestamp, BASE_TIMESTAMP_SIZE);
   in.read(dtMultiplier, DT_MULTIPLIER_SIZE);
   in.read(dtSize, DT_SIZE_SIZE);
   in.read(umberFrames, UMBER_FRAMES_SIZE);
   in.read(numberDt, NUMBER_DT_SIZE);
   in.read(dtn, DTn_SIZE);
   in.read(reservedLenArea1, RESERVED_LEN_AREA1_SIZE);
   in.read(numImgOps, NUM_IMG_OPS_SIZE);
   in.read(tgtIdLen, TGT_ID_LEN_SIZE);
   in.read(tgtId, TGT_ID_SIZE);
   in.read(tgtNameLen, TGT_NAME_LEN_SIZE);
   in.read(tgtName, TGT_NAME_SIZE);
   in.read(tgtTypeLen, TGT_TYPE_LEN_SIZE);
   in.read(tgtType, TGT_TYPE_SIZE);
   in.read(tgtLat, TGT_LAT_SIZE);
   in.read(tgtLon, TGT_LON_SIZE);
   in.read(tgtHt, TGT_HT_SIZE);
   in.read(tgtDateTime, TGT_DATE_TIME_SIZE);
   in.read(tgtAz, TGT_AZ_SIZE);
   in.read(tgtElevAng, TGT_ELEV_ANG_SIZE);
   in.read(tgtBidecAng, TGT_BIDEC_ANG_SIZE);
   in.read(collReqIdLen, COLL_REQ_ID_LEN_SIZE);
   in.read(collReqId, COLL_REQ_ID_SIZE);
   in.read(collectStratLen, COLLECT_STRAT_LEN_SIZE);
   in.read(collectStrat, COLLECT_STRAT_SIZE);
   in.read(collectTypeLen, COLLECT_TYPE_LEN_SIZE);
   in.read(collectType, COLLECT_TYPE_SIZE);
   in.read(collCodeLen, COLL_CODE_LEN_SIZE);
   in.read(collCode, COLL_CODE_SIZE);
   in.read(numCollectCriteria, NUM_COLLECT_CRITERIA_SIZE);
   in.read(collectCriteriaNameLen, COLLECT_CRITERIA_NAME_LEN_SIZE);
   in.read(collectCriteriaNamen, COLLECT_CRITERIA_NAMEn_SIZE);
   in.read(collectCriteriaUnitLenn, COLLECT_CRITERIA_UNIT_LENn_SIZE);
   in.read(collectCriteriaUnitn, COLLECT_CRITERIA_UNITn_SIZE);
   in.read(collectCriteriaValueLenn, COLLECT_CRITERIA_VALUE_LENn_SIZE);
   in.read(collectCriteriaValuen, COLLECT_CRITERIA_VALUEn_SIZE);
   in.read(numImgOpsData, NUM_IMG_OPS_DATA_SIZE);
   in.read(cmIdLenn, CM_ID_LENn_SIZE);
   in.read(cmIdn, CM_IDn_SIZE);
   in.read(sensorConfigLenn, SENSOR_CONFIG_LENn_SIZE);
   in.read(sensorConfign, SENSOR_CONFIGn_SIZE);
   in.read(imgOpIdLenn, IMG_OP_ID_LENn_SIZE);
   in.read(imgOpIdn, IMG_OP_IDn_SIZE);
   in.read(numExpn, NUM_EXPn_SIZE);
   in.read(indexSizen, INDEX_SIZEn_SIZE);
   in.read(numIndicesn, NUM_INDICESn_SIZE);
   in.read(indexInImgOpIdnm, INDEX_IN_IMG_OP_IDnm_SIZE);
   in.read(numQualityMetricsn, NUM_QUALITY_METRICSn_SIZE);
   in.read(qualityMetricNameLennm, QUALITY_METRIC_NAME_LENnm_SIZE);
   in.read(qualityMetricNamenm, QUALITY_METRIC_NAMEnm_SIZE);
   in.read(qualityMetricUnitLennm, QUALITY_METRIC_UNIT_LENnm_SIZE);
   in.read(qualityMetricUnitnm, QUALITY_METRIC_UNITnm_SIZE);
   in.read(qualityMetricTypenm, QUALITY_METRIC_TYPEnm_SIZE);
   in.read(qualityMetricValueLennm, QUALITY_METRIC_VALUE_LENnm_SIZE);
   in.read(qualityMetricValuenm, QUALITY_METRIC_VALUEnm_SIZE);
   in.read(reservedLenAreai, RESERVED_LEN_AREAi_SIZE);
   in.read(reservedAreai, RESERVED_AREAi_SIZE);
}

void ossimNitfCsexrbTag::writeStream(std::ostream& out)
{
   out.write(cetag, CETAG_SIZE);
   out.write(cel, CEL_SIZE);
   out.write(imageUuid, IMAGE_UUID_SIZE);
   out.write(numAssocDes, NUM_ASSOC_DES_SIZE);
   out.write(numAssocDes, NUM_ASSOC_DES_SIZE);
   out.write(assocDesUuidi, ASSOC_DES_UUIDi_SIZE);
   out.write(platformId, PLATFORM_ID_SIZE);
   out.write(payloadId, PAYLOAD_ID_SIZE);
   out.write(sensorId, SENSOR_ID_SIZE);
   out.write(sensorType, SENSOR_TYPE_SIZE);
   out.write(groundRefPointX, GROUND_REF_POINT_X_SIZE);
   out.write(groundRefPointY, GROUND_REF_POINT_Y_SIZE);
   out.write(groundRefPointZ, GROUND_REF_POINT_Z_SIZE);
   out.write(dayFirstLineImage, DAY_FIRST_LINE_IMAGE_SIZE);
   out.write(timeFirstLineImage, TIME_FIRST_LINE_IMAGE_SIZE);
   out.write(timeImageDuration, TIME_IMAGE_DURATION_SIZE);
   out.write(timeStampLoc, TIME_STAMP_LOC_SIZE);
   out.write(refrenceFrameNum, REFRENCE_FRAME_NUM_SIZE);
   out.write(baseTimestamp, BASE_TIMESTAMP_SIZE);
   out.write(dtMultiplier, DT_MULTIPLIER_SIZE);
   out.write(dtSize, DT_SIZE_SIZE);
   out.write(umberFrames, UMBER_FRAMES_SIZE);
   out.write(numberDt, NUMBER_DT_SIZE);
   out.write(dtn, DTn_SIZE);
   out.write(reservedLenArea1, RESERVED_LEN_AREA1_SIZE);
   out.write(numImgOps, NUM_IMG_OPS_SIZE);
   out.write(tgtIdLen, TGT_ID_LEN_SIZE);
   out.write(tgtId, TGT_ID_SIZE);
   out.write(tgtNameLen, TGT_NAME_LEN_SIZE);
   out.write(tgtName, TGT_NAME_SIZE);
   out.write(tgtTypeLen, TGT_TYPE_LEN_SIZE);
   out.write(tgtType, TGT_TYPE_SIZE);
   out.write(tgtLat, TGT_LAT_SIZE);
   out.write(tgtLon, TGT_LON_SIZE);
   out.write(tgtHt, TGT_HT_SIZE);
   out.write(tgtDateTime, TGT_DATE_TIME_SIZE);
   out.write(tgtAz, TGT_AZ_SIZE);
   out.write(tgtElevAng, TGT_ELEV_ANG_SIZE);
   out.write(tgtBidecAng, TGT_BIDEC_ANG_SIZE);
   out.write(collReqIdLen, COLL_REQ_ID_LEN_SIZE);
   out.write(collReqId, COLL_REQ_ID_SIZE);
   out.write(collectStratLen, COLLECT_STRAT_LEN_SIZE);
   out.write(collectStrat, COLLECT_STRAT_SIZE);
   out.write(collectTypeLen, COLLECT_TYPE_LEN_SIZE);
   out.write(collectType, COLLECT_TYPE_SIZE);
   out.write(collCodeLen, COLL_CODE_LEN_SIZE);
   out.write(collCode, COLL_CODE_SIZE);
   out.write(numCollectCriteria, NUM_COLLECT_CRITERIA_SIZE);
   out.write(collectCriteriaNameLen, COLLECT_CRITERIA_NAME_LEN_SIZE);
   out.write(collectCriteriaName, COLLECT_CRITERIA_NAME_SIZE);
   out.write(collectCriteriaUnitLen, COLLECT_CRITERIA_UNIT_LEN_SIZE);
   out.write(collectCriteriaUnit, COLLECT_CRITERIA_UNIT_SIZE);
   out.write(collectCriteriaValueLen, COLLECT_CRITERIA_VALUE_LEN_SIZE);
   out.write(collectCriteriaValue, COLLECT_CRITERIA_VALUE_SIZE);
   out.write(numImgOpsData, NUM_IMG_OPS_DATA_SIZE);
   out.write(cmIdLen, CM_ID_LEN_SIZE);
   out.write(cmId, CM_ID_SIZE);
   out.write(sensorConfigLen, SENSOR_CONFIG_LEN_SIZE);
   out.write(sensorConfig, SENSOR_CONFIG_SIZE);
   out.write(imgOpIdLen, IMG_OP_ID_LEN_SIZE);
   out.write(imgOpId, IMG_OP_ID_SIZE);
   out.write(numExp, NUM_EXP_SIZE);
   out.write(indexSize, INDEX_SIZE_SIZE);
   out.write(numIndices, NUM_INDICES_SIZE);
   out.write(indexInImgOpId, INDEX_IN_IMG_OP_ID_SIZE);
   out.write(numQualityMetrics, NUM_QUALITY_METRICS_SIZE);
   out.write(qualityMetricNameLen, QUALITY_METRIC_NAME_LEN_SIZE);
   out.write(qualityMetricName, QUALITY_METRIC_NAME_SIZE);
   out.write(qualityMetricUnitLen, QUALITY_METRIC_UNIT_LEN_SIZE);
   out.write(qualityMetricUnit, QUALITY_METRIC_UNIT_SIZE);
   out.write(qualityMetricType, QUALITY_METRIC_TYPE_SIZE);
   out.write(qualityMetricValueLen, QUALITY_METRIC_VALUE_LEN_SIZE);
   out.write(qualityMetricValue, QUALITY_METRIC_VALUE_SIZE);
   out.write(reservedLenArea, RESERVED_LEN_AREA_SIZE);
   out.write(reservedArea, RESERVED_AREA_SIZE);
}

std::ostream& ossimNitfCsexrbTag::print(std::ostream& out, const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";

   out << setiosflags(std::ios::left)
       << pfx << std::setw(24) << "CETAG:" << getTagName() << "\n"
       << pfx << std::setw(24) << "CEL:" << getTagLength() << "\n"
       << pfx << std::setw(24) << "CETAG:" << "\n"
       << pfx << std::setw(24) << "CEL:" << "\n"
       << pfx << std::setw(24) << "IMAGE_UUID:" << "\n"
       << pfx << std::setw(24) << "NUM_ASSOC_DES:" << "\n"
       << pfx << std::setw(24) << "NUM_ASSOC_DES:" << "\n"
       << pfx << std::setw(24) << "ASSOC_DES_UUIDi:" << "\n"
       << pfx << std::setw(24) << "PLATFORM_ID:" << "\n"
       << pfx << std::setw(24) << "PAYLOAD_ID:" << "\n"
       << pfx << std::setw(24) << "SENSOR_ID:" << "\n"
       << pfx << std::setw(24) << "SENSOR_TYPE:" << "\n"
       << pfx << std::setw(24) << "GROUND_REF_POINT_X:" << "\n"
       << pfx << std::setw(24) << "GROUND_REF_POINT_Y:" << "\n"
       << pfx << std::setw(24) << "GROUND_REF_POINT_Z:" << "\n"
       << pfx << std::setw(24) << "DAY_FIRST_LINE_IMAGE:" << "\n"
       << pfx << std::setw(24) << "TIME_FIRST_LINE_IMAGE:" << "\n"
       << pfx << std::setw(24) << "TIME_IMAGE_DURATION:" << "\n"
       << pfx << std::setw(24) << "TIME_STAMP_LOC:" << "\n"
       << pfx << std::setw(24) << "REFRENCE_FRAME_NUM:" << "\n"
       << pfx << std::setw(24) << "BASE_TIMESTAMP:" << "\n"
       << pfx << std::setw(24) << "DT_MULTIPLIER:" << "\n"
       << pfx << std::setw(24) << "DT_SIZE:" << "\n"
       << pfx << std::setw(24) << "UMBER_FRAMES:" << "\n"
       << pfx << std::setw(24) << "NUMBER_DT:" << "\n"
       << pfx << std::setw(24) << "DTn:" << "\n"
       << pfx << std::setw(24) << "RESERVED_LEN_AREA1:" << "\n"
       << pfx << std::setw(24) << "NUM_IMG_OPS:" << "\n"
       << pfx << std::setw(24) << "TGT_ID_LEN:" << "\n"
       << pfx << std::setw(24) << "TGT_ID:" << "\n"
       << pfx << std::setw(24) << "TGT_NAME_LEN:" << "\n"
       << pfx << std::setw(24) << "TGT_NAME:" << "\n"
       << pfx << std::setw(24) << "TGT_TYPE_LEN:" << "\n"
       << pfx << std::setw(24) << "TGT_TYPE:" << "\n"
       << pfx << std::setw(24) << "TGT_LAT:" << "\n"
       << pfx << std::setw(24) << "TGT_LON:" << "\n"
       << pfx << std::setw(24) << "TGT_HT:" << "\n"
       << pfx << std::setw(24) << "TGT_DATE_TIME:" << "\n"
       << pfx << std::setw(24) << "TGT_AZ:" << "\n"
       << pfx << std::setw(24) << "TGT_ELEV_ANG:" << "\n"
       << pfx << std::setw(24) << "TGT_BIDEC_ANG:" << "\n"
       << pfx << std::setw(24) << "COLL_REQ_ID_LEN:" << "\n"
       << pfx << std::setw(24) << "COLL_REQ_ID:" << "\n"
       << pfx << std::setw(24) << "COLLECT_STRAT_LEN:" << "\n"
       << pfx << std::setw(24) << "COLLECT_STRAT:" << "\n"
       << pfx << std::setw(24) << "COLLECT_TYPE_LEN:" << "\n"
       << pfx << std::setw(24) << "COLLECT_TYPE:" << "\n"
       << pfx << std::setw(24) << "COLL_CODE_LEN:" << "\n"
       << pfx << std::setw(24) << "COLL_CODE:" << "\n"
       << pfx << std::setw(24) << "NUM_COLLECT_CRITERIA:" << "\n"
       << pfx << std::setw(24) << "COLLECT_CRITERIA_NAME_LEN:" << "\n"
       << pfx << std::setw(24) << "COLLECT_CRITERIA_NAME:" << "\n"
       << pfx << std::setw(24) << "COLLECT_CRITERIA_UNIT_LEN:" << "\n"
       << pfx << std::setw(24) << "COLLECT_CRITERIA_UNIT:" << "\n"
       << pfx << std::setw(24) << "COLLECT_CRITERIA_VALUE_LEN:" << "\n"
       << pfx << std::setw(24) << "COLLECT_CRITERIA_VALUE:" << "\n"
       << pfx << std::setw(24) << "NUM_IMG_OPS_DATA:" << "\n"
       << pfx << std::setw(24) << "CM_ID_LEN:" << "\n"
       << pfx << std::setw(24) << "CM_ID:" << "\n"
       << pfx << std::setw(24) << "SENSOR_CONFIG_LEN:" << "\n"
       << pfx << std::setw(24) << "SENSOR_CONFIG:" << "\n"
       << pfx << std::setw(24) << "IMG_OP_ID_LEN:" << "\n"
       << pfx << std::setw(24) << "IMG_OP_ID:" << "\n"
       << pfx << std::setw(24) << "NUM_EXP:" << "\n"
       << pfx << std::setw(24) << "INDEX_SIZE:" << "\n"
       << pfx << std::setw(24) << "NUM_INDICES:" << "\n"
       << pfx << std::setw(24) << "INDEX_IN_IMG_OP_ID:" << "\n"
       << pfx << std::setw(24) << "NUM_QUALITY_METRICS:" << "\n"
       << pfx << std::setw(24) << "QUALITY_METRIC_NAME_LEN:" << "\n"
       << pfx << std::setw(24) << "QUALITY_METRIC_NAME:" << "\n"
       << pfx << std::setw(24) << "QUALITY_METRIC_UNIT_LEN:" << "\n"
       << pfx << std::setw(24) << "QUALITY_METRIC_UNIT:" << "\n"
       << pfx << std::setw(24) << "QUALITY_METRIC_TYPE:" << "\n"
       << pfx << std::setw(24) << "QUALITY_METRIC_VALUE_LEN:" << "\n"
       << pfx << std::setw(24) << "QUALITY_METRIC_VALUE:" << "\n"
       << pfx << std::setw(24) << "RESERVED_LEN_AREA:" << "\n"
       << pfx << std::setw(24) << "RESERVED_AREA:" << "\n"
}

void ossimNitfCsexrbTag::clearFields()
{
   memset(cetag, ' ', CETAG_SIZE);
   memset(cel, ' ', CEL_SIZE);
   memset(imageUuid, ' ', IMAGE_UUID_SIZE);
   memset(numAssocDes, ' ', NUM_ASSOC_DES_SIZE);
   memset(numAssocDes, ' ', NUM_ASSOC_DES_SIZE);
   memset(assocDesUuidi, ' ', ASSOC_DES_UUIDi_SIZE);
   memset(platformId, ' ', PLATFORM_ID_SIZE);
   memset(payloadId, ' ', PAYLOAD_ID_SIZE);
   memset(sensorId, ' ', SENSOR_ID_SIZE);
   memset(sensorType, ' ', SENSOR_TYPE_SIZE);
   memset(groundRefPointX, ' ', GROUND_REF_POINT_X_SIZE);
   memset(groundRefPointY, ' ', GROUND_REF_POINT_Y_SIZE);
   memset(groundRefPointZ, ' ', GROUND_REF_POINT_Z_SIZE);
   memset(dayFirstLineImage, ' ', DAY_FIRST_LINE_IMAGE_SIZE);
   memset(timeFirstLineImage, ' ', TIME_FIRST_LINE_IMAGE_SIZE);
   memset(timeImageDuration, ' ', TIME_IMAGE_DURATION_SIZE);
   memset(timeStampLoc, ' ', TIME_STAMP_LOC_SIZE);
   memset(refrenceFrameNum, ' ', REFRENCE_FRAME_NUM_SIZE);
   memset(baseTimestamp, ' ', BASE_TIMESTAMP_SIZE);
   memset(dtMultiplier, ' ', DT_MULTIPLIER_SIZE);
   memset(dtSize, ' ', DT_SIZE_SIZE);
   memset(umberFrames, ' ', UMBER_FRAMES_SIZE);
   memset(numberDt, ' ', NUMBER_DT_SIZE);
   memset(dtn, ' ', DTn_SIZE);
   memset(reservedLenArea1, ' ', RESERVED_LEN_AREA1_SIZE);
   memset(numImgOps, ' ', NUM_IMG_OPS_SIZE);
   memset(tgtIdLen, ' ', TGT_ID_LEN_SIZE);
   memset(tgtId, ' ', TGT_ID_SIZE);
   memset(tgtNameLen, ' ', TGT_NAME_LEN_SIZE);
   memset(tgtName, ' ', TGT_NAME_SIZE);
   memset(tgtTypeLen, ' ', TGT_TYPE_LEN_SIZE);
   memset(tgtType, ' ', TGT_TYPE_SIZE);
   memset(tgtLat, ' ', TGT_LAT_SIZE);
   memset(tgtLon, ' ', TGT_LON_SIZE);
   memset(tgtHt, ' ', TGT_HT_SIZE);
   memset(tgtDateTime, ' ', TGT_DATE_TIME_SIZE);
   memset(tgtAz, ' ', TGT_AZ_SIZE);
   memset(tgtElevAng, ' ', TGT_ELEV_ANG_SIZE);
   memset(tgtBidecAng, ' ', TGT_BIDEC_ANG_SIZE);
   memset(collReqIdLen, ' ', COLL_REQ_ID_LEN_SIZE);
   memset(collReqId, ' ', COLL_REQ_ID_SIZE);
   memset(collectStratLen, ' ', COLLECT_STRAT_LEN_SIZE);
   memset(collectStrat, ' ', COLLECT_STRAT_SIZE);
   memset(collectTypeLen, ' ', COLLECT_TYPE_LEN_SIZE);
   memset(collectType, ' ', COLLECT_TYPE_SIZE);
   memset(collCodeLen, ' ', COLL_CODE_LEN_SIZE);
   memset(collCode, ' ', COLL_CODE_SIZE);
   memset(numCollectCriteria, ' ', NUM_COLLECT_CRITERIA_SIZE);
   memset(collectCriteriaNameLen, ' ', COLLECT_CRITERIA_NAME_LEN_SIZE);
   memset(collectCriteriaName, ' ', COLLECT_CRITERIA_NAME_SIZE);
   memset(collectCriteriaUnitLen, ' ', COLLECT_CRITERIA_UNIT_LEN_SIZE);
   memset(collectCriteriaUnit, ' ', COLLECT_CRITERIA_UNIT_SIZE);
   memset(collectCriteriaValueLen, ' ', COLLECT_CRITERIA_VALUE_LEN_SIZE);
   memset(collectCriteriaValue, ' ', COLLECT_CRITERIA_VALUE_SIZE);
   memset(numImgOpsData, ' ', NUM_IMG_OPS_DATA_SIZE);
   memset(cmIdLen, ' ', CM_ID_LEN_SIZE);
   memset(cmId, ' ', CM_ID_SIZE);
   memset(sensorConfigLen, ' ', SENSOR_CONFIG_LEN_SIZE);
   memset(sensorConfig, ' ', SENSOR_CONFIG_SIZE);
   memset(imgOpIdLen, ' ', IMG_OP_ID_LEN_SIZE);
   memset(imgOpId, ' ', IMG_OP_ID_SIZE);
   memset(numExp, ' ', NUM_EXP_SIZE);
   memset(indexSize, ' ', INDEX_SIZE_SIZE);
   memset(numIndices, ' ', NUM_INDICES_SIZE);
   memset(indexInImgOpId, ' ', INDEX_IN_IMG_OP_ID_SIZE);
   memset(numQualityMetrics, ' ', NUM_QUALITY_METRICS_SIZE);
   memset(qualityMetricNameLen, ' ', QUALITY_METRIC_NAME_LEN_SIZE);
   memset(qualityMetricName, ' ', QUALITY_METRIC_NAME_SIZE);
   memset(qualityMetricUnitLen, ' ', QUALITY_METRIC_UNIT_LEN_SIZE);
   memset(qualityMetricUnit, ' ', QUALITY_METRIC_UNIT_SIZE);
   memset(qualityMetricType, ' ', QUALITY_METRIC_TYPE_SIZE);
   memset(qualityMetricValueLen, ' ', QUALITY_METRIC_VALUE_LEN_SIZE);
   memset(qualityMetricValue, ' ', QUALITY_METRIC_VALUE_SIZE);
   memset(reservedLenArea, ' ', RESERVED_LEN_AREA_SIZE);
   memset(reservedArea, ' ', RESERVED_AREA_SIZE);

   cetag[const] = '\0';
   cel[const] = '\0';
   imageUuid[const] = '\0';
   numAssocDes[const] = '\0';
   numAssocDes[const] = '\0';
   assocDesUuidi[const] = '\0';
   platformId[const] = '\0';
   payloadId[const] = '\0';
   sensorId[const] = '\0';
   sensorType[const] = '\0';
   groundRefPointX[const] = '\0';
   groundRefPointY[const] = '\0';
   groundRefPointZ[const] = '\0';
   dayFirstLineImage[const] = '\0';
   timeFirstLineImage[const] = '\0';
   timeImageDuration[const] = '\0';
   timeStampLoc[const] = '\0';
   refrenceFrameNum[const] = '\0';
   baseTimestamp[const] = '\0';
   dtMultiplier[const] = '\0';
   dtSize[const] = '\0';
   umberFrames[const] = '\0';
   numberDt[const] = '\0';
   dtn[const] = '\0';
   reservedLenArea1[const] = '\0';
   numImgOps[const] = '\0';
   tgtIdLen[const] = '\0';
   tgtId[const] = '\0';
   tgtNameLen[const] = '\0';
   tgtName[const] = '\0';
   tgtTypeLen[const] = '\0';
   tgtType[const] = '\0';
   tgtLat[const] = '\0';
   tgtLon[const] = '\0';
   tgtHt[const] = '\0';
   tgtDateTime[const] = '\0';
   tgtAz[const] = '\0';
   tgtElevAng[const] = '\0';
   tgtBidecAng[const] = '\0';
   collReqIdLen[const] = '\0';
   collReqId[const] = '\0';
   collectStratLen[const] = '\0';
   collectStrat[const] = '\0';
   collectTypeLen[const] = '\0';
   collectType[const] = '\0';
   collCodeLen[const] = '\0';
   collCode[const] = '\0';
   numCollectCriteria[const] = '\0';
   collectCriteriaNameLen[const] = '\0';
   collectCriteriaName[const] = '\0';
   collectCriteriaUnitLen[const] = '\0';
   collectCriteriaUnit[const] = '\0';
   collectCriteriaValueLen[const] = '\0';
   collectCriteriaValue[const] = '\0';
   numImgOpsData[const] = '\0';
   cmIdLen[const] = '\0';
   cmId[const] = '\0';
   sensorConfigLen[const] = '\0';
   sensorConfig[const] = '\0';
   imgOpIdLen[const] = '\0';
   imgOpId[const] = '\0';
   numExp[const] = '\0';
   indexSize[const] = '\0';
   numIndices[const] = '\0';
   indexInImgOpId[const] = '\0';
   numQualityMetrics[const] = '\0';
   qualityMetricNameLen[const] = '\0';
   qualityMetricName[const] = '\0';
   qualityMetricUnitLen[const] = '\0';
   qualityMetricUnit[const] = '\0';
   qualityMetricType[const] = '\0';
   qualityMetricValueLen[const] = '\0';
   qualityMetricValue[const] = '\0';
   reservedLenArea[const] = '\0';
   reservedArea[const] = '\0';
}

ossimString ossimCsexrbTag::getCetag() const
{
   return ossimString(cetag);
}

ossimString ossimCsexrbTag::getCel() const
{
   return ossimString(cel);
}

ossimString ossimCsexrbTag::getImageUuid() const
{
   return ossimString(imageUuid);
}

ossimString ossimCsexrbTag::getNumAssocDes() const
{
   return ossimString(numAssocDes);
}

ossimString ossimCsexrbTag::getNumAssocDes() const
{
   return ossimString(numAssocDes);
}

ossimString ossimCsexrbTag::getAssocDesUuidi() const
{
   return ossimString(assocDesUuidi);
}

ossimString ossimCsexrbTag::getPlatformId() const
{
   return ossimString(platformId);
}

ossimString ossimCsexrbTag::getPayloadId() const
{
   return ossimString(payloadId);
}

ossimString ossimCsexrbTag::getSensorId() const
{
   return ossimString(sensorId);
}

ossimString ossimCsexrbTag::getSensorType() const
{
   return ossimString(sensorType);
}

ossimString ossimCsexrbTag::getGroundRefPointX() const
{
   return ossimString(groundRefPointX);
}

ossimString ossimCsexrbTag::getGroundRefPointY() const
{
   return ossimString(groundRefPointY);
}

ossimString ossimCsexrbTag::getGroundRefPointZ() const
{
   return ossimString(groundRefPointZ);
}

ossimString ossimCsexrbTag::getDayFirstLineImage() const
{
   return ossimString(dayFirstLineImage);
}

ossimString ossimCsexrbTag::getTimeFirstLineImage() const
{
   return ossimString(timeFirstLineImage);
}

ossimString ossimCsexrbTag::getTimeImageDuration() const
{
   return ossimString(timeImageDuration);
}

ossimString ossimCsexrbTag::getTimeStampLoc() const
{
   return ossimString(timeStampLoc);
}

ossimString ossimCsexrbTag::getRefrenceFrameNum() const
{
   return ossimString(refrenceFrameNum);
}

ossimString ossimCsexrbTag::getBaseTimestamp() const
{
   return ossimString(baseTimestamp);
}

ossimString ossimCsexrbTag::getDtMultiplier() const
{
   return ossimString(dtMultiplier);
}

ossimString ossimCsexrbTag::getDtSize() const
{
   return ossimString(dtSize);
}

ossimString ossimCsexrbTag::getUmberFrames() const
{
   return ossimString(umberFrames);
}

ossimString ossimCsexrbTag::getNumberDt() const
{
   return ossimString(numberDt);
}

ossimString ossimCsexrbTag::getDtn() const
{
   return ossimString(dtn);
}

ossimString ossimCsexrbTag::getReservedLenArea1() const
{
   return ossimString(reservedLenArea1);
}

ossimString ossimCsexrbTag::getNumImgOps() const
{
   return ossimString(numImgOps);
}

ossimString ossimCsexrbTag::getTgtIdLen() const
{
   return ossimString(tgtIdLen);
}

ossimString ossimCsexrbTag::getTgtId() const
{
   return ossimString(tgtId);
}

ossimString ossimCsexrbTag::getTgtNameLen() const
{
   return ossimString(tgtNameLen);
}

ossimString ossimCsexrbTag::getTgtName() const
{
   return ossimString(tgtName);
}

ossimString ossimCsexrbTag::getTgtTypeLen() const
{
   return ossimString(tgtTypeLen);
}

ossimString ossimCsexrbTag::getTgtType() const
{
   return ossimString(tgtType);
}

ossimString ossimCsexrbTag::getTgtLat() const
{
   return ossimString(tgtLat);
}

ossimString ossimCsexrbTag::getTgtLon() const
{
   return ossimString(tgtLon);
}

ossimString ossimCsexrbTag::getTgtHt() const
{
   return ossimString(tgtHt);
}

ossimString ossimCsexrbTag::getTgtDateTime() const
{
   return ossimString(tgtDateTime);
}

ossimString ossimCsexrbTag::getTgtAz() const
{
   return ossimString(tgtAz);
}

ossimString ossimCsexrbTag::getTgtElevAng() const
{
   return ossimString(tgtElevAng);
}

ossimString ossimCsexrbTag::getTgtBidecAng() const
{
   return ossimString(tgtBidecAng);
}

ossimString ossimCsexrbTag::getCollReqIdLen() const
{
   return ossimString(collReqIdLen);
}

ossimString ossimCsexrbTag::getCollReqId() const
{
   return ossimString(collReqId);
}

ossimString ossimCsexrbTag::getCollectStratLen() const
{
   return ossimString(collectStratLen);
}

ossimString ossimCsexrbTag::getCollectStrat() const
{
   return ossimString(collectStrat);
}

ossimString ossimCsexrbTag::getCollectTypeLen() const
{
   return ossimString(collectTypeLen);
}

ossimString ossimCsexrbTag::getCollectType() const
{
   return ossimString(collectType);
}

ossimString ossimCsexrbTag::getCollCodeLen() const
{
   return ossimString(collCodeLen);
}

ossimString ossimCsexrbTag::getCollCode() const
{
   return ossimString(collCode);
}

ossimString ossimCsexrbTag::getNumCollectCriteria() const
{
   return ossimString(numCollectCriteria);
}

ossimString ossimCsexrbTag::getCollectCriteriaNameLen() const
{
   return ossimString(collectCriteriaNameLen);
}

ossimString ossimCsexrbTag::getCollectCriteriaName() const
{
   return ossimString(collectCriteriaName);
}

ossimString ossimCsexrbTag::getCollectCriteriaUnitLen() const
{
   return ossimString(collectCriteriaUnitLen);
}

ossimString ossimCsexrbTag::getCollectCriteriaUnit() const
{
   return ossimString(collectCriteriaUnit);
}

ossimString ossimCsexrbTag::getCollectCriteriaValueLen() const
{
   return ossimString(collectCriteriaValueLen);
}

ossimString ossimCsexrbTag::getCollectCriteriaValue() const
{
   return ossimString(collectCriteriaValue);
}

ossimString ossimCsexrbTag::getNumImgOpsData() const
{
   return ossimString(numImgOpsData);
}

ossimString ossimCsexrbTag::getCmIdLen() const
{
   return ossimString(cmIdLen);
}

ossimString ossimCsexrbTag::getCmId() const
{
   return ossimString(cmId);
}

ossimString ossimCsexrbTag::getSensorConfigLen() const
{
   return ossimString(sensorConfigLen);
}

ossimString ossimCsexrbTag::getSensorConfig() const
{
   return ossimString(sensorConfig);
}

ossimString ossimCsexrbTag::getImgOpIdLen() const
{
   return ossimString(imgOpIdLen);
}

ossimString ossimCsexrbTag::getImgOpId() const
{
   return ossimString(imgOpId);
}

ossimString ossimCsexrbTag::getNumExp() const
{
   return ossimString(numExp);
}

ossimString ossimCsexrbTag::getIndexSize() const
{
   return ossimString(indexSize);
}

ossimString ossimCsexrbTag::getNumIndices() const
{
   return ossimString(numIndices);
}

ossimString ossimCsexrbTag::getIndexInImgOpId() const
{
   return ossimString(indexInImgOpId);
}

ossimString ossimCsexrbTag::getNumQualityMetrics() const
{
   return ossimString(numQualityMetrics);
}

ossimString ossimCsexrbTag::getQualityMetricNameLen() const
{
   return ossimString(qualityMetricNameLen);
}

ossimString ossimCsexrbTag::getQualityMetricName() const
{
   return ossimString(qualityMetricName);
}

ossimString ossimCsexrbTag::getQualityMetricUnitLen() const
{
   return ossimString(qualityMetricUnitLen);
}

ossimString ossimCsexrbTag::getQualityMetricUnit() const
{
   return ossimString(qualityMetricUnit);
}

ossimString ossimCsexrbTag::getQualityMetricType() const
{
   return ossimString(qualityMetricType);
}

ossimString ossimCsexrbTag::getQualityMetricValueLen() const
{
   return ossimString(qualityMetricValueLen);
}

ossimString ossimCsexrbTag::getQualityMetricValue() const
{
   return ossimString(qualityMetricValue);
}

ossimString ossimCsexrbTag::getReservedLenArea() const
{
   return ossimString(reservedLenArea);
}

ossimString ossimCsexrbTag::getReservedArea() const
{
   return ossimString(reservedArea);
}
