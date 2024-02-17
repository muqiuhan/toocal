module Toocal.Core.DataAccessLayer.Page

open System
open ZeroLog

type PageNum = uint64

type Page = {
  num: PageNum
  data: array<Byte>
} with

  static let logger = LogManager.GetLogger ("Toocal.Core.DataAccessLayer.Page")
  static member public SIZE = 8
