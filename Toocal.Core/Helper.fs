module Toocal.Core.Helper

open System.Collections.Generic

type IEnumerable<'T> with

  member inline public this.Fold (f : 'State -> 'T -> 'State, state : 'State) =
    let mutable state = state
    let enumerator = this.GetEnumerator ()

    while enumerator.MoveNext () do
      state <- f state enumerator.Current

    state

type Result<'Ok, 'Error> with

  static member inline public Unwrap (result : Result<'Ok, exn>) =
    match result with
    | Error (e : exn) ->
      printfn $"{e.ToString ()}"
      raise e
    | Ok value -> value
