package com.example.simple_receiver.data

import androidx.room.Dao
import androidx.room.Insert
import androidx.room.OnConflictStrategy
import androidx.room.Query
import kotlinx.coroutines.flow.Flow

@Dao
interface ListDao {

    // Returns a list of all entries in database, ordered by step value, ascending
    @Query("SELECT * FROM list_table ORDER BY step ASC")
    fun getAscendingByStepDown(): Flow<List<ListData>>

    // Returns a list of all entries in database, ordered by step value, descending
    @Query("SELECT * FROM list_table ORDER BY step DESC")
    fun getDescendingByStep(): Flow<List<ListData>>

    // Returns a list of all entries in database, ordered by time values
    @Query("SELECT * FROM list_table ORDER BY time ASC")
    fun getOrderedByTime(): List<ListData>

    // Conflict strategy will replace old value with new value if there is a conflict
    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insert(entry: ListData)

    // Clears all entries from the table
    @Query("DELETE FROM list_table")
    suspend fun deleteAll()

    @Query("SELECT COUNT(step) FROM list_table")
    suspend fun countDB(): Int

}
