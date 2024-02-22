module Toocal.Core.DataAccessLayer.Serializer

open System

[<Interface>]
type Serializer =
  abstract serialize: Byte[] -> Unit
  abstract deserialize: Byte[] -> Unit
