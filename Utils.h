#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <cstring> // pour memcpy

//Convert uint16_t little endian to big endian
inline uint16_t littleToBigEndian16(uint16_t value) {
    return (value << 8) | (value >> 8);
}

//Convert uint32_t little endian to big endian
inline uint32_t littleToBigEndian32(uint32_t value) {
    return ((value << 24) & 0xFF000000) |
        ((value << 8) & 0x00FF0000) |
        ((value >> 8) & 0x0000FF00) |
        ((value >> 24) & 0x000000FF);
}

//Check if CPU little endian
inline bool isLittleEndian() {
    uint32_t testValue = 0x12345678;
    uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&testValue);
    return (*bytePtr == 0x78);
}

#endif // UTILS_H