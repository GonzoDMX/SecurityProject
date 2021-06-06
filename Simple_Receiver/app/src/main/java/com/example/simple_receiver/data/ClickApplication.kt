package com.example.simple_receiver.data

import android.app.Application
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.SupervisorJob

class ClickApplication : Application() {

    val applicationScope = CoroutineScope((SupervisorJob()))

    // Use Lazy to ensure Database and repository are only created when they are needed
    val database by lazy { ListDatabase.getDatabase(this, applicationScope) }
    val repository by lazy { ListRepository(database.listDao()) }
}