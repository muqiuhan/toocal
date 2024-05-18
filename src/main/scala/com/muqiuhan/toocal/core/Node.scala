package com.muqiuhan.toocal.core

import java.nio.ByteBuffer

sealed case class Item(
    key: Array[Byte],
    value: Array[Byte]
)

class Node():
    var pageNumber: PageNumber = -1
    var items                  = new collection.mutable.ArrayBuffer[Item]()
    var children               = new collection.mutable.ArrayBuffer[PageNumber]()

    inline def isLeaf(): Boolean = children.length == 0

    def serialize(buffer: ByteBuffer): Unit =
        val isLeafFlag = isLeaf()

        buffer.put((if isLeafFlag then 1 else 0).toByte)
        buffer.putInt(items.length)

        for i <- 0 until items.length do
            val item = items(i)
            if !isLeafFlag then buffer.putLong(children(i))

            buffer
                .putInt(item.key.length)
                .putInt(item.value.length)
                .put(item.key)
                .put(item.value)
        end for

        if !isLeafFlag then buffer.putLong(children.last)
    end serialize

    def deserialize(buffer: ByteBuffer): Unit =
        val isLeafFlag = buffer.get() == 0

        for i <- 0 until buffer.getInt() do
            if !isLeafFlag then children.addOne(buffer.getLong())

            val key   = new Array[Byte](buffer.getInt())
            val value = new Array[Byte](buffer.getInt())

            buffer.get(key).get(value)
            items.addOne(Item(key, value))
        end for
    end deserialize
end Node
