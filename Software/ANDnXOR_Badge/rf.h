#ifndef ANX_RF_H
#define ANX_RF_H

#include <RFM69-ANDnXOR.h>
#include "ANX.h"
#include "settings.h"

#define ANX_RF_MAX_SIZE     128

#define RFM69_CS            PB10
#define RFM69_INT           PB11
#define RFM69_INT_NUM       PB11
#define RFM69_NETWORK_ID    24
#define PORT_HELLO          0
#define PORT_CHAT           1
#define PORT_PONG           2
#define PORT_PING           3
#define PORT_NINJA          4
#define PORT_MASTER         88
#define PORT_ALERT          99
#define PROCESS_DATA_PERIOD 25 //millis
#define MAX_HOPS            3
#define KEY_LENGTH          16

extern RFM69 radio;

struct PeerNode {
  byte nodeid = 0;
  char name[USERNAME_MAX_LENGTH];
  int lastSeen = 0;
  int16_t rssi = 0;
  uint8_t level = 1;
};

/**
   Logged packet
   Useful for debugging purposed
*/
struct LoggedPacket {
  byte port;
  byte src;
  int16_t rssi;
  uint32_t timestamp;
};
extern LoggedPacket packetLog[PACKET_LOG_MAX];

/**
   Hello packet
   Used for peer presence discovery
   Size = 11 Bytes
*/
struct HelloPacket {
  byte port = PORT_HELLO;
  byte src;
  byte sequence;
  byte level;
  char name[USERNAME_MAX_LENGTH];
};
#define HELLO_PACKET_SIZE 12

/**
   Chat packet
   Basic structure to send chat messages
   Size = 42 Bytes
*/
struct ChatPacket {
  byte port = PORT_CHAT;
  byte src;
  byte sequence;
  char name[USERNAME_MAX_LENGTH];
  char message[ANX_CHAT_MAX_LENGTH];
};

/**
   Alert Packet
   Used to send popups from master badges to human badges.
   Size = 42 Bytes
*/
struct AlertPacket {
  byte port = PORT_ALERT;
  byte src;
  byte sequence;
  byte hops;
  char name[USERNAME_MAX_LENGTH];
  char message[ANX_ALERT_MAX_LENGTH];
};

/**
   Packet used for master badges to do special things
*/
struct MasterPacket {
  byte port = PORT_MASTER;
  byte src;
  byte dest;
  byte sequence;
  byte type;
  char name[USERNAME_MAX_LENGTH];
};
#define MASTER_PACKET_SIZE      5 + USERNAME_MAX_LENGTH
#define MASTER_TYPE_UNLOCK      24
#define MASTER_TYPE_LEVELUP     18
#define MASTER_TYPE_TROLL_RICK  99

/**
   Generic ninja packet
*/
struct NinjaPacket {
  byte port = PORT_NINJA;
  byte src;
  byte dest;
  byte gameid;
  byte round;
  byte type;
  byte data;
};
#define NINJA_PACKET_SIZE 7
extern bool allowNinja;  //flag to temporarily disable ninja challenges

/**
   Packet for pings between badges
*/
struct PingPacket {
  byte port = PORT_PING;
  byte src;
  byte dest;
  byte seq;
  byte type;
};
#define PING_PACKET_SIZE  5
#define PING_TYPE_PING    1
#define PING_TYPE_PONG    2

//Debug data
extern uint32_t lastPacket;
extern uint32_t lastSend;

//Flags to disable specific features
extern bool allowAlerts;
extern bool allowHello;
extern bool allowNinja;
extern bool allowMaster;

//Peer storage
extern PeerNode peers[PEER_NODES_MAX];

//Chat message storage
extern ChatMessage messages[CHAT_MESSAGES_MAX];
extern uint8_t messageCount;

//Ninja packet storage
extern NinjaPacket lastNinjaPacket;
extern bool ninjaPacketAvailable;

//Ping packet storage
extern PingPacket lastPingPacket;
extern bool pingPacketAvailable;

extern void doRFDebug();
extern void doSpectrumAnalyzer();
extern void disablePopups();
extern void enablePopups();
extern void ANXRFBegin();
extern void ANXAlertHandler(uint8_t length);
extern void ANXChatHandler(uint8_t length);
extern void ANXHelloHandler(uint8_t length, int16_t rssi);
extern void ANXMasterHandler(uint8_t length);
extern void ANXNinjaHandler(uint8_t length);
extern void ANXPingHandler(uint8_t length);
extern bool ANXRFAvailable();
extern void rfHandler();
extern bool ANXRFSend(const void* buffer, uint8_t bufferSize);
extern void ANXRFSendAlert(char *message);
extern void ANXRFSendChat(char *message);
extern void ANXRFSendHello();
extern int ANXRFSendPing(uint8_t nodeid);
#ifdef MASTER
extern void ANXRFSendLevelUp(uint8_t nodeid);
extern void ANXRFSendTrollRick(uint8_t nodeid);
extern void ANXRFSendUnlock(uint8_t nodeid);
#endif
extern void ANXRFSleep();

#endif
