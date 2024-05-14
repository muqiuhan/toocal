import com.muqiuhan.toocal.*
import com.muqiuhan.toocal.core.DataAccessLayer

import java.nio.charset.StandardCharsets

@main
def main(): Unit =
    val dal = new DataAccessLayer("db.db", 4096)
    val page = dal.allocateEmptyPage(dal.getNextPage)
    page.data.put("data".getBytes(StandardCharsets.UTF_8))
    dal.writePage(page)
    dal.close()
