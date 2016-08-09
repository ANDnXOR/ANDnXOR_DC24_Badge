#include "ninja.h"


//Matrix that stores all possible game results
//PUNCH = 0
//KICK = 1
//SHIELD =2
//NONE=3
//TROLL=4
//Player 1 = row
//Player 2 = col
static uint8_t results[5][5] = {
  {RESULT_TIE, RESULT_WIN, RESULT_LOSE, RESULT_WIN, RESULT_LOSE},
  {RESULT_LOSE, RESULT_TIE, RESULT_WIN, RESULT_WIN, RESULT_LOSE},
  {RESULT_WIN, RESULT_LOSE, RESULT_TIE, RESULT_WIN, RESULT_LOSE},
  {RESULT_LOSE, RESULT_LOSE, RESULT_LOSE, RESULT_TIE, RESULT_LOSE},
  {RESULT_WIN, RESULT_WIN, RESULT_WIN, RESULT_WIN, RESULT_TIE}
};


/**
   Handle packets, updating game state as appropriate
*/
static bool _handlePackets(NinjaState *state, NinjaPacket *packet) {
  //Get a packet
  if (ninjaPacketAvailable) {

    //If we received a move packet, do something with it and ACK
    if (lastNinjaPacket.type == TYPE_MOVE) {

      //Only accept moves for this round
      if (lastNinjaPacket.round == state->round) {
        state->p2Move = lastNinjaPacket.data;

        //ACK the move
        packet->type = TYPE_ACK;
        packet->data = TYPE_MOVE;   //ACK the move

        //Try to send the packet 5 times
        for (uint8_t i = 0; i < 5; i++) {
          //Delay a bit
          deepSleep(random(30) + 40);
          if (ANXRFSend(packet, NINJA_PACKET_SIZE)) break;
        }
      }
    }

    //ACK any latent INIT packets (challenger never got our previous ACKs)
    if (lastNinjaPacket.type == TYPE_INIT &&
        state->state == STATE_SELECT_MOVE &&
        lastNinjaPacket.src == state->p2nodeid) {
      packet->type = TYPE_ACK;
      packet->dest = state->p2nodeid;
      packet->src = getNodeID();
      packet->data = TYPE_INIT;

      //Try to send the packet 5 times
      for (uint8_t i = 0; i < 5; i++) {
        //Delay a bit
        deepSleep(random(30) + 40);
        if (ANXRFSend(packet, NINJA_PACKET_SIZE)) break;
      }
    }

    //Handle if it's an ACK
    if (lastNinjaPacket.type == TYPE_ACK) {
      state->lastACK = lastNinjaPacket.data;
    }

    //Handle if it's a NACK
    if (lastNinjaPacket.type == TYPE_NACK) {
      state->lastNACK = lastNinjaPacket.data;
    }

    //Reset packet buffer
    ninjaPacketAvailable = false;
  }
}

/**
   Animate and perform the fight routine
*/
static void _fight(NinjaState *state, NinjaPacket *packet) {

  uint8_t result = results[state->p1Move][state->p2Move];
  bool up = true;
  char username[USERNAME_MAX_LENGTH];
  ANXGetUsername(username);

  //Animate the players getting ready to fight
  for (uint8_t i = 0; i < 6; i++) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(state->p1Score);
    display.setCursor(120, 0);
    display.print(state->p2Score);
    display.setCursor(40, 28);
    display.print("Round "); display.print(state->round);
    display.drawFastHLine(0, display.height() - 10, display.width(), WHITE);

    //Draw the usernames
    display.setCursor(0, 56);
    display.print(username);
    printAlignRight(state->p2Name, 56);

    //Animate the players idling
    if (up) {
      drawBitmapFlash(state->p1Idle1, PLAYER_1_X, PLAYER_1_Y);
      drawBitmapFlash(state->p2Idle1, PLAYER_2_X, PLAYER_2_Y);
    } else {
      drawBitmapFlash(state->p1Idle2, PLAYER_1_X, PLAYER_1_Y);
      drawBitmapFlash(state->p2Idle2, PLAYER_2_X, PLAYER_2_Y);
    }
    up = !up;
    safeDisplay();

    for (uint8_t i = 0; i < 15; i++) {
      deepSleep(25);
      tick();
      _handlePackets(state, packet);
    }
  }

  for (uint8_t i = 0; i < 6; i++) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(state->p1Score);
    display.setCursor(120, 0);
    display.print(state->p2Score);
    display.drawFastHLine(0, display.height() - 10, display.width(), WHITE);

    //Draw the names
    display.setCursor(0, 56);
    display.print(username);
    printAlignRight(state->p2Name, 56);

    //Setup the animation graphics
    if (up) {
      if (state->p1Move == MOVE_PUNCH)   drawBitmapFlash(state->p1Punch1,     37, PLAYER_1_Y);
      if (state->p1Move == MOVE_KICK)    drawBitmapFlash(state->p1Kick1,      37, PLAYER_1_Y);
      if (state->p1Move == MOVE_NONE)    drawBitmapFlash(state->p1Idle1,      37, PLAYER_1_Y);
      if (state->p1Move == MOVE_SHIELD)  drawBitmapFlash(state->p1Shield1,    37, PLAYER_1_Y);

      if (state->p2Move == MOVE_PUNCH)   drawBitmapFlash(state->p2Punch2,     64, PLAYER_2_Y);
      if (state->p2Move == MOVE_KICK)    drawBitmapFlash(state->p2Kick2,      64, PLAYER_2_Y);
      if (state->p2Move == MOVE_NONE)    drawBitmapFlash(state->p2Idle2,      64, PLAYER_2_Y);
      if (state->p2Move == MOVE_SHIELD)  drawBitmapFlash(state->p2Shield2,    64, PLAYER_2_Y);
    } else {
      if (state->p1Move == MOVE_PUNCH)   drawBitmapFlash(state->p1Punch2,     37, PLAYER_1_Y);
      if (state->p1Move == MOVE_KICK)    drawBitmapFlash(state->p1Kick2,      37, PLAYER_1_Y);
      if (state->p1Move == MOVE_NONE)    drawBitmapFlash(state->p1Idle2,      37, PLAYER_1_Y);
      if (state->p1Move == MOVE_SHIELD)  drawBitmapFlash(state->p1Shield2,    37, PLAYER_2_Y);

      if (state->p2Move == MOVE_PUNCH)   drawBitmapFlash(state->p2Punch1,     64, PLAYER_2_Y);
      if (state->p2Move == MOVE_KICK)    drawBitmapFlash(state->p2Kick1,      64, PLAYER_2_Y);
      if (state->p2Move == MOVE_NONE)    drawBitmapFlash(state->p2Idle1,      64, PLAYER_2_Y);
      if (state->p2Move == MOVE_SHIELD)  drawBitmapFlash(state->p2Shield1,    64, PLAYER_2_Y);
    }

    if (state->p1Move == MOVE_TROLL)     drawBitmapFlash(state->troll,        23, PLAYER_1_Y);
    if (state->p2Move == MOVE_TROLL)     drawBitmapFlash(state->troll,        64, PLAYER_2_Y);

    up = !up;
    safeDisplay();

    for (uint8_t i = 0; i < 5; i++) {
      deepSleep(100);
      tick();
    }
  }



  display.clearDisplay();
  display.drawFastHLine(0, display.height() - 10, display.width(), WHITE);

  //Draw the names
  display.setCursor(0, 56);
  display.print(username);
  printAlignRight(state->p2Name, 56);

  //Check the tiebreaker
  if (result == RESULT_TIE) {
    if (state->tiebreaker[state->round - 1]) {
      result = RESULT_WIN;
    } else {
      result = RESULT_LOSE;
    }
  }

  switch (result) {
    case RESULT_WIN:
      display.setCursor(67, 28);
      display.print("You Win!");
      state->p1Score++;
      drawBitmapFlash(state->p1Idle1,    37, PLAYER_1_Y);
      drawBitmapFlash(state->p2Dead,     64, PLAYER_2_Y);
      break;
    case RESULT_LOSE:
      display.setCursor(4, 28);
      display.print("You Lose :(");
      state->p2Score++;
      drawBitmapFlash(state->p1Dead,    37, PLAYER_1_Y);
      drawBitmapFlash(state->p2Idle1,     64, PLAYER_2_Y);
      break;
  }

  display.setCursor(0, 0);
  display.print(state->p1Score);
  display.setCursor(120, 0);
  display.print(state->p2Score);

  safeDisplay();

  //3 second delay
  for (uint8_t i = 0; i < 15; i++) {
    deepSleep(200);
    tick();
  }
}

/**
   Handle game over event
*/
static void _gameOver(NinjaState *state) {
  bool win = (state->p1Score == 2);

  if (win) {
    setAllLeds(0, 255, 0);
  } else {
    setAllLeds(255, 0, 0);
  }

  int16_t xp = 0;

  char buffer[32];

  if (win) {
    xp += 15;
    int16_t bonus = 6 * (state->p2Level - ANXGetLevel());
    if (bonus < 0) bonus = 0;
    xp += bonus;
    sprintf(buffer, "Victory!\n%d XP", xp);
  } else {
    xp += state->p1Score * 5;
    sprintf(buffer, "Fail :(\n%d XP", xp);
  }

  //Give them credit
  xp = max(0, xp);
  addExperience(xp);

  statusDialog(buffer);
  safeDisplay();
  safeWaitForButton();

  ledsOff();
}

/**
   GUI to get the player moves
*/
static uint8_t _getPlayerMove(NinjaState * state, NinjaPacket * packet) {
  uint32_t endTime = rtMillis() + 20000;

  while (1) {
    display.clearDisplay();
    drawBitmapFlash(state->punch, 8, 0);
    drawBitmapFlash(state->kick, 48, 0);
    drawBitmapFlash(state->shield, 88, 0);
    drawBitmapFlash(state->right, 8, 32);
    drawBitmapFlash(state->down, 48, 32);
    drawBitmapFlash(state->up, 88, 32);

    //Time left
    display.setCursor(0, 0);
    display.print((endTime - rtMillis()) / 1000);

    safeDisplay();

    uint8_t button = getButtonState();
    if ((button & BUTTON_RIGHT) > 0) {
      clearButtonState();
      return MOVE_PUNCH;
    } else if ((button & BUTTON_DOWN) > 0) {
      clearButtonState();
      return MOVE_KICK;
    } else if ((button & BUTTON_UP) > 0) {
      clearButtonState();
      return MOVE_SHIELD;
    }

#ifdef MASTER
    else if ((button & BUTTON_ENTER) > 0) {
      clearButtonState();
      return MOVE_TROLL;
    }
#endif

    deepSleep(200);
    tick();

    _handlePackets(state, packet);

    //If they're too slow, don't move
    if (endTime < rtMillis()) {
      return MOVE_NONE;
    }
  }
}

/**
   Primary state machine for ninja fight game handles exchange of packets and setting up game
   between the two players
*/
static void _doNinja(NinjaState *state, NinjaPacket *packet) {
  uint32_t endTime = 0;

  //Init the game state
  state->p1Idle1 = getBitmapMetadata(NINJA_P1_IDLE1_address);
  state->p2Idle1 = getBitmapMetadata(NINJA_P2_IDLE1_address);
  state->p1Idle2 = getBitmapMetadata(NINJA_P1_IDLE2_address);
  state->p2Idle2 = getBitmapMetadata(NINJA_P2_IDLE2_address);

  state->p1Punch1 = getBitmapMetadata(NINJA_P1_PUNCH1_address);
  state->p2Punch1 = getBitmapMetadata(NINJA_P2_PUNCH1_address);
  state->p1Punch2 = getBitmapMetadata(NINJA_P1_PUNCH2_address);
  state->p2Punch2 = getBitmapMetadata(NINJA_P2_PUNCH2_address);

  state->p1Kick1 = getBitmapMetadata(NINJA_P1_KICK1_address);
  state->p2Kick1 = getBitmapMetadata(NINJA_P2_KICK1_address);
  state->p1Kick2 = getBitmapMetadata(NINJA_P1_KICK2_address);
  state->p2Kick2 = getBitmapMetadata(NINJA_P2_KICK2_address);

  state->p1Shield1 = getBitmapMetadata(NINJA_P1_SHIELD1_address);
  state->p2Shield1 = getBitmapMetadata(NINJA_P2_SHIELD1_address);
  state->p1Shield2 = getBitmapMetadata(NINJA_P1_SHIELD2_address);
  state->p2Shield2 = getBitmapMetadata(NINJA_P2_SHIELD2_address);

  state->p1Dead = getBitmapMetadata(NINJA_P1_DEAD_address);
  state->p2Dead = getBitmapMetadata(NINJA_P2_DEAD_address);

  state->troll = getBitmapMetadata(NINJA_DT_address);

  state->punch = getBitmapMetadata(NINJA_PUNCH_address);
  state->kick = getBitmapMetadata(NINJA_KICK_address);
  state->shield = getBitmapMetadata(NINJA_SHIELD_address);
  state->up = getBitmapMetadata(NINJA_UP_address);
  state->down = getBitmapMetadata(NINJA_DOWN_address);
  state->right = getBitmapMetadata(NINJA_RIGHT_address);
  state->state = STATE_SELECT_MOVE;
  state->round = 1;
  state->p1Score = 0;
  state->p2Score = 0;

  while (state->p1Score < 2 && state->p2Score < 2) {
    if (state->state == STATE_SELECT_MOVE) {
      state->lastACK = -1;
      //Set p2Move to -1 *before* _getPlayerMove (which could change it!)
      state->p2Move = -1; //other player move
      state->p1Move = _getPlayerMove(state, packet);


      endTime = rtMillis() + 20000;
      bool ackRecv = false;
      bool moveRecv = false;

      //Wait for a bit sending and receiving until we're synced with other player
      while (state->lastACK != TYPE_MOVE || state->p2Move < 0) {

        //Show a dialog to the player
        char wait[32];
        sprintf(wait, "Waiting for\nplayer %d", (endTime - rtMillis()) / 1000);
        statusDialog(wait);
        safeDisplay();

        //Keep sending until an ACK is received
        if (state->lastACK != TYPE_MOVE) {
          //Fill out the rest of the packet
          packet->type = TYPE_MOVE;
          packet->data = state->p1Move;
          packet->round = state->round;
          ANXRFSend(packet, NINJA_PACKET_SIZE);
        }

        //Delay and do some ticking (400ms)
        for (uint8_t i = 0; i < 16; i++) {
          deepSleep(25);
          tick();
          //Process any data that comes in
          _handlePackets(state, packet);
        }

        //give up on other player
        if (rtMillis() > endTime) {
          state->state = STATE_ABORT;
          break;
        }
      }

      //Bump them over to fight
      if (state->state != STATE_ABORT) state->state = STATE_FIGHT;

    } else if (state->state == STATE_FIGHT) {
      _fight(state, packet);
      state->state = STATE_SELECT_MOVE;
      state->round++;
    } else if (state->state == STATE_ABORT) {
      break;
    }

    deepSleep(200);
    tick();
  }

  _gameOver(state);
  enablePopups();
}

/**
   Challenge another player at ninja
*/
void ninja() {
  //Used to store game state
  NinjaState state;
  state.lastACK = -1;
  state.lastNACK = -1;

  //Used as a timer to timeout of things
  uint32_t endTime;

  //Reset game packet state
  ninjaPacketAvailable = false;

  //Select a peer
  int16_t nodeid = getPeerFromUser();
  if (nodeid == -1) {
    return;
  }

  //Up until this point we can interrupt the user
  disablePopups();

  //Establish game state
  PeerNode peer = peers[nodeid];
  state.p2nodeid = peer.nodeid;
  state.p2Level = peer.level;
  state.p2Name = peer.name;

  //Pre-determine tiebreakers by round based on selected player
  int16_t wthreshold = (100 * (ANXGetLevel() - state.p2Level + 3)) / 6;
  for (uint8_t i = 0; i < 3; i++) {
    state.tiebreaker[i] = random(100) <= wthreshold;
  }

  //Construct most of the packet now, we'll re-use it later
  NinjaPacket packet;
  packet.port = PORT_NINJA;
  packet.src = getNodeID();
  packet.dest = peer.nodeid;
  packet.gameid = random(256);

  //Setup game with another player
  packet.type = TYPE_INIT;
  packet.data = state.tiebreaker[0] << 2 | state.tiebreaker[1] << 1 | state.tiebreaker[2];

  //Wait for the other player to accept
  endTime = rtMillis() + 20000;
  char challenge[40];
  memset(challenge, '\0', 40);
  sprintf(challenge, "Challenging\n%s\nLevel %d...", peer.name, peer.level);
  statusDialog(challenge);
  safeDisplay();

  //Wait for ack
  while (state.lastACK != TYPE_INIT) {
    ANXRFSend(&packet, NINJA_PACKET_SIZE);

    //Roughly wait 200ms until next send
    for (uint8_t i = 0; i < 8; i++) {
      tick();
      _handlePackets(&state, &packet);
      deepSleep(random(80));
    }

    //Quit if time expires
    if (rtMillis() > endTime) {
      statusDialog("Challenge\nTimed out.");
      safeWaitForButton();
      enablePopups();
      return;
    }

    //Quit if NACKed by other user
    if ( state.lastNACK == TYPE_INIT) {
      statusDialog("Challenge\nRejected :(");
      safeWaitForButton();
      enablePopups();
      return;
    }
  }

  //Play the game
  _doNinja(&state, &packet);
}



/**
   Special entry point if the player is challeged
*/
void ninjaChallenged(NinjaPacket packet) {
  //Ignore RF-related popups while they play
  disablePopups();
  bool accept = false;
  PeerNode peer;
  NinjaState state;

  //Find the peer
  for (uint8_t i = 0; i < PEER_NODES_MAX; i++) {
    if (peers[i].lastSeen > 0) {
      if (peers[i].nodeid == packet.src) {
        peer = peers[i];
        char b[50];
        sprintf(b, "Accept Ninja\nChallenge from\n%s?", packet.src);
        accept = yesNoDialog(b);
      }
    }
  }

  //If the peer is known and the user accepts, ACK
  if (accept) {
    packet.type = TYPE_ACK;
    state.p2nodeid = peer.nodeid;
    state.p2Level = peer.level;
    state.p2Name = peer.name;
    state.lastACK = -1;
    state.lastNACK = -1;

    //Unwrap the tiebreaker
    if ((packet.data & B00000100) > 0) state.tiebreaker[0] = false; else state.tiebreaker[0] = true;
    if ((packet.data & B00000010) > 0) state.tiebreaker[1] = false; else state.tiebreaker[1] = true;
    if ((packet.data & B00000001) > 0) state.tiebreaker[2] = false; else state.tiebreaker[2] = true;
  } else {
    packet.type = TYPE_NACK;
  }

  //Repurpose the packet to send it back
  packet.dest = packet.src;
  packet.src = getNodeID();
  packet.data = TYPE_INIT;

  //Send the NACK or ACK three times
  for (uint8_t i = 0; i < 5; i + 0) {
    if (ANXRFAvailable()) {
      ANXRFSend(&packet, NINJA_PACKET_SIZE);
      i++;
    }
    deepSleep(random(100) + 100);
    tick();
  }

  //Play the game if they ack
  if (accept) {
    _doNinja(&state, &packet);
  }

  enablePopups();
}

