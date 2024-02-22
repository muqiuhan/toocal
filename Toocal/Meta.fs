module Toocal.Core.DataAccessLayer.Meta

open System
open Toocal.Core.DataAccessLayer.Page

type Meta () =
  let mutable _FreelistPage: PageNum = 0UL

  member this.FreelistPage
    with get () = _FreelistPage
    and set (freelistPage: PageNum) = _FreelistPage <- freelistPage

  static member public META_PAGE_NUM: PageNum = 0UL

  member this.Serialize (buffer: Byte[]) =
    let serialized_freelist = BitConverter.GetBytes _FreelistPage
    Array.blit serialized_freelist 0 buffer 0 serialized_freelist.Length

  member this.Deserialize (buffer: Byte[]) =
    _FreelistPage <- BitConverter.ToUInt64 buffer
