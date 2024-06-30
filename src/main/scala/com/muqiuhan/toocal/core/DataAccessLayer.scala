package com.muqiuhan.toocal.core

import java.io.RandomAccessFile
import java.nio.ByteBuffer
import scala.util.{Failure, Success, Try}
import com.muqiuhan.toocal.errors.Error
import java.io.File
import java.io.FileNotFoundException

/** Data Access Layer (DAL) handles all disk operations and how data is organized on the disk.
  * It’s responsible for:
  *     1. managing the underlying data structure,
  *     2. writing the database pages to the disk, 
  *     3. and reclaiming free pages to avoid fragmentation.
  *
  * @param databaseFilePath The path to the target database file
  * */
class DataAccessLayer(databaseFilePath: String, config: Config):
    var meta     = new Meta()
    var freelist = new FreeList()

    private val isNewDatabase         = !File(databaseFilePath).exists()
    private val MinFillPercent: Float = 0.5
    private val MaxFillPercent: Float = 0.95

    private val file: RandomAccessFile =
        if isNewDatabase then File(databaseFilePath).createNewFile()

        Try[RandomAccessFile](
            new RandomAccessFile(databaseFilePath, "rw")
        ) match
            case Success(file) => file
            case Failure(e)    => Error.DataAccessLayerCannotInitializeDatabase.raise()
        end match
    end file

    if isNewDatabase then
        meta.freelistPage = freelist.getNextPage
        meta.root = freelist.getNextPage
        writeFreeList().fold(_.raise(), identity)
        writeMeta(meta).fold(_.raise(), identity)
    else
        meta = readMeta().fold(_.raise(), identity)
        freelist = readFreeList().fold(_.raise(), identity)
    end if

    inline def getPageSize: Int    = config.pageSize
    inline def maxThreshold: Float = config.maxFillPercent * config.pageSize
    inline def minThreshold: Float = config.minFillPercent * config.pageSize

    /** Helper functions for reading and writing pages.
      * 
      * @see DataAccessLayer.writePage
      * @param pageNumber The target page number that needs to be operated
      * @param operating Detailed operations
      * @return
      */
    inline private[core] def operatingAtPage[Return](pageNumber: PageNumber, operating: () => Return): Return =
        Try[Unit](file.seek(pageNumber * config.pageSize)) match
            case Failure(e) => Error.DataAccessLayerCannotSeekPage(pageNumber).raise()
            case _          => operating()
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
                data = ByteBuffer.wrap(new Array[Byte](config.pageSize))
            )) match
            case Success(file) => file
            case Failure(e)    => Error.DataAccessLayerCannotAllocatePage(pageNumber).raise()
    end allocateEmptyPage

    /** Read and return a page.
      *
      * @param pageNumber The target page number to be read
      * @return If the read fails, Error.DataAccessLayerCannotReadPage is returned.
      */
    def readPage(pageNumber: PageNumber): Either[Error, Page] =
        operatingAtPage(
            pageNumber,
            () =>
                val page = allocateEmptyPage(pageNumber)
                Try[Unit](file.read(page.data.array())) match
                    case Failure(e) => Left(Error.DataAccessLayerCannotReadPage)
                    case _          => Right(page)
        )
    end readPage

    /** Write a page.
      *
      * @param page The target page to be write
      * @return If the read fails, Error.DataAccessLayerCannotWritePage is returned.
      */
    def writePage(page: Page): Either[Error, Unit] =
        operatingAtPage(
            page.number,
            () =>
                Try[Unit](file.write(page.data.array())) match
                    case Failure(e) => Left(Error.DataAccessLayerCannotWritePage)
                    case _          => Right(())
        )
    end writePage

    /** Serialize and write the meta to the corresponding page.
      * 
      * @see Meta.PAGE_NUMBER
      * @param meta The meta that needs to be written
      * @return If successful, return the written page, otherwise return Error.DataAccessLayerCannotWriteMeta
      */
    def writeMeta(meta: Meta): Either[Error, Page] =
        val page = allocateEmptyPage(Meta.PAGE_NUMBER)
        meta.serialize(page.data)

        operatingAtPage(
            Meta.PAGE_NUMBER,
            () =>
                writePage(page) match
                    case Left(_)   => Left(Error.DataAccessLayerCannotWriteMeta)
                    case Right(()) => Right(page)
        )
    end writeMeta

    /** Read the meta from the corresponding page and deserialize it.
      * 
      * @see Meta.PAGE_NUMBER
      * @return If failure returns Error.DataAccessLayerCannotReadMeta
      */
    def readMeta(): Either[Error, Meta] =
        operatingAtPage(
            Meta.PAGE_NUMBER,
            () =>
                readPage(Meta.PAGE_NUMBER).flatMap(page =>
                    val meta = new Meta()
                    meta.deserialize(page.data)
                    Right(meta)
                ).orElse(Left(Error.DataAccessLayerCannotReadMeta))
        )
    end readMeta

    /** Serialize and write the freelist to the corresponding page.
      * 
      * @see Meta.freelistPage
      * @return If successful, return the written page, otherwise return Error.DataAccessLayerCannotWriteFreelist
      */
    def writeFreeList(): Either[Error, Page] =
        val page = allocateEmptyPage(meta.freelistPage)
        freelist.serialize(page.data)

        operatingAtPage(
            meta.freelistPage,
            () =>
                writePage(page) match
                    case Left(_)      => Left(Error.DataAccessLayerCannotWriteFreeList)
                    case Right(value) => Right(page)
        )
    end writeFreeList

    /** Read the meta from the corresponding page and deserialize it.
      * 
      * @see Meta.PAGE_NUMBER
      * @return If failure returns Error.DataAccessLayerCannotReadMeta
      */
    def readFreeList(): Either[Error, FreeList] =
        operatingAtPage(
            Meta.PAGE_NUMBER,
            () =>
                readPage(meta.freelistPage).flatMap(page =>
                    val freelist = new FreeList()
                    freelist.deserialize(page.data)
                    Right(freelist)
                ).orElse(Left(Error.DataAccessLayerCannotReadFreeList))
        )
    end readFreeList

end DataAccessLayer
