module Toocal.Core.DataAccessLayer.Page

open System
open ZeroLog

type PageNum = uint64

type Page(num: PageNum, data: Byte[]) =
  static let logger = LogManager.GetLogger("Toocal.Core.DataAccessLayer.Page")

  let mutable _num = num
  let mutable _data = data

  member this.Num
    with get () = _num
    and set (num: PageNum) = _num <- num

  member this.Data
    with get () = _data
    and set (data: Byte[]) = _data <- data

  static member public SIZE = 8
