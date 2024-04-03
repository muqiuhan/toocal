package com.muqiuhan.toocal.dal

import java.nio.ByteBuffer

case class Meta(var freeListPage: PageNum = 1):
  def serialize(buffer: Array[Byte]): Unit =
    ByteBuffer.allocate(Page.PAGE_NUM_SIZE).putLong(freeListPage).array.copyToArray(buffer)

  def deserialize(buffer: Array[Byte]): Unit = freeListPage = ByteBuffer.wrap(buffer).getLong()

object Meta:
  val PAGE_NUM: Int = 0
