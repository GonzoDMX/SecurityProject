package com.example.messenger_ble.data

import android.icu.text.SimpleDateFormat
import androidx.room.ColumnInfo
import androidx.room.Entity
import androidx.room.PrimaryKey
import java.util.*

@Entity(tableName = "list_table")
data class ListData(
    @PrimaryKey(autoGenerate = true) val click_id: Int,
    @ColumnInfo(name = "step") var step: Int,
    @ColumnInfo(name = "direction") val direction: String,
    @ColumnInfo(name = "message") val message: String,
    @ColumnInfo(name = "time")
    val time: String = SimpleDateFormat("HH:mm:ss:SSS").format(Date())
)