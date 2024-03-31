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

import java.io.{File, RandomAccessFile}

class DataAccessLayer(path: String, pageSize: Int):
  private val file = DataAccessLayer.openDataBaseFile(path)

  def close(): Unit = file.close()

  def allocateEmptyPage(): Page = Page(num = pageSize, data = new Array[Byte](pageSize))

  def readPage(pageNum: PageNum): Page =
    val page = allocateEmptyPage()

    file.read(page.data, pageNum.asInstanceOf[Int] * pageSize, page.data.length) match
      case -1 =>
        throw ReadPageException("There is no more data because the end of the database file has been reached.")
      case bytes: Int if bytes != page.num =>
        throw ReadPageException(s"Incomplete read, successfully read $bytes bytes")
      case _ =>
        page

  def writePage(page: Page): Unit = file.write(page.data, page.num.asInstanceOf[Int] * pageSize, page.data.length)

private object DataAccessLayer:
  def openDataBaseFile(path: String): RandomAccessFile =
    val file = File(path)

    if !file.exists() then
      file.createNewFile()
    else
      if !file.canRead then
        DataBaseFileException("The database file does not have read permission!")
      if !file.canWrite then
        DataBaseFileException("The database file does not have write permission!")

    RandomAccessFile(path, "rw")

class DataBaseFileException(msg: String) extends RuntimeException(msg)
class ReadPageException(msg: String)     extends RuntimeException(msg)
