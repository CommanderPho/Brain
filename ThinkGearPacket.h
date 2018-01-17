//
// Created by Pho Hale on 2/21/17.
//

#ifndef PHOBRAIN_THINKGEARPACKET_H
#define PHOBRAIN_THINKGEARPACKET_H

#include "Arduino.h"
#include "ThinkGearPacketProtocol.h"

class ThinkGearPacket {
    public:
        ThinkGearPacket(int packetNumber);
        //ThinkGearPacket(int packetNumber, uint8_t signalQuality, uint8_t attention, uint8_t meditation, uint32_t eegPower[]);
        boolean setup(uint32_t packetNumber, uint8_t signalQuality, uint8_t attention, uint8_t meditation, uint32_t eegPower[]);
        uint32_t getID();
        char*  readCSV();

    private:
        //uint32_t _eegPower[EEG_POWER_BANDS];
        uint32_t* _eegPower;
        uint32_t _packetNumber;
        uint8_t _signalQuality;
        uint8_t _attention;
        uint8_t _meditation;

        boolean _isComplete;
};


#endif //PHOBRAIN_THINKGEARPACKET_H
