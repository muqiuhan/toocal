package com.muqiuhan.toocal.core

import scala.collection
import java.nio.ByteBuffer

/** FreeList is used to manage pages. It indicates which pages are occupied or free.
  * FreeList can reclaim free pages to prevent fragmentation.
  */
class FreeList():
    private var maxPage: PageNumber                                 = 0
    private val releasedPages: collection.mutable.Stack[PageNumber] = collection.mutable.Stack[PageNumber]()

    /** Get the next page number.
      * 
      * @return If there is a free page number, return it, otherwise return maxPage + 1.
      */
    inline def getNextPage: PageNumber =
        if !releasedPages.isEmpty then
            releasedPages.pop()
        else
            maxPage += 1
            maxPage

    /** Release a page, here the instructions are to simply push it into releasedPages
      *
      * @param pageNumber Page number to be released
      */
    inline def releasePage(pageNumber: PageNumber): Unit = releasedPages.push(pageNumber)

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
