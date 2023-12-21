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
	DSHKTelemetryDecoder(const std::string& filePath);
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
	//! Show values of all the packets reacded fron the binary file
	/*!
	  \fn TelemetrySecondaryHeaderTime 
	  getTelemetrySecondaryHeaderTime(CFE_MSG_TelemetrySecondaryHeader_t secondaryHeader)
	  \param secondaryHeader the CFE_MSG_TelemetrySecondaryHeader_t (time stamp of HK telemetry)
	  \return seconds and subseconds in a struct
	*/
	TelemetrySecondaryHeaderTime 
		getTelemetrySecondaryHeaderTime(CFE_MSG_TelemetrySecondaryHeader_t secondaryHeader);
	//! Get the packet datestamp in utc time from J2000Time
	/*!
	  \fn std::chrono::time_point<std::chrono::system_clock> 
	  getPacketDatestampInUtcTimeFromJ2000Time(uint32_t seconds, uint16_t subSeconds)
	  \param seconds packet seconds
	  \param subseconds packet ms
	  \return a time point in utc time from J2000Time
	*/
	std::chrono::time_point<std::chrono::system_clock> 
		getPacketDatestampInUtcTimeFromJ2000Time(uint32_t seconds, uint16_t subSeconds);

	std::vector<DS_HkPacket_t> _packets;
	std::string _filePath;
};
#endif //DSHK_TELEMETRY_DECODER_H