module Toocal.Core.DataAccessLayer.Meta

open System
open Toocal.Core.DataAccessLayer.Page
open Toocal.Core.ISerializable

type Meta() =

  let mutable _FreelistPage: PageNum = 0UL

  member this.FreelistPage
    with get () = _FreelistPage
    and set (freelistPage: PageNum) = _FreelistPage <- freelistPage

  static member public META_PAGE_NUM: PageNum = 0UL

  interface ISerializable with
    member this.Serialize(buffer: Byte[]) =
      let serialized_freelist = BitConverter.GetBytes this.FreelistPage
      Array.blit serialized_freelist 0 buffer 0 serialized_freelist.Length

    member this.Deserialize(buffer: Byte[]) =
      this.FreelistPage <- BitConverter.ToUInt64 buffer

  member public this.Serialize = (this :> ISerializable).Serialize
  member public this.Deserialize = (this :> ISerializable).Deserialize
