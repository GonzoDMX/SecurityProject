package com.example.simple_receiver.ui

import androidx.lifecycle.*
import com.example.simple_receiver.data.ListData
import com.example.simple_receiver.data.ListRepository
import kotlinx.coroutines.launch
import java.lang.IllegalArgumentException

class MainViewModel(private val repository: ListRepository) : ViewModel() {

    val allCLicks: LiveData<List<ListData>> = repository.allClicks.asLiveData()

    var msgCount: Int = 0

    fun insert(entry: ListData) = viewModelScope.launch {
        repository.insert(entry)
    }

    fun delete() = viewModelScope.launch {
        repository.delete()
    }

    fun count() = viewModelScope.launch {
        msgCount = repository.count()
    }


}

class MainViewModelFactory(private val repository: ListRepository) : ViewModelProvider.Factory {
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if(modelClass.isAssignableFrom(MainViewModel::class.java)) {
            @Suppress("UNCHECKED_CAST")
            return MainViewModel(repository) as T
        }
        throw IllegalArgumentException("Unknown ViewModel class")
    }
}