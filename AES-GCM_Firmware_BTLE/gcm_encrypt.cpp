/*
 *    Created by: Andrew O'Shei
 * 
 *    Encrypt and Decrypt messages
 *    with AES128-GCM Cipher
 * 
 */

#include "gcm_encrypt.h"


// "AES-128 GCM"
GCM<AES128> *gcmaes128 = 0;
byte buffer[1024];

const uint8_t *CIPHER_KEY;

// Set IV and Tag Sizes
int IV_SIZE;
int TAG_SIZE;
// Stores size of Outgoing messages
size_t *pSize;

// First byte of message is a counter to prevent replay attacks
// These variables are used to track and compare messages
byte mCounter = 0x01;
byte rCounter = 0x00;

// Holds plainText of a decrypted message
uint8_t plainText[1024] = { 0x00 };

// Set error message
int notify;
// Empty string returned in event of failure
String fail = "";

AuthenticatedCipher *cipher;


// Setup the Cipher Key, Size and Memory allocation
void setupCipher(const uint8_t *key, int iv_s, int tag_s, size_t *ref_s) {

  cipher = new GCM<AES128>();

  // Initialize the cipher
  cipher->clear();
  cipher->setKey(key, cipher->keySize());

  IV_SIZE = iv_s;
  TAG_SIZE = tag_s;
  pSize = ref_s;
}


// Return error message code
int getNotify() {
  return notify;
}

byte get_rCount() {
  return rCounter;
}

byte get_mCount() {
  return mCounter;
}

void set_mCount(byte c) {
  mCounter = c;
}

void incr_mCount() {
  mCounter += 1;
}

// Generate a random Init Vector for outgoing messages
uint8_t * generateIV() {
  // Allocate memory for IV
  uint8_t *iv = (uint8_t*)malloc(IV_SIZE * sizeof(uint8_t));
  // Generates a random IV
  RNG.rand(iv, IV_SIZE);
  // Initialize the cipher and set IV
  cipher->setIV(iv, IV_SIZE);
  // Free IV memory space
  return iv;
}

// Encrypts a message in AES128-GCM
uint8_t* encryptMessage(String message) {

  // Indexing variables
  size_t pos, len, inc, off_set = IV_SIZE + TAG_SIZE;
  // Set size of outgoing cipher text
  size_t dataSize = (message.length() + 1);
  inc = dataSize;
  *pSize = dataSize + off_set;

  // If message is too big for buffer do not encrypt
  if (*pSize > 999) {
    *pSize = 0;
    return { 0x00 };
  }

  // Allocate memory for text component of cipher message
  uint8_t *cipherText = (uint8_t*)malloc(dataSize * sizeof(uint8_t));
  
  // Convert message to char array
  cipherText[0] = mCounter;
  for (pos = 1; pos < dataSize; pos++) {
    cipherText[pos] = message[pos - 1];
  }

  // Increment counter
  if (mCounter == 0xFF) {
    mCounter = 0x01;
  } else {
    mCounter += 1;
  }

  // Set the outgoing Init Vector
  uint8_t *iv = generateIV();
  
  // Keep the watchdog happy
  crypto_feed_watchdog();
  
  // Initialize the buffer
  memset(buffer, 0x00, sizeof(buffer));

  for (pos = 0; pos < dataSize; pos += inc) {
    len = dataSize - pos;
    if (len > inc) {
      len = inc;
    }
    cipher->encrypt(buffer + pos + off_set, cipherText + pos, len);
  } 

  uint8_t *tag = (uint8_t*)malloc(TAG_SIZE * sizeof(uint8_t));
  // Generate encrypt tag
  cipher->computeTag(tag, TAG_SIZE);

  // Prepend the IV and Tag to outgoing message
  // Set IV
  for (pos = 0; pos < IV_SIZE; pos++) {
    buffer[pos] = iv[pos];
  }
  // Set Tag
  for (pos = 0; pos < TAG_SIZE; pos++) {
    buffer[pos + IV_SIZE] = tag[pos];
  }

  // Free Dynamic Memory
  free(cipherText);
  free(iv);
  free(tag);
  
  // Clear the cipher
  // delete cipher;
  return buffer;
}



String decryptMessage(byte* message, int mSize) {

  // Indexing variables
  size_t pos, len, inc, dataSize = 0;
  
  // Get size of encrypted message
  pos = IV_SIZE + TAG_SIZE;

  dataSize = mSize - pos;
  
  inc = dataSize;

  // Allocate memory for IV
  uint8_t *iv = (uint8_t*)malloc(IV_SIZE * sizeof(uint8_t));
  // Get Incoming IV value
  for (pos = 0; pos < IV_SIZE; pos++) {
    iv[pos] = message[pos];
  }
  // Set the Init Vector
  cipher->setIV(iv, IV_SIZE);
  free(iv);

  // Allocate memory for tag
  uint8_t *tag = (uint8_t*)malloc(TAG_SIZE * sizeof(uint8_t));
  // Get the incoming message tag
  for (pos = 0; pos < TAG_SIZE; pos++) {
    tag[pos] = message[pos + IV_SIZE];
  }

  // Allocate memory for text component of cipher message
  uint8_t *cipherText = (uint8_t*)malloc(dataSize * sizeof(uint8_t));
  
  // Get the encrypted message
  for (pos = 0; pos < dataSize; pos++) {
    cipherText[pos] = message[pos + IV_SIZE + TAG_SIZE];
  }
  
  // Keep the watchdog happy
  crypto_feed_watchdog();

  // Initialize the buffer
  memset(buffer, 0x00, sizeof(buffer));

  // Decrypt the message to the buffer
  for (pos = 0; pos < dataSize; pos += inc) {
    len = dataSize - pos;
    if (len > inc) {
      len = inc;
    }
    cipher->decrypt(buffer + pos, cipherText + pos, len);
  }

  // Free memory
  free(cipherText);
  
  // Validate message tag authentification
  uint8_t tagCheck = cipher->checkTag(tag, TAG_SIZE);
  free(tag);
  
  // Validate the message tag
  if (!tagCheck) {
    notify = 5;
    return fail;
  }

  // Parse the decrypted message
  rCounter = buffer[0];
  pos = 1;
  while (buffer[pos] != 0x00) {
    plainText[pos - 1] = buffer[pos];
    pos += 1;
  }
  plainText[pos - 1] = 0x00;

  // Clear cipher object
  // delete cipher;
  return (char*)plainText;
}
