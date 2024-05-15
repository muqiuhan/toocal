package com.muqiuhan.toocal.core

import java.nio.ByteBuffer

class Meta extends Serializable:
    private var _freelistPage: PageNumber = 1

    def freelistPage = _freelistPage

    def serialize(buffer: ByteBuffer) =
        buffer.putLong(freelistPage)

    def deserialize(buffer: ByteBuffer) =
        _freelistPage = buffer.getLong()

end Meta

object Meta:
    val PAGE_NUMBER: PageNumber = 0
