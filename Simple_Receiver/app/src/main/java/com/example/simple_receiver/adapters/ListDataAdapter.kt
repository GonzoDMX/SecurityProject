package com.example.simple_receiver.adapters

import android.annotation.SuppressLint
import android.graphics.Color
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.core.content.ContextCompat
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import com.example.simple_receiver.MainActivity
import com.example.simple_receiver.R
import com.example.simple_receiver.connexion
import com.example.simple_receiver.data.ListData

class ListDataAdapter(activity: MainActivity) :
    ListAdapter<ListData, ListDataAdapter.ClickViewHolder>(STEP_COMPARATOR) {


    private var act = activity
    val selColor = ContextCompat.getColor(act, R.color.select_gray)
    var currentHolder: ClickViewHolder? = null

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ClickViewHolder {

        val v: View = LayoutInflater.from(parent.context)
            .inflate(R.layout.recycler_item, parent, false)
        val holder = ClickViewHolder(v)
        setClickListeners(holder)
        return holder
    }

    override fun onBindViewHolder(holder: ClickViewHolder, position: Int) {
        val current = getItem(position)
        holder.bind(current)
    }

    private fun setClickListeners(holder: ClickViewHolder) {
        holder.itemView.setOnClickListener {
            if (!connexion) {
                if (currentHolder != null) {
                    currentHolder!!.itemView.setBackgroundColor(Color.WHITE)
                }
                currentHolder = holder
                holder.itemView.setBackgroundColor(selColor)
            }
        }
    }

    class ClickViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        var clickData: ListData? = null
        private val stepItem: TextView = itemView.findViewById(R.id.text_step)
        private val directionItem: TextView = itemView.findViewById(R.id.text_direction)
        private val messageItem: TextView = itemView.findViewById(R.id.text_message)
        private val timeItem: TextView = itemView.findViewById(R.id.text_time)

        @SuppressLint("SetTextI18nval", "SetTextI18n")
        fun bind(data: ListData) {
            clickData = data
            stepItem.text = data.step.toString()
            directionItem.text = data.direction
            messageItem.text = data.message
            timeItem.text = data.time
        }

    }

    companion object {
        private val STEP_COMPARATOR = object : DiffUtil.ItemCallback<ListData>() {
            override fun areItemsTheSame(oldItem: ListData, newItem: ListData): Boolean {
                return oldItem === newItem
            }

            override fun areContentsTheSame(oldItem: ListData, newItem: ListData): Boolean {
                return oldItem.step == newItem.step
            }
        }
    }
}