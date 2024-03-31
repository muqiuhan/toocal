package dal

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
