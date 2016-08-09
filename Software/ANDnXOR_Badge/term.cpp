#include <Adafruit_NeoPixel-ANDnXOR.h>
#include <Arduino.h>
#include <ntlibc.h>
#include <ntopt.h>
#include <ntshell.h>

#include "anim.h"
#include "ANX.h"
#include "life.h"
#include "rf.h"
#include "serial.h"
#include "settings.h"
#include "term.h"

#define UNUSED_VARIABLE(N)  do { (void)(N); } while (0)

extern Adafruit_NeoPixel  leds;

typedef int (*USRCMDFUNC)(int argc, char **argv);
uint8_t who_count = 0;
static int terminal_chat(int argc, char **argv);
static int terminal_emacs(int argc, char **argv);
static int terminal_vim(int argc, char **argv);
static int terminal_exit(int argc, char **argv);
static int terminal_help(int argc, char **argv);
static int terminal_led(int argc, char **argv);
static int terminal_ls(int argc, char **argv);
static int terminal_peer(int argc, char **argv);
static int terminal_ping(int argc, char **argv);
static int terminal_play(int argc, char **argv);
static int terminal_rm(int argc, char **argv);
static int terminal_set(int argc, char **argv);
static int terminal_sudo(int argc, char **argv);
static int terminal_uname(int argc, char **argv);
static int terminal_whoami(int argc, char **argv);

typedef struct {
  char *cmd;
  char *desc;
  USRCMDFUNC func;
} cmd_table_t;

static bool quit = false;

static const cmd_table_t cmdlist[] = {
  { "chat", "Private AND!XOR chat room", terminal_chat },
  { "emacs", "GNU Emacs editor", terminal_emacs},
  { "vim", "The better? editor", terminal_vim},
  { "exit", "Exit terminal mode", terminal_exit},
  { "help", "This is a description text string for help command.", terminal_help },
  { "led", "Set the color of the LEDs.", terminal_led},
  { "ls", "List directory contents", terminal_ls},
  { "peer", "List and show details on known peer badges", terminal_peer},
  { "ping", "Ping another badge", terminal_ping},
  { "play", "Play an animation", terminal_play},
  { "rm", "Reset badge to defaults", terminal_rm},
  { "set", "Change a setting", terminal_set},
  { "sudo", "Elevate permissions", terminal_sudo},
  { "uname", "Print system information", terminal_uname},
  { "whoami", "Become one with yourself", terminal_whoami}
};
#define CMD_LIST_SIZE 15
/**
   Required by NT-Shell, callback to read a given number of bytes from serial
*/
static int func_read(char *buf, int cnt, void *extobj) {
  int i;
  UNUSED_VARIABLE(extobj);
  for (i = 0; i < cnt; i++) {
    buf[i] = Serial.read();
  }
  tick();
  return cnt;
}

/**
   Required by NT-Shell, callback to write a given number of bytes to serial
*/
static int func_write(const char *buf, int cnt, void *extobj) {
  int i;
  UNUSED_VARIABLE(extobj);
  for (i = 0; i < cnt; i++) {
    Serial.write(buf[i]);
  }
  return cnt;
}

static int ntOptionCallback(int argc, char **argv, void *extobj) {
  if (argc == 0) {
    return 0;
  }
  const cmd_table_t *p = &cmdlist[0];

  for (int i = 0; i < sizeof(cmdlist) / sizeof(cmdlist[0]); i++) {
    if (strcmp((const char *)argv[0], p->cmd) == 0) {
      return p->func(argc, argv);
    }
    p++;
  }
  Serial.println("Unknown command found.\r\n");
  return 0;
}

static int ntPromptCallback(const char *text, void *extobj) {
  return ntopt_parse(text, ntOptionCallback, 0);
}

/**
   Run the shell
   This is a customized version of ntshell's implementation
*/
static void executeShell(ntshell_t *ntshell) {

  /*
    Check the initialization code.
  */
  if (ntshell->initcode != INITCODE) {
    return;
  }

  /*
     User input loop.
  */
  PROMPT_WRITE(ntshell);
  while (1) {
    unsigned char ch;
    SERIAL_READ(ntshell, (char *)&ch, sizeof(ch));
    vtrecv_execute(&(ntshell->vtrecv), &ch, sizeof(ch));

    if (quit) {
      quit = false;
      return;
    }
  }
}

/**
   Primary entry point into the terminal. Runs forever until exit command given.
*/
void terminalMode() {
  ledsOff();

  statusDialog("SKYNET...\nI mean,\nTerminal Mode\nEngaged");
  safeDisplay();


  Serial.println("                      .-.");
  Serial.println("                     (   )");
  Serial.println("                      '-'");
  Serial.println("                      J L");
  Serial.println("                      | |");
  Serial.println("                     J   L");
  Serial.println("                     |   |");
  Serial.println("                    J     L");
  Serial.println("                  .-'.___.'-.");
  Serial.println("                 /___________\\");
  Serial.println("            _. - ""'           `bmw._");
  Serial.println("          .'                       `.");
  Serial.println("        J                            `.");
  Serial.println("       F                               L");
  Serial.println("      J                                 J");
  Serial.println("     J                                  `");
  Serial.println("     |            Welcome to the         L");
  Serial.println("     |       AND!XOR DEFCON 24 Badge     | ");
  Serial.println("     |                                   | ");
  Serial.println("     |          Brought to you by:       J");
  Serial.println("     |        @andnxor  @lacosteaef       L");
  Serial.println("     |            @andrewnriley            | ");
  Serial.println("     |             , .___          ___....--._");
  Serial.println("     |           , '     `\"\"\"\"\"\"\"\"'           ` -._");
  Serial.println("     |          J           _____________________` -.");
  Serial.println("     |         F         . - '   `-88888-'    `Y8888b.`.");
  Serial.println("     |         |       .'         `P'         `88888b \\");
  Serial.println("     |         |      J       #     L      #    q8888b L");
  Serial.println("     |         |      |             |           )8888D )");
  Serial.println("     |         J      \\             J           d8888P P");
  Serial.println("     |          L      `.         .b.         , 88888P / ");
  Serial.println("     |           `.      ` -.___, o88888o.___, o88888P'.'");
  Serial.println("     |             ` -.__________________________.. - '");
  Serial.println("     |                                    |");
  Serial.println("     |         .-----.........____________J");
  Serial.println("     |        .' |       |      |       |");
  Serial.println("     |       J---|-----..|...___|_______|");
  Serial.println("     |       |   |       |      |       |");
  Serial.println("     |       Y---|-----..|...___|_______|");
  Serial.println("     |        `. |       |      |       |");
  Serial.println("     |          `'-------:....__|______.J");
  Serial.println("     |                                  |");
  Serial.println("     L___                               |");
  Serial.println("          \"\"\"----...______________....--'");
  ntshell_t ntshell;

  ntshell_init(
    &ntshell,
    func_read,
    func_write,
    ntPromptCallback,
    (void *)&ntshell);

  //Root shell or not
  if ((ANXGetUnlocked() & UNLOCK_ANDNXOR) > 0) {
    ntshell_set_prompt(&ntshell, "AND!XOR# ");
  } else {
    ntshell_set_prompt(&ntshell, "AND!XOR$ ");
  }

  executeShell(&ntshell);
}

/**
  Handle chat room commands
  chat listen - Listen to chat room, everything should be dumped to terminal, any key to stop
  chat send <text> - Send a message to the chat room [limited to 31 characters]
*/
static int terminal_chat(int argc, char **argv) {
  //Help
  if (argc == 1) {
    Serial.println("Listen to the chat room and send messages");
    Serial.println("Usage:");
    Serial.println("\tchat listen\tListen to chat room, everything should be dumped to terminal, any key to stop");
    Serial.println("\tchat send <text>\tSend a message to the chat room, <text> is limited to 31 characters and will be truncated");
    return 0;
  }

  if (argc >= 2) {
    if (strcmp(argv[1], "listen") == 0) {
      chatToTerminal = true;
      while (!Serial.available()) {
        tick();
      }
      chatToTerminal = false;
    } else if (strcmp(argv[1], "send") == 0) {

      char chatBuffer[ANX_CHAT_MAX_LENGTH];
      memset(chatBuffer, '\0', ANX_CHAT_MAX_LENGTH);
      uint8_t counter = 0;
      //Append remaining arguments into one string to send limited to 31 chars
      for (uint8_t i = 2; i < argc; i++) {
        for (uint8_t j = 0; j < strlen(argv[i]); j++) {
          if (counter < ANX_CHAT_MAX_LENGTH) {
            chatBuffer[counter] = argv[i][j];
            counter++;
          }
        }
        if (counter < ANX_CHAT_MAX_LENGTH) {
          chatBuffer[counter] = ' ' ;
          counter++;
        }
      }

      ANXRFSendChat(chatBuffer);
    }
  }
}

/**
  Troll emacs users
*/
static int terminal_emacs(int argc, char **argv) {
  Serial.println("#vi4life");
}

/**Vim is lord
**/
static int terminal_vim(int argc, char **argv) {
  Serial.println("You have choosen....wisely.");
}
/**
  Exit the terminal
*/
static int terminal_exit(int argc, char **argv) {
  chatToTerminal = false; //Make sure we stop dumping chat
  quit = true;
}

/**
   Help command that shows available commands and their syntax
*/
static int terminal_help(int argc, char **argv) {
  if (argc == 1) {
    for (uint8_t i = 0; i < CMD_LIST_SIZE; i++) {
      Serial.print(cmdlist[i].cmd);
      Serial.print(": ");
      Serial.println(cmdlist[i].desc);
    }
    return 0;
  }

  return -1;
}

/**
  Handle LED command in the terminal
  Syntax
  led <index> <r> <g> <b> - Set single LED
  led <r> <g> <b> - set all LEDs
*/
static int terminal_led(int argc, char **argv) {
  if (argc == 1) {
    Serial.println("Set the color of the LEDs.");
    Serial.println("Usage:");
    Serial.println("\tled <red> <green> <blue>\tSet all LEDs to a specific color. RGB values are 0-255.");
    Serial.println("\tled <index> <red> <green> <blue>\tSet a single LED to a specific Color. Index is counter-clockwise, 0 is on the Left Eye.");
    return -1;
  }

  int16_t red = 0, green = 0, blue = 0;

  if (argc == 4 || argc == 5) {
    int16_t index = -1;
    red = atol(argv[argc - 3]);
    green = atol(argv[argc - 2]);
    blue = atol(argv[argc - 1]);

    if (argc == 5) {
      index = atol(argv[1]);
    }

    //Validate the data, this is DEF CON after all!
    if (index >= -1 && index < 8 &&
        red >= 0 && red <= 255 &&
        green >= 0 && green <= 255 &&
        blue >= 0 && blue <= 255) {
      //Set all leds
      if (index == -1) {
        setAllLeds(red, green, blue);
      }
      //Set individual leds
      else {
        leds.setPixelColor(index, red, green, blue);
        leds.show();
      }

      return 0;
    }
  }

  //Some error happened
  Serial.println("Bite my shiny metal badge! - Invalid syntax");
  return -1;
}

/**
  List directory contents
*/
static int terminal_ls(int argc, char **argv) {
  Serial.println("");
}

/**
  Handle peers command in terminal
  Syntax:
  peer -l - List all peers
  peer -i <id> - Get known info on a given peer
*/
static int terminal_peer(int argc, char **argv) {
  //Help
  if (argc == 1) {
    Serial.println("List and show details on known peer badges. Only badges that we've received a 'hello' from will be here");
    Serial.println("Usage:");
    Serial.println("\tpeer -l\tList all known peers");
    Serial.println("\tpeer -i <id>\tShow details on a known peer");
    return 0;
  }

  //List all peers
  if (argc == 2) {
    if (strcmp(argv[1], "-l") == 0) {

      uint8_t count = 0;
      for (uint8_t i = 0; i < PEER_NODES_MAX; i++) {
        if (peers[i].lastSeen > 0) {
          Serial.print(peers[i].nodeid); Serial.print(": "); Serial.println(peers[i].name);
          count++;
        }
      }
      if (count == 0) {
        Serial.println("No peers found, why so lonely ? : -(");
      } else {
        Serial.print("Total: "); Serial.println(count);
      }
    }
  }

  //Get info on a peer
  if (argc == 3) {
    if (strcmp(argv[1], "-i") == 0) {
      uint8_t id = atol(argv[2]);
      for (uint8_t i = 0; i < PEER_NODES_MAX; i++) {
        if (peers[i].nodeid == id) {
          Serial.print(peers[i].nodeid); Serial.print(": "); Serial.println(peers[i].name);

          //Format last seen text
          char lastSeen[32];
          uint32_t lastSeenMillis = rtMillis() - peers[i].lastSeen;
          if (lastSeenMillis < 2000) {
            sprintf(lastSeen, "  A second ago");
          } else if (lastSeenMillis < 60 * 1000) {
            sprintf(lastSeen, "  % d seconds ago", lastSeenMillis / 1000);
          } else if (lastSeenMillis < 2 * 60 * 1000) {
            sprintf(lastSeen, "  A minute ago");
          } else {
            sprintf(lastSeen, "  % d minutes ago", lastSeenMillis / 60 / 1000);
          }

          //Print RSSI data
          Serial.print("Last RSSI: "); Serial.print(peers[i].rssi); Serial.println("dBm");
          return 0;
        }
      }

      Serial.println("Peer not found");
    }
  }

  //Some error occurred
  Serial.println("Bite my shiny metal badge! - Invalid syntax");
  return -1;
}

/**
  Ping a node
  Syntax:
  ping <node id>
*/
static int terminal_ping(int argc, char **argv) {
  //Help
  if (argc == 1) {
    Serial.println("Ping another badge by ID. IDs can be obtained by looking at the peer list");
    Serial.println("Usage:");
    Serial.println("\tping <id>");
    return 0;
  }

  if (argc == 2) {
    uint8_t nodeId = atol(argv[1]);

    for (uint8_t i = 0; i < 5; i++) {
      Serial.print("Pinging "); Serial.print(nodeId);

      int t = ANXRFSendPing(nodeId);
      if (t >= 0) {
        Serial.print(" - time = "); Serial.print(t); Serial.println("ms");
      } else {
        Serial.println(" - No response from peer");
      }
    }

    Serial.println("");
    return 0;
  }

  Serial.println("Bite my shiny metal badge! - Invalid syntax");

  return -1;
}

/**
  Play an animation
  Syntax:
  play <animation name>
*/
static int terminal_play(int argc, char **argv) {
  uint8_t unlock = ANXGetUnlocked();

  //Help
  if (argc == 1) {
    Serial.println("Play an animation. Use left button on badge to return to terminal");
    Serial.println("Usage:");
    Serial.println("\tsudo <animation>\t\t\tRun <animation>");
    Serial.println("Available animations:");
    Serial.print("\tglow, knightrider, matrix, netscape, party, rainbow, scroll, snake, toaster, gameoflife, flames, defcon, nayan, wargames");
    if ((unlock & UNLOCK_ANDNXOR) > 0)
      Serial.print(", majorlazer, rager");
    if ((unlock & UNLOCK_EFF) > 0)
      Serial.print(", eff");
    if ((unlock & UNLOCK_HACKADAY) > 0)
      Serial.print(", hackaday");
    if ((unlock & UNLOCK_MASTER) > 0)
      Serial.print(", rick");
    if ((unlock & UNLOCK_PIRATES) > 0)
      Serial.print(", pirate");
    if ((unlock & UNLOCK_SCROLL) > 0)
      Serial.print(", lycos, pathogen");
    if ((unlock & UNLOCK_WHOAMI) > 0)
      Serial.print(", rememberme");

    Serial.print("\n");
    return 0;
  }

  if (argc == 2) {
    if (strcmp(argv[1], "glow") == 0) glow();
    if (strcmp(argv[1], "knightrider") == 0) knightRider();
    if (strcmp(argv[1], "matrix") == 0) matrix();
    if (strcmp(argv[1], "netscape") == 0) netscape();
    if (strcmp(argv[1], "party") == 0) party(100);
    if (strcmp(argv[1], "rainbow") == 0) rainbow();
    if (strcmp(argv[1], "scroll") == 0) scrollingText();
    if (strcmp(argv[1], "snake") == 0) snake();
    if (strcmp(argv[1], "toaster") == 0) flyingToasters();
    if (strcmp(argv[1], "nayan") == 0) nayan();
    if (strcmp(argv[1], "defcon") == 0) defcon();
    if (strcmp(argv[1], "flames") == 0) flames();
    if (strcmp(argv[1], "hackers") == 0) hackers();
    if (strcmp(argv[1], "gameoflife") == 0) gameOfLife();
    if (strcmp(argv[1], "wargames") == 0) warGames();

    if ((unlock & UNLOCK_ANDNXOR) > 0) {
      if (strcmp(argv[1], "majorlazer") == 0) majorLazer();
      if (strcmp(argv[1], "rager") == 0) party(0);
    }

    if ((unlock & UNLOCK_EFF) > 0) {
      if (strcmp(argv[1], "eff") == 0) eff();
    }

    if ((unlock & UNLOCK_HACKADAY) > 0) {
      if (strcmp(argv[1], "hackaday") == 0) hackaday();
    }

    if ((unlock & UNLOCK_MASTER) > 0) {
      if (strcmp(argv[1], "rick") == 0) rick();
    }

    if ((unlock & UNLOCK_PIRATES) > 0) {
      if (strcmp(argv[1], "pirate") == 0) pirate();
    }

    if ((unlock & UNLOCK_SCROLL) > 0) {
      if (strcmp(argv[1], "lycos") == 0) lycos();
      if (strcmp(argv[1], "pathogen") == 0) cyberPathogen();
    }

    if ((unlock & UNLOCK_WHOAMI) > 0) {
      if (strcmp(argv[1], "rememberme") == 0) rememberme();
    }

    return 0;
  }

  return -1;
}

/**
  Terminal command to reset badge settings
*/
static int terminal_rm(int argc, char **argv) {
  resetSettings();
  Serial.println("Badge reset");
  return 0;
}

/**
   Change a setting
*/
static int terminal_set(int argc, char **argv) {
  //Help
  if (argc == 1) {
    Serial.println("Change settings");
    Serial.println("Usage:");
    Serial.println("\tset username\t\t\tReturn the current username");
    Serial.println("\tset username <username>\tChange the username, invalid characters will be stripped, 3-8 characters");
    return 0;
  }

  //Display a setting
  if (argc == 2) {
    if (strcmp(argv[1], "username") == 0) {
      char username[USERNAME_MAX_LENGTH];
      ANXGetUsername(username);
      Serial.println(username);
      return 0;
    }
  }

  //Change a setting
  if (argc == 3) {
    if (strcmp(argv[1], "username") == 0) {
      char username[USERNAME_MAX_LENGTH + 1];
      memset(username, '\0', USERNAME_MAX_LENGTH + 1);
      uint8_t counter = 0;

      //Copy character by character validating each
      for (uint8_t i = 0; i < min(strlen(argv[2]), USERNAME_MAX_LENGTH); i++) {

        //Make sure the input is valid, then copy
        if (ANXIndexOf(INPUT_CHARS, argv[2][i]) >= 0) {
          username[counter] = argv[2][i];
          counter++;
        } else {
          Serial.print("Invalid character: "); Serial.println(argv[2][i]);
        }

        if (argv[2][i] == '\0') break;
      }

      Serial.print("Setting username to: \""); Serial.print(username); Serial.println("\"");

      //Make sure it's of minimum length
      if (counter < USERNAME_MIN_LENGTH) {
        Serial.print("Username must be at least "); Serial.print(USERNAME_MIN_LENGTH); Serial.println(" characters long");
        return -1;
      }

      ANXSetUsername(username);
      Serial.println("Username set!");
      return 0;
    }
  }

  Serial.println("Bite my shiny metal badge! - Invalid setting");
}

/**
  Elevate permissions (just unlock the badge)
*/
static int terminal_sudo(int argc, char **argv) {
  //Help
  if (argc == 1) {
    Serial.println("Execute a command with elevated privileges, may not work with all commands");
    Serial.println("Usage:");
    Serial.println("\tsudo <command>\t\t\tExecute <command> with elevated privileges");
    return 0;
  }

  if (argc == 2) {
    if (strcmp(argv[1], "unlock") == 0) {
      addExperience(XP_PER_LEVEL);
      ANXSetUnlocked(ANXGetUnlocked() | UNLOCK_ANDNXOR);
      Serial.println("AND!XOR Unlocked");
      return 0;
    }
  }

  return -1;
}

/**
  Print system information
*/
static int terminal_uname(int argc, char **argv) {
  if (argc == 1) {
    Serial.println("AND!XOR Badge");
    return 0;
  } else if (argc == 2) {
    if (strcmp(argv[1], " - a") == 0) {
      char username[16];
      ANXGetUsername(username);
      Serial.print("AND!XOR Badge ");
      Serial.print(username);
      Serial.print(" ");
      Serial.print(VERSION);
      Serial.print(" + ");
      Serial.print(getFlashVersion());
      Serial.println(" DEFCON 24 AUG 4 - 7 2016");
      return 0;
    }
  }
  Serial.println("Bite my shiny metal badge! - Invalid Syntax");
}

/*
  Question your life decisions, find ones-self
*/
static int terminal_whoami(int argc, char **argv) {
  if (who_count == 10) {
    who_count = 0;
  }
  switch (who_count) {
    case 0:
      Serial.println("First, you must ask who do you think you are...");
      break;
    case 1:
      Serial.println("...Only then can you find peace...");
      break;
    case 2:
      Serial.println("Look deep into your mind and search for your true identity...");
      break;
    case 3:
      Serial.println("As the Dalai Lama once....wait....why do you keep asking? ");
      break;
    case 4:
      Serial.println("Okay, I admit.  I don't know... just stop, that's all I ask");
      break;
    case 5:
      Serial.println("STOP, for the love of god...");
      break;
    case 6:
      Serial.println("First, you must ask who do you think you are...");
      break;
    case 7:
      Serial.println("Ah, you thought I started over didn't you, but you have to be so damn persistent");
      break;
    case 8:
      Serial.println("Really have nothing better to do?");
      break;
    case 9:
      Serial.println("FINE, you win, SKYNET initialization sequence unlocked");
      addExperience(XP_PER_LEVEL);
      ANXSetUnlocked(ANXGetUnlocked() | UNLOCK_WHOAMI);
      break;
  }
  who_count ++;
}

