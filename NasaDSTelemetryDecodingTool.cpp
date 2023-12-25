// NasaDSTelemetryDecodingTool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <chrono>
#include "DS-HK-Telemetry.h"
#include "DSHKTelemetryDecoder.h"
#include "Utils.h"

using namespace std;
using namespace ds_telemetry_decoder;

//! Show the hexadecimal of the binary telemetry binary file
/*!
    \fn void showBinaryFileInHexa(const std::string& filePath)
    \param filePath the FilePath to the binary file
*/
void showBinaryFileInHexa(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Can't open File" << std::endl;
        return;
    }

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(file), {});
    file.close();

    // Show data in hexadecimal
    for (size_t i = 0; i < buffer.size(); ++i) {
        std::cout << std::setw(2) << std::setfill('0') 
            << std::hex << static_cast<int>(buffer[i]) << " ";
        if ((i + 1) % 16 == 0) {
            std::cout << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    std::string filePath;
    if (argc < 2) {
        std::cout << "Usage: " << std::endl;
        std::cout << argv[0] << " binary" << " <filepath>" << std::endl;
        std::cout << argv[0] << " run" << " <filepath>" << std::endl;
        return 1;
    }
    if (argc >= 2) {
        std::string command = argv[1];
        std::string filePath = argv[2];
        if (command == "binary" && argc >= 3) {
            showBinaryFileInHexa(filePath);
        }
        else if (command == "run" && argc >= 3) {       
            if (utils::isLittleEndian()) {
                std::cout << "CPU is little-endian." << std::endl;
            }
            else {
                std::cout << "CPU is big-endian." << std::endl;
            }
            DSHKTelemetryDecoder DSHKTelemetryDecoder(filePath);
            DSHKTelemetryDecoder.processBinaryFile();
            DSHKTelemetryDecoder.showPackets();
        }
        else {
            std::cout << "Unknown command." << std::endl;
            return 1;
        }
    }
    else {
        std::cout << "No command specified." << std::endl;
        return 1;
    }
    return 0;
}