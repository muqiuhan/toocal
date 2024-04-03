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

import com.muqiuhan.toocal.utils.SystemVirtualMemoryPageSize

import java.io.File
import java.nio.charset.StandardCharsets
import java.io.RandomAccessFile
import java.io.FileInputStream
import scala.util.chaining.*
import java.nio.ByteBuffer

class TestDataAccessLayer extends munit.FunSuite:
  test("openDataBaseFile") {
    val randomFileName = Math.random().toString
    com.muqiuhan.toocal.dal.DataAccessLayer.openDataBaseFile(randomFileName)

    assert(File(randomFileName).exists())
    assert(File(randomFileName).canRead)
    assert(File(randomFileName).canWrite)

    File(randomFileName).delete()
  }

  test("initialize db") {
    val dal =
      try DataAccessLayer("db.db", SystemVirtualMemoryPageSize.SIZE)
      catch
        case e: Exception =>
          File("db.db").delete()
          throw e

    val page = dal.allocateEmptyPage()
    val data = "data".getBytes(StandardCharsets.UTF_8)

    page.num = dal.nextPage
    Array.copy(data, 0, page.data, 0, data.length)

    try dal.writePage(page)
    catch
      case e: Exception =>
        File("db.db").delete()
        dal.close()
        throw e

    dal.writeFreelist()
    dal.close()
  }

  test("load db") {
    val dal =
      try DataAccessLayer("db.db", SystemVirtualMemoryPageSize.SIZE)
      catch
        case e: Exception =>
          File("db.db").delete()
          throw e

    dal.close()
  }

  test("write a new page") {
    val dal =
      try DataAccessLayer("db.db", SystemVirtualMemoryPageSize.SIZE)
      catch
        case e: Exception =>
          File("db.db").delete()
          throw e

    val page = dal.allocateEmptyPage()
    val data = "data2".getBytes(StandardCharsets.UTF_8)

    page.num = dal.nextPage
    Array.copy(data, 0, page.data, 0, data.length)

    try dal.writePage(page)
    catch
      case e: Exception =>
        File("db.db").delete()
        dal.close()
        throw e

    dal.writeFreelist()
    dal.close()
  }
