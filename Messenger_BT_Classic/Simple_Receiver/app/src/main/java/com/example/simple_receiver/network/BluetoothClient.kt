package com.example.simple_receiver.network

import android.bluetooth.BluetoothDevice
import android.util.Log
import com.example.simple_receiver.*
import com.example.simple_receiver.utilities.EncryptDecryptTest.parseDecrypt
import com.example.simple_receiver.utilities.errorDialog
import com.example.simple_receiver.utilities.messageToast
import java.io.IOException
import java.util.*

// Déclarer la classe client Bluetooth et hériter du thread
class BluetoothClient(private val activity: MainActivity, device: BluetoothDevice): Thread() {

    var sizeInt = 0;
    var recving = false


    // Déclarer l'UUID Bluetooth pour l'application
    private val uuid: UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")
    // Déclarez la prise Bluetooth et définissez l'UUID
    private val socket = device.createRfcommSocketToServiceRecord(uuid)
    // Fonction appelée au démarrage du thread
    override fun run() {
        // Si la socket ne renvoie pas null
        if(this.socket != null) {
            // Mettre à jour le journal des messages
            writeToast("Starting Connection...")
            // Retard de 250 millisecondes
            sleep(250)
            try {
                // Connectez le client socket
                this.socket.connect()
                // Déclarer le flux de sortie pour les messages sortants
                val outputStream = this.socket.outputStream
                // Déclarer le flux d'entrée pour les messages entrants
                val inputStream = this.socket.inputStream
                // Essayez, pour éviter de planter le programme en cas d'erreur
                try {
                    // Écrire un message dans le flux de sortie
                    outputStream.write(outgoing)
                    // Assurez-vous que tout le message est envoyé
                    outputStream.flush()
                    // Pendant que la connexion est en boucle ouverte
                    while (!terminateConn) {
                        // Déverrouille le bouton de connexion
                        unlock()
                        // Recevoir des messages du flux d'entrée
                        recMsg()
                        // If a new message needs to be sent
                        if (trigMessage) {
                            // Write message to output stream
                            outputStream.write(outgoing)
                            // Flush output stream
                            outputStream.flush()
                            // Drop flag
                            trigMessage = false
                        }
                    }
                    // Écrire un message dans le flux de sortie
                    outputStream.write(outgoing)
                    // Assurez-vous que tout le message est envoyé
                    outputStream.flush()
                    // Mettre à jour le journal des messages
                    writeToast("Closing Connection...")
                    // Retard de 500 millisecondes
                    sleep(500)
                    // Recevoir un message de déconnexion avant de fermer
                    recMsg()
                    // S'il y a une exception
                } catch (e: Exception) {
                    // Ecrire le message d'erreur dans le journal des messages
                    writeError("Connection Failed,\n$e")
                    // réinitialiser la connexion
                    resetConnex()
                    // finalement
                } finally {
                    // Fermer le flux de sortie
                    outputStream.close()
                    // Fermer le flux d'entrée
                    inputStream.close()
                    // Fermer le socket
                    this.socket.close()
                    // Mettre à jour le journal des messages
                    writeToast("Connection Closed.")
                }
                // S'il y a une exception
            } catch (e: IOException) {
                // Ecrire le message d'erreur dans le journal des messages
                writeError("Connection Failed,\nBluetooth Device not connected!")
                // réinitialiser la connexion
                resetConnex()
            }
            // Si la socket renvoie null
        } else {
            // Ecrire le message d'erreur dans le journal des messages
            writeError("Connection Failed,\nFailed to connect socket!")
            // réinitialiser la connexion
            resetConnex()
        }
    }

    // Post progress update
    private fun writeToast(msg: String) {
        activity.runOnUiThread(Runnable {
            messageToast(activity, msg)
        })
    }

    // Ecrire un message d'erreur dans le Log
    private fun writeError(msg: String){
        // Pour éviter les avertissements, exécutez sur le thread d'interface utilisateur
        activity.runOnUiThread(Runnable {
            // Mettre à jour le journal des messages
            errorDialog(activity, msg)
        })
    }

    private fun resetConnex(){
        // Pour éviter les avertissements, exécutez sur le thread d'interface utilisateur
        activity.runOnUiThread(Runnable {
            // réinitialiser la connexion
            activity.resetConnection()
            // Déverrouille le bouton de connexion
            activity.unlockButton()
        })
    }

    private fun unlock(){
        // Pour éviter les avertissements, exécutez sur le thread d'interface utilisateur
        activity.runOnUiThread(Runnable {
            // Déverrouille le bouton de connexion
            activity.unlockButton()
        })
    }

    // Envoyer le message reçu à la fonction parseMessage dans MainActivity
    private fun parse(msg: Pair<String, UByte>){
        // Pour éviter les avertissements, exécutez sur le thread d'interface utilisateur
        activity.runOnUiThread(Runnable {
            // Envoyer un message à l'analyseur de messages pour traitement
            activity.parseMessage(msg)
        })
    }

    // La fonction reçoit des messages du flux d'entrée
    private fun recMsg(){
        // Obtenir le nombre d'octets dans le flux d'entrée
        var available = this.socket.inputStream.available()

        if (available >= 3 && !this.recving) {
            var bytes = ByteArray(3)
            this.socket.inputStream.read(bytes)
            var size = String(bytes)
            // Deal with leading zeroes
            if (size[0] == '0') {
                size = size.drop(1)
                if (size[0] == '0') {
                    size = size.drop(1)
                }
            }
            sizeInt = size.toInt()
            recving = true
            Log.d("CHECK_SIZE", String(bytes))
        }
        else if (available >= sizeInt && recving) {
            try {
                if (sizeInt > 92) {
                    Log.d("CHECK_ERR", "Error out of sync!")
                }
                // Déclarer un tableau d'octets vide de la taille du message entrant
                var mBytes = ByteArray(sizeInt)
                // Lire le message entrant et écrire dans le tableau d'octets
                this.socket.inputStream.read(mBytes)

                // Returns a Pair with message as string and counter as UByte
                val textPlusCount = parseDecrypt(mBytes, sizeInt)

                // Envoyer un message à l'analyseur de messages pour traitement
                parse(textPlusCount)

            } catch (e: Exception) {
                e.printStackTrace()
                Log.d("ERROR", e.toString())
            }
            recving = false
        }
    }
}