package com.muqiuhan.toocal.core

import java.nio.ByteBuffer

/** Meta is a special page that stores the status information of the database system itself.
  * The page number of Meta is always 0.
  */
class Meta() extends Serializable:
    var freelistPage: PageNumber = -1

    def serialize(buffer: ByteBuffer)   = buffer.putLong(freelistPage)
    def deserialize(buffer: ByteBuffer) = freelistPage = buffer.getLong()
end Meta

object Meta:
    val PAGE_NUMBER: PageNumber = 0
