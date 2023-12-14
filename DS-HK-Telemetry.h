#pragma once
#include <stdint.h>
/*
** Maximum length for an absolute path name
*/
#define OS_MAX_PATH_LEN     32

/*
 * Abstract Message Base Types
 *
 * The concrete definition of these is provided by "cfe_msg_hdr.h" which is
 * user-selectable depending on the actual desired message definition.  These
 * abstract types are used in the API definition; the API is defined based
 * on these abstract types, independent of the actual message definition.
 */

 /**
  * \brief cFS generic base message
  */
typedef union CFE_MSG_Message CFE_MSG_Message_t;

/**
 * \brief cFS telemetry header
 */
typedef struct CFE_MSG_TelemetryHeader CFE_MSG_TelemetryHeader_t;

typedef struct
{
    uint8_t  CmdAcceptedCounter;                 /**< \brief Count of valid commands received */
    uint8_t  CmdRejectedCounter;                 /**< \brief Count of invalid commands received */
    uint8_t  DestTblLoadCounter;                 /**< \brief Count of destination file table loads */
    uint8_t  DestTblErrCounter;                  /**< \brief Count of failed attempts to get table data pointer */
    uint8_t  FilterTblLoadCounter;               /**< \brief Count of packet filter table loads */
    uint8_t  FilterTblErrCounter;                /**< \brief Count of failed attempts to get table data pointer */
    uint8_t  AppEnableState;                     /**< \brief Application enable/disable state */
    uint8_t  Spare8;                             /**< \brief Structure alignment padding */
    uint16_t FileWriteCounter;                   /**< \brief Count of good destination file writes */
    uint16_t FileWriteErrCounter;                /**< \brief Count of bad destination file writes */
    uint16_t FileUpdateCounter;                  /**< \brief Count of good updates to secondary header */
    uint16_t FileUpdateErrCounter;               /**< \brief Count of bad updates to secondary header */
    uint32_t DisabledPktCounter;                 /**< \brief Count of packets discarded (DS was disabled) */
    uint32_t IgnoredPktCounter;                  /**< \brief Count of packets discarded
                                                *
                                                * Incoming packets will be discarded when:
                                                *  - The File and/or Filter Table has failed to load
                                                *  - A packet (that is not a DS HK or command packet) has been received
                                                *    that is not listed in the Filter Table
                                                */
    uint32_t FilteredPktCounter;                 /**< \brief Count of packets discarded (failed filter test) */
    uint32_t PassedPktCounter;                   /**< \brief Count of packets that passed filter test */
    char   FilterTblFilename[OS_MAX_PATH_LEN]; /**< \brief Name of filter table file */
} DS_HkTlm_Payload_t;

typedef struct CCSDS_PrimaryHeader
{

    uint8_t StreamId[2]; /**< \brief packet identifier word (stream ID) */
    /*  bits  shift   ------------ description ---------------- */
    /* 0x07FF    0  : application ID                            */
    /* 0x0800   11  : secondary header: 0 = absent, 1 = present */
    /* 0x1000   12  : packet type:      0 = TLM, 1 = CMD        */
    /* 0xE000   13  : CCSDS version:    0 = ver 1, 1 = ver 2    */

    uint8_t Sequence[2]; /**< \brief packet sequence word */
    /*  bits  shift   ------------ description ---------------- */
    /* 0x3FFF    0  : sequence count                            */
    /* 0xC000   14  : segmentation flags:  3 = complete packet  */

    uint8_t Length[2]; /**< \brief packet length word */
    /*  bits  shift   ------------ description ---------------- */
    /* 0xFFFF    0  : (total packet length) - 7                 */

} CCSDS_PrimaryHeader_t;

/**
 * \brief cFS telemetry secondary header
 */
typedef struct
{

    uint8_t Time[6]; /**< \brief Time, big endian: 4 byte seconds, 2 byte subseconds */

} CFE_MSG_TelemetrySecondaryHeader_t;

typedef struct
{
    CCSDS_PrimaryHeader_t Pri; /**< \brief CCSDS Primary Header */
} CCSDS_SpacePacket_t;

/**
 * \brief cFS generic base message
 *
 * This provides the definition of CFE_MSG_Message_t
 */
union CFE_MSG_Message
{
    CCSDS_SpacePacket_t CCSDS;                             /**< \brief CCSDS Header (Pri or Pri + Ext) */
    uint8_t               Byte[sizeof(CCSDS_SpacePacket_t)]; /**< \brief Byte level access */
};

struct CFE_MSG_TelemetryHeader
{
    CFE_MSG_Message_t                  Msg;      /**< \brief Base message */
    CFE_MSG_TelemetrySecondaryHeader_t Sec;      /**< \brief Secondary header */
};

/**
 * \brief Application housekeeping packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader; /**< \brief cFE Software Bus telemetry message header */

    DS_HkTlm_Payload_t Payload;
} DS_HkPacket_t;
