module Toocal.Core.DataAccessLayer.Meta

open System
open Toocal.Core.DataAccessLayer.Page

type Meta () =
  let mutable freelistPage : PageNum = 0UL

  static member public META_PAGE_NUM : PageNum = 0UL

  member public this.FreelistPage
    with get () = freelistPage
    and set (num : PageNum) = freelistPage <- num

  member public this.Serialize (buffer : array<Byte>) =
    let freelistSerialized = BitConverter.GetBytes(freelistPage)
    Array.blit freelistSerialized 0 buffer 0 freelistSerialized.Length

  member public this.Deserialize (buffer : array<Byte>) =
    freelistPage <- BitConverter.ToUInt64(buffer)
