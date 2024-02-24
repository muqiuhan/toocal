module Toocal.Core.DataAccessLayer.Freelist

open System
open Toocal.Core.DataAccessLayer.Page
open Toocal.Core.DataAccessLayer
open Toocal.Core.ISerializable

/// Which pages are free and which are occupied.
/// Pages can also be freed if they become empty,
/// so we need to reclaim them for future use to avoid fragmentation.
type Freelist() =

  /// Holds the maximum page allocated. _MaxPage*PageSize = fileSize
  let mutable _MaxPage: PageNum = 0UL

  /// Pages that were previouslly allocated but are now free
  let _ReleasedPages = new Collections.Generic.Stack<PageNum>()

  member this.MaxPage
    with get () = _MaxPage
    and set (maxPage: PageNum) = _MaxPage <- maxPage

  member this.ReleasedPages = _ReleasedPages

  /// If possible, fetch pages first from the released pages.
  /// Else, increase the maximum page
  member public this.NextPage() =
    if _ReleasedPages.Count <> 0 then
      _ReleasedPages.Pop()
    else
      _MaxPage <- _MaxPage + 1UL
      _MaxPage

  member public this.ReleasePage(page: PageNum) = _ReleasedPages.Push page

  member this.Serialize = (this :> ISerializable).Serialize
  member this.Deserialize = (this :> ISerializable).Deserialize

  interface ISerializable with
    member this.Serialize(buffer: Byte[]) =
      let mutable pos = 0
      let serializedMaxPage = this.MaxPage |> uint16 |> BitConverter.GetBytes

      let serializedPageCount =
        this.ReleasedPages.Count |> uint16 |> BitConverter.GetBytes

      Array.blit serializedMaxPage 0 buffer pos serializedMaxPage.Length
      pos <- pos + 2

      Array.blit serializedPageCount 0 buffer pos serializedPageCount.Length
      pos <- pos + 2

      let mutable page = this.ReleasedPages.GetEnumerator()

      while page.MoveNext() do
        let serializedPage = BitConverter.GetBytes page.Current
        Array.blit serializedPage 0 buffer pos serializedPage.Length
        pos <- pos + Page.SIZE

    member this.Deserialize(buffer: Byte[]) =
      let mutable pos = 0
      this.MaxPage <- BitConverter.ToUInt16(buffer[pos..]) |> uint64
      pos <- pos + 2

      let releasedPagesCount = BitConverter.ToUInt16(buffer[pos..]) |> uint64
      pos <- pos + 2

      for _ = 0 to (BitConverter.ToUInt16 buffer[pos..] |> int) - 1 do
        buffer[pos..] |> BitConverter.ToUInt64 |> this.ReleasedPages.Push
        pos <- pos + Page.SIZE
