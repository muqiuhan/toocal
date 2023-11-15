module Toocal.Core.DataAccessLayer.Page

open System
open ZeroLog

type PageNum = uint64

type Page (Num : PageNum, Data : array<Byte>) =
  static let logger = LogManager.GetLogger("Toocal.Core.DataAccessLayer.Page")

  static member public PAGE_NUM_SIZE = 8

  member public this.Num = Num
  member public this.Data = Data
