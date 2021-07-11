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
void setupCipher(const uint8_t *key, int iv_s, int tag_s, size_t *ref_s);

// Returns error code
int getNotify();

byte get_rCount();

byte get_mCount();

void set_mCount(byte c);

void incr_mCount();

// Encrypts a message in AES128-GCM
uint8_t* encryptMessage(String message);

// Decrypts a message in AES128-GCM
String decryptMessage(byte* message, int mSize);

#endif
