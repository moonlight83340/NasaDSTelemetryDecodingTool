#pragma once
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include "DS-HK-Telemetry.h"
#include <vector>


//Telemetry secondary Header Time in J2000 time format
struct TelemetrySecondaryHeaderTime {
	uint32_t seconds;
	uint16_t subSeconds;
};

class DSHKTelemetryDecoder
{
public:
	DSHKTelemetryDecoder(const std::string& filePathp);
	int processBinaryFile();
	std::vector<DS_HkPacket_t> getPackets();
private:
	//Utility function could be from an another file
	uint16_t ConvertToDecimal(uint8_t bytes[2]);
	TelemetrySecondaryHeaderTime getTelemetrySecondaryHeaderTime(CFE_MSG_TelemetrySecondaryHeader_t secondaryHeader);
	std::tm getPacketDatestampInUtcTimeFromJ2000Time(uint32_t seconds, uint16_t subSeconds);

	std::vector<DS_HkPacket_t> packets;
	std::string filePath;
	bool isBigEndian;
};

