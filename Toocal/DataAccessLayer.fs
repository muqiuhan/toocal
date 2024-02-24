module Toocal.Core.DataAccessLayer.Dal

open System
open Toocal.Core.DataAccessLayer.Freelist
open Toocal.Core.DataAccessLayer.Page
open Toocal.Core.DataAccessLayer.Meta
open ZeroLog

/// Data Access Layer (DAL) handles all disk operations and how data is
/// organized on the disk. It’s responsible for managing the underlying data
/// structure, writing the database pages to the disk, and reclaiming free pages
/// to avoid fragmentation.
type Dal(Path: String, PageSize: int32) as self =
  static let logger = LogManager.GetLogger("Toocal.Core.DataAccessLayer.Dal")

  let mutable _Freelist: Freelist = new Freelist()
  let mutable _Meta: Meta = new Meta()

  let _File: IO.FileStream =
    IO.File.Open(Path, IO.FileMode.OpenOrCreate, IO.FileAccess.ReadWrite)

  do
    if IO.Path.Exists(Path) then
      _Meta <- self.ReadMeta()
      _Freelist <- self.ReadFreelist()

    else
      _Meta.FreelistPage <- _Freelist.NextPage()
      self.WriteFreelist() |> ignore
      self.WriteMeta _Meta |> ignore

  member public this.Freelist = _Freelist

  interface IDisposable with
    member this.Dispose() = _File.Dispose()

  member public this.AllocEmptyPage() = new Page(0UL, Array.zeroCreate<Byte> PageSize)

  member public this.ReadPage(num: PageNum) =
    let data = Array.zeroCreate<Byte> PageSize
    let offset = ((num |> int32) * PageSize) |> int64

    _File.Seek(offset, IO.SeekOrigin.Begin) |> ignore
    _File.Read data |> ignore

    new Page(num, data)

  member public this.WritePage(page: Page) =
    let offset = ((page.Num |> int32) * PageSize) |> int64

    _File.Seek(offset, IO.SeekOrigin.Begin) |> ignore
    _File.Write page.Data |> ignore

  member public this.WriteMeta(meta: Meta) =
    let page = Page(Meta.META_PAGE_NUM, Array.zeroCreate<Byte> PageSize)

    meta.Serialize page.Data
    this.WritePage page

    page

  member public this.ReadMeta() =
    let page = this.ReadPage Meta.META_PAGE_NUM
    let meta = new Meta()

    meta.Deserialize(page.Data)
    meta

  member public this.WriteFreelist() =
    let page = this.AllocEmptyPage()
    page.Num <- _Meta.FreelistPage
    _Freelist.Serialize page.Data
    this.WritePage page

  member public this.ReadFreelist() =
    let freelist = new Freelist()
    freelist.Deserialize(this.ReadPage(_Meta.FreelistPage).Data)
    freelist
