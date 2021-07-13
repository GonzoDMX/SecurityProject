package com.example.messenger_ble

import android.Manifest
import android.annotation.SuppressLint
import android.app.Activity
import android.app.AlertDialog
import android.app.Dialog
import android.bluetooth.*
import android.bluetooth.BluetoothDevice.TRANSPORT_LE
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.content.res.ColorStateList
import android.graphics.Color
import android.os.*
import android.util.Log
import android.view.Window
import android.view.animation.Animation
import android.view.animation.AnimationUtils
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.ImageView
import android.widget.TextView
import androidx.activity.result.contract.ActivityResultContracts
import androidx.activity.viewModels
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.app.AppCompatDelegate
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.example.messenger_ble.adapters.ListDataAdapter
import com.example.messenger_ble.data.ClickApplication
import com.example.messenger_ble.data.ListData
import com.example.messenger_ble.databinding.ActivityMainBinding
import com.example.messenger_ble.ui.MainViewModel
import com.example.messenger_ble.ui.MainViewModelFactory
import com.example.messenger_ble.utilities.`Encryption_AES128-GCM`.parseEncrypt
import com.example.messenger_ble.utilities.errorDialog
import com.example.messenger_ble.utilities.messageToast
import java.util.*


// Declare string, pour passer des messages de MainActivity à BluetoothClient Class
var message = ""
// Déclarer l'indicateur d'état de la connexion
var connexion = false
// Indicateur de fin de connexion, indique quand l'utilisateur a fermé la connexion pour que le fil puisse se fermer
var terminateConn = false
// Set flag to true in order to send a new message
var trigMessage = false
// Set new message to send
var newMessage = ""
// ByteArray for outgoing messages
var outgoing: ByteArray = ByteArray(1)

@kotlin.ExperimentalUnsignedTypes
var mCounter: UByte = 1.toUByte()

class MainActivity : AppCompatActivity() {

    private var msgCount: Int = 0

    private var connectionStatus: String = "Device is not connected."

    private lateinit var binding: ActivityMainBinding

    private val mViewModel: MainViewModel by viewModels {
        MainViewModelFactory((application as ClickApplication).repository)
    }

    // Déclarer la baie contient l'ID de périphérique Bluetooth
    private var devices = ArrayList<BluetoothDevice>()
    // Déclarer l'adaptateur Array, contient des chaînes pour Bluetooth AlertDialog
    private var mArrayAdapter: ArrayAdapter<String>? = null

    lateinit var devCounter : TextView

    lateinit var bluetoothGatt: BluetoothGatt

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        setSupportActionBar(findViewById(R.id.toolbar_main))
        AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM)
        // Bind activity to access buttons and whatnot
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val recyclerView = findViewById<RecyclerView>(R.id.recyclerLog)
        val adapter = ListDataAdapter(this)
        recyclerView.adapter = adapter
        recyclerView.layoutManager = LinearLayoutManager(this)

        // Turn off recyclervier animation, otherwise it flashes each time the database is updated
        recyclerView.itemAnimator = null
        recyclerView.setHasFixedSize(true)

        mViewModel.allCLicks.observe(this) { clicks ->
            clicks.let { adapter.submitList(it) }
        }

        // Définir le layout de la liste des appareils AlertDialog
        mArrayAdapter = ArrayAdapter(this, R.layout.dialog_device)

        bluetoothPermissions()
        bindButtonListeners()

    }


    private fun bluetoothPermissions() {
        // Si les autorisations Bluetooth ne sont pas définies
        if (ContextCompat.checkSelfPermission(this,
                        Manifest.permission.ACCESS_FINE_LOCATION)
            != PackageManager.PERMISSION_GRANTED) {
            // Demandez l'autorisation d'accéder au Bluetooth lors du premier lancement de l'application
            ActivityCompat.requestPermissions(this,
                    arrayOf(Manifest.permission.ACCESS_FINE_LOCATION), 1)
        }
    }


    private fun bindButtonListeners() {
        binding.buttonConn.setOnClickListener {
            if (isScanning) {
                stopBleScan()
            } else {
                startBleScan()
            }
        }

        binding.buttonSend.setOnClickListener {
            sendMessage()
        }

        binding.buttonClear.setOnClickListener {
            if (binding.editOutput.text.toString() == "") {
                clearDatabase()
            } else {
                binding.editOutput.setText("")
            }
        }

        binding.buttonOnline.setOnClickListener {
            messageToast(this, connectionStatus)
        }
    }


    @kotlin.ExperimentalUnsignedTypes
    private fun sendMessage() {
        newMessage = binding.editOutput.text.toString()
        if (connexion) {
            if (newMessage != "") {
                // Encrypt the outgoing message
                outgoing = parseEncrypt(newMessage, mCounter)

                // Increment counter, or roll over if maxed out
                mCounter = if (mCounter == UByte.MAX_VALUE) { 1.toUByte() } else { mCounter.inc() }

                trigMessage = true
                msgCount += 1
                val data = ListData(0, msgCount, "OUT", newMessage)
                // Write values to Database
                writeToDatabase(data)
            } else {
                messageToast(this, "Cannot send NULL message.")
            }
        } else {
            messageToast(this, connectionStatus)
        }
        binding.editOutput.setText("")
    }


    @kotlin.ExperimentalUnsignedTypes
    private fun setConnect() {
        clearDatabase()
        // Définir le message de connexion
        message = "<CONNECT>"
        outgoing = parseEncrypt(message, 0.toUByte())
        // Déclarer la connexion ouverte
        connexion = true
        // Réinitialiser l'indicateur de fin de connexion
        terminateConn = false
        // Modifier le texte sur le bouton de connexion
        binding.buttonConn.text = "Disconnect"
        // Réinitialiser le compteur de messages
        msgCount = 0
        // Verrouiller le bouton de connexion, évite les erreurs de faux état
        binding.buttonConn.isEnabled = false
    }


    @kotlin.ExperimentalUnsignedTypes
    private fun setDisconnect() {
        // Définir le message de déconnexion
        message = "<DISCONN>"
        outgoing = parseEncrypt(message, 0.toUByte())
        // Déclarer la connexion fermée
        connexion = false
        // Modifier le texte sur le bouton de connexion
        binding.buttonConn.text = "Connect"
        // Définir l'indicateur pour fermer le thread client
        terminateConn = true
    }


    // réinitialiser l'etat de la connexion
    fun resetConnection(){
        // Déclarer la connexion fermée
        connexion = false
        // Déclarer la connexion fermée
        terminateConn = true
        // Modifier le texte sur le bouton de connexion
        binding.buttonConn.text = "Connect"
    }


    // Déverrouille le bouton de connexion
    fun unlockButton(){
        // Définir le bouton sur actif
        binding.buttonConn.isEnabled = true
    }


    // Receive and parse bluetooth messages
    // TODO Messages should be decrypted before arriving at this function
    @SuppressLint("SetTextI18n")
    @kotlin.ExperimentalUnsignedTypes
    fun parseMessage(text: Pair<String, UByte>) {
        msgCount += 1
        var data = ListData(0, msgCount, "IN", text.first)

        if (text.second == mCounter) {
            // Increment counter, or roll over if maxed out
            mCounter = if (mCounter == UByte.MAX_VALUE) {
                1.toUByte()
            } else {
                mCounter.inc()
            }
        } else {
            if (text.second == 0.toUByte()) {
                if (text.first == "<CONNECT>") {
                    Log.d("CHECK_CONN", "Connect")
                }
                else if (text.first == "<DISCONN>") {
                    Log.d("CHECK_CONN", "Disconnect")
                }
            } else {
                data = ListData(0, msgCount, "IN", "<COUNTER FAILED>")
            }
        }
        writeToDatabase(data)
    }


    @SuppressLint("UseCompatTextViewDrawableApis")
    private fun parseConnectionStatus(msg: String) {
        // Si le message reçu correspond
        when (msg) {
            "<CON>GOOD_CON\r\n" -> {
                connectionStatus = "Device is connected."
                val colorInt = ContextCompat.getColor(this, R.color.online_green)
                binding.buttonOnline.compoundDrawableTintList = ColorStateList.valueOf(colorInt)
            }
            "<CON>GOOD_DIS\r\n" -> {
                connectionStatus = "Device is not connected."
                val colorInt = ContextCompat.getColor(this, R.color.offline_gray)
                binding.buttonOnline.compoundDrawableTintList = ColorStateList.valueOf(colorInt)
            }
            "<MSG>ERROR(1)" -> {
                connectionStatus = "There is a connection error."
                binding.buttonOnline.compoundDrawableTintList = ColorStateList.valueOf(Color.RED)
            }
            else -> {
                connectionStatus = "There is a connection error"
                binding.buttonOnline.compoundDrawableTintList = ColorStateList.valueOf(Color.RED)
            }
        }
    }


    private fun writeToDatabase(data: ListData) {
        mViewModel.insert(data)
    }


    private fun clearDatabase() {
        msgCount = 0
        mViewModel.delete()
    }

    // Setup BLE BLuetooth Adapter
    private val bluetoothAdapter: BluetoothAdapter by lazy {
        val bluetoothManager = getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothManager.adapter
    }

    // Setup BLE Scanner
    private val bleScanner by lazy {
        bluetoothAdapter.bluetoothLeScanner
    }


    // Setup BLE Scanner Settings
    private val scanSettings = ScanSettings.Builder()
        .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build()


    /*
    // Declare UUID for scan filter
    private val UART_SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e"

    // Filter for BLE Scan
    val filterUART = ScanFilter.Builder().setServiceUuid(
        ParcelUuid.fromString(UART_SERVICE_UUID)
    ).build()
    */


    // On Resume check that Bluetooth is Enabled, else prompt user
    override fun onResume() {
        super.onResume()
        if (!bluetoothAdapter.isEnabled) {
            promptEnableBluetooth()
        }
    }


    private var resultLauncher = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
        if (result.resultCode != Activity.RESULT_OK) {
            promptEnableBluetooth()
        }
    }

    private fun promptEnableBluetooth() {
        if (!bluetoothAdapter.isEnabled) {
            val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
            resultLauncher.launch(enableBtIntent)
        }
    }


    // For requesting fine location access on BLE Scan
    private val isLocationPermissionGranted
        get() = hasPermission(Manifest.permission.ACCESS_FINE_LOCATION)

    private fun Context.hasPermission(permissionType: String): Boolean {
        return ContextCompat.checkSelfPermission(this, permissionType) ==
                PackageManager.PERMISSION_GRANTED
    }


    // If permissions are properly set, Start scanning for BLE Devices
    private fun startBleScan() {
        if (!isLocationPermissionGranted) {
            messageToast(this, "Location access required\n to scan BLE devices.")
        }
        else {
            devices.clear()
            mArrayAdapter!!.clear()
            bleScanner.startScan(null, scanSettings, scanCallback)
            isScanning = true
            val dialog = Dialog(this)
            dialog.requestWindowFeature(Window.FEATURE_NO_TITLE)
            dialog.setCancelable(false)
            dialog.setContentView(R.layout.scan_dialog)
            dialog.window?.setBackgroundDrawableResource(android.R.color.transparent)
            devCounter = dialog.findViewById(R.id.textCount) as TextView
            val pip1 = dialog.findViewById(R.id.imagePip1) as ImageView
            val pip2 = dialog.findViewById(R.id.imagePip2) as ImageView
            val pip3 = dialog.findViewById(R.id.imagePip3) as ImageView
            val pip4 = dialog.findViewById(R.id.imagePip4) as ImageView
            val pip5 = dialog.findViewById(R.id.imagePip5) as ImageView
            val a: Animation = AnimationUtils.loadAnimation(this, R.anim.progress_anim)
            a.duration = 3000
            pip1.startAnimation(a)
            val b: Animation = AnimationUtils.loadAnimation(this, R.anim.progress_anim)
            b.duration = 2800
            b.startOffset = 200
            val c: Animation = AnimationUtils.loadAnimation(this, R.anim.progress_anim)
            c.duration = 2600
            c.startOffset = 400
            val d: Animation = AnimationUtils.loadAnimation(this, R.anim.progress_anim)
            d.duration = 2400
            d.startOffset = 600
            val e: Animation = AnimationUtils.loadAnimation(this, R.anim.progress_anim)
            e.duration = 2200
            e.startOffset = 800
            pip1.startAnimation(a)
            pip2.startAnimation(b)
            pip3.startAnimation(c)
            pip4.startAnimation(d)
            pip5.startAnimation(e)
            val yesBtn = dialog.findViewById(R.id.buttStopScan) as Button
            yesBtn.setOnClickListener {
                dialog.dismiss()
                stopBleScan()
            }
            dialog.show()
        }
    }


    // Stop the BLE Scan
    private fun stopBleScan() {
        bleScanner.stopScan(scanCallback)
        isScanning = false
        if (devices.isNotEmpty()) {
            val dialog: AlertDialog.Builder = AlertDialog.Builder(this)
            dialog.setTitle("Select Device:")
            dialog.setCancelable(false)
            dialog.setAdapter(mArrayAdapter) { _, which: Int ->
                btStart(this, devices[which])
            }
            dialog.setNegativeButton("Cancel") { dialog, _ ->
                dialog.dismiss()
            }
            dialog.show()
        } else {
            errorDialog(this, "No devices detected!")
        }
    }


    private fun btStart(c: Context, device: BluetoothDevice) {
        val name = device.name
        val addr = device.address
        Log.d("SelectDevice", "$name, $addr ")
        device.connectGatt(c, false, gattCallback, TRANSPORT_LE)
    }


    // Scan results are sent to callback when discovered
    private val scanCallback = object : ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult) {
            if (!devices.contains(result.device)) {
                with(result.device) {
                    Log.i("ScanCallback", "Found BLE Device - " +
                            "Name: ${name ?: "UnNamed"}, address: $address")
                }
                devices.add(result.device)
                var dCnt = devices.size
                devCounter.text = dCnt.toString()
                mArrayAdapter!!.add((if (result.device.name != null)
                    result.device.name else "Unknown") + "\n" +
                        result.device.address + "\n" +
                        "RSSI: " + result.rssi)
            }
        }
        override fun onScanFailed(errorCode: Int) {
            Log.e("ScanCallback", "onScanFailed: code $errorCode")
        }
    }


    // If scanning for BLE Devices toggle scan button label
    private var isScanning = false
        //Save Setter example for future use
        //set(value) {
        //    field = value
        //    runOnUiThread { binding.buttonConn.text = if (value) "Stop Scan"
        //                    else "Start Scan" }
        //}


    // CHECK HERE TRY TO FIND WHY CONNECTION FAILS
    private val gattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            val deviceAddress: String = gatt.device.address
            Log.d("BT_STATUS", status.toString())
            if (status == BluetoothGatt.GATT_SUCCESS) {
                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    Log.w("BluetoothGattCallback",
                            "Successfully connected to $deviceAddress")
                    this@MainActivity.bluetoothGatt = gatt
                    Handler(Looper.getMainLooper()).post {
                        bluetoothGatt.discoverServices()
                    }
                } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                    Log.w("BluetoothGattCallback",
                            "Successfully disconnected from $deviceAddress")
                    gatt.close()
                }
            } else {
                Log.w("BluetoothGattCallback",
                        "Error $status encountered for $deviceAddress! Disconnecting...")
                gatt.close()
            }
        }
        override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
            with(gatt) {
                Log.w("BluetoothGattCallback",
                        "Discovered ${services.size} services for ${device.address}")
                printGattTable()
            }
        }
    }


    // Function for printing available Bluetooth Services
    private fun BluetoothGatt.printGattTable() {
        if (services.isEmpty()) {
            Log.i("printGattTable",
                    "No service and characteristic available, call discoverServices() first?")
            return
        }
        services.forEach { service ->
            val characteristicsTable = service.characteristics.joinToString(
                    separator = "\n|--",
                    prefix = "|--"
            ) { it.uuid.toString() }
            Log.i("printGattTable",
                    "\nService ${service.uuid}\nCharacteristics:\n$characteristicsTable")
        }
    }


    // On close activity
    override fun onDestroy() {
        super.onDestroy()
        if (isScanning) {
            bleScanner.stopScan(scanCallback)
            isScanning = false
        }
    }
}

