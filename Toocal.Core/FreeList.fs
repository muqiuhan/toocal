module Toocal.Core.FreeList

open Page
open System.Collections.Generic

type FreeList () =
  let mutable maxPage : PageNum = 0UL
  let releasedPage : Stack<PageNum> = new Stack<PageNum> ()

  member public this.MaxPage = maxPage
  member public this.ReleasedPage = releasedPage

  member public this.NextPage () =
    if releasedPage.Count <> 0 then
      releasedPage.Pop ()
    else
      maxPage <- maxPage + 1UL
      maxPage

  member public this.ReleasePage (pageNum : PageNum) =
    releasedPage.Push (pageNum)
