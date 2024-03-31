package dal

import java.io.File

class TestDataAccessLayer extends munit.FunSuite:
  test("openDataBaseFile") {
    val randomFileName = Math.random().toString
    DataAccessLayer.openDataBaseFile(randomFileName)

    assert(File(randomFileName).exists())
    assert(File(randomFileName).canRead)
    assert(File(randomFileName).canWrite)

    File(randomFileName).deleteOnExit()
  }