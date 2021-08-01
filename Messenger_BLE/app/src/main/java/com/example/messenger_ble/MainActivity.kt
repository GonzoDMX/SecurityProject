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
import com.example.messenger_ble.utilities.`Encryption_AES128-GCM`.checkHexValues
import com.example.messenger_ble.utilities.`Encryption_AES128-GCM`.parseDecrypt
import com.example.messenger_ble.utilities.`Encryption_AES128-GCM`.parseEncrypt
import com.example.messenger_ble.utilities.errorDialog
import com.example.messenger_ble.utilities.messageToast
import java.util.*


@ExperimentalUnsignedTypes
class MainActivity : AppCompatActivity() {

    private var txCounter: UByte = 1.toUByte()
    private var rxCounter: UByte = 1.toUByte()

    private var msgCount: Int = 0

    private var connectionStatus: String = "Device not connected."

    private lateinit var binding: ActivityMainBinding

    private val mViewModel: MainViewModel by viewModels {
        MainViewModelFactory((application as ClickApplication).repository)
    }

    // Déclarer la baie contient l'ID de périphérique Bluetooth
    private var devices = ArrayList<BluetoothDevice>()
    // Déclarer l'adaptateur Array, contient des chaînes pour Bluetooth AlertDialog
    private var mArrayAdapter: ArrayAdapter<String>? = null

    lateinit var devCounter : TextView

    lateinit var myGatt: BluetoothGatt

    // If scanning for BLE Devices toggle scan button label
    private var isScanning = false

    // Déclarer l'indicateur d'état de la connexion
    var connexion = false
        @SuppressLint("UseCompatTextViewDrawableApis")
        set(value) {
            field = value
            val colorInt = ContextCompat.getColor(this, R.color.online_green)
            runOnUiThread { binding.buttonOnline.compoundDrawableTintList = if (value) ColorStateList.valueOf(colorInt)
            else ColorStateList.valueOf(Color.RED) }
            runOnUiThread { binding.buttonConn.text = if (value) "Disconnect"
            else "Start Scan"}
        }



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
            if (!connexion) {
                startBleScan()
            } else {
                disconnectBle()
            }
        }

        binding.buttonSend.setOnClickListener {
            if (connexion) {
                val message: String = binding.editOutput.text.toString()
                sendMessage(message)
            } else {

            }
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


    private fun sendMessage(message: String) {
        if (message != "") {
            // Encrypt Outgoing message
            val cipherText = parseEncrypt(message, txCounter)
            writeToBLE(cipherText)

            msgCount += 1
            val data = ListData(0, msgCount, "OUT", message)
            // Write values to Database
            writeToDatabase(data)
        } else {
            messageToast(this, "Cannot send NULL message.")
        }
        binding.editOutput.setText("")
    }



    // Receive and parse bluetooth messages
    @SuppressLint("SetTextI18n")
    fun parseMessage(text: Pair<String, UByte>) {
        msgCount += 1
        val data = ListData(0, msgCount, "IN", text.first)

        if (text.second == rxCounter) {
            // Increment counter, or roll over if maxed out
            rxCounter = if (rxCounter == UByte.MAX_VALUE) {
                0.toUByte()
            } else {
                rxCounter.inc()
            }
        } else {
            Log.d("BadCounter", "${text.second}")
        }
        writeToDatabase(data)
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
        else if (!bluetoothAdapter.isEnabled) {
            promptEnableBluetooth()
        }
        else {
            clearDatabase()
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
            dialog.setNegativeButton("Cancel") { dlg, _ ->
                dlg.dismiss()
            }
            dialog.show()
        } else {
            errorDialog(this, "No devices detected!")
        }
    }

    private fun disconnectBle() {
        this.connectionStatus = "Device not connected"
        this.connexion = false
        this.myGatt.close()
        this.txCounter = 1.toUByte()
        this.rxCounter = 1.toUByte()
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
                val dCnt = devices.size
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

    // CHECK HERE TRY TO FIND WHY CONNECTION FAILS
    private val gattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            val dAddr: String = gatt.device.address
            val dName: String = gatt.device.name
            Log.d("BT_STATUS", status.toString())
            if (status == BluetoothGatt.GATT_SUCCESS) {
                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    // Set message for connection status toast
                    this@MainActivity.connectionStatus = "Connected to $dName"
                    connexion = true
                            Log.w("BluetoothGattCallback",
                            "Successfully connected to $dAddr")
                    this@MainActivity.myGatt = gatt
                    Handler(Looper.getMainLooper()).post {
                        myGatt.discoverServices()
                    }
                } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                    Log.w("BluetoothGattCallback",
                            "Successfully disconnected from $dAddr")
                    disconnectBle()
                }
            } else {
                Log.w("BluetoothGattCallback",
                        "Error $status encountered for $dAddr! Disconnecting...")
                disconnectBle()
            }
        }
        override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
            with(gatt) {
                Log.w("BluetoothGattCallback",
                        "Discovered ${services.size} services for ${device.address}")
                printGattTable()
                val mtuSize = 515
                myGatt.requestMtu(mtuSize)
            }
        }
        override fun onMtuChanged(gatt: BluetoothGatt?, mtu: Int, status: Int) {
            super.onMtuChanged(gatt, mtu, status)
            Log.d("GattCallback", "ATT MTU Size changed to: $mtu")
            testRead()
        }
        override fun onCharacteristicChanged(gatt: BluetoothGatt?,
                                             characteristic: BluetoothGattCharacteristic?) {
            with (characteristic) {
                Log.d("GattNotify", "Notify on ${this?.uuid} | value: ${checkHexValues(this!!.value)}")
                if (!this?.value?.equals(null)!!) {
                    receiveFromBLE(value)
                }
            }
        }
        override fun onCharacteristicRead(gatt: BluetoothGatt?,
                                          characteristic: BluetoothGattCharacteristic?,
                                          status: Int) {
            with (characteristic) {
                when (status) {
                    BluetoothGatt.GATT_SUCCESS -> {
                        Log.d("GattRead", "Read ${this?.uuid} \nvalue: ${this?.value?.toUByteArray()}")
                    }
                    BluetoothGatt.GATT_READ_NOT_PERMITTED -> {
                        Log.d("GattRead", "Read not permitted!")
                    }
                    else -> {
                        Log.d("GattRead", "Read failed! error: $status")
                    }
                }
            }
        }
        override fun onCharacteristicWrite(gatt: BluetoothGatt,
                                           characteristic: BluetoothGattCharacteristic,
                                           status: Int) {
            with (characteristic) {
                when (status) {
                    BluetoothGatt.GATT_SUCCESS -> {
                        Log.d("GattWrite", "Wrote to $uuid | value: ${checkHexValues(value)}")
                    }
                    BluetoothGatt.GATT_INVALID_ATTRIBUTE_LENGTH -> {
                        Log.d("GattWrite", "Write exceeds ATT MTU")
                    }
                    BluetoothGatt.GATT_WRITE_NOT_PERMITTED -> {
                        Log.d("GattWrite", "Write not permitted for $uuid")
                    }
                    else -> {
                        Log.d("GattWrite", "Write failed! error: $status")
                    }
                }
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

    val uartServiceUuid = UUID.fromString("6e400001-b5a3-f393-e0a9-e50e24dcca9e")
    val uartTxUuid = UUID.fromString("6e400002-b5a3-f393-e0a9-e50e24dcca9e")
    val uartRxUuid = UUID.fromString("6e400003-b5a3-f393-e0a9-e50e24dcca9e")


    private fun writeToBLE(outgoing: ByteArray) {
        val gattChar = myGatt.getService(uartServiceUuid).getCharacteristic(uartTxUuid)
        val writeType = when {
            gattChar.isWritable() -> {
                Log.d("GattWrite", "Type: With Response")
                BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
            }
            gattChar.isWritableWithoutResponse() -> {
                Log.d("GattWrite", "Type: No Response")
                BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
            }
            else -> Log.d("GattCharError", "Characteristic is not writable")
        }
        myGatt.let {gatt ->
            gattChar.writeType = writeType
            gattChar.value = outgoing
            gatt.writeCharacteristic(gattChar)
        }
    }

    private fun receiveFromBLE(received: ByteArray): String {
        val cipherText: ByteArray = received.drop(3).toByteArray()
        val plainText = parseDecrypt(cipherText, cipherText.size)
        if (plainText.first == "${0x06}ACK") {
            if (plainText.second == UByte.MAX_VALUE) {
                txCounter = 0.toUByte()
            } else {
                txCounter = plainText.second
                txCounter = txCounter.inc()
            }
        } else {
            parseMessage(plainText)
        }
        return plainText.first
    }

    private fun testRead() {
        val gattChar = myGatt.getService(uartServiceUuid).getCharacteristic(uartRxUuid)
        if (gattChar.isReadable()) {
            Log.d("GattRead", "Attempting to read")
            myGatt.readCharacteristic(gattChar)
        }
        if (gattChar.isIndicatable()) {
            Log.d("GattRead", "It is indicatable")
        }
        if (gattChar.isNotifiable()) {
            Log.d("GattRead", "It is Notifiable")
            enableNotifiactions(gattChar)
        }
    }


    fun enableNotifiactions(characteristic: BluetoothGattCharacteristic) {
        val cccdUuid = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb")
        val payload: ByteArray = when {
            characteristic.isIndicatable() -> BluetoothGattDescriptor.ENABLE_INDICATION_VALUE
            characteristic.isNotifiable() -> BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
            else -> {
                Log.d("GattDescriptor", "${characteristic.uuid} doesn't support this")
                return
            }
        }
        characteristic.getDescriptor(cccdUuid)?.let { cccDescriptor ->
            if (!myGatt.setCharacteristicNotification(characteristic, true)) {
                Log.d("GattDescriptor", "Set notify failed for ${characteristic.uuid}")
                return
            }
            writeDescriptor(cccDescriptor, payload)
        } ?: Log.d("GattDescriptor", "${characteristic.uuid} doesn't contain the CCC Descriptor")
    }

    fun writeDescriptor(descriptor: BluetoothGattDescriptor, payload: ByteArray) {
        myGatt.let { gatt ->
            descriptor.value = payload
            gatt.writeDescriptor(descriptor)
        }
    }


    // For determining characteristics of a BLE UUID Property Readable, Writable etc...
    fun BluetoothGattCharacteristic.isReadable(): Boolean =
            containsProperty(BluetoothGattCharacteristic.PROPERTY_READ)
    fun BluetoothGattCharacteristic.isWritable(): Boolean =
            containsProperty(BluetoothGattCharacteristic.PROPERTY_WRITE)
    fun BluetoothGattCharacteristic.isWritableWithoutResponse(): Boolean =
            containsProperty(BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE)
    fun BluetoothGattCharacteristic.isIndicatable(): Boolean =
            containsProperty(BluetoothGattCharacteristic.PROPERTY_INDICATE)
    fun BluetoothGattCharacteristic.isNotifiable(): Boolean =
            containsProperty(BluetoothGattCharacteristic.PROPERTY_NOTIFY)
    fun BluetoothGattCharacteristic.containsProperty(property: Int): Boolean {
        return properties and property != 0
    }


    // On close activity
    override fun onDestroy() {
        super.onDestroy()
        if (isScanning) {
            bleScanner.stopScan(scanCallback)
            isScanning = false
        }
        if (connexion) {
            connectionStatus = "Device not connected"
            connexion = false
            myGatt.close()
            txCounter = 1.toUByte()
            rxCounter = 1.toUByte()
        }
    }
}

