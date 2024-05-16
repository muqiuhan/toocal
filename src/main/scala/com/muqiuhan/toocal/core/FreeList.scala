package com.muqiuhan.toocal.core

import scala.collection
import java.nio.ByteBuffer

case class FreeList(
    var maxPage: PageNumber = 0,
    val releasedPages: collection.mutable.Stack[PageNumber] = collection.mutable.Stack[PageNumber]()
):
    inline def getNextPage: PageNumber =
        if !releasedPages.isEmpty then
            releasedPages.pop()
        else
            maxPage += 1
            maxPage

    inline def releasePage(pageNumber: PageNumber): Unit =
        releasedPages.push(pageNumber)

    def serialize(buffer: ByteBuffer) =
        buffer.putLong(maxPage)
        buffer.putInt(releasedPages.length)
        releasedPages.foreach(buffer.putLong(_))
    end serialize

    def deserialize(buffer: ByteBuffer): Unit =
        maxPage = buffer.getLong()

        for i <- 0 until buffer.getInt() do
            releasedPages.push(buffer.getLong())
    end deserialize

end FreeList
