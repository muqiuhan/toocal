package com.muqiuhan.toocal.core

import java.io.RandomAccessFile
import java.nio.ByteBuffer
import scala.util.{Failure, Success, Try}
import com.muqiuhan.toocal.errors.Error

class DataAccessLayer(databaseFilePath: String, pageSize: Int) extends FreeList:
    private val file: RandomAccessFile =
        Try[RandomAccessFile](
            new RandomAccessFile(databaseFilePath, "rw")
        ) match
            case Success(file) => file
            case Failure(e)    => Error.DataBaseFileNotFound(databaseFilePath).raise()

    /** Helper functions for reading and writing pages.
      * 
      * @see DataAccessLayer.writePage
      * @param pageNumber The target page number that needs to be operated
      * @param args
      * @param operating
      * @return
      */
    inline private def operatingAtPage[Return, Args](
        pageNumber: PageNumber,
        args: Args,
        operating: (Args => Return)
    ): Return =
        Try[Unit](file.seek(pageNumber * pageSize)) match
            case Failure(e) => Error.DataAccessLayerCannotSeekPage(pageNumber).raise()
            case _          => operating(args)
    end operatingAtPage

    inline def close(): Unit = file.close()

    /** Allocate an empty page
      * 
      * @note Crash with Error.DataAccessLayerCannotAllocatePage if allocation fails
      * @param pageNumber Page numbers to be allocated
      * @return Successfully allocated Page
      */
    inline def allocateEmptyPage(pageNumber: PageNumber = -1): Page =
        Try[Page](Page(
                number = pageNumber,
                data = ByteBuffer.wrap(new Array[Byte](pageSize))
            )) match
            case Success(file) => file
            case Failure(e)    => Error.DataAccessLayerCannotAllocatePage(pageNumber).raise()
    end allocateEmptyPage

    def readPage(pageNumber: PageNumber): Either[Error, Page] =
        operatingAtPage(
            pageNumber,
            (),
            Unit =>
                val page = allocateEmptyPage(pageNumber)
                Try[Unit](file.read(page.data.array())) match
                    case Failure(e) => Left(Error.DataAccessLayerCannotReadPage)
                    case _          => Right(page)
        )
    end readPage

    def writePage(page: Page): Either[Error, Unit] =
        operatingAtPage(
            page.number,
            (),
            Unit =>
                Try[Unit](file.write(page.data.array())) match
                    case Failure(e) => Left(Error.DataAccessLayerCannotWritePage)
                    case _          => Right(())
        )
    end writePage

    def writeMeta(meta: Meta): Either[Error, Unit] =
        operatingAtPage(
            Meta.PAGE_NUMBER,
            (),
            Unit =>
                val page = allocateEmptyPage(pageNumber = Meta.PAGE_NUMBER)
                meta.serialize(page.data)
                writePage(page).orElse(Left(Error.DataAccessLayerCannotWriteMeta))
        )
    end writeMeta

    def readMeta(): Either[Error, Meta] =
        operatingAtPage(
            Meta.PAGE_NUMBER,
            (),
            Unit =>
                readPage(Meta.PAGE_NUMBER).flatMap(page =>
                    val meta = new Meta()
                    meta.deserialize(page.data)
                    Right(meta)
                ).orElse(Left(Error.DataAccessLayerCannotReadMeta))
        )
    end readMeta

end DataAccessLayer
