#ifndef __PacketHeader_h__
#define __PacketHeader_h__


enum PacketID: uint8_t {
    PACKET_ID_MOTION = 0,
    PACKET_ID_SESSION = 1,
    PACKET_ID_LAP_DATA = 2,
    PACKET_ID_EVENT = 3,
    PACKET_ID_PARTICIPANTS = 4,
    PACKET_ID_CAR_SETUPS = 5,
    PACKET_ID_CAR_TELEMETRY = 6,
    PACKET_ID_CAR_STATUS = 7,
};

static const char * const PacketID_name[] = {
    "MOTION", "SESSION", "LAP_DATA", "EVENT", "PARTICIPANTS", "CAR_SETUPS",
    "CAR_TELEMETRY", "CAR_STATUS",
};

struct PacketHeader
{
    uint16_t    m_packetFormat;       // 2018
    uint8_t     m_packetVersion;      // Version of this packet type, all start from 1
    uint8_t     m_packetId;           // Identifier for the packet type, see below
    uint64_t    m_sessionUID;         // Unique identifier for the session
    float       m_sessionTime;        // Session timestamp
    uint8_t      m_frameIdentifier;    // Identifier for the frame the data was retrieved on
    uint8_t     m_playerCarIndex;     // Index of player's car in the array

};

#endif
