package com.muqiuhan.toocal.core

import java.io.RandomAccessFile
import java.nio.ByteBuffer
import scala.util.{Failure, Success, Try}

class DataAccessLayer(databaseFilePath: String, pageSize: Int) extends FreeList:

    private val file: RandomAccessFile =
        Try[RandomAccessFile](new RandomAccessFile(databaseFilePath, "rw")) match
            case Success(file) => file
            case Failure(e)    => Error.DataBaseFileNotFound.throwWithMessage(databaseFilePath)

    inline def close(): Unit = file.close()

    inline def allocateEmptyPage(pageNumber: PageNumber = -1): Page =
        Page(number = pageNumber, data = ByteBuffer.wrap(new Array[Byte](pageSize)))

    inline private def operatingAtPage[Return, Args](
        pageNumber: PageNumber,
        args: Args,
        operating: (Args => Return)
    ): Return = Try[Unit](file.seek(pageNumber * pageSize)) match
        case Failure(e) => Error.DataAccessLayerCannotSeekPage.throwWithException(e)
        case _          => operating(args)
    end operatingAtPage

    def readPage(pageNumber: PageNumber): Page = operatingAtPage(
      pageNumber,
      (),
      Unit =>
          val page = allocateEmptyPage(pageNumber)
          Try[Unit](file.read(page.data.array())) match
              case Failure(e) => Error.DataAccessLayerCannotReadPage.throwWithException(e)
              case _          => page
    )

    def writePage(page: Page): Unit = operatingAtPage(
      page.number,
      (),
      Unit =>
          Try[Unit](file.write(page.data.array())) match
              case Failure(e) => Error.DataAccessLayerCannotWritePage.throwWithException(e)
              case _          => ()
    )

end DataAccessLayer
