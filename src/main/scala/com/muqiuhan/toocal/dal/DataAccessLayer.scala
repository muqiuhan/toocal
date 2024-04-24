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

import java.io.{File, RandomAccessFile}
import com.muqiuhan.toocal.log.ToocalLogger.log

/** Data Access Layer (DAL) handles all disk operations and how data is
  * organized on the disk. It’s responsible for managing the underlying data
  * structure, writing the database pages to the disk, and reclaiming free pages
  * to avoid fragmentation.
  */
class DataAccessLayer(path: String, pageSize: Int):
  private val (file, databaseFileStatus) =
    DataAccessLayer.openDataBaseFile(path)

  private var freeList = FreeList()
  private var meta     = Meta()

  /* Initialization logic,
   * if the database file exists, load it,
   * otherwise create a new database file and write freelist and meta */
  databaseFileStatus match
    case DataBaseFileStatus.Exist =>
      log.info(s"Loading the database $path...")
      meta = readMeta()
      freeList = readFreeList()
    case DataBaseFileStatus.NotExist =>
      log.info(s"Initialize the database $path...")
      meta.freeListPage = nextPage
      writeFreelist()
      writeMeta(meta)

  inline def nextPage    = freeList.getNextPage
  inline def releasePage = freeList.releasePage

  def close(): Unit = file.close()

  def allocateEmptyPage(): Page =
    Page(num = pageSize, data = new Array[Byte](pageSize))

  def readPage(pageNum: PageNum): Page =
    val page = allocateEmptyPage()

    file.seek(pageNum.toInt * pageSize)
    file.read(page.data) match
      case -1 =>
        throw ReadPageException(
          "There is no more data because the end of the database file has been reached."
        )
      case bytes: Int if bytes != page.num =>
        throw ReadPageException(
          s"Incomplete read, successfully read $bytes bytes"
        )
      case _ =>
        page

  def writePage(page: Page): Unit =
    file.seek(page.num.toInt * pageSize)
    file.write(page.data)

  def writeMeta(meta: Meta): Page =
    val page = allocateEmptyPage()
    page.num = Meta.PAGE_NUM
    meta.serialize(page.data)
    writePage(page)
    page

  def readMeta(): Meta =
    val meta = Meta()
    meta.deserialize(readPage(Meta.PAGE_NUM).data)
    meta

  def writeFreelist(): Unit =
    val page = allocateEmptyPage()
    page.num = meta.freeListPage
    freeList.serialize(page.data)
    writePage(page)
    meta.freeListPage = page.num

  def readFreeList(): FreeList =
    val freeList = FreeList()
    freeList.deserialize(readPage(meta.freeListPage).data)
    freeList

  def getNode(pageNum: PageNum): Node =
    val page = readPage(pageNum)
    val node = Node(this)

    node.deserialize(page.data)
    node.pageNum = page.num

    node

  def writeNode(node: Node): Node =
    val page = allocateEmptyPage()
    if node.pageNum == 0 then
      page.num = nextPage
      node.pageNum = page.num
    else page.num = node.pageNum

    node.serialize(page.data)
    writePage(page)

    node

  inline def deleteNode(pageNum: PageNum): Unit = releasePage(pageNum)

private object DataAccessLayer:
  def openDataBaseFile(path: String): (RandomAccessFile, DataBaseFileStatus) =
    val file = File(path)

    if !file.exists() then
      file.createNewFile()
      (RandomAccessFile(path, "rw"), DataBaseFileStatus.NotExist)
    else
      if !file.canRead then
        DataBaseFileException(
          "The database file does not have read permission!"
        )
      if !file.canWrite then
        DataBaseFileException(
          "The database file does not have write permission!"
        )
      (RandomAccessFile(path, "rw"), DataBaseFileStatus.Exist)

private enum DataBaseFileStatus:
  case Exist
  case NotExist

class DataBaseFileException(msg: String) extends RuntimeException(msg)
class ReadPageException(msg: String)     extends RuntimeException(msg)
