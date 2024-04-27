module Toocal.Core.DataAccessLayer

open System.IO
open Page
open FreeList
open Meta

type DataAccessLayer (path : string, pageSize : int) =
  let file : FileStream =
    File.Open (path, FileMode.OpenOrCreate, FileAccess.ReadWrite)

  let freeList : FreeList = FreeList ()
  let meta : Meta = Meta ()

  member public this.Meta = meta
  member public this.FreeList = freeList

  member public this.File = file

  member public this.Close () = file.Close ()

  member inline public this.NextPage = this.FreeList.NextPage

  member public this.AllocateEmptyPage () =
    Page (Array.zeroCreate<byte> pageSize)

  member public this.ReadPage (pageNum : PageNum) =
    let page = this.AllocateEmptyPage ()
    let offset = int pageNum * pageSize

    file.Seek (offset, SeekOrigin.Begin) |> ignore

    task {
      let! _ = file.ReadAsync page.Data
      return page
    }

  member public this.WritePage (page : Page) =
    let offset = int page.Num * pageSize
    file.Seek (offset, SeekOrigin.Begin) |> ignore

    file.WriteAsync page.Data

  member public this.ReadMeta () =
    let meta = Meta ()

    task {
      let! page = this.ReadPage Meta.PAGE_NUM
      meta.Deserialize page.Data
      return meta
    }

  member public this.WriteMeta () =
    let page = this.AllocateEmptyPage ()

    page.SetNum Meta.PAGE_NUM
    meta.Serialize page.Data

    task {
      do! this.WritePage page
      return page
    }

  member public this.ReadFreeList () =
    let freeList = FreeList ()

    task {
      let! page = this.ReadPage meta.FreeListPage
      freeList.Deserialize page.Data
      return freeList
    }

  member public this.WriteFreeList () =
    let page = this.AllocateEmptyPage ()
    page.SetNum meta.FreeListPage

    freeList.Deserialize page.Data

    task {
      do! this.WritePage page
      meta.SetFreeListPage page.Num
      return page
    }
