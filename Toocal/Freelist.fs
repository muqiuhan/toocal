module Toocal.Core.DataAccessLayer.Freelist

open System
open Toocal.Core.DataAccessLayer.Page

/// Which pages are free and which are occupied.
/// Pages can also be freed if they become empty,
/// so we need to reclaim them for future use to avoid fragmentation.
type Freelist = {
  /// Holds the maximum page allocated. this.max_page*PageSize = fileSize
  mutable max_page: PageNum

  /// Pages that were previouslly allocated but are now free
  released_pages: Collections.Generic.Stack<PageNum>
} with

  static member public make () = {
    max_page = 0UL
    released_pages = new Collections.Generic.Stack<PageNum> ()
  }

  /// If possible, fetch pages first from the released pages.
  /// Else, increase the maximum page
  member public this.next_page () =
    if this.released_pages.Count <> 0 then
      this.released_pages.Pop ()
    else
      this.max_page <- this.max_page + 1UL
      this.max_page

  member public this.release_page (page: PageNum) =
    this.released_pages.Push page

  member public this.serialize (buffer: array<Byte>) =
    let mutable pos = 0
    let serialized_max_page = this.max_page |> uint16 |> BitConverter.GetBytes

    let serialized_page_count =
      this.released_pages.Count |> uint16 |> BitConverter.GetBytes

    Array.blit serialized_max_page 0 buffer pos serialized_max_page.Length

    pos <- pos + 2
    Array.blit serialized_page_count 0 buffer pos serialized_page_count.Length
    pos <- pos + 2

    let mutable page = this.released_pages.GetEnumerator ()

    while page.MoveNext () do
      let serialized_page = BitConverter.GetBytes page.Current
      Array.blit serialized_page 0 buffer pos serialized_page.Length
      pos <- pos + Page.SIZE

  member public this.deserialize (buffer: array<Byte>) =
    let mutable pos = 0
    this.max_page <- BitConverter.ToUInt16 buffer |> uint64

    pos <- pos + 2

    let mutable released_page_count = BitConverter.ToUInt16 buffer[pos..] |> int

    pos <- pos + 2

    while released_page_count <> 0 do
      buffer[pos..] |> BitConverter.ToUInt64 |> this.released_pages.Push
      pos <- pos + Page.SIZE
      released_page_count <- released_page_count - 1
