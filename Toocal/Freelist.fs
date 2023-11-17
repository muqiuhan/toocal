module Toocal.Core.DataAccessLayer.Freelist

open System
open Toocal.Core.DataAccessLayer.Page
open Toocal.Core.DataAccessLayer.Meta

/// Which pages are free and which are occupied.
/// Pages can also be freed if they become empty,
/// so we need to reclaim them for future use to avoid fragmentation.
type Freelist () =

  /// Holds the maximum page allocated. maxPage*PageSize = fileSize
  let mutable maxPage : PageNum = Meta.META_PAGE_NUM

  /// Pages that were previouslly allocated but are now free
  let releasePages = new Collections.Generic.Stack<PageNum>()

  /// If possible, fetch pages first from the released pages.
  /// Else, increase the maximum page
  member public this.NextPage () =
    if releasePages.Count <> 0 then
      releasePages.Pop()
    else
      maxPage <- maxPage + 1UL
      maxPage

  member public this.ReleasePage (page : PageNum) = releasePages.Push(page)

  member public this.Serialize (buffer : array<Byte>) =
    let mutable pos = 0
    let maxPageSerialized = BitConverter.GetBytes(maxPage |> uint16)

    let releasePageCountSerialized =
      BitConverter.GetBytes(releasePages.Count |> uint16)

    Array.Copy(maxPageSerialized, buffer[pos..], maxPageSerialized.Length)
    pos <- pos + 2

    Array.Copy(
      releasePageCountSerialized,
      buffer[pos..],
      releasePageCountSerialized.Length
    )

    pos <- pos + 2

    let mutable page = releasePages.GetEnumerator()

    while page.MoveNext() do
      let pageSerialized = BitConverter.GetBytes(page.Current)
      Array.Copy(pageSerialized, buffer[pos..], pageSerialized.Length)
      pos <- pos + Page.PAGE_NUM_SIZE

  member public this.Deserialize (buffer : array<Byte>) =
    let mutable pos = 0
    maxPage <- BitConverter.ToUInt16(buffer) |> uint64

    pos <- pos + 2
    let releasePageCount = BitConverter.ToUInt16(buffer[pos..]) |> int
    pos <- pos + 2

    for _ = 0 to releasePageCount do
      releasePages.Push(BitConverter.ToUInt64(buffer[pos..]))
      pos <- pos + Page.PAGE_NUM_SIZE
