module Toocal.Core.FreeList

open Page
open System.Collections.Generic
open System

type FreeList () =
  let mutable maxPage : PageNum = 0UL
  let releasedPages : Stack<PageNum> = new Stack<PageNum> ()

  member public this.MaxPage = maxPage
  member public this.ReleasedPage = releasedPages

  member public this.NextPage () =
    if releasedPages.Count <> 0 then
      releasedPages.Pop ()
    else
      maxPage <- maxPage + 1UL
      maxPage

  member public this.ReleasePage (pageNum : PageNum) =
    releasedPages.Push (pageNum)

  member public this.Serialize (buffer : array<byte>) =
    this.SerializeMaxPage (buffer, 0) |> this.SerializeReleasedPages |> fst

  member public this.Deserialize (buffer : array<byte>) =
    this.DeserializeMaxPage (buffer, 0)
    |> this.DeserializeReleasedPages
    |> ignore

  member private this.SerializeMaxPage (buffer : array<byte>, pos : int) =
    let maxPage = BitConverter.GetBytes (maxPage)
    Array.blit maxPage 0 buffer pos maxPage.Length
    (buffer, pos + Page.SIZE)

  member private this.SerializeReleasedPages (buffer : array<byte>, pos : int) =
    let releasePagesCount = BitConverter.GetBytes (releasedPages.Count)
    Array.blit releasePagesCount 0 buffer pos releasePagesCount.Length

    let mutable releasedPageEnumerator = releasedPages.GetEnumerator ()
    let mutable releasedPage = [||]
    let mutable pos = pos + 4

    while releasedPageEnumerator.MoveNext () do
      releasedPage <- BitConverter.GetBytes (releasedPageEnumerator.Current)
      Array.blit releasedPage 0 buffer pos releasedPage.Length
      pos <- pos + Page.SIZE

    (buffer, pos)

  member private this.DeserializeMaxPage (buffer : array<byte>, pos : int) =
    maxPage <- BitConverter.ToUInt64 (buffer[pos..])
    (buffer, pos + Page.SIZE)

  member private this.DeserializeReleasedPages
    (
      buffer : array<byte>,
      pos : int
    )
    =
    let mutable releasePagesCount = BitConverter.ToInt32 (buffer[pos..])
    let mutable pos = pos + 4

    while releasePagesCount <> 0 do
      releasedPages.Push (BitConverter.ToUInt64 (buffer[pos..]))
      pos <- pos + Page.SIZE
      releasePagesCount <- releasePagesCount - 1

    (buffer, pos)
