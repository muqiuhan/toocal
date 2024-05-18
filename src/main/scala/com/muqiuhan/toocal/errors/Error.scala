package com.muqiuhan.toocal.errors

import com.muqiuhan.toocal.core.PageNumber

enum Error(exception: Throwable = new Exception(this.toString), message: String = this.toString):
    case DataBaseFileNotFound(databaseFilePath: String)            extends Error(message = databaseFilePath)
    case DataAccessLayerCannotSeekPage(pageNumber: PageNumber)     extends Error(message = pageNumber.toString)
    case DataAccessLayerCannotAllocatePage(pageNumber: PageNumber) extends Error(message = pageNumber.toString)
    case DataAccessLayerCannotInitializeDatabase
    case DataAccessLayerCannotReadPage
    case DataAccessLayerCannotWritePage
    case DataAccessLayerCannotWriteMeta
    case DataAccessLayerCannotReadMeta
    case DataAccessLayerCannotReadFreeList
    case DataAccessLayerCannotWriteFreeList
    case DataAccessLayerCannotGetNode
    case DataAccessLayerCannotWriteNode
    case FindNodeWithKeyError(key: Array[Byte]) extends Error(message = key.toString())
    case InvalidPageNumber
    case Unknown

    override def toString: String = s"${exception.getMessage()}: $message"
    inline def raise(): Nothing   = throw exception
end Error
