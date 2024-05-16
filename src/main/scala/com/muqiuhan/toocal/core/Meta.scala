package com.muqiuhan.toocal.core

import java.nio.ByteBuffer

case class Meta(var freelistPage: PageNumber = 1) extends Serializable:

    def serialize(buffer: ByteBuffer)   = buffer.putLong(freelistPage)
    def deserialize(buffer: ByteBuffer) = freelistPage = buffer.getLong()

end Meta

object Meta:
    val PAGE_NUMBER: PageNumber = 0
