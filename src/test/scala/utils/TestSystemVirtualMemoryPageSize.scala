package utils

class TestSystemVirtualMemoryPageSize extends munit.FunSuite:
  test("get") {
    println(s"Current system virtual memory page size: ${SystemVirtualMemoryPageSize.get()}")
  }