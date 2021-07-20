/*
 *    Created by: Andrew O'Shei
 * 
 *    Manages OLED Screen
 * 
 */

#include "oled_interface.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE);

// Track animations
int frameStep = 0;
// Tracks incoming Log Entries
int logStep = 0;
// Store Log Entries
char logBuffer[6][22];

char displayChat[30][21];
int chatIndex = 0;

// Set scroll range for chat
int chatRange = 0;

// Animation parameters
int anim_a = 0;
int anim_b = 0;
int anim_c = 0;



// Initialize Display Array
void initChat() {
    for (int i = 0; i < 30; i++) {
        memset(displayChat[i], 0x00, 20);
    }
}

void shiftChat(int rows) {
    for (int i = 29; i > -1; i--) {
        if (i - rows < 0) {
            memset(displayChat[i], 0x00, 21);
        } else {
            for (int j = 0; j < 20; j++) {
                displayChat[i][j] = displayChat[i - rows][j];    
            }
        }
    }
}

void writeChat(String text) {
    // Add header
    String str = ">> " + text;
    // Get Size
    size_t len = str.length();
    // Sift rows to make room for new message
    int rows = (len / 20);
    if (len% 20 != 0) {
        rows += 1;
    }
    if (chatRange < 30) {
        chatRange += rows;
    }
    shiftChat(rows);
    int x = 0;
    int y = 0;
    for(int i = 0; i < len; i++) {
        displayChat[y][x] = str[i];
        x += 1;
        if ((i + 1) % 20 == 0) {
            y += 1;
            x = 0;
        }
    }
    chatIndex = 0;
}

void resetChat() {
    chatIndex = 0;
    chatRange = 0;
    initChat();
}

void initLog() {
    logStep = 0;
    for (int i = 0; i < 6; i++) {
        memset(logBuffer[i], 0x00, sizeof(logBuffer[i]));
    }
}


void shiftLog() {
    for (int i = 0; i < 5; i++) {
        for(int j = 0; j < 21; j++) {
            logBuffer[i][j] = logBuffer[i+1][j];
        }
    }
    memset(logBuffer[5], 0x00, sizeof(logBuffer[5]));
}


void writeLog(String message) {
    int i;
    if (logStep == 6) {
        logStep -= 1;
        shiftLog();
    }
    
    for(i = 0; i < message.length(); i++) {
        if (i > 21) {
            break;
        }
        logBuffer[logStep][i] = message[i];
    }
    logStep += 1;
    
    u8g2.clearBuffer();
    for (i = 0; i < 6; i++) {
        u8g2.drawStr( 0, i*10, logBuffer[i]);
    }
    u8g2.sendBuffer();
    delay(250);
}


// Utility to clear OLED Display -------------- //
void clearOLED() {
    u8g2.clearBuffer();
    u8g2.sendBuffer();
}


// Init BLE BLASTER --------------------------- //
void initOLED() {
    initLog();
    initChat();
    resetSender();
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setFontRefHeightExtendedText();
    u8g2.setDrawColor(1);
    u8g2.setFontPosTop();
    u8g2.setFontDirection(0);
    clearOLED();
}


// DISPLAY BLE BLASTER Splash Screen ---------- //
void splashOLED() {
    u8g2.clearBuffer();
    u8g2.drawLine(0,19, 24, 19);
    u8g2.drawLine(12, 27, 24,19);
    u8g2.drawLine(12, 27, 24,34);
    u8g2.drawLine(2, 27, 14,19);
    u8g2.drawLine(2, 27, 14,34);
    u8g2.drawLine(0,34, 24, 34);
    u8g2.drawLine(103,19, 128, 19);
    u8g2.drawLine(115, 27, 103,19);
    u8g2.drawLine(115, 27, 103,34);
    u8g2.drawLine(125, 27, 113,19);
    u8g2.drawLine(125, 27, 113,34);
    u8g2.drawLine(103,34, 128, 34);
    u8g2.drawStr( 32, 22, "BLE Blaster");
    u8g2.drawRFrame( 24, 12, 80, 30, 10);
    u8g2.drawStr( 18, 54, "andrewoshei.com");
    u8g2.sendBuffer();
    delay(5000);
    clearOLED();
}
// -------------------------------------------- //

// Display the charging battery animation --------------------- //
void animateCharging() {
  u8g2.clearBuffer();
  switch(frameStep) {
    case 1:
      u8g2.drawStr( 34, 54, "Charging.");
      break;
    case 2:
      u8g2.drawStr( 34, 54, "Charging..");
      break;
    case 3:
      u8g2.drawStr( 34, 54, "Charging...");
      break;
    default:
      u8g2.drawStr( 34, 54, "Charging");
  }
  frameStep += 1;
  if (frameStep > 3) {
    frameStep = 0;
  }
  u8g2.drawRFrame( 28, 10, 70, 32, 5);
  u8g2.drawRFrame( 97, 15, 8, 22, 2);
  
  u8g2.drawLine(60, 19, 60, 25);
  u8g2.drawLine(60, 19, 80, 25);
  u8g2.drawLine(60, 25, 44, 25);
  u8g2.drawLine(44, 25, 64, 31);
  u8g2.drawLine(64, 31, 64, 25);
  u8g2.drawLine(64, 25, 80, 25);
  u8g2.sendBuffer();
}
// ------------------------------------------------------------ //


// Builds the main window border ------------------------------ //
void buildMainWindow(int btMode, bool btRecvd, int cntRecvd, int scanMode) {
    // Draw the Border
    u8g2.drawRFrame(0, 0, 128, 64, 3);
    // Line seperator for btRecvd indicator
    u8g2.drawLine( 115,0, 115,12 );
    // Top dividing line
    u8g2.drawLine( 0,12, 128,12 );
    // Bottom dividing line
    u8g2.drawLine( 0,50, 128,50 );

    u8g2.drawLine( 13,50, 13,64 );
    u8g2.drawLine( 115,50, 115,64 );
    
    // Display Bluetooth operating mode Bluetooth Classic = BTC, Bluetooth Low Eneregy = BLE
    u8g2.drawStr(3, 2, "Status:");
    switch(btMode) {
        case 1:
            u8g2.drawStr( 46, 2, "Advertising");
            break;
        case 2:
            u8g2.drawStr( 46, 2, "Connected");
            break;
        case 3:
            u8g2.drawStr( 46, 2, "Disconnect");
            break;
        case 4:
            u8g2.drawStr( 46, 2, "Scanning");
            break;
        default:
            u8g2.drawStr( 46, 2, "Offline");
    }
    
    // If a Bluetooth message was received
    if (btRecvd) {
        u8g2.drawRBox(116, 1, 11, 11, 0);
    }

    u8g2.drawStr(20, 52, "Msg Count:");
    u8g2.drawStr(82, 52, getIntAsCharArray(cntRecvd));

    switch (scanMode) {
        case 1:
            u8g2.drawLine( 10, 53,  10, 61 );
            u8g2.drawLine( 10, 61,  3, 57 );
            u8g2.drawLine( 10, 53, 3, 57);
            u8g2.drawTriangle(118, 53,  118, 61,  125, 57); 
            break;
        case 2:
            u8g2.drawLine( 118, 53,  118, 61 );
            u8g2.drawLine( 118, 61,  125, 57 );
            u8g2.drawLine( 118, 53, 125, 57);
            u8g2.drawTriangle(10, 53,  10, 61,  3, 57);
            break;
        default:
            u8g2.drawLine( 10, 53,  10, 61 );
            u8g2.drawLine( 10, 61,  3, 57 );
            u8g2.drawLine( 10, 53, 3, 57);
            u8g2.drawLine( 118, 53,  118, 61 );
            u8g2.drawLine( 118, 61,  125, 57 );
            u8g2.drawLine( 118, 53, 125, 57);
    }
}
// ------------------------------------------------------------ //



// Display the main window select screen ---------------------- //
void displayMain(int mIndex, int btMode, bool btRecvd, int cntRecvd) {
    u8g2.clearBuffer();
    buildMainWindow(btMode, btRecvd, cntRecvd, 1);

    if (mIndex != 0) {
        if (chatRange > 3) {
            if (mIndex == 1) {
                if (chatIndex < chatRange - 3) {
                    chatIndex += 1;
                }
            }
            if (mIndex == -1) {
                if (chatIndex > 0) {
                    chatIndex -= 1;
                }
            }
        }
    }

    u8g2.drawStr( 4, 14, displayChat[chatIndex]);
    u8g2.drawStr( 4, 26, displayChat[chatIndex + 1]);
    u8g2.drawStr( 4, 38, displayChat[chatIndex + 2]);

    u8g2.sendBuffer();
}
// ------------------------------------------------------------ //

char senderChar[92] = { ' ',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '.', ',', ':', ';', '?', '!', '@', '#', '$', '%',
    '&', '*', '+', '-', '=', '/', '_', '(', ')', '[',
    ']', '{', '}', '^', '<', '>', '\"', '\'', '~' 
    };

// Contains the actual message in Sender
char senderMessage[2][21];

// Set reference to senderChar Lookup for senderMessage
int senderRef[2][20];

int capsIndex = 1;
int lowcIndex = 27;
int numsIndex = 53;
int puncIndex = 63;

// Sets the currently selected Row and Col in Sender
int sendRow;
int sendCol;

// Sets the current operation Send, Reset or Cancel
int sendSelection;

// Sets edit mode False = Set Position, True = Modify a Character
bool senderMode = false;

// Display the main window select screen ---------------------- //
void displaySender(bool editMode, int sRows, int sCols, int btMode, bool btRecvd, int cntRecvd) {
    int yOff = 0;
    u8g2.clearBuffer();
    buildMainWindow(btMode, btRecvd, cntRecvd, 1);

    // Toggle edit Mode
    if (editMode) {
        senderMode = !senderMode;
    }

    // If editing a character
    if (senderMode) {
        if (sRows != 0 || sCols != 0) {
            if (sRows == 1) {
                if (senderRef[sendRow][sendCol] == 0) {
                    senderRef[sendRow][sendCol] = 91;
                } else {
                    senderRef[sendRow][sendCol] -= 1;
                }
            }
            if (sRows == -1) {
                if (senderRef[sendRow][sendCol] == 91) {
                    senderRef[sendRow][sendCol] = 0;
                } else {
                    senderRef[sendRow][sendCol] += 1;
                }
            }
            if (sCols == -1) {
                if (senderRef[sendRow][sendCol] > 62 ) {
                    senderRef[sendRow][sendCol] = 53;
                }
                else if (senderRef[sendRow][sendCol] > 52) {
                    senderRef[sendRow][sendCol] = 27;    
                }
                else if (senderRef[sendRow][sendCol] > 26) {
                    senderRef[sendRow][sendCol] = 1;    
                } else {
                    senderRef[sendRow][sendCol] = 63;
                }
            }
            if (sCols == 1) {
                if (senderRef[sendRow][sendCol] == 0 ) {
                    senderRef[sendRow][sendCol] = 1;
                }
                else if (senderRef[sendRow][sendCol] < 27 ) {
                    senderRef[sendRow][sendCol] = 27;
                }
                else if (senderRef[sendRow][sendCol] < 53) {
                    senderRef[sendRow][sendCol] = 53;    
                }
                else if (senderRef[sendRow][sendCol] < 63) {
                    senderRef[sendRow][sendCol] = 63;    
                } else {
                    senderRef[sendRow][sendCol] = 1;
                }
            }
            senderMessage[sendRow][sendCol] = senderChar[senderRef[sendRow][sendCol]];
        }    
    } else {
        // If NOT senderMode set position
        // Set Row Selection
        if (sRows == -1 && sendRow > 0) {
            sendRow -= 1;
        }
        if (sRows == 1 && sendRow < 2) {
            sendRow += 1;
        }
    
        // Set Col slection
        if (sCols == -1) {
            if (sendRow < 2) {
                if (sendCol > 0) {
                    sendCol -= 1;
                }
            } else {
                if (sendSelection > 1) {
                    sendSelection -= 1;
                }
            }
        }
        if (sCols == 1) {
            if (sendRow < 2) {
                if (sendCol < 19) {
                    sendCol += 1;
                }
            } else {
                if (sendSelection < 3) {
                    sendSelection += 1;
                }
            }
        }
    }
    
    // If second row selected set offset
    if (sendRow) {
        yOff = 10;
    }

    int xOff = sendCol*6;
        
    if (sendRow < 2) {
        if (senderMode) {
            u8g2.drawLine(4+xOff, 24+yOff, 8+xOff, 24+yOff);
        } else {
            u8g2.drawLine(6+xOff, 24+yOff,6+xOff, 24+yOff);
        }

    } else {
        switch(sendSelection) {
            case 1:
                // Send
                u8g2.drawTriangle( 2,39,  2,45, 8,42);
                break;
            case 2:
                // Reset
                u8g2.drawTriangle( 38,39,  38,45, 44,42);
                break;
            case 3:
                // Cancel
                u8g2.drawTriangle( 80,39,  80,45, 86,42);
        }
    }

    u8g2.drawStr( 4, 14, senderMessage[0]);
    u8g2.drawStr( 4, 26, senderMessage[1]);
    u8g2.drawStr( 4, 38, " Send  Reset  Exit");

    u8g2.sendBuffer();
}

// Get the currently selected row in sender
int getSenderRow() {
    return sendRow;
}

// Get the selected operation from sender
int getSenderSelection() {
    return sendSelection;
}

// Get message created in sender
String getSenderMessage() {
    return (String)senderMessage[0] + (String)senderMessage[1];
}

// Reset the sender variables
void resetSender() {
    memset(senderMessage[0], ' ', 21);
    memset(senderMessage[1], ' ', 21);
    senderMessage[0][20] = 0x00;
    senderMessage[1][20] = 0x00;
    sendRow = 0;
    sendCol = 0;
    sendSelection = 1;
    senderMode = false;
    // Clear sender index Reference
    for (int i = 0; i < 2; i++) {
        for(int j = 0; j < 20; j++) {
            senderRef[i][j] = 0;
        }
    }
}
// ------------------------------------------------------------ //


void displayScan(int index, int btMode, bool btRecvd, int cntRecvd) {
    u8g2.clearBuffer();
    buildMainWindow(btMode, btRecvd, cntRecvd, 2);
    
    // Set the available items
    u8g2.drawStr( 20, 20, "Scan for devices");
    u8g2.drawStr( 20, 32, "Clear devices");

    // Set the cursor position
    int off_x = 10;
    int off_y = 21;
    off_y += 12*(index-1);
    
    // Draw the cursor
    u8g2.drawTriangle( 0+off_x,0+off_y,  0+off_x,6+off_y, 6+off_x,3+off_y);

    u8g2.sendBuffer();
}


void scanWarning(int index, int btMode, bool btRecvd, int cntRecvd) {
    u8g2.clearBuffer();
    buildMainWindow(btMode, btRecvd, cntRecvd, 0);


    // Set the available items
    u8g2.drawStr( 5, 14, "Scan will disconnect");
    u8g2.drawStr( 48, 26, "Continue");
    u8g2.drawStr( 48, 38, "Cancel");

    // Set the cursor position
    int off_x = 38;
    int off_y = 27;
    off_y += 12*(index-1);
    
    // Draw the cursor
    u8g2.drawTriangle( 0+off_x,0+off_y,  0+off_x,6+off_y, 6+off_x,3+off_y);

    u8g2.sendBuffer();
}

// Animation displayed during a scan for new devices ----------------------
void animateScan(int deviceCount) {
  u8g2.clearBuffer();
  // Draw border
  u8g2.drawRFrame(0, 0, 128, 64, 3);
  // Get the device count
  String devices = "Devices found: " + (String)getIntAsCharArray(deviceCount);
  // Set the information
  u8g2.drawStr(42, 2, "Scanning");
  u8g2.drawStr(7, 42, getCharArray(devices));
  u8g2.drawStr(7, 52, "Press button to end");

  // Draw the animation elements
  if (anim_c < 3) {
    u8g2.drawCircle(64, 24, 10-anim_c);
  }
  else if (anim_c < 6) {
    u8g2.drawCircle(64, 24, 7+anim_c);
  }
  else {
    u8g2.drawCircle(64, 24, 10);
  }
  u8g2.drawLine(63-anim_a, 14+anim_b, 63-anim_a, 34-anim_b);
  u8g2.drawLine(64+anim_a, 14+anim_b, 64+anim_a, 34-anim_b);

  u8g2.sendBuffer();

  // Increment animation elements
  anim_a += 2;
  anim_c += 1;
  if (anim_a > 50) {
    anim_a = 0;
    anim_b = 0;
    anim_c = 0;
  }
  if (anim_b < 25) {
    anim_b += 1;
  }
}

void delDevWarning(int index, int btMode, bool btRecvd, int cntRecvd) {
    u8g2.clearBuffer();
    buildMainWindow(btMode, btRecvd, cntRecvd, 0);


    // Set the available items
    u8g2.drawStr( 8, 14, "Delete device file?");
    u8g2.drawStr( 58, 26, "Yes");
    u8g2.drawStr( 58, 38, "No");

    // Set the cursor position
    int off_x = 48;
    int off_y = 27;
    off_y += 12*(index-1);
    
    // Draw the cursor
    u8g2.drawTriangle( 0+off_x,0+off_y,  0+off_x,6+off_y, 6+off_x,3+off_y);
    u8g2.sendBuffer();
}


// Display warnings inside the main window --------------------
void displayWarning(String text_1, String text_2, int btMode, bool btRecvd, int cntRecvd){
    u8g2.clearBuffer();
    buildMainWindow(btMode, btRecvd, cntRecvd, 0);
    // Set text position
    int x_1 = 5;
    int x_2 = 5;
    int len_1 = text_1.length();
    int len_2 = text_2.length();
    x_1 = x_1 + ((20 - len_1)/2 * 5);
    x_2 = x_2 + ((20 - len_2)/2 * 5);
    // Draw text
    u8g2.drawStr(x_1, 20, getCharArray(text_1));
    u8g2.drawStr(x_2, 32, getCharArray(text_2));
    u8g2.sendBuffer();
}
