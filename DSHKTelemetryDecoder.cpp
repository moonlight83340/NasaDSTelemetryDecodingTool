#include "DSHKTelemetryDecoder.h"

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
    // Date de départ en UTC avec millisecondes
    std::tm j2000_time = {};
    j2000_time.tm_year = 100;  // 2000 (years - 1900)
    j2000_time.tm_mon = 0;     // january (month 0-based)
    j2000_time.tm_mday = 1;    // first day of the month
    j2000_time.tm_hour = 11;
    j2000_time.tm_min = 58;
    j2000_time.tm_sec = 55;

    // Ajouter les millisecondes à la date de départ
    std::chrono::time_point<std::chrono::system_clock> j2000_time_point = std::chrono::system_clock::from_time_t(std::mktime(&j2000_time));
    j2000_time_point += std::chrono::milliseconds(816);

    // Calculer la nouvelle date en ajoutant les secondes et les sous-secondes
    auto newTimePoint = j2000_time_point + std::chrono::seconds(seconds) + std::chrono::milliseconds(subSeconds);

    // Afficher la nouvelle date au format UTC
    std::time_t time = std::chrono::system_clock::to_time_t(newTimePoint);
    std::tm timeInfo;
#ifdef _WIN32  // Sous Windows
    gmtime_s(&timeInfo, &time);
#else  // Sous Linux
    gmtime_r(&time, &timeInfo);
#endif
    std::cout << "Date/Time: " << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") << "."
        << std::chrono::duration_cast<std::chrono::milliseconds>(newTimePoint.time_since_epoch() % std::chrono::seconds(1)).count() << " UTC" << std::endl;
    return timeInfo;
}

//gerer les endians little and big currently big is okay
int DSHKTelemetryDecoder::processBinaryFile() {
    vector<DS_HkPacket_t> packets;
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

        std::cout << "Buffer size() : " << buffer.size() << std::endl;
        std::cout << "FileCursor : " << fileCursor << std::endl;
        // Vérifier si le buffer est suffisamment grand pour contenir la structure CCSDS_PrimaryHeader_t
        if ((buffer.size() - fileCursor) < sizeof(CCSDS_PrimaryHeader_t)) {
            std::cerr << "File to small to contain the CCSDS_PrimaryHeader_t." << std::endl;
            return 1;
        }
        // Copy buffer data to struct CCSDS_PrimaryHeader_t
        CCSDS_SpacePacket_t spacePacket;
        std::memcpy(&spacePacket.Pri, buffer.data(), sizeof(CCSDS_PrimaryHeader_t));

        // Afficher les données de la structure CCSDS_PrimaryHeader_t
        std::cout << "StreamId: " << std::dec << ConvertToDecimal(spacePacket.Pri.StreamId) << std::endl;
        std::cout << "Sequence: " << std::dec << ConvertToDecimal(spacePacket.Pri.Sequence) << std::endl;
        std::cout << "Length: " << std::dec << ConvertToDecimal(spacePacket.Pri.Length) << std::endl;
        uint16_t msgLength = ConvertToDecimal(spacePacket.Pri.Length);

        // Récupération des valeurs spécifiques
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
                // Extraction des secondes
            uint32_t seconds = 0;
            for (int i = 0; i < 4; ++i) {
                seconds |= static_cast<uint32_t>(secondaryHeader.Time[i]) << ((3 - i) * 8);
            }
            // Extraction des sous-secondes
            uint16_t subseconds = (secondaryHeader.Time[4] << 8) | secondaryHeader.Time[5];
            // Affichage des résultats
            std::cout << "Seconds: " << seconds << std::endl;
            std::cout << "Subseconds: " << subseconds << std::endl;
            fileCursor += sizeof(CFE_MSG_TelemetrySecondaryHeader_t);

            // Date de départ en UTC avec millisecondes
            std::tm j2000_time = {};
            j2000_time.tm_year = 100;  // 2000 (years - 1900)
            j2000_time.tm_mon = 0;     // january (month 0-based)
            j2000_time.tm_mday = 1;    // first day of the month
            j2000_time.tm_hour = 11;
            j2000_time.tm_min = 58;
            j2000_time.tm_sec = 55;

            // Ajouter les millisecondes à la date de départ
            std::chrono::time_point<std::chrono::system_clock> j2000_time_point = std::chrono::system_clock::from_time_t(std::mktime(&j2000_time));
            j2000_time_point += std::chrono::milliseconds(816);

            // Calculer la nouvelle date en ajoutant les secondes et les sous-secondes
            auto newTimePoint = j2000_time_point + std::chrono::seconds(seconds) + std::chrono::milliseconds(subseconds);

            // Afficher la nouvelle date au format UTC
            std::time_t time = std::chrono::system_clock::to_time_t(newTimePoint);
            std::tm timeInfo;
#ifdef _WIN32  // Sous Windows
            gmtime_s(&timeInfo, &time);
#else  // Sous Linux
            gmtime_r(&time, &timeInfo);
#endif
            std::cout << "Date/Time: " << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") << "."
                << std::chrono::duration_cast<std::chrono::milliseconds>(newTimePoint.time_since_epoch() % std::chrono::seconds(1)).count() << " UTC" << std::endl;
            //Assign secondary header packet readed
            currentPacket.TelemetryHeader.Sec = secondaryHeader;
        }
        DS_HkTlm_Payload_t payload;
        if (buffer.size() >= (fileCursor + sizeof(DS_HkTlm_Payload_t))) {
            std::memcpy(&payload, buffer.data() + fileCursor, sizeof(DS_HkTlm_Payload_t));
            fileCursor += sizeof(DS_HkTlm_Payload_t);
            std::cout << "CmdAcceptedCounter: " << static_cast<int>(payload.CmdAcceptedCounter) << std::endl;
            std::cout << "CmdRejectedCounter: " << static_cast<int>(payload.CmdRejectedCounter) << std::endl;
            std::cout << "DestTblLoadCounter: " << static_cast<int>(payload.DestTblLoadCounter) << std::endl;
            std::cout << "DestTblErrCounter: " << static_cast<int>(payload.DestTblErrCounter) << std::endl;
            std::cout << "FilterTblLoadCounter: " << static_cast<int>(payload.FilterTblLoadCounter) << std::endl;
            std::cout << "FilterTblErrCounter: " << static_cast<int>(payload.FilterTblErrCounter) << std::endl;
            std::cout << "AppEnableState: " << static_cast<int>(payload.AppEnableState) << std::endl;
            std::cout << "FileWriteCounter: " << payload.FileWriteCounter << std::endl;
            std::cout << "FileWriteErrCounter: " << payload.FileWriteErrCounter << std::endl;
            std::cout << "FileUpdateCounter: " << payload.FileUpdateCounter << std::endl;
            std::cout << "FileUpdateErrCounter: " << payload.FileUpdateErrCounter << std::endl;
            std::cout << "DisabledPktCounter: " << payload.DisabledPktCounter << std::endl;
            std::cout << "IgnoredPktCounter: " << payload.IgnoredPktCounter << std::endl;
            std::cout << "FilteredPktCounter: " << payload.FilteredPktCounter << std::endl;
            std::cout << "PassedPktCounter: " << payload.PassedPktCounter << std::endl;
            std::cout << "FilterTblFilename: " << payload.FilterTblFilename << std::endl;
            //Assign payload readed
            currentPacket.Payload = payload;
        }
        packets.push_back(currentPacket);
    }
    file.close();
    return 0;
}