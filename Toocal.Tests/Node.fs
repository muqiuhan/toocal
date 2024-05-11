module Toocal.Tests.Node

open System
open System.IO
open NUnit.Framework
open Toocal.Core.DataAccessLayer
open Toocal.Core.Node

[<TestFixture>]
type NodeTests () =
  static let dbFileName = "db.db"

  [<Test>]
  static member public ``Initialize DB`` () =
    let dal = DataAccessLayer ("db.db", 4096)

    task {
      let! node = dal.GetNode dal.Meta.Root
      node.UpdateDal dal
      return! node.FindKey ("Key1" |> Text.Encoding.UTF8.GetBytes)
    }
    |> Async.AwaitTask
    |> Async.RunSynchronously
    |> fun (index, containingNode) -> containingNode |> Option.map _.Items[index]
    |> function
      | Some item ->
        Assert.AreEqual ("Key1", item.Key)
        Assert.AreEqual ("value1", item.Value)
      | _ -> Assert.Fail "Find Key Error"

    dal.Close ()
    File.Delete (dbFileName)
