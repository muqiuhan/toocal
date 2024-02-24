module Toocal.Core.ISerializable

open System

[<Interface>]
type ISerializable =
  abstract Serialize: buffer: Byte[] -> Unit
  abstract Deserialize: buffer: Byte[] -> Unit
