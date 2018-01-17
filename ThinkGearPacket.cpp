//
// Created by Pho Hale on 2/21/17.
//

#include "ThinkGearPacket.h"

// define the class constructor
ThinkGearPacket::ThinkGearPacket(int packetNumber): _eegPower(new uint32_t[EEG_POWER_BANDS]), _packetNumber(0), _signalQuality(201), _attention(201), _meditation(201), _isComplete(false) {


}

boolean ThinkGearPacket::setup(uint32_t packetNumber, uint8_t signalQuality, uint8_t attention, uint8_t meditation, uint32_t eegPower[])
{
    _packetNumber = packetNumber;
    _signalQuality = signalQuality;
    _attention = attention;
    _meditation = meditation;

    //uint32_t _eegPower[EEG_POWER_BANDS] = {};

    //Deep copy
    //For each power band
    for (int i = 0; i < EEG_POWER_BANDS; i++) {
        _eegPower[i] = eegPower[i];
    }

    //If all is well, set that it is complete
    _isComplete = true;

    return _isComplete;
}

char* ThinkGearPacket::readCSV() {
    // spit out a big string?
    // find out how big this really needs to be
    // should be popped off the stack once it goes out of scope?
    // make the character array as small as possible
    char csvBuffer[100];

    if(_isComplete) {
        sprintf(csvBuffer,"%d,%d,%d,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu",
                _signalQuality,
                _attention,
                _meditation,
                _eegPower[0],
                _eegPower[1],
                _eegPower[2],
                _eegPower[3],
                _eegPower[4],
                _eegPower[5],
                _eegPower[6],
                _eegPower[7]
        );
        return csvBuffer;
    }
    else {
        sprintf(csvBuffer,"%d,%d,%d",
                _signalQuality,
                _attention,
                _meditation
        );
        return csvBuffer;
    }
}

uint32_t ThinkGearPacket::getID() {
    return _packetNumber;
}