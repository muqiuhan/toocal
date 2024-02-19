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
type Dal = {
  path: String
  page_size: int32
  freelist: Freelist
  meta: Meta
  file: IO.FileStream
} with

  static member public make (path: String, page_size: int32) : Dal =
    let self = {
      path = path
      page_size = page_size
      file = Dal.init path
      meta = Meta.make()
      freelist = Freelist.make()
    }

    if IO.Path.Exists (path) then
      { self with meta = self.read_meta(); freelist = self.read_freelist() }
    else
      self.meta.freelist_page <- self.freelist.next_page()
      self.write_freelist() |> ignore
      self.write_meta self.meta |> ignore
      self

  static let logger = LogManager.GetLogger ("Toocal.Core.DataAccessLayer.Dal")

  interface IDisposable with
    member this.Dispose () = this.file.Dispose ()

  static member public init (path: String) : IO.FileStream =
    IO.File.Open (path, IO.FileMode.OpenOrCreate, IO.FileAccess.ReadWrite)

  member public this.alloc_empty_page (num: PageNum) = {
    num = num
    data = Array.zeroCreate<Byte> this.page_size
  }

  member public this.read_page (num: PageNum) =
    let data = Array.zeroCreate<Byte> this.page_size
    let offset = ((num |> int32) * this.page_size) |> int64

    this.file.Seek (offset, IO.SeekOrigin.Begin) |> ignore
    this.file.Read data |> ignore

    { num = num; data = data }

  member public this.write_page (page: Page) =
    let offset = ((page.num |> int32) * this.page_size) |> int64

    this.file.Seek (offset, IO.SeekOrigin.Begin) |> ignore
    this.file.Write page.data |> ignore

  member public this.write_meta (meta: Meta) =
    let page = {
      num = Meta.META_PAGE_NUM
      data = Array.zeroCreate<Byte> this.page_size
    }

    meta.serialize page.data
    this.write_page page

    page

  member public this.read_meta () =
    let page = this.read_page Meta.META_PAGE_NUM
    let meta = Meta.make()

    meta.deserialize(page.data)
    meta

  member public this.write_freelist () =
    let page = this.alloc_empty_page this.meta.freelist_page
    this.freelist.serialize page.data
    this.write_page page

  member public this.read_freelist () =
    let freelist = Freelist.make()
    freelist.deserialize(this.read_page(this.meta.freelist_page).data)
    freelist
