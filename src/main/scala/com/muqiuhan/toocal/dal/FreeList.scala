// Copyright (c) 2023 Muqiu Han
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimer in the documentation
//       and/or other materials provided with the distribution.
//     * Neither the name of Terifo nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

package com.muqiuhan.toocal.dal

import java.nio.ByteBuffer
import scala.collection.mutable

class FreeList:
  private var maxPage: PageNum = FreeList.META_PAGE
  private val releasePages: mutable.Stack[PageNum] = mutable.Stack()

  def getNextPage: PageNum =
    if releasePages.nonEmpty then releasePages.pop()
    else
      maxPage = maxPage + 1
      maxPage

  def releasePage(page: PageNum): Unit = releasePages.push(page)

  def serialize(buffer: Array[Byte]): Array[Byte] =
    var pos = 0

    ByteBuffer.allocate(2).putShort(maxPage.toShort).array().copyToArray(buffer, pos)
    pos = pos + 2

    ByteBuffer.allocate(2).putShort(releasePages.length.toShort).array().copyToArray(buffer, pos)
    pos = pos + 2

    releasePages.foreach(page =>
      ByteBuffer.allocate(Page.PAGE_NUM_SIZE).putLong(page).array().copyToArray(buffer, pos)
      pos = pos + Page.PAGE_NUM_SIZE
    )

    buffer

  def deserialize(buffer: Array[Byte]): Unit =
    val bufferView = ByteBuffer.wrap(buffer)
    maxPage = bufferView.getShort()

    for i <- 0 until bufferView.getShort() do releasePages.push(bufferView.getLong())

private object FreeList:
  private val META_PAGE: PageNum = 0L
