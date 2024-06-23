import com.muqiuhan.toocal.*
import com.muqiuhan.toocal.core.DataAccessLayer

import java.nio.charset.StandardCharsets
import com.muqiuhan.toocal.core.writeNode
import com.muqiuhan.toocal.core.Node
import com.muqiuhan.toocal.core.Config

@main
def main(): Unit =
    println("Initialize the database ...")
    var dal  = new DataAccessLayer("db.db", Config.DEFAULT)
    var page = dal.allocateEmptyPage(dal.freelist.getNextPage)
    page.data.put("data".getBytes(StandardCharsets.UTF_8))
    dal.writePage(page)
    dal.writeFreeList()
    dal.close()
    println("close it ...")

    println("open and write a new page to database ...")
    dal = new DataAccessLayer("db.db", Config.DEFAULT)
    page = dal.allocateEmptyPage(dal.freelist.getNextPage)
    page.data.put("data2".getBytes(StandardCharsets.UTF_8))
    dal.writePage(page)
    dal.freelist.releasePage(pageNumber = dal.freelist.getNextPage)
    dal.writeFreeList()
    dal.close()
    println("close it ...")

    println("open and read a page in database ...")
    dal = new DataAccessLayer("db.db", Config.DEFAULT)
    dal.readPage(3) match
        case Left(value) => value.raise()
        case Right(value) => println(value.data.array().toList.slice(0, 4))
    dal.close()
    println("close it ...")
end main
