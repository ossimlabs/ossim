//---
// File: ossimNitfCsexrbTag.h
//---
#ifndef ossimNitfCsexrbTag_HEADER
#define ossimNitfCsexrbTag_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredTag.h>

/**
 * @class ossimNitfCsexrbTag
 * RSM Identification: Id's, time-of-image model, optional illumination model,
 * footprint information.
 */
class OSSIM_DLL ossimNitfCsexrbTag : public ossimNitfRegisteredTag
{
protected:
    int CETAG_SIZE = 6
    int CEL_SIZE = 5
    int IMAGE_UUID_SIZE = 36
    int NUM_ASSOC_DES_SIZE = 3
    int NUM_ASSOC_DES_SIZE = 3
    int ASSOC_DES_UUIDi_SIZE = 36
    int PLATFORM_ID_SIZE = 6
    int PAYLOAD_ID_SIZE = 6
    int SENSOR_ID_SIZE = 6
    int SENSOR_TYPE_SIZE = 1
    int GROUND_REF_POINT_X_SIZE = 12
    int GROUND_REF_POINT_Y_SIZE = 12
    int GROUND_REF_POINT_Z_SIZE = 12
    int DAY_FIRST_LINE_IMAGE_SIZE = 8
    int TIME_FIRST_LINE_IMAGE_SIZE = 15
    int TIME_IMAGE_DURATION_SIZE = 16
    int TIME_STAMP_LOC_SIZE = 1
    int REFRENCE_FRAME_NUM_SIZE = 9
    int BASE_TIMESTAMP_SIZE = 24
    int DT_MULTIPLIER_SIZE = 8
    int DT_SIZE_SIZE = 1
    int UMBER_FRAMES_SIZE = 4
    int NUMBER_DT_SIZE = 4
    int DTn_SIZE = 0
    int RESERVED_LEN_AREA1_SIZE = 5
    int NUM_IMG_OPS_SIZE = 2
    int TGT_ID_LEN_SIZE = 2
    int TGT_ID_SIZE = 0
    int TGT_NAME_LEN_SIZE = 2
    int TGT_NAME_SIZE = 0
    int TGT_TYPE_LEN_SIZE = 2
    int TGT_TYPE_SIZE = 0
    int TGT_LAT_SIZE = 9
    int TGT_LON_SIZE = 10
    int TGT_HT_SIZE = 8
    int TGT_DATE_TIME_SIZE = 14
    int TGT_AZ_SIZE = 7
    int TGT_ELEV_ANG_SIZE = 7
    int TGT_BIDEC_ANG_SIZE = 7
    int COLL_REQ_ID_LEN_SIZE = 3
    int COLL_REQ_ID_SIZE = 0
    int COLLECT_STRAT_LEN_SIZE = 2
    int COLLECT_STRAT_SIZE = 0
    int COLLECT_TYPE_LEN_SIZE = 2
    int COLLECT_TYPE_SIZE = 0
    int COLL_CODE_LEN_SIZE = 2
    int COLL_CODE_SIZE = 0
    int NUM_COLLECT_CRITERIA_SIZE = 2
    int COLLECT_CRITERIA_NAME_LEN_SIZE = 2
    int COLLECT_CRITERIA_NAME_SIZE = 0
    int COLLECT_CRITERIA_UNIT_LEN_SIZE = 2
    int COLLECT_CRITERIA_UNIT_SIZE = 0
    int COLLECT_CRITERIA_VALUE_LEN_SIZE = 2
    int COLLECT_CRITERIA_VALUE_SIZE = 0
    int NUM_IMG_OPS_DATA_SIZE = 2
    int CM_ID_LEN_SIZE = 2
    int CM_ID_SIZE = 0
    int SENSOR_CONFIG_LEN_SIZE = 2
    int SENSOR_CONFIG_SIZE = 0
    int IMG_OP_ID_LEN_SIZE = 2
    int IMG_OP_ID_SIZE = 0
    int NUM_EXP_SIZE = 2
    int INDEX_SIZE_SIZE = 1
    int NUM_INDICES_SIZE = 2
    int INDEX_IN_IMG_OP_ID_SIZE = 0
    int NUM_QUALITY_METRICS_SIZE = 2
    int QUALITY_METRIC_NAME_LEN_SIZE = 2
    int QUALITY_METRIC_NAME_SIZE = 0
    int QUALITY_METRIC_UNIT_LEN_SIZE = 2
    int QUALITY_METRIC_UNIT_SIZE = 0
    int QUALITY_METRIC_TYPE_SIZE = 1
    int QUALITY_METRIC_VALUE_LEN_SIZE = 2
    int QUALITY_METRIC_VALUE_SIZE = 0
    int RESERVED_LEN_AREA_SIZE = 5
    int RESERVED_AREA_SIZE = 0

    char cetag[CETAG_SIZE + 1]
    char cel[CEL_SIZE + 1]
    char imageUuid[IMAGE_UUID_SIZE + 1]
    char numAssocDes[NUM_ASSOC_DES_SIZE + 1]
    char numAssocDes[NUM_ASSOC_DES_SIZE + 1]
    char assocDesUuidi[ASSOC_DES_UUIDi_SIZE + 1]
    char platformId[PLATFORM_ID_SIZE + 1]
    char payloadId[PAYLOAD_ID_SIZE + 1]
    char sensorId[SENSOR_ID_SIZE + 1]
    char sensorType[SENSOR_TYPE_SIZE + 1]
    char groundRefPointX[GROUND_REF_POINT_X_SIZE + 1]
    char groundRefPointY[GROUND_REF_POINT_Y_SIZE + 1]
    char groundRefPointZ[GROUND_REF_POINT_Z_SIZE + 1]
    char dayFirstLineImage[DAY_FIRST_LINE_IMAGE_SIZE + 1]
    char timeFirstLineImage[TIME_FIRST_LINE_IMAGE_SIZE + 1]
    char timeImageDuration[TIME_IMAGE_DURATION_SIZE + 1]
    char timeStampLoc[TIME_STAMP_LOC_SIZE + 1]
    char refrenceFrameNum[REFRENCE_FRAME_NUM_SIZE + 1]
    char baseTimestamp[BASE_TIMESTAMP_SIZE + 1]
    char dtMultiplier[DT_MULTIPLIER_SIZE + 1]
    char dtSize[DT_SIZE_SIZE + 1]
    char umberFrames[UMBER_FRAMES_SIZE + 1]
    char numberDt[NUMBER_DT_SIZE + 1]
    char dtn[DTn_SIZE + 1]
    char reservedLenArea1[RESERVED_LEN_AREA1_SIZE + 1]
    char numImgOps[NUM_IMG_OPS_SIZE + 1]
    char tgtIdLen[TGT_ID_LEN_SIZE + 1]
    char tgtId[TGT_ID_SIZE + 1]
    char tgtNameLen[TGT_NAME_LEN_SIZE + 1]
    char tgtName[TGT_NAME_SIZE + 1]
    char tgtTypeLen[TGT_TYPE_LEN_SIZE + 1]
    char tgtType[TGT_TYPE_SIZE + 1]
    char tgtLat[TGT_LAT_SIZE + 1]
    char tgtLon[TGT_LON_SIZE + 1]
    char tgtHt[TGT_HT_SIZE + 1]
    char tgtDateTime[TGT_DATE_TIME_SIZE + 1]
    char tgtAz[TGT_AZ_SIZE + 1]
    char tgtElevAng[TGT_ELEV_ANG_SIZE + 1]
    char tgtBidecAng[TGT_BIDEC_ANG_SIZE + 1]
    char collReqIdLen[COLL_REQ_ID_LEN_SIZE + 1]
    char collReqId[COLL_REQ_ID_SIZE + 1]
    char collectStratLen[COLLECT_STRAT_LEN_SIZE + 1]
    char collectStrat[COLLECT_STRAT_SIZE + 1]
    char collectTypeLen[COLLECT_TYPE_LEN_SIZE + 1]
    char collectType[COLLECT_TYPE_SIZE + 1]
    char collCodeLen[COLL_CODE_LEN_SIZE + 1]
    char collCode[COLL_CODE_SIZE + 1]
    char numCollectCriteria[NUM_COLLECT_CRITERIA_SIZE + 1]
    char collectCriteriaNameLen[COLLECT_CRITERIA_NAME_LEN_SIZE + 1]
    char collectCriteriaName[COLLECT_CRITERIA_NAME_SIZE + 1]
    char collectCriteriaUnitLen[COLLECT_CRITERIA_UNIT_LEN_SIZE + 1]
    char collectCriteriaUnit[COLLECT_CRITERIA_UNIT_SIZE + 1]
    char collectCriteriaValueLen[COLLECT_CRITERIA_VALUE_LEN_SIZE + 1]
    char collectCriteriaValue[COLLECT_CRITERIA_VALUE_SIZE + 1]
    char numImgOpsData[NUM_IMG_OPS_DATA_SIZE + 1]
    char cmIdLen[CM_ID_LEN_SIZE + 1]
    char cmId[CM_ID_SIZE + 1]
    char sensorConfigLen[SENSOR_CONFIG_LEN_SIZE + 1]
    char sensorConfig[SENSOR_CONFIG_SIZE + 1]
    char imgOpIdLen[IMG_OP_ID_LEN_SIZE + 1]
    char imgOpId[IMG_OP_ID_SIZE + 1]
    char numExp[NUM_EXP_SIZE + 1]
    char indexSize[INDEX_SIZE_SIZE + 1]
    char numIndices[NUM_INDICES_SIZE + 1]
    char indexInImgOpId[INDEX_IN_IMG_OP_ID_SIZE + 1]
    char numQualityMetrics[NUM_QUALITY_METRICS_SIZE + 1]
    char qualityMetricNameLen[QUALITY_METRIC_NAME_LEN_SIZE + 1]
    char qualityMetricName[QUALITY_METRIC_NAME_SIZE + 1]
    char qualityMetricUnitLen[QUALITY_METRIC_UNIT_LEN_SIZE + 1]
    char qualityMetricUnit[QUALITY_METRIC_UNIT_SIZE + 1]
    char qualityMetricType[QUALITY_METRIC_TYPE_SIZE + 1]
    char qualityMetricValueLen[QUALITY_METRIC_VALUE_LEN_SIZE + 1]
    char qualityMetricValue[QUALITY_METRIC_VALUE_SIZE + 1]
    char reservedLenArea[RESERVED_LEN_AREA_SIZE + 1]
    char reservedArea[RESERVED_AREA_SIZE + 1]

public:
    ossimNitfCsexraTag();

    virtual ~ossimNitfCsexraTag();

    virtual void parseStream(std::istream& in);

    virtual void writeStream(std::ostream& out);

    virtual void clearFields();

    virtual std::ostream& print(std::ostream& out,
                                const std::string& prefix) const;

    ossimString ossimCsexrbTag::getCetag() const;
    ossimString ossimCsexrbTag::getCel() const;
    ossimString ossimCsexrbTag::getImageUuid() const;
    ossimString ossimCsexrbTag::getNumAssocDes() const;
    ossimString ossimCsexrbTag::getNumAssocDes() const;
    ossimString ossimCsexrbTag::getAssocDesUuidi() const;
    ossimString ossimCsexrbTag::getPlatformId() const;
    ossimString ossimCsexrbTag::getPayloadId() const;
    ossimString ossimCsexrbTag::getSensorId() const;
    ossimString ossimCsexrbTag::getSensorType() const;
    ossimString ossimCsexrbTag::getGroundRefPointX() const;
    ossimString ossimCsexrbTag::getGroundRefPointY() const;
    ossimString ossimCsexrbTag::getGroundRefPointZ() const;
    ossimString ossimCsexrbTag::getDayFirstLineImage() const;
    ossimString ossimCsexrbTag::getTimeFirstLineImage() const;
    ossimString ossimCsexrbTag::getTimeImageDuration() const;
    ossimString ossimCsexrbTag::getTimeStampLoc() const;
    ossimString ossimCsexrbTag::getRefrenceFrameNum() const;
    ossimString ossimCsexrbTag::getBaseTimestamp() const;
    ossimString ossimCsexrbTag::getDtMultiplier() const;
    ossimString ossimCsexrbTag::getDtSize() const;
    ossimString ossimCsexrbTag::getUmberFrames() const;
    ossimString ossimCsexrbTag::getNumberDt() const;
    ossimString ossimCsexrbTag::getDtn() const;
    ossimString ossimCsexrbTag::getReservedLenArea1() const;
    ossimString ossimCsexrbTag::getNumImgOps() const;
    ossimString ossimCsexrbTag::getTgtIdLen() const;
    ossimString ossimCsexrbTag::getTgtId() const;
    ossimString ossimCsexrbTag::getTgtNameLen() const;
    ossimString ossimCsexrbTag::getTgtName() const;
    ossimString ossimCsexrbTag::getTgtTypeLen() const;
    ossimString ossimCsexrbTag::getTgtType() const;
    ossimString ossimCsexrbTag::getTgtLat() const;
    ossimString ossimCsexrbTag::getTgtLon() const;
    ossimString ossimCsexrbTag::getTgtHt() const;
    ossimString ossimCsexrbTag::getTgtDateTime() const;
    ossimString ossimCsexrbTag::getTgtAz() const;
    ossimString ossimCsexrbTag::getTgtElevAng() const;
    ossimString ossimCsexrbTag::getTgtBidecAng() const;
    ossimString ossimCsexrbTag::getCollReqIdLen() const;
    ossimString ossimCsexrbTag::getCollReqId() const;
    ossimString ossimCsexrbTag::getCollectStratLen() const;
    ossimString ossimCsexrbTag::getCollectStrat() const;
    ossimString ossimCsexrbTag::getCollectTypeLen() const;
    ossimString ossimCsexrbTag::getCollectType() const;
    ossimString ossimCsexrbTag::getCollCodeLen() const;
    ossimString ossimCsexrbTag::getCollCode() const;
    ossimString ossimCsexrbTag::getNumCollectCriteria() const;
    ossimString ossimCsexrbTag::getCollectCriteriaNameLen() const;
    ossimString ossimCsexrbTag::getCollectCriteriaName() const;
    ossimString ossimCsexrbTag::getCollectCriteriaUnitLen() const;
    ossimString ossimCsexrbTag::getCollectCriteriaUnit() const;
    ossimString ossimCsexrbTag::getCollectCriteriaValueLen() const;
    ossimString ossimCsexrbTag::getCollectCriteriaValue() const;
    ossimString ossimCsexrbTag::getNumImgOpsData() const;
    ossimString ossimCsexrbTag::getCmIdLen() const;
    ossimString ossimCsexrbTag::getCmId() const;
    ossimString ossimCsexrbTag::getSensorConfigLen() const;
    ossimString ossimCsexrbTag::getSensorConfig() const;
    ossimString ossimCsexrbTag::getImgOpIdLen() const;
    ossimString ossimCsexrbTag::getImgOpId() const;
    ossimString ossimCsexrbTag::getNumExp() const;
    ossimString ossimCsexrbTag::getIndexSize() const;
    ossimString ossimCsexrbTag::getNumIndices() const;
    ossimString ossimCsexrbTag::getIndexInImgOpId() const;
    ossimString ossimCsexrbTag::getNumQualityMetrics() const;
    ossimString ossimCsexrbTag::getQualityMetricNameLen() const;
    ossimString ossimCsexrbTag::getQualityMetricName() const;
    ossimString ossimCsexrbTag::getQualityMetricUnitLen() const;
    ossimString ossimCsexrbTag::getQualityMetricUnit() const;
    ossimString ossimCsexrbTag::getQualityMetricType() const;
    ossimString ossimCsexrbTag::getQualityMetricValueLen() const;
    ossimString ossimCsexrbTag::getQualityMetricValue() const;
    ossimString ossimCsexrbTag::getReservedLenArea() const;
    ossimString ossimCsexrbTag::getReservedArea() const;
}
