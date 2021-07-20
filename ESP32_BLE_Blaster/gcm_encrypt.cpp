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
byte txCounter = 0x01;
byte rxCounter = 0x00;

// Holds plainText of a decrypted message
char plainText[512] = { 0x00 };

// Set error message
int notify;
// Empty string returned in event of failure
String fail = "";

AuthenticatedCipher *cipher;


// Setup the Cipher Key, Size and Memory allocation
void initCipher(const uint8_t *key, int iv_s, int tag_s, size_t *ref_s) {

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

// In case of disconnect reset Counters
void resetCounters() {
    txCounter = 0x01;
    rxCounter = 0x00;
}


// ENCRYPTION ----------------------------------------------------- //
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
    size_t pos, len, inc, off_set = 3 + IV_SIZE + TAG_SIZE;
    // Set size of outgoing cipher text
    size_t dataSize = (message.length() + 1);
    inc = dataSize;
    *pSize = dataSize + off_set;
    
    // If message is too big for buffer do not encrypt
    if (*pSize > 512) {
        *pSize = 0;
        return { 0x00 };
    }
    
    // Allocate memory for text component of cipher message
    uint8_t *cipherText = (uint8_t*)malloc(dataSize * sizeof(uint8_t));
    
    // Convert message to char array
    // If sending an ACK
    if (message[0] == 0x06) {
        cipherText[0] = rxCounter;   
    }
    // Else if sending normal message
    else {
        cipherText[0] = txCounter;
    }
    for (pos = 1; pos < dataSize; pos++) {
        cipherText[pos] = message[pos - 1];
    }
    
    // Increment counter
    if (txCounter == 0xFF) {
        txCounter = 0x00;
    } else {
        txCounter += 1;
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
        buffer[pos + 3] = iv[pos];
    }
    // Set Tag
    for (pos = 0; pos < TAG_SIZE; pos++) {
        buffer[pos + 3 + IV_SIZE] = tag[pos];
    }
    
    // Free Dynamic Memory
    free(cipherText);
    free(iv);
    free(tag);

    pos = *pSize - 3;
    // Set message size
    buffer[0] = (pos / 100) % 10 + 48;
    buffer[1] = (pos / 10) % 10 + 48;
    buffer[2] = pos % 10 + 48;

    Serial.print("Outgoing size: ");
    for (int i = 0; i < 3; i++) {
        Serial.print((char)buffer[i]);
    }
    Serial.println();
    // Clear the cipher
    // delete cipher;
    return buffer;
}
// ---------------------------------------------------------------- //


// DECRYPTION ----------------------------------------------------- //
String getDecryptedText() {
    return (String)plainText;
}


// Converts Char[3] to int
int getBtSize(char* buf) {
    return ((buf[0] - 48) * 100) + ((buf[1] - 48) * 10) + (buf[2] - 48);
}


// Validate size and prepare Cipher text to be decrypted ---------- //
bool prepareDecrypt(byte* cText) {
    // Get size of message from first three bytes
    char* sizeBuffer = (char*)malloc(sizeof(char)*3);
    // Get declared message size
    for (int i = 0; i < 3; i++) {
        sizeBuffer[i] = cText[i];
    }
    // Report the size
    int mSize = getBtSize(sizeBuffer);
    free(sizeBuffer);
    Serial.print("Message size: ");
    Serial.println(mSize);

    // Check bounds of the message size
    if (mSize < 29) {
        Serial.println("Message is too small to decrypt");
        return false;
    }
    if (mSize > 512) {
        Serial.println("Message size is too big to decrypt");
        return false;
    }

    // Declare arrays for cipher message components dynamically
    uint8_t * decrypt_iv = (uint8_t*)malloc(sizeof(uint8_t)*IV_SIZE);
    uint8_t * decrypt_tag = (uint8_t*)malloc(sizeof(uint8_t)*TAG_SIZE);
    uint8_t * decrypt_text = (uint8_t*)malloc(sizeof(uint8_t)*(mSize - (TAG_SIZE + IV_SIZE)));

    // Retreive Message Init Vector
    for (int i = 0; i < IV_SIZE; i++) {
        decrypt_iv[i] = cText[i + 3];
    }
    // Retreive message Tag
    for (int i = 0; i < TAG_SIZE; i++) {
        decrypt_tag[i] = cText[i + 3 + IV_SIZE];
    }
    // Retreive cipher text
    size_t tSize = mSize - (IV_SIZE + TAG_SIZE);
    for (int i = 0; i < tSize; i++) {
        decrypt_text[i] = cText[i + 3 + IV_SIZE + TAG_SIZE];
    }

    // Attempt to decrypt the message
    Serial.println("Decrypting...");
    bool decryptSuccess = decryptMessage(decrypt_iv, decrypt_tag, decrypt_text, tSize);

    // Free up that memory
    free(decrypt_iv);
    free(decrypt_tag);
    free(decrypt_text);

    Serial.println("Finished decrypting");
    return decryptSuccess;
}


// We decrypt cipher text with this function
bool decryptMessage(uint8_t* cIV, uint8_t* cTAG, uint8_t* cTEXT, size_t tSize) {

    size_t len, inc = tSize;

    // Set the Init vector for decryption
    cipher->setIV(cIV, IV_SIZE);

    // Initialize the buffer
    memset(buffer, 0x00, sizeof(buffer));

    // Decrypt the message to the buffer
    for (size_t i = 0; i < tSize; i += inc) {
        len = tSize - i;
        if (len > inc) {
            len = inc;
        }
        cipher->decrypt(buffer + i, cTEXT + i, len);
    }
    // Validate message tag authentification
    uint8_t tagCheck = cipher->checkTag(cTAG, TAG_SIZE);
    
    // Validate the message tag
    if (!tagCheck) {
        Serial.println("Tag check failed!");
        return false;
    }

    // Validate the message counter
    if (buffer[0] != (rxCounter + 1)) {
        Serial.println("Bad counter value");
        return false;
    }
    rxCounter = buffer[0];
    
    // Convert plain text to char array for easy reading (aka to make a String)
    int pos = 1;
    while (buffer[pos] != 0x00) {
        plainText[pos - 1] = buffer[pos];
        pos += 1;
    }
    plainText[pos - 1] = 0x00;

    // If we've made it this far we're good
    return true;
}
// ---------------------------------------------------------------- //
