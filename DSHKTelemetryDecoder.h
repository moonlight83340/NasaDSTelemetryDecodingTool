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
	//! Get the packets extrated from the binary file
	/*!
	  \fn std::vector<DS_HkPacket_t> getPackets()
	  \return vector of DS_HkPacket_t readed from the binary file
	*/
	std::vector<DS_HkPacket_t> getPackets();
	//! Show values of the packet in param
	/*!
	  \fn void showPacket(DS_HkPacket_t packet)
	  \param packet a packet to show values
	*/
	void showPacket(DS_HkPacket_t packet);
	//! Show values of all the packets reacded fron the binary file
	/*!
	  \fn void showPackets()
	*/
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