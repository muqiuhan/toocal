package dal

type PageNum = Long

case class Page(num: PageNum, data: Array[Byte])
