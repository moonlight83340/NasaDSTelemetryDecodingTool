﻿#include "DSHKTelemetryDecoder.h"
#include "Utils.h"

using namespace std;

DSHKTelemetryDecoder::DSHKTelemetryDecoder(const string& filePathp) {
    filePath = filePathp;
}

std::vector<DS_HkPacket_t> DSHKTelemetryDecoder::getPackets() {
    return packets;
}

uint16_t DSHKTelemetryDecoder::ConvertToDecimal(uint8_t bytes[2]) {
    return (static_cast<uint16_t>(bytes[0]) << 8) | bytes[1];
}

//Get Telemetry secondary Header Time in J2000 time format
TelemetrySecondaryHeaderTime DSHKTelemetryDecoder::getTelemetrySecondaryHeaderTime(CFE_MSG_TelemetrySecondaryHeader_t secondaryHeader) {
    TelemetrySecondaryHeaderTime time{};
    time.seconds = 0;
    for (int i = 0; i < 4; ++i) {
        time.seconds |= static_cast<uint32_t>(secondaryHeader.Time[i]) << ((3 - i) * 8);
    }
    // Extraction des sous-secondes
    time.subSeconds = (secondaryHeader.Time[4] << 8) | secondaryHeader.Time[5];
    return time;
}

// Get UTC Datestamp fron Telemetry secondary Header Time in J2000 time format
std::tm DSHKTelemetryDecoder::getPacketDatestampInUtcTimeFromJ2000Time(uint32_t seconds, uint16_t subSeconds) {
    // J2000.0 in UTC
    std::tm j2000_time = {};
    j2000_time.tm_year = 100;  // 2000 (years - 1900)
    j2000_time.tm_mon = 0;     // january (month 0-based)
    j2000_time.tm_mday = 1;    // first day of the month
    j2000_time.tm_hour = 11;
    j2000_time.tm_min = 58;
    j2000_time.tm_sec = 55;

    // add ms to J2000.0 date
    std::chrono::time_point<std::chrono::system_clock> j2000_time_point = std::chrono::system_clock::from_time_t(std::mktime(&j2000_time));
    j2000_time_point += std::chrono::milliseconds(816);

    //Calcul the date with secondes and subseconds
    auto newTimePoint = j2000_time_point + std::chrono::seconds(seconds) + std::chrono::milliseconds(subSeconds);

    // Afficher la nouvelle date au format UTC
    std::time_t time = std::chrono::system_clock::to_time_t(newTimePoint);
    std::tm timeInfo;
#ifdef _WIN32  //  Windows
    gmtime_s(&timeInfo, &time);
#else  //  Linux
    gmtime_r(&time, &timeInfo);
#endif
    std::cout << "Date/Time: " << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") << "."
        << std::chrono::duration_cast<std::chrono::milliseconds>(newTimePoint.time_since_epoch() % std::chrono::seconds(1)).count() << " UTC" << std::endl;
    return timeInfo;
}

void DSHKTelemetryDecoder::showPacket(DS_HkPacket_t packet) {

    // Show the CCSDS_PrimaryHeader_t structure data
    std::cout << "StreamId: " << std::dec << ConvertToDecimal(packet.TelemetryHeader.Msg.CCSDS.Pri.StreamId) << std::endl;
    std::cout << "Sequence: " << std::dec << ConvertToDecimal(packet.TelemetryHeader.Msg.CCSDS.Pri.Sequence) << std::endl;
    std::cout << "Length: " << std::dec << ConvertToDecimal(packet.TelemetryHeader.Msg.CCSDS.Pri.Length) << std::endl;

    uint16_t streamID = ConvertToDecimal(packet.TelemetryHeader.Msg.CCSDS.Pri.StreamId);
    uint16_t sequence = ConvertToDecimal(packet.TelemetryHeader.Msg.CCSDS.Pri.Sequence);

    uint16_t applicationID = streamID & 0x07FF;
    uint8_t secondaryHeaderPresent = (streamID & 0x0800) >> 11;
    uint8_t packetType = (streamID & 0x1000) >> 12;
    uint8_t ccsdsVersion = (streamID & 0xE000) >> 13;

    uint16_t sequenceCount = sequence & 0x3FFF;
    uint8_t segmentationFlags = (sequence & 0xC000) >> 14;


    std::cout << "applicationID: " << static_cast<int>(applicationID) << std::endl;
    std::cout << "secondaryHeaderPresent: " << static_cast<int>(secondaryHeaderPresent) << std::endl;
    std::cout << "packetType: " << static_cast<int>(packetType) << std::endl;
    std::cout << "ccsdsVersion: " << static_cast<int>(ccsdsVersion) << std::endl;

    std::cout << "SequenceCount: " << static_cast<int>(sequenceCount) << std::endl;
    std::cout << "SegmentationFlags: " << static_cast<int>(segmentationFlags) << std::endl;


    //Secondary header
    if ((packet.TelemetryHeader.Msg.CCSDS.Pri.StreamId[1] & 0x20) != 0) {
        TelemetrySecondaryHeaderTime timeStruct = getTelemetrySecondaryHeaderTime(packet.TelemetryHeader.Sec);

        std::cout << "Seconds: " << timeStruct.seconds << std::endl;
        std::cout << "Subseconds: " << timeStruct.subSeconds << std::endl;
        std::tm timeInfo = getPacketDatestampInUtcTimeFromJ2000Time(timeStruct.seconds, timeStruct.subSeconds);
    }

    //Console Show Payload
    std::cout << "CmdAcceptedCounter: " << static_cast<int>(packet.Payload.CmdAcceptedCounter) << std::endl;
    std::cout << "CmdRejectedCounter: " << static_cast<int>(packet.Payload.CmdRejectedCounter) << std::endl;
    std::cout << "DestTblLoadCounter: " << static_cast<int>(packet.Payload.DestTblLoadCounter) << std::endl;
    std::cout << "DestTblErrCounter: " << static_cast<int>(packet.Payload.DestTblErrCounter) << std::endl;
    std::cout << "FilterTblLoadCounter: " << static_cast<int>(packet.Payload.FilterTblLoadCounter) << std::endl;
    std::cout << "FilterTblErrCounter: " << static_cast<int>(packet.Payload.FilterTblErrCounter) << std::endl;
    std::cout << "AppEnableState: " << static_cast<int>(packet.Payload.AppEnableState) << std::endl;
    std::cout << "FileWriteCounter: " << packet.Payload.FileWriteCounter << std::endl;
    std::cout << "FileWriteErrCounter: " << packet.Payload.FileWriteErrCounter << std::endl;
    std::cout << "FileUpdateCounter: " << packet.Payload.FileUpdateCounter << std::endl;
    std::cout << "FileUpdateErrCounter: " << packet.Payload.FileUpdateErrCounter << std::endl;
    std::cout << "DisabledPktCounter: " << packet.Payload.DisabledPktCounter << std::endl;
    std::cout << "IgnoredPktCounter: " << packet.Payload.IgnoredPktCounter << std::endl;
    std::cout << "FilteredPktCounter: " << packet.Payload.FilteredPktCounter << std::endl;
    std::cout << "PassedPktCounter: " << packet.Payload.PassedPktCounter << std::endl;
    std::cout << "FilterTblFilename: " << packet.Payload.FilterTblFilename << std::endl;
}

void DSHKTelemetryDecoder::showPackets() {
    for (DS_HkPacket_t packet : packets) {
        showPacket(packet);
    }
}

int DSHKTelemetryDecoder::processBinaryFile() {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Can't open File" << std::endl;
        return 1;
    }
    // Lire les données du fichier dans un vecteur de bytes
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(file), {});
    unsigned int fileCursor = 0;
    while (fileCursor < buffer.size()) {
        DS_HkPacket_t currentPacket;
        // Check if buffer can contain the CCSDS_PrimaryHeader_t structure
        if ((buffer.size() - fileCursor) < sizeof(CCSDS_PrimaryHeader_t)) {
            std::cerr << "File to small to contain the CCSDS_PrimaryHeader_t." << std::endl;
            return 1;
        }
        // Copy buffer data to struct CCSDS_PrimaryHeader_t
        CCSDS_SpacePacket_t spacePacket;
        std::memcpy(&spacePacket.Pri, buffer.data(), sizeof(CCSDS_PrimaryHeader_t));
        uint16_t msgLength = ConvertToDecimal(spacePacket.Pri.Length);

        uint16_t applicationID = (spacePacket.Pri.StreamId[0] << 3) | (spacePacket.Pri.StreamId[1] >> 5);
        bool secondaryHeaderPresent = (spacePacket.Pri.StreamId[1] & 0x20) != 0;
        uint8_t packetType = (spacePacket.Pri.StreamId[1] & 0x10) != 0;
        uint8_t ccsdsVersion = (spacePacket.Pri.StreamId[1] & 0x0C) >> 2;
        //update file cursor
        fileCursor += sizeof(CCSDS_PrimaryHeader_t);
        //Check if buffer size match with length of the packet
        if (buffer.size() < msgLength) {
            std::cerr << "Buffer size isn't sufficient according to the Length in the CCSDS_PrimaryHeader_t" << std::endl;
            return 1;
        }
        //assign primary header readed
        currentPacket.TelemetryHeader.Msg.CCSDS = spacePacket;
        CFE_MSG_TelemetrySecondaryHeader_t secondaryHeader;
        if (secondaryHeaderPresent
            && (buffer.size() >= (fileCursor + sizeof(CFE_MSG_TelemetrySecondaryHeader_t)))) {
            std::memcpy(&secondaryHeader, buffer.data() + fileCursor, sizeof(CFE_MSG_TelemetrySecondaryHeader_t));
            //uint8 Time[6]; /**< \brief Time, big endian: 4 byte seconds, 2 byte subseconds */
            fileCursor += sizeof(CFE_MSG_TelemetrySecondaryHeader_t);
            //Assign secondary header packet readed
            currentPacket.TelemetryHeader.Sec = secondaryHeader;
        }
        DS_HkTlm_Payload_t payload{0};
        if (buffer.size() >= (fileCursor + sizeof(DS_HkTlm_Payload_t))) {
            std::memcpy(&payload, buffer.data() + fileCursor, sizeof(DS_HkTlm_Payload_t));
            fileCursor += sizeof(DS_HkTlm_Payload_t);
            if (isLittleEndian()) {
                payload.FileWriteCounter = littleToBigEndian16(payload.FileWriteCounter);
                payload.FileWriteErrCounter = littleToBigEndian16(payload.FileWriteErrCounter);
                payload.FileUpdateCounter = littleToBigEndian16(payload.FileUpdateCounter);
                payload.FileUpdateErrCounter = littleToBigEndian16(payload.FileUpdateErrCounter);
                payload.DisabledPktCounter = littleToBigEndian32(payload.DisabledPktCounter);
                payload.IgnoredPktCounter = littleToBigEndian32(payload.IgnoredPktCounter);
                payload.FilteredPktCounter = littleToBigEndian32(payload.FilteredPktCounter);
                payload.PassedPktCounter = littleToBigEndian32(payload.PassedPktCounter);
            }
            //Assign payload readed
            currentPacket.Payload = payload;
        }
        packets.push_back(currentPacket);
    }
    file.close();
    return 0;
}