/*
 *    Created by: Andrew O'Shei
 * 
 *    Encrypt and Decrypt messages
 *    with AES128-GCM Cipher
 * 
 */


#ifndef GCM_ENCRYPT_H
#define GCM_ENCRYPT_H

#include <Arduino.h>
#include <Crypto.h>
#include <AES.h>
#include <GCM.h>
#include <RNG.h>


// Initialize Cipher Values
void initCipher(const uint8_t *key, int iv_s, int tag_s, size_t *ref_s);

// Returns error code
int getNotify();

void resetCounters();

// ENCRYPTION ------------------------------------ //
// Encrypts a message in AES128-GCM
uint8_t* encryptMessage(String message);
// ----------------------------------------------- //

// DECRYPTION -------------------------------------//
// Returns the decrypted plain text message
String getDecryptedText();

// Parse message before decrypt
bool prepareDecrypt(byte* cText);

// Decrypts a message in AES128-GCM -------------- //
bool decryptMessage(uint8_t* cIV, uint8_t* cTAG, uint8_t* cTEXT, size_t tSize);
// ----------------------------------------------- //

#endif
