package utils

import java.lang.reflect.Field
import sun.misc.Unsafe

object SystemVirtualMemoryPageSize:
  /**
    * Get the virtual memory page size of the system. This return value is usually in kb.
    * For example, on most Linux machines, the return value is 4096.
    * 
    * More about the system virtual memory page: https://en.wikipedia.org/wiki/Page_(computer_memory)
    *
    * @return system virtual memory page size in kb.
    */
  def get(): Int =
    val field: Field = classOf[Unsafe].getDeclaredField("theUnsafe")
    field.setAccessible(true)
    field.get(null).asInstanceOf[Unsafe].pageSize()