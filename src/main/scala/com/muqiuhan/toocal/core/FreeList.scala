package com.muqiuhan.toocal.core

import scala.collection

class FreeList():

    private var _maxPage: PageNumber = 0
    private val releasedPages = collection.mutable.Stack[PageNumber]()

    inline def getNextPage: PageNumber =
        if !releasedPages.isEmpty then releasedPages.pop()
        else
            _maxPage += 1
            _maxPage

    inline def releasePage(pageNumber: PageNumber): Unit = releasedPages.push(pageNumber)
