module Toocal.Core.DataAccessLayer.Dal

open System
open ZeroLog
open Toocal.Core.Function.Retry
open Toocal.Core.Errors
open Toocal.Core.Errors.DataAccessLayer

/// Data Access Layer (DAL) handles all disk operations and how data is
/// organized on the disk. It’s responsible for managing the underlying data
/// structure, writing the database pages to the disk, and reclaiming free pages
/// to avoid fragmentation.
type Dal (path : String) =
  static let logger = LogManager.GetLogger("Toocal.Core.DataAccessLayer.Dal")
  let file = Dal.InitFile(path) => (fun e -> logger.Error(e.ToString()))

  static member public InitFile (path : String) : Dal.Result<IO.FileStream> =
    try
      IO.File.Open(path, IO.FileMode.OpenOrCreate, IO.FileAccess.ReadWrite)
      |> Ok
    with e ->
      Dal.CannotOpenFile path |> Error

  interface IDisposable with
    member this.Dispose () = (fun () -> file.Dispose()) |= 3
