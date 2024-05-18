import com.muqiuhan.toocal.*
import com.muqiuhan.toocal.core.DataAccessLayer

import java.nio.charset.StandardCharsets
import com.muqiuhan.toocal.core.writeNode
import com.muqiuhan.toocal.core.Node

@main
def main(): Unit =
    var dal  = new DataAccessLayer("db.db", 4096)
    var page = dal.allocateEmptyPage(dal.freelist.getNextPage)
    page.data.put("data".getBytes(StandardCharsets.UTF_8))
    dal.writePage(page)
    dal.writeFreeList()
    dal.close()

    dal = new DataAccessLayer("db.db", 4096)
    page = dal.allocateEmptyPage(dal.freelist.getNextPage)
    page.data.put("data2".getBytes(StandardCharsets.UTF_8))
    dal.writePage(page)
    dal.freelist.releasePage(pageNumber = dal.freelist.getNextPage)
    dal.writeFreeList()
    dal.close()
end main
