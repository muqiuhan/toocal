module Toocal.Core.Meta

open Page
open System

type Meta () =

  let mutable freeListPage : PageNum = 1UL
  let mutable root : PageNum = 2UL

  static member public PAGE_NUM : PageNum = 0UL

  member public this.Root = root
  member public this.FreeListPage = freeListPage

  member public this.SetFreeListPage (newFreeListPage : PageNum) = freeListPage <- newFreeListPage

  member public this.SetRoot (newRoot : PageNum) = root <- newRoot

  member public this.Serialize (buffer : array<byte>) = this.SerializeFreeListPage (buffer, 0) |> this.SerializeRoot

  member public this.Deserialize (buffer : array<byte>) =
    this.DeserializeFreeListPage (buffer, 0) |> this.DeserializeRoot

  member private this.SerializeFreeListPage (buffer : array<byte>, pos : int) =
    let freeListPage = BitConverter.GetBytes freeListPage
    Array.blit freeListPage 0 buffer pos freeListPage.Length
    (buffer, pos + Page.SIZE)

  member private this.SerializeRoot (buffer : array<byte>, pos : int) =
    let root = BitConverter.GetBytes root
    Array.blit root 0 buffer pos root.Length
    (buffer, pos + Page.SIZE)

  member private this.DeserializeFreeListPage (buffer : array<byte>, pos : int) =
    freeListPage <- BitConverter.ToUInt64 buffer
    (buffer, pos + Page.SIZE)

  member private this.DeserializeRoot (buffer : array<byte>, pos : int) =
    root <- BitConverter.ToUInt64 buffer
    (buffer, pos + Page.SIZE)
