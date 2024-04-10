module Toocal.Core.Page

open System
open System.Text

type PageNum = uint64

type Page (data : array<byte>) =
  let mutable num : PageNum = 0UL

  member public this.Num
    with get () = num
    and set (newNum : PageNum) = num <- newNum

  member public this.Data = data

  member public this.SetNum (newNum : PageNum) = num <- newNum

  member public this.AddObject (obj : 'T, toBytes : 'T -> array<byte>) =
    let obj : array<byte> = toBytes (obj)
    Array.blit obj 0 data 0 obj.Length

  member inline public this.AddString (str : string) =
    this.AddObject (str, Encoding.Default.GetBytes)
