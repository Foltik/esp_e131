#include <Ethernet.h>
#include <EthernetUdp.h>

#include "SAMD51_E131.h"

#include <string.h>

SAMD51E131 E131;

/* E1.17 ACN Packet Identifier */
const byte SAMD51E131::ACN_ID[12] = { 0x41, 0x53, 0x43, 0x2d, 0x45, 0x31, 0x2e, 0x31, 0x37, 0x00, 0x00, 0x00 };

/* Constructor */
SAMD51E131::SAMD51E131() {
    memset(pbuff1.raw, 0, sizeof(pbuff1.raw));
    //memset(pbuff2.raw, 0, sizeof(pbuff2.raw));
    packet = &pbuff1;
    //pwbuff = &pbuff2;

    stats.num_packets = 0;
    stats.packet_errors = 0;
}

void SAMD51E131::initUnicast() {
    delay(100);
    udp.begin(E131_DEFAULT_PORT);
//    if (Serial) {
//        Serial.print(F("- Unicast port: "));
//        Serial.println(E131_DEFAULT_PORT);
//    }
}

//void SAMD51E131::initMulticast(uint16_t universe, uint8_t n) {
//    delay(100);
//    IPAddress address = IPAddress(239, 255, ((universe >> 8) & 0xff),
//            ((universe >> 0) & 0xff));
//
//    if (Serial) {
//        Serial.println(F("- Multicast Enabled"));
//    }
//}

void SAMD51E131::begin(uint8_t *mac, IPAddress ip, IPAddress subnet) {
    IPAddress dns(0, 0, 0, 0);
    IPAddress gateway(0, 0, 0, 0);
    
    Ethernet.begin(mac, ip, dns, gateway, subnet);
//    if (Serial) {
//        Serial.println("");
//        Serial.println(F("Static Configuration"));
//        Serial.println(F("- MAC: "));
//        for (int i = 0; i < 6; i++)
//            Serial.print(mac[i], HEX);
//        Serial.print(F("- IP Address: "));
//        Serial.println(Ethernet.localIP());
//    }

    initUnicast();
}

//void SAMD51E131::beginMulticast(uint8_t *mac, uint16_t universe,
//        IPAddress ip, IPAddress netmask, IPAddress gateway,
//        IPAddress dns, uint8_t n) {
//    //TODO: Add ethernet multicast support
//}

void SAMD51E131::dumpError(e131_error_t error) {
    switch (error) {
        case ERROR_ACN_ID:
            Serial.print(F("INVALID PACKET ID: "));
            for (int i = 0; i < sizeof(ACN_ID); i++)
                Serial.print(pwbuff->acn_id[i], HEX);
            Serial.println("");
            break;
        case ERROR_PACKET_SIZE:
            Serial.println(F("INVALID PACKET SIZE: "));
            break;
        case ERROR_VECTOR_ROOT:
            Serial.print(F("INVALID ROOT VECTOR: 0x"));
            Serial.println(htonl(pwbuff->root_vector), HEX);
            break;
        case ERROR_VECTOR_FRAME:
            Serial.print(F("INVALID FRAME VECTOR: 0x"));
            Serial.println(htonl(pwbuff->frame_vector), HEX);
            break;
        case ERROR_VECTOR_DMP:
            Serial.print(F("INVALID DMP VECTOR: 0x"));
            Serial.println(pwbuff->dmp_vector, HEX);
    }
}
