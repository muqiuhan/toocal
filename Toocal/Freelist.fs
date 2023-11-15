module Toocal.Core.DataAccessLayer.Freelist

open System
open Toocal.Core.DataAccessLayer.Page

/// Which pages are free and which are occupied. Pages can also be freed if they become empty, so we need to reclaim them for future use to avoid Fragmentation.
type Freelist () =

  /// Holds the maximum page allocated. maxPage*PageSize = fileSize
  let mutable maxPage : PageNum = 0UL

  /// Pages that were previouslly allocated but are now free
  let releasePages = new Collections.Generic.List<PageNum>()

  /// If possible, fetch pages first from the released pages.
  /// Else, increase the maximum page
  member public this.NextPage () =
    if releasePages.Count <> 0 then
      releasePages[releasePages.Count - 1]
    else
      maxPage <- maxPage + 1UL
      maxPage

  member inline public this.ReleasePage (page : PageNum) =
    releasePages.Add(page)
