module Toocal.Core.DataAccessLayer.Meta

open System
open Toocal.Core.DataAccessLayer.Page

type Meta = {
  mutable freelist_page: PageNum
} with

  static member public make () = { freelist_page = 0UL }
  static member public META_PAGE_NUM: PageNum = 0UL

  member public this.serialize (buffer: Byte[]) =
    let serialized_freelist = BitConverter.GetBytes this.freelist_page
    Array.blit serialized_freelist 0 buffer 0 serialized_freelist.Length

  member public this.deserialize (buffer: Byte[]) =
    this.freelist_page <- BitConverter.ToUInt64 buffer
