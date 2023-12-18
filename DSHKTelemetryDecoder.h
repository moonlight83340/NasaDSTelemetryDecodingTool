#ifndef DSHK_TELEMETRY_DECODER_H
#define DSHK_TELEMETRY_DECODER_H
#include "DS-HK-Telemetry.h"
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <ctime>



//Telemetry secondary Header Time in J2000 time format
struct TelemetrySecondaryHeaderTime {
	uint32_t seconds;
	uint16_t subSeconds;
};

class DSHKTelemetryDecoder
{
public:
	DSHKTelemetryDecoder(const std::string& filePathp);
	//! Process the binary file to read the HK telemetry
	/*!
	  \fn int processBinaryFile()
	  \return 0 if everything fine 1 if error
	*/
	int processBinaryFile();
	std::vector<DS_HkPacket_t> getPackets();
	void showPacket(DS_HkPacket_t packet);
	void showPackets();
private:
	//Utility function could be from an another file
	uint16_t ConvertToDecimal(uint8_t bytes[2]);
	TelemetrySecondaryHeaderTime getTelemetrySecondaryHeaderTime(CFE_MSG_TelemetrySecondaryHeader_t secondaryHeader);
	std::tm getPacketDatestampInUtcTimeFromJ2000Time(uint32_t seconds, uint16_t subSeconds);

	std::vector<DS_HkPacket_t> packets;
	std::string filePath;
	bool isBigEndian;
};
#endif //DSHK_TELEMETRY_DECODER_H