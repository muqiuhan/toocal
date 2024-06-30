package com.muqiuhan.toocal.core

import java.nio.ByteBuffer
import java.lang.reflect.Field
import sun.misc.Unsafe
import scala.util.Try
import scala.util.Failure
import scala.util.Success
import com.muqiuhan.toocal.errors.Error

type PageNumber = Long

case class Page(
    var number: PageNumber = -1,
    data: ByteBuffer
)

object Page:
    val SIZE: Int = 8
    val DATA_SIZE: Int =
        Try[Int](
            Page.getSystemPageSize
        ) match
            case Failure(e)     => Error.CannotGetSystemPageSize.raise()
            case Success(value) => value

    def getSystemPageSize: Int =
        val field = classOf[Unsafe].getDeclaredField("theUnsafe")
        field.setAccessible(true)
        field.get(null).asInstanceOf[Unsafe].pageSize()
    end getSystemPageSize
end Page
