/*
 *    Created by: Andrew O'Shei
 * 
 *    Manages OLED Screen
 * 
 */

#ifndef OLED_INTERFACE_H
#define OLED_INTERFACE_H

#include <Wire.h>
#include <U8g2lib.h>
#include "data_helper.h"
#include <Arduino.h>

void writeChat(String text);

void resetChat();

void writeLog(String message);

void initOLED();

void splashOLED();

void animateCharging();

void displayMain(int index, int btMode, bool btRecvd, int cntRecvd);

// SEND MESSAGES Display --------------------------- //
void displaySender(bool editMode, int sRows, int sCols, int btMode, bool btRecvd, int cntRecvd);

int getSenderRow();

int getSenderSelection();

String getSenderMessage();

void resetSender();
// SEND MESSAGES Display --------------------------- //

void displayScan(int index, int btMode, bool btRecvd, int cntRecvd);

void scanWarning(int index, int btMode, bool btRecvd, int cntRecvd);

void animateScan(int deviceCount);

void delDevWarning(int index, int btMode, bool btRecvd, int cntRecvd);

void displayWarning(String text_1, String text_2, int btMode, bool btRecvd, int cntRecvd);

#endif
