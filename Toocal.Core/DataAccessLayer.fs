module Toocal.Core.DataAccessLayer

open System.IO
open Page
open FreeList

type DataAccessLayer (path : string, pageSize : int) =
  inherit FreeList ()

  let file : FileStream =
    File.Open (path, FileMode.OpenOrCreate, FileAccess.ReadWrite)

  member public this.File = file

  member public this.Close () = file.Close ()

  member public this.AllocateEmptyPage () =
    new Page (Array.zeroCreate<byte> (pageSize))

  member public this.ReadPage (pageNum : PageNum) =
    let page = this.AllocateEmptyPage ()
    let offset = int (pageNum) * pageSize

    file.Seek (offset, SeekOrigin.Begin) |> ignore

    task {
      let! _ = file.ReadAsync (page.Data)
      return page
    }

  member public this.WritePage (page : Page) =
    let offset = int (page.Num) * pageSize
    file.Seek (offset, SeekOrigin.Begin) |> ignore

    file.WriteAsync (page.Data)
