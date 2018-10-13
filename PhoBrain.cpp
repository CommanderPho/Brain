//
//
//

#include "PhoBrain.h"

PhoBrain::PhoBrain(Stream &_brainStream) {
	brainStream = &_brainStream;

	// Keep the rest of the initialization process in a separate method in case
	// we overload the constructor.
	init();
}

void PhoBrain::init() {
	// It's up to the calling code to start the stream
	// Usually Serial.begin(9600);
	freshPacket = false;
	inPacket = false;
	inPacketByteIndex = 0;
	packetPayloadLength = 0;
	eegPowerLength = 0;
	hasPower = false;
	checksum = 0;
	checksumAccumulator = 0;

	signalQuality = 200;
	attention = 0;
	meditation = 0;

	currentPacketIndex = 0;
	successPacketCount = 0;
	errorPacketCount = 0;

	clearEegPower();
}

boolean PhoBrain::update() {
	//Serial.print(F("PhoBrain::update()"));

	//available() gets the number of bytes available in the stream. This is only for bytes that have already arrived.
	if (brainStream->available()) {
		//read() reads characters from an incoming stream to the buffer.
		// returns the first byte of incoming data available (or -1 if no data is available, but this won't happen since we ensured at least one byte is available.)
		latestByte = brainStream->read();
		//If we are currently in the middle of a packet.
		if (inPacket) {
			// First byte after the sync bytes is the length of the upcoming packet.
			if (inPacketByteIndex == 0)
				packetPayloadLength = latestByte;
			// Catch error if packet is too long
			if (packetPayloadLength > MAX_PACKET_LENGTH) {
				// Packet exceeded max length
				// Send an error
				sprintf(latestError, "ERROR: Packet too long %i", packetPayloadLength);
				inPacket = false;
				Serial.print(F("PhoBrain::update() -> assembly failed! Packet too long!"));
			}
		}
		else if (inPacketByteIndex <= packetPayloadLength) {
			// Run of the mill data bytes.

			// Print them here

			// Store the byte in an array for parsing later.
			packetData[inPacketByteIndex - 1] = latestByte;

			// Keep building the checksum.
			checksumAccumulator += latestByte;
		}
		else if (inPacketByteIndex > packetPayloadLength) {
			// We're at the end of the data payload, meaning the latestByte is the payload checksum

			// Check the checksum.
			checksum = latestByte;
			checksumAccumulator = 255 - checksumAccumulator;

			// Do they match?
			if (checksum == checksumAccumulator) {
				//if the checksum is valid, parse the packet.
				boolean parseSuccess = parsePacket();

				//If parseSuccess then we have a new valid packet
				if (parseSuccess) {
					freshPacket = true;
					//successPacketCount++;
					Serial.println(F("PhoBrain::update() -> parsing success!"));
				}
				else {
					// Parsing failed, send an error.
					sprintf(latestError, "ERROR: Could not parse");
					// good place to print the packet if debugging
					//errorPacketCount++;
					Serial.println(F("PhoBrain::update() -> parsing failed!"));
				}
			}
			else {
				// Checksum mismatch, send an error.
				sprintf(latestError, "ERROR: Checksum");
				// good place to print the packet if debugging
				//errorPacketCount++;
				Serial.println(F("PhoBrain::update() -> checksum failed!"));
			}
			// End of packet

			// Reset, prep for next packet
			inPacket = false;
		}

		inPacketByteIndex++;

		// Keep track of the last byte so we can find the sync byte pairs that signal the start of a packet.
		lastByte = latestByte;
	}
	else {
		// Look for the start of the packet by checking for the two sync bytes (decimal 170)
		// Ensure that we are not currently inPacket (because two consecutive sync bytes could appear within a packet.
		if ((latestByte == 170) && (lastByte == 170)) {
			// Start of packet
			inPacket = true;
			inPacketByteIndex = 0;
			checksumAccumulator = 0;
		}
	}

	//update() has been called before a new packet from TGAM is available.
	if (freshPacket) {
		freshPacket = false;
		return true;
	}
	else {
		return false;
	}
}

void PhoBrain::clearPacket() {
	for (uint8_t i = 0; i < MAX_PACKET_LENGTH; i++) {
		packetData[i] = 0;
	}
}

void PhoBrain::clearEegPower() {
	// Zero the power bands.
	for (uint8_t i = 0; i < EEG_POWER_BANDS; i++) {
		eegPower[i] = 0;
	}
}

boolean PhoBrain::parsePacket() {
	Serial.println(F("PhoBrain::parsePacket()"));
	// Loop through the packet, extracting data.
	// Based on mindset_communications_protocol.pdf from the Neurosky Mindset SDK.
	// Returns true if passing succeeds
	hasPower = false;
	boolean parseSuccess = true;
	int rawValue = 0;

	clearEegPower();    // clear the eeg power to make sure we're honest about missing values

	for (uint8_t i = 0; i < packetPayloadLength; i++) {
		switch (packetData[i]) {
		case 0x2:
			signalQuality = packetData[++i];
			break;
		case 0x4:
			attention = packetData[++i];
			break;
		case 0x5:
			meditation = packetData[++i];
			break;
		case 0x83:
			// ASIC_EEG_POWER: eight big-endian 3-uint8_t unsigned integer values representing delta, theta, low-alpha high-alpha, low-beta, high-beta, low-gamma, and mid-gamma EEG band power values
			// The next uint8_t sets the length, usually 24 (Eight 24-bit numbers... big endian?)
			// We dont' use this value so let's skip it and just increment i
			i++;

			// Extract the values
			for (int j = 0; j < EEG_POWER_BANDS; j++) {
				eegPower[j] = ((uint32_t)packetData[++i] << 16) | ((uint32_t)packetData[++i] << 8) | (uint32_t)packetData[++i];
			}

			hasPower = true;
			// This seems to happen once during start-up on the force trainer. Strange. Wise to wait a couple of packets before
			// you start reading.
			break;
		case 0x80:
			// We dont' use this value so let's skip it and just increment i
			// uint8_t packetPayloadLength = packetData[++i];
			i++;
			rawValue = ((int)packetData[++i] << 8) | packetData[++i];
			break;
		default:
			// Broken packet ?
			/*
			Serial.print(F("parsePacket UNMATCHED data 0x"));
			Serial.print(packetData[i], HEX);
			Serial.print(F(" in position "));
			Serial.print(i, DEC);
			printPacket();
			*/
			parseSuccess = false;
			break;
		}
	}
	return parseSuccess;
}

char* PhoBrain::readErrors() {
	return latestError;
}

//ThinkGearPacket Brain::readPacket() {
//    //Get current packet count
//    uint32_t currentPacketCount = successPacketCount + errorPacketCount;
//
//    //Create new packet
//    ThinkGearPacket currPacket = ThinkGearPacket(currentPacketIndex);
//
//    boolean setupSuccess = currPacket.setup(currentPacketIndex,signalQuality,attention,meditation,eegPower);
//    if(setupSuccess) {
//        //success!
//    }
//    else {
//        sprintf(latestError, "ERROR: Failed to parse to packet!");
//    }
//
//    //Increment the next packet index
//    currentPacketIndex++;
//
//    return currPacket;
//}

//todo: convert to use the readPacket
char* PhoBrain::readCSV() {
	// spit out a big string?
	// find out how big this really needs to be
	// should be popped off the stack once it goes out of scope?
	// make the character array as small as possible

	if (hasPower) {
		sprintf(csvBuffer, "%d,%d,%d,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu",
			signalQuality,
			attention,
			meditation,
			eegPower[0],
			eegPower[1],
			eegPower[2],
			eegPower[3],
			eegPower[4],
			eegPower[5],
			eegPower[6],
			eegPower[7]
		);

		return csvBuffer;
	}
	else {
		sprintf(csvBuffer, "%d,%d,%d",
			signalQuality,
			attention,
			meditation
		);

		return csvBuffer;
	}
}

// For debugging, print the entire contents of the packet data array.
void PhoBrain::printPacket() {
	brainStream->print("[");
	for (uint8_t i = 0; i < MAX_PACKET_LENGTH; i++) {
		brainStream->print(packetData[i], DEC);

		if (i < MAX_PACKET_LENGTH - 1) {
			brainStream->print(", ");
		}
	}
	brainStream->println("]");
}

// Keeping this around for debug use
void PhoBrain::printCSV() {
	// Print the CSV over serial
	brainStream->print(signalQuality, DEC);
	brainStream->print(",");
	brainStream->print(attention, DEC);
	brainStream->print(",");
	brainStream->print(meditation, DEC);

	if (hasPower) {
		for (int i = 0; i < EEG_POWER_BANDS; i++) {
			brainStream->print(",");
			brainStream->print(eegPower[i], DEC);
		}
	}

	brainStream->println("");
}

void PhoBrain::printDebug() {
	brainStream->println("");
	brainStream->println("--- Start Packet ---");
	brainStream->print("Signal Quality: ");
	brainStream->println(signalQuality, DEC);
	brainStream->print("Attention: ");
	brainStream->println(attention, DEC);
	brainStream->print("Meditation: ");
	brainStream->println(meditation, DEC);

	if (hasPower) {
		brainStream->println("");
		brainStream->println("EEG POWER:");
		brainStream->print("Delta: ");
		brainStream->println(eegPower[0], DEC);
		brainStream->print("Theta: ");
		brainStream->println(eegPower[1], DEC);
		brainStream->print("Low Alpha: ");
		brainStream->println(eegPower[2], DEC);
		brainStream->print("High Alpha: ");
		brainStream->println(eegPower[3], DEC);
		brainStream->print("Low Beta: ");
		brainStream->println(eegPower[4], DEC);
		brainStream->print("High Beta: ");
		brainStream->println(eegPower[5], DEC);
		brainStream->print("Low Gamma: ");
		brainStream->println(eegPower[6], DEC);
		brainStream->print("Mid Gamma: ");
		brainStream->println(eegPower[7], DEC);
	}

	brainStream->println("");
	brainStream->print("Checksum Calculated: ");
	brainStream->println(checksumAccumulator, DEC);
	brainStream->print("Checksum Expected: ");
	brainStream->println(checksum, DEC);

	brainStream->println("--- End Packet ---");
	brainStream->println("");
}

uint8_t PhoBrain::readSignalQuality() {
	return signalQuality;
}

uint8_t PhoBrain::readAttention() {
	return attention;
}

uint8_t PhoBrain::readMeditation() {
	return meditation;
}

uint32_t* PhoBrain::readPowerArray() {
	return eegPower;
}

uint32_t PhoBrain::readDelta() {
	return eegPower[0];
}

uint32_t PhoBrain::readTheta() {
	return eegPower[1];
}

uint32_t PhoBrain::readLowAlpha() {
	return eegPower[2];
}

uint32_t PhoBrain::readHighAlpha() {
	return eegPower[3];
}

uint32_t PhoBrain::readLowBeta() {
	return eegPower[4];
}

uint32_t PhoBrain::readHighBeta() {
	return eegPower[5];
}

uint32_t PhoBrain::readLowGamma() {
	return eegPower[6];
}

uint32_t PhoBrain::readMidGamma() {
	return eegPower[7];
}