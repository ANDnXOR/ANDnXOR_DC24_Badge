#include <Adafruit_SSD1306_STM32-ANDnXOR.h>
#include <RFM69-ANDnXOR.h>
#include <RFM69registers-ANDnXOR.h>
#include <SPI.h>

#include "ANX.h"
#include "anim.h"
#include "buttons.h"
#include "flash.h"
#include "ninja.h"
#include "rf.h"
#include "serial.h"

extern Adafruit_SSD1306   display;
extern ANXFlash           flash;
extern RFM69              radio;

static uint32_t lastRFHandler = 0;
static uint32_t lastHello = 0;
static byte packetBuffer[ANX_RF_MAX_SIZE];
static bool radioInitPassed = false;
static bool radioAvailable = true;

//Packet log
LoggedPacket packetLog[PACKET_LOG_MAX];
static int8_t logPtr = 0;

//Debug data
uint32_t lastPacket = 0;
uint32_t lastSend = 0;

//Peer storage
PeerNode peers[PEER_NODES_MAX];
//static uint8_t peerCount = 0;

//Chat message storage
ChatMessage messages[CHAT_MESSAGES_MAX];
uint8_t messageCount = 0;

//Ninja packet storage
NinjaPacket lastNinjaPacket;
bool ninjaPacketAvailable = false;

//Ping packet storage
PingPacket lastPingPacket;
bool pingPacketAvailable;

//Alert storage for meshing
static uint16_t alertLog[ALERT_LOG_MAX];
static uint8_t alertLogPtr = 0;

//Flags to disable specific features
bool allowAlerts = true;
bool allowHello = true;
bool allowNinja = true;
bool allowMaster = true;

/**
   Initialize the radio
*/
void ANXRFBegin() {
  uint8_t nodeid = getNodeID();
  radioInitPassed = false;

  //Initialize peers storage
  for (uint8_t i = 0; i < PEER_NODES_MAX; i++) {
    peers[i].lastSeen = 0;
  }

  //Initialize radio
  if (radio.initialize(RF69_433MHZ, nodeid, RFM69_NETWORK_ID)) {
    radio.encrypt("AND!xorDC24VeGaS");
    radio.promiscuous(true);
    radio.writeReg(REG_AFCFEI, RF_AFCFEI_AFCAUTOCLEAR_ON | RF_AFCFEI_AFCAUTO_ON | RF_AFCFEI_AFC_START);
    radioInitPassed = true;
  }

  //Test for frequency synthensizer mode, if it occurs, ask the user to pull a battery
  radio.receiveDone();
}

/**
  "Background" process for handling RF
*/
void rfHandler() {
  //If airplane mode, quit early
  if (ANXGetAirplane()) {
    return;
  }

  //Quit early if hasn't been long enough since last handler
  if ((rtMillis() - lastRFHandler) < PROCESS_DATA_PERIOD) {
    return;
  }
  lastRFHandler = rtMillis();

  uint8_t length;

  //Check if any data is available to process
  if (radio.receiveDone()) {
    length = radio.DATALEN;

    //Get our own copy of the received data to free up the FIFO
    for (uint8_t i = 0; i < min(ANX_RF_MAX_SIZE, radio.DATALEN); i++) {
      packetBuffer[i] = radio.DATA[i];
    }

    //Log the packets
    LoggedPacket packet;
    packet.port = packetBuffer[0];
    packet.src = packetBuffer[1];
    packet.rssi = radio.RSSI;
    packet.timestamp = rtMillis();
    packetLog[logPtr] = packet;
    logPtr = (logPtr + 1) % PACKET_LOG_MAX;

    //Save some debug info
    lastPacket = rtMillis();

    //Route based on port
    switch (packet.port) {
      case PORT_HELLO:
        ANXHelloHandler(length, radio.RSSI);
        break;
      case PORT_CHAT:
        ANXChatHandler(length);
        break;
      case PORT_ALERT:
        ANXAlertHandler(length);
        break;
      case PORT_MASTER:
        ANXMasterHandler(length);
        break;
      case PORT_PING:
        ANXPingHandler(length);
        break;
      case PORT_NINJA:
        ANXNinjaHandler(length);
        break;
    }
  }

  //Check if we need send a hello
  if ((rtMillis() - lastHello) > HELLO_PERIOD) {
    ANXRFSendHello();
    lastHello = rtMillis();
  }
}

/**
  Helper function to send data safely
*/
bool ANXRFSend(const void* buffer, uint8_t bufferSize) {
  if (!ANXRFAvailable()) {
    radio.setMode(RF69_MODE_RX);
    deepSleep(20);
  }

  if (ANXRFAvailable()) {
    radio.send(RF69_BROADCAST_ADDR, buffer, bufferSize);
    return true;
  } else {
#ifdef ANX_DEBUG
    Serial.println("Not sending packet, radio not available");
#endif
  }

  return false;
}

/**
  Construct and send an alert packet
*/
void ANXRFSendAlert(char *message) {
  if (ANXGetAirplane()) {
    return;
  }

  //Build the packet
  AlertPacket alert;
  alert.src = getNodeID();
  alert.sequence = rtMillis() % 256;
  ANXGetUsername(alert.name);

  //Set the message of the alert ensuring no extra data in the body
  uint8_t len = min(strlen(message), ANX_CHAT_MAX_LENGTH);
  //Clear the packet buffer
  memset(alert.message, '\0', ANX_CHAT_MAX_LENGTH);
  //Copy into the packet buffer
  memcpy(alert.message, message, len);

  //Send the packet
  ANXRFSend(&alert, sizeof(alert));
}

/**
  Construct and send a hello packet
*/
void ANXRFSendHello() {
  if (ANXGetAirplane()) {
    return;
  }

  //Build the packet
  HelloPacket hello;
  hello.src = getNodeID();
  hello.sequence = rtMillis() % 256;
  hello.level = ANXGetLevel();
  ANXGetUsername(hello.name);

  //Send it just once, it's okay if it's lost
  ANXRFSend(&hello, sizeof(hello));

  lastSend = rtMillis();
}

/**
  Construct and send a chat packet
*/
void ANXRFSendChat(char *message) {
  if (ANXGetAirplane()) {
    return;
  }

  //Build the chat packet
  ChatPacket chat;
  chat.src = getNodeID();
  chat.sequence = rtMillis() % 256;
  ANXGetUsername(chat.name);
  uint8_t len = min(strlen(message), ANX_CHAT_MAX_LENGTH);

  //Clear the packet buffer
  memset(chat.message, '\0', ANX_CHAT_MAX_LENGTH);
  //Copy into the packet buffer
  memcpy(chat.message, message, len);

  //Send the packet
  for (uint8_t i = 0; i < 3; i++) {
    ANXRFSend(&chat, sizeof(chat));
    deepSleep(random(60) + 40);
  }

  //Construct chat message and put it into the buffer with everything else received
  ChatMessage chatMessage;
  memcpy(chatMessage.name, chat.name, USERNAME_MAX_LENGTH);
  memcpy(chatMessage.message, chat.message, ANX_CHAT_MAX_LENGTH);
  chatMessage.received = rtMillis();
  addToChatBuffer(chatMessage);
}

/**
  Ping a node
  Returns roundtrip time or -1 if nothing
*/
int ANXRFSendPing(uint8_t nodeid) {
  PingPacket packet;
  packet.src = getNodeID();
  packet.dest = nodeid;
  packet.type = PING_TYPE_PING;
  packet.seq = rtMillis() % 256;
  pingPacketAvailable = false;

  ANXRFSend(&packet, PING_PACKET_SIZE);
  uint32_t startTime = rtMillis();
  uint32_t endTime = 0;

  while (1) {
    if ((rtMillis() - startTime) > 500) {
      return -1;
    }

    if (pingPacketAvailable) {
      pingPacketAvailable = false;
      if (lastPingPacket.seq == packet.seq) {
        return rtMillis() - startTime;
      } else {
      }
    }

    deepSleep(50);
    tick();
  }
}

#ifdef MASTER
/**
  Master badge level up
*/
void ANXRFSendLevelUp(uint8_t nodeid) {
  MasterPacket packet;
  packet.type = MASTER_TYPE_LEVELUP;
  packet.dest = nodeid;
  packet.sequence = rtMillis() % 256;
  ANXGetUsername(packet.name);
  ANXRFSend(&packet, MASTER_PACKET_SIZE);
}

/**
  Troll somebody
*/
void ANXRFSendTrollRick(uint8_t nodeid) {
  MasterPacket packet;
  packet.type = MASTER_TYPE_TROLL_RICK;
  packet.dest = nodeid;
  packet.sequence = rtMillis() % 256;
  ANXGetUsername(packet.name);
  ANXRFSend(&packet, MASTER_PACKET_SIZE);
}

/**
  Master badge unlock
*/
void ANXRFSendUnlock(uint8_t nodeid) {
  MasterPacket packet;
  packet.type = MASTER_TYPE_UNLOCK;
  packet.dest = nodeid;
  packet.sequence = rtMillis() % 256;
  ANXGetUsername(packet.name);
  ANXRFSend(&packet, MASTER_PACKET_SIZE);
}

#endif

/**
  Handle alerts received
*/
void ANXAlertHandler(uint8_t length) {
  //If user disabled alerts stop
  if (!ANXGetAlert()) {
    return;
  }

  //If alerts temporarily disabled stop
  if (!allowAlerts) {
    return;
  }

  //If alert length is not valid stop - stupid simple verification method
  if (length != 43) {
    return;
  }

  //Save the display buffer
  uint32_t buffSize = (display.height() * display.width()) / 8;
  uint8_t displayBuffer[buffSize];
  memcpy(displayBuffer, display.getBuffer(), buffSize);

  //Deserialize the data
  AlertPacket alert;
  alert.src = packetBuffer[1];
  alert.sequence = packetBuffer[2];
  alert.hops = packetBuffer[3];
  memcpy(alert.name, packetBuffer + 4, USERNAME_MAX_LENGTH);
  memcpy(alert.message, packetBuffer + 12, ANX_ALERT_MAX_LENGTH);

  //Process alerts with valid hop counts
  if (alert.hops == 0) {
    return;
  }

  //*basic* meshing for alerts
  if (alert.hops < MAX_HOPS) {
    bool newAlert = true;
    uint16_t alertHash = (alert.src << 8) | alert.sequence;

    //Walk through alert log, check for any previous alerts
    for (uint8_t i = 0; i < ALERT_LOG_MAX; i++) {
      if (alertLog[i] == alertHash) newAlert = false;
    }

    //If this is a new alert, repeat and store
    if (newAlert) {
      alert.hops++;

      //Replay the alert
      ANXRFSend(&alert, sizeof(alert));

      //Store the alert
      alertLog[alertLogPtr] = alertHash;

      //Move the pointer, wrapping around
      alertLogPtr = (alertLogPtr + 1) % ALERT_LOG_MAX;
    }
  }

  //Stop allowing alerts while UI shown
  allowAlerts =  false;

  //Build the UI for the alerts
  char title[32];
  snprintf(title, 20, "ALERT from @ %s", alert.name);
  window(title);
  display.setCursor(0, 24);
  display.setTextWrap(true);
  display.print(alert.message);
  display.setTextWrap(false);
  printAlignRight("Ok -- > ", display.height() - ANX_FONT_HEIGHT);
  safeDisplay();

  //Wait for user to acknoledge the alert
  while (waitForButton() != BUTTON_RIGHT);
  clearButtonState();

  //Copy display buffer back to the display
  memcpy(display.getBuffer(), displayBuffer, buffSize);
  safeDisplay();

  allowAlerts = true;
}

/**
  handle chat packets received
*/
void ANXChatHandler(uint8_t length) {
  if (length != 42) {
    return;
  }

  //Reconstruct a chat packet from the radio
  ChatPacket chat;
  chat.src = packetBuffer[1];
  chat.sequence = packetBuffer[2];
  memcpy(chat.name, packetBuffer + 3, USERNAME_MAX_LENGTH);
  memcpy(chat.message, packetBuffer + 11, ANX_CHAT_MAX_LENGTH);

  bool newMessage = true;

  //Determine if this is new or not
  for (uint8_t i = 0; i < CHAT_MESSAGES_MAX; i++) {
    ChatMessage m = getFromChatBuffer(i);
    if (strcmp(m.name, chat.name) == 0 &&
        strcmp(m.message, chat.message) == 0) {
      newMessage = false;
      break;
    }
  }

  //Since chat messages are sent three times, only do anything if it's a new message
  if (newMessage) {
    //Grab the important parts for the UI chat message
    ChatMessage message;
    memcpy(message.name, chat.name, USERNAME_MAX_LENGTH);
    memcpy(message.message, chat.message, ANX_CHAT_MAX_LENGTH);
    message.received = rtMillis();

    //Drop it in the buffer
    addToChatBuffer(message);

    //Possibly dump it to serial too
    if (chatToTerminal) {
      Serial.print("\n\r#AND!XOR Chat @"); Serial.print(message.name); Serial.print(": "); Serial.println(message.message);
    }
  }
}

/**
  Handle all hello packets here by recording other nodes
*/
void ANXHelloHandler(uint8_t length, int16_t rssi) {
  if (length != HELLO_PACKET_SIZE) return;

  HelloPacket hello;
  hello.src = packetBuffer[1];
  hello.sequence = packetBuffer[2];
  hello.level = packetBuffer[3];
  memcpy(hello.name, packetBuffer + 4, USERNAME_MAX_LENGTH);

  //Update the peers
  peers[hello.src].nodeid = hello.src;
  memcpy(peers[hello.src].name, hello.name, USERNAME_MAX_LENGTH);
  peers[hello.src].lastSeen = rtMillis();
  peers[hello.src].rssi = rssi;
  peers[hello.src].level = hello.level;

  //If the person is close, scroll their name if the user is interruptable
  if (rssi > -40 && allowHello) {
    allowHello = false;
    //Save the display buffer
    uint32_t buffSize = (display.height() * display.width()) / 8;
    uint8_t displayBuffer[buffSize];
    memcpy(displayBuffer, display.getBuffer(), buffSize);

    //say hello
    char text[USERNAME_MAX_LENGTH + 10];
    memset(text, '\0', USERNAME_MAX_LENGTH + 10);
    sprintf(text, "HELLO %s", hello.name);
    scroll(text, 0x0, false, false);

    //Copy display buffer back to the display
    memcpy(display.getBuffer(), displayBuffer, buffSize);
    safeDisplay();
    allowHello = true;
  }
}

/**
  Handle master packets received
*/
void ANXMasterHandler(uint8_t length) {
  if (length != MASTER_PACKET_SIZE) return;
  if (!allowMaster) return;

  //ignore others
  allowMaster = false;

  MasterPacket packet;
  packet.src = packetBuffer[1];
  packet.dest = packetBuffer[2];
  packet.sequence = packetBuffer[3];
  packet.type = packetBuffer[4];
  memcpy(packet.name, packetBuffer + 5, USERNAME_MAX_LENGTH);

  //If this was not destined for us drop it
  if (packet.dest != getNodeID() && packet.dest != 0xFF) {
    return;
  }

  //Save the display buffer
  uint32_t buffSize = (display.height() * display.width()) / 8;
  uint8_t displayBuffer[buffSize];
  memcpy(displayBuffer, display.getBuffer(), buffSize);

  //Unlock their badge
  if (packet.type == MASTER_TYPE_UNLOCK) {
    ANXSetUnlocked(ANXGetUnlocked() | UNLOCK_MASTER);
    char b[32];
    sprintf(b, " %s\nJust unlocked\nYour badge!", packet.name);
    statusDialog(b);
  }
  //Level them up
  else if (packet.type == MASTER_TYPE_LEVELUP) {
    addExperience(XP_PER_LEVEL);
    char b[32];
    sprintf(b, " %s\nJust leveled up\nYour badge!", packet.name);
    statusDialog(b);

  }
  //Rick Roll
  else if (packet.type == MASTER_TYPE_TROLL_RICK) {
    rick();
    char b[32];
    sprintf(b, "Rick Rolled by\n %s", packet.name);
    statusDialog(b);
  }

  safeWaitForButton();

  //Copy display buffer back to the display
  memcpy(display.getBuffer(), displayBuffer, buffSize);
  safeDisplay();

  allowMaster = true;
}

/**
  Handle ninja packets by dropping them in the queue
*/
void ANXNinjaHandler(uint8_t length) {
  if (length != 7) {
    return;
  }

  NinjaPacket packet;
  packet.src = packetBuffer[1];
  packet.dest = packetBuffer[2];
  packet.gameid = packetBuffer[3];
  packet.round = packetBuffer[4];
  packet.type = packetBuffer[5];
  packet.data = packetBuffer[6];

  //If this was not destined for us drop it
  if (packet.dest != getNodeID()) {
    return;
  }

  lastNinjaPacket = packet;
  ninjaPacketAvailable = true;

  //If it's a challenge packet ask the user
  if (packet.type == TYPE_INIT && allowNinja) {
    //Save the display buffer
    uint32_t buffSize = (display.height() * display.width()) / 8;
    uint8_t displayBuffer[buffSize];
    memcpy(displayBuffer, display.getBuffer(), buffSize);

    allowNinja = false;
    ninjaChallenged(packet);
    allowNinja = true;

    //Copy display buffer back to the display
    memcpy(display.getBuffer(), displayBuffer, buffSize);
    safeDisplay();
  }
}

/**
  Handle pings by responding
*/
void ANXPingHandler(uint8_t length) {
  if (length != PING_PACKET_SIZE) {
    return;
  }

  PingPacket packet;
  packet.src = packetBuffer[1];
  packet.dest = packetBuffer[2];
  packet.seq = packetBuffer[3];
  packet.type = packetBuffer[4];

  //If this was not destined for us drop it
  if (packet.dest != getNodeID()) {
    return;
  }

  //Return the ping
  if (packet.type == PING_TYPE_PING) {
    packet.dest = packet.src;
    packet.src = getNodeID();
    packet.type = PING_TYPE_PONG;

    deepSleep(100);
    ANXRFSend(&packet, PING_PACKET_SIZE);
  } else if (packet.type == PING_TYPE_PONG) {
    //Store it for anybody that wants to use it
    pingPacketAvailable = true;
    lastPingPacket = packet;
  }
}

/**
  Determine if radio available
*/
bool ANXRFAvailable() {
  //Short cut if init work when the badge was turned on
  if (!radioInitPassed) {
    return false;
  }

  uint8_t synth = (radio.readReg(REG_OPMODE) & RF_OPMODE_SYNTHESIZER);
  uint8_t irq1 = radio.readReg(REG_IRQFLAGS1);
  uint8_t ready = (irq1 & RF_IRQFLAGS1_MODEREADY) >> 7;

  //If radio isnt ready delay just a bit and check again
  if (ready == 0) {
    deepSleep(10);
    irq1 = radio.readReg(REG_IRQFLAGS1);
    ready = (irq1 & RF_IRQFLAGS1_MODEREADY) >> 7;
  }

  return (ready == 1) && (synth == 0);
}

/**
  Put the radio to sleep to save power
*/
void ANXRFSleep() {
  radio.sleep();
}

/**
  RF Debug display for the user
  Show simple live data of what's occuring on the network
*/
void doRFDebug() {
  while (1) {

    uint8_t opmode = radio.readReg(REG_OPMODE);
    uint8_t irq1 = radio.readReg(REG_IRQFLAGS1);
    uint8_t irq2 = radio.readReg(REG_IRQFLAGS2);
    char title[20];
    char o = '?';
    uint8_t pll = (irq1 & RF_IRQFLAGS1_PLLLOCK) >> 4;
    uint8_t seq = opmode >> 7;
    uint8_t ready = (irq1 & RF_IRQFLAGS1_MODEREADY) >> 7;

    if ((opmode & RF_OPMODE_SYNTHESIZER) > 0) o = 'f';
    else if ((opmode & RF_OPMODE_TRANSMITTER) > 0) o = 't';
    else if ((opmode & RF_OPMODE_STANDBY) > 0) o = 's';
    else if ((opmode & RF_OPMODE_RECEIVER) > 0) o = 'r';
    else {
      Serial.print("Unknown RF opmode: 0x"); Serial.println(opmode,BIN);
    }
    

    sprintf(title, "RF Debug M%c P%d R%d S%d", o, pll, ready, seq);
    window(title);

    int16_t y = display.height();
    for (uint8_t i = 0; i < PACKET_LOG_MAX; i++) {

      //Walk backwards from the log pointer (which points to the *next* position in the packetBuffer
      LoggedPacket packet = packetLog[(logPtr - 1 - i + PACKET_LOG_MAX) % PACKET_LOG_MAX];

      //Only display valid packets
      if (packet.rssi < 0) {
        //Move up two rows
        y -= (ANX_FONT_HEIGHT * 2);
        //If we will overlap with the title bar at all, quit
        if (y < ANX_FONT_HEIGHT) break;
        display.setCursor(0, y);

        switch (packet.port) {
          case PORT_HELLO:
            display.print("HELLO ");
            break;
          case PORT_CHAT:
            display.print("CHAT ");
            break;
          case PORT_ALERT:
            display.print("ALERT ");
            break;
          case PORT_PING:
            display.print("PING ");
            break;
          case PORT_NINJA:
            display.print("NINJA ");
            break;
          default:
            display.print("UNK ["); display.print(packet.port); display.print("] ");
            break;
        }

        uint32_t t = (rtMillis() - packet.timestamp) / 1000;

        display.print("from "); display.println(packet.src);

        display.print("  ");
        display.print(packet.rssi); display.print("dBm - ");
        display.print(t); display.println(" sec(s)");
      }
    }

    safeDisplay();

    uint8_t button = getButtonState();
    if (button > 0) {
      clearButtonState();
      return;
    }

    deepSleep(100);
    tick();
  }
}

/**
   Super simple spectrum analyzer mode. Each bucket is a column of on the display. No FFT here
   either, just basic averaging on each column so each spike is persistent enough so the user
   sees it.
*/
void doSpectrumAnalyzer() {
  //Sample as wide as we can
  uint8_t origRecvBW = radio.readReg(REG_RXBW);
  radio.writeReg(REG_RXBW, RF_RXBW_DCCFREQ_000 | RF_RXBW_MANT_16 | RF_RXBW_EXP_0);

  uint8_t bins = display.width();
  uint32_t origFreq = radio.getFrequency();
  int16_t minDB = -120;
  int16_t maxDB = -30;
  uint32_t freqStep = 125000;

  //Current freq should be in the middle of the sample
  uint32_t startFreq = origFreq - (bins * freqStep / 2);
  uint32_t freq = startFreq;
  uint8_t i = 0;
  int16_t y = 0; //Start point (top) of spectrum line
  int16_t y0 = 0; //Start point (top) of previous
  int16_t h = 0; //Height (value) of spectrum line
  int16_t values[bins];

  //Y coord of threshold line
  uint8_t threshY = map(CSMA_LIMIT, minDB, maxDB, display.height(), 0);

  //Text buffer for labels
  char buffer[50];

  //Init the values
  for (int j = 0; j < bins; j++) {
    values[j] = 0;
  }

  int16_t value;

  display.clearDisplay();
  while (1) {
    freq = startFreq + (i * freqStep);
    radio.setFrequency(freq);
    delay(1);
    radio.setMode(RF69_MODE_RX);
    uint32_t t = rtMillis();
    uint8_t opmode = radio.readReg(REG_OPMODE);
    uint8_t irq1 = radio.readReg(REG_IRQFLAGS1);

    //Make a weak attempt to lock PLL since frequency was just changed
    //But don't try too hard and slow down display
    uint8_t pll = (irq1 & RF_IRQFLAGS1_PLLLOCK) >> 4;
    if (pll == 0) {
      deepSleep(20);
    }

    //Make a weak attempt to get radio ready since frequency was just changed
    //But don't try too hard and slow down display
    uint8_t ready = (irq1 & RF_IRQFLAGS1_MODEREADY) >> 7;
    if (ready == 0) {
      deepSleep(20);
    }

    //Read RSSI value off of radio at the current frequency
    value = radio.readRSSI();

    h = map(value, minDB, maxDB, 0, display.height());

    //Rolling average
    //    values[i] = floor((values[i] + y) / 2);
    if (h < values[i]) {
      values[i] = max((float)(values[i]) * .9, h);
    } else {
      values[i] = h;
    }
    y = display.height() - values[i];

    //Debug raw values, slows everything way down
    //        Serial.print(" ["); Serial.print(radio.getFrequency()); Serial.print("]"); Serial.print(value); Serial.print(" - h = "); Serial.print(h); Serial.print(" - y = "); Serial.println(y);

    //Advance i
    i = (i + 1) % bins;

    if (i == 0) {
      display.clearDisplay();
      int16_t peak = -200;
      uint32_t peakFreq = 0;
      for (int j = 1; j < bins; j++) {
        //        display.drawFastVLine(j, 0, display.height(), BLACK);
        y0 = display.height() - values[j - 1];
        y = display.height() - values[j];

        display.drawFastVLine(j, y, values[j], WHITE);
        //display.drawLine(j - 1, y0, j, y, WHITE);
        if (values[j] > peak) {
          peak = values[j];
          peakFreq = startFreq + (j * freqStep);
        }
      }
      //Print maxDB
      display.setTextColor(INVERSE);
      display.setCursor(0, 0);
      display.print(maxDB); display.print("dBm");

      //Print minDB
      display.setCursor(0, display.height() - ANX_FONT_HEIGHT);
      display.print(minDB); display.print("dBm");

      //Print Freq Rang
      sprintf(buffer, " %d - %dMhz", startFreq / 1000000, (startFreq + (bins * freqStep)) / 1000000);
      uint8_t labelx = display.width() - (ANX_FONT_WIDTH * strlen(buffer));
      display.setCursor(display.width() - (strlen(buffer)*ANX_FONT_WIDTH), display.height() - ANX_FONT_HEIGHT);
      display.print(buffer);

      //Print thresh DB
      display.setCursor(0, threshY - ANX_FONT_HEIGHT);
      display.print(CSMA_LIMIT); display.print("dBm");
      display.drawFastHLine(0, threshY, display.width(), WHITE);

      safeDisplay();
    }

    if (getButtonState() > 0) {
      clearButtonState();
      break;
    }
  }

  //Cleanup
  radio.writeReg(REG_RXBW, origRecvBW);
  radio.setFrequency(origFreq);
}

/**
   Helper function to ignore all RF-induced popups that might annoy the user or interrupt their usage
*/
void disablePopups() {
  allowAlerts = false;
  allowMaster = false;
  allowHello = false;
  allowNinja = false;
}

/**
   Helper function to allow all RF-induced popups that might annoy the user or interrupt their usage
*/
void enablePopups() {
  allowAlerts = true;
  allowMaster = true;
  allowHello = true;
  allowNinja = true;
}

