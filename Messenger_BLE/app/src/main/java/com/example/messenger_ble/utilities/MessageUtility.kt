package com.example.messenger_ble.utilities

import android.app.AlertDialog
import android.content.Context
import android.widget.Toast


fun messageToast(context: Context, message: String) {
    Toast.makeText(
        context,
        message,
        Toast.LENGTH_LONG
    ).show()
}

fun errorDialog(context: Context, message: String) {
    // build alert dialog
    val dialogBuilder = AlertDialog.Builder(context)

    // set message of alert dialog
    dialogBuilder.setMessage(message)
        // if the dialog is cancelable
        .setCancelable(true)
        // positive button text and action
        .setPositiveButton("Ok") { dialog, _ -> dialog.dismiss() }

    // create dialog box
    val alert = dialogBuilder.create()
    // set title for alert dialog box
    alert.setTitle("Error")
    // show alert dialog
    alert.show()
}