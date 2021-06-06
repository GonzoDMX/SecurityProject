package com.example.simple_receiver.utilities

import android.util.Base64
import javax.crypto.Cipher
import javax.crypto.SecretKey
import javax.crypto.spec.GCMParameterSpec
import javax.crypto.spec.SecretKeySpec
import android.util.Log

object EncryptDecryptTest {

    const val TAG_LENGTH = 16

    val my_key = "48656c6c6f206d7920667269656e6421"
    val other_key = "Hello my friend!"


    class EncryptionOutput(
            val iv: ByteArray,
            val tag: ByteArray,
            val ciphertext: ByteArray,
    )


    fun encrypt(key: SecretKey, message: ByteArray): ByteArray {
        val cipher = Cipher.getInstance("AES_128/GCM/NoPadding")
        cipher.init(Cipher.ENCRYPT_MODE, key)
        val iv = cipher.iv.copyOf()
        val result = cipher.doFinal(message)
        val ciphertext = result.copyOfRange(0, result.size - TAG_LENGTH)
        val tag = result.copyOfRange(result.size - TAG_LENGTH, result.size)
        val encrypted = EncryptionOutput(iv, tag, ciphertext)

        val size = encrypted.iv.size + encrypted.tag.size + encrypted.ciphertext.size

        var cryptoText = ByteArray(size + 3)

        // Encode size
        cryptoText[0] = (((size / 100) % 10) + 48).toByte()
        cryptoText[1] = (((size / 10) % 10) + 48).toByte()
        cryptoText[2] = ((size % 10) + 48).toByte()

        for (i in encrypted.iv.indices) { cryptoText[i + 3] = encrypted.iv[i] }

        for (i in encrypted.tag.indices) { cryptoText[i + 3 + encrypted.iv.size] = encrypted.tag[i] }

        for (i in encrypted.ciphertext.indices) {
            cryptoText[i + 3 + encrypted.iv.size + encrypted.tag.size] = encrypted.ciphertext[i]
        }

        Log.d("CHECK_ENCRYPT", checkHexValues(cryptoText))
        return cryptoText
    }


    fun parseEncrypt(message: String): ByteArray {
        val plaintext = message.toByteArray()
        val newKey = SecretKeySpec(other_key.toByteArray(), "AES")
        return encrypt(newKey, plaintext)
    }


    fun decrypt(key: SecretKey, iv: ByteArray, tag: ByteArray, ciphertext: ByteArray): ByteArray {
        val cipher = Cipher.getInstance("AES_128/GCM/NoPadding")
        val spec = GCMParameterSpec(TAG_LENGTH * 8, iv)
        cipher.init(Cipher.DECRYPT_MODE, key, spec)
        return cipher.doFinal(ciphertext + tag)
    }

    fun parseDecrypt(message: ByteArray, size: Int): ByteArray {
        val iv = ByteArray(12)
        val tag = ByteArray(16)
        val text = ByteArray(size - iv.size - tag.size)

        for (i in iv.indices) { iv[i] = message[i] }

        for (i in tag.indices) { tag[i] = message[i + iv.size] }

        for (i in text.indices) {
            text[i] = message[i + iv.size + tag.size]
        }


        Log.d("CHECK_IV", checkHexValues(iv))
        //Log.d("CHECK_TAG", checkHexValues(tag))
        //Log.d("CHECK_TEXT", checkHexValues(text))

        val newKey = SecretKeySpec(other_key.toByteArray(), "AES")
        return decrypt(newKey, iv, tag, text)
    }

    private fun hexStringToByteArray(hexInputString: String): ByteArray {
        val bts = ByteArray(hexInputString.length / 2)
        for (i in bts.indices) {
            bts[i] = hexInputString.substring(2 * i, 2 * i + 2).toInt(16).toByte()
        }
        return bts
    }

    fun checkHexValues(item: ByteArray): String {
        var container: String = ""
        for (b in item) {
            container += String.format("%02X ", b)
        }
        return container
    }
}