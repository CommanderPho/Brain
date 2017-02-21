//
// Created by Pho Hale on 2/21/17.
//
// Defines conventions and constants defined by the ThinkGear Packet Protocol by Neurosky.

#ifndef PHOBRAIN_THINKGEARPACKETPROTOCOL_H
#define PHOBRAIN_THINKGEARPACKETPROTOCOL_H

//TODO: Investigate why they chose 32 for this length....
//#define MAX_PACKET_LENGTH 32


#define EEG_POWER_BANDS 8

//Corresponding to an empty data payload.
#define MIN_PACKET_LENGTH 4

//Corresponding to a 169 Byte long data payload
#define MAX_PACKET_LENGTH 173

//SYNC bytes begin a packet header
#define PACKET_SYNC_BYTE 0xAA


#endif //PHOBRAIN_THINKGEARPACKETPROTOCOL_H
