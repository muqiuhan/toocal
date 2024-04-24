/* Copyright (c) 2023 Muqiu Han
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
 *     * Neither the name of Terifo nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package com.muqiuhan.toocal.dal

import scala.collection.mutable
import java.nio.ByteBuffer
import com.muqiuhan.toocal.dal.Page.PAGE_NUM_SIZE

case class Item(key: Array[Byte], value: Array[Byte])

/** Nodes in the database are implemented as B-tree. */
class Node(dal: DataAccessLayer):
  var pageNum: PageNum = 0L
  var items            = mutable.Queue[Item]()
  var childNodes       = mutable.Queue[PageNum]()

  def isLeaf(): Boolean = childNodes.length == 0

  inline def writeNode                = dal.writeNode
  inline def writeNodes(nodes: Node*) = nodes.foreach(writeNode)
  inline def getNode                  = dal.getNode

  /** Node is stored using slotted pages technique. The page is divided into two
    * memory regions. At end of the page lie the keys and values, whereas at the
    * beginning are fixed size offsets to the records.
    */
  def serialize(buffer: Array[Byte]): Array[Byte] =
    var leftPos    = 0
    var rightPos   = buffer.length - 1
    val isLeafFlag = isLeaf()

    /* page header: isLeaf, key-value pairs count, node num */
    buffer(leftPos) = (if isLeafFlag then 1 else 0).toByte
    leftPos += 1

    ByteBuffer
      .allocate(2)
      .putShort(items.length.toShort)
      .array()
      .copyToArray(buffer, leftPos)
    leftPos += 2

    /* The actual keys and values (the cells) are appended to right of the page
     * whereas offsets have a fixed size and are appended from the left.
     *
     * It's easier to preserve the logical order (alphabetical in the case of b-tree)
     * using the metadata and performing pointer arithmetic.
     *
     * Using the data itself is harder as it varies by size. Page structure is:
     * ----------------------------------------------------------------------------------
     * |  Page  | key-value /  child node    key-value 		      |    key-value		 |
     * | Header |   offset /	 pointer	  offset         .... |      data      ..... |
     * ---------------------------------------------------------------------------------- */
    for i <- 0 until items.length do
      val item = items(i)
      if isLeafFlag then
        /* Write the child page as a fixed size of PAGE_NUM_SIZE bytes */
        ByteBuffer
          .allocate(PAGE_NUM_SIZE)
          .putLong(childNodes(i))
          .array()
          .copyToArray(buffer, leftPos)
        leftPos += Page.PAGE_NUM_SIZE

      /* Write offset */
      ByteBuffer
        .allocate(2)
        .putShort((rightPos - item.key.length - item.value.length - 2).toShort)
        .array()
        .copyToArray(buffer, leftPos)
      leftPos += 2

      rightPos -= item.value.length
      item.value.copyToArray(buffer, rightPos)

      rightPos -= 1
      buffer(rightPos) = item.value.length.toByte

      rightPos -= item.key.length
      item.key.copyToArray(buffer, rightPos)

      rightPos -= 1
      buffer(rightPos) = item.key.length.toByte

    if isLeafFlag then
      ByteBuffer
        .allocate(Page.PAGE_NUM_SIZE)
        .putLong(childNodes.last)
        .array()
        .copyToArray(buffer, leftPos)

    buffer

  def deserialize(buffer: Array[Byte]): Unit =
    val bufferView = ByteBuffer.wrap(buffer)
    val isLeaf     = bufferView.get() == 0

    for i <- 0 until bufferView.getShort() do
      if !isLeaf then childNodes.append(bufferView.getLong())

      var offset = bufferView.getShort().toInt

      val keyLen = bufferView.get(offset)
      offset += 1

      val valueLen = bufferView.get(offset)
      offset += 1

      val key   = new Array[Byte](keyLen)
      val value = new Array[Byte](valueLen)
      items.append(
        Item(
          key = bufferView.get(offset, key).array(),
          value = bufferView.get(offset + keyLen, key).array()
        )
      )

    if !isLeaf then childNodes.append(bufferView.getLong())
