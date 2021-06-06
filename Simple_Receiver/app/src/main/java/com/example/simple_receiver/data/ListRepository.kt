package com.example.simple_receiver.data

import androidx.annotation.WorkerThread
import kotlinx.coroutines.flow.Flow


class ListRepository(private val clickDao: ListDao) {

    // Flow ensures queries to database are performed on a different thread than main/UI
    val allClicks: Flow<List<ListData>> = clickDao.getDescendingByStep()

    @Suppress("RedundantSuspendModifier")
    @WorkerThread
    // suspend tells the compiler this must be run from a coroutine
    suspend fun insert(entry: ListData) {
        clickDao.insert(entry)
    }

    @WorkerThread
    // suspend tells the compiler this must be run from a coroutine
    suspend fun delete() {
        clickDao.deleteAll()
    }

    @WorkerThread
    // suspend tells the compiler this must be run from a coroutine
    suspend fun count(): Int {
        return clickDao.countDB()
    }
}