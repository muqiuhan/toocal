module Toocal.Core.Meta

open Page
open System

type Meta () =
  let mutable freeListPage : PageNum = 1UL

  static member public PAGE_NUM : PageNum = 0UL

  member public this.FreeListPage = freeListPage

  member public this.SetFreeListPage (newFreeListPage : PageNum) =
    freeListPage <- newFreeListPage

  member public this.Serialize (buffer : array<byte>) =
    let (buffer, pos) = this.SerializeFreeListPage (buffer, 0)
    ()

  member public this.Deserialize (buffer : array<byte>) =
    let (buffer, pos) = this.DeserializeFreeListPage (buffer, 0)
    ()

  member private this.SerializeFreeListPage (buffer : array<byte>, pos : int) =
    let freeListPage = BitConverter.GetBytes (freeListPage)
    Array.blit freeListPage 0 buffer pos freeListPage.Length
    (buffer, pos + Page.SIZE)

  member private this.DeserializeFreeListPage
    (
      buffer : array<byte>,
      pos : int
    )
    =
    freeListPage <- BitConverter.ToUInt64 (buffer)
    (buffer, pos + Page.SIZE)
