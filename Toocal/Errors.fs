module Toocal.Core.Errors

open System
open FsToolkit.ErrorHandling

module DataAccessLayer =
  module Dal =
    type Errors =
      | CannotOpenFile of String
      | CouldNotCloseFile of String

    type Result<'Ok> = Result<'Ok, Errors>

  type Errors = Dal of Dal.Errors
  type Result<'Ok> = Result<'Ok, Errors>

type Errors = DataAccessLayer of DataAccessLayer.Errors

type Result<'Ok> = Result<'Ok, Errors>

/// Unwrap operator
let (=>) (result : Result<'Ok, 'Error>) (processer : 'Error -> unit) =
  match result with
  | Ok value -> value
  | Error e ->
    processer (e)
    failwith (e.ToString())