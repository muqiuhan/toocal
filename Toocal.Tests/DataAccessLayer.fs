module Toocal.Tests.Core.DataAccessLayer

open Toocal.Core.DataAccessLayer
open NUnit.Framework
open System.IO
open System.Text

[<TestFixture>]
type DataAccessLayerTests () =
  static let dbFileName = "db.db"

  [<Test>]
  static member public InitDatabase () =
    let dal = new DataAccessLayer (dbFileName, 4096)

    if not (File.Exists (dbFileName)) then
      Assert.Fail ("Failed to initialize the database: did not successfully create the database file.")
    else
      dal.Close ()
      File.Delete (dbFileName)

  [<Test>]
  static member public AllocateEmptyPage () =
    let dal = new DataAccessLayer (dbFileName, 4096)
    let page = dal.AllocateEmptyPage ()

    Assert.AreEqual (page.Data.Length, 4096)
    Assert.AreEqual (page.Num, 0UL)

    dal.Close ()
    File.Delete (dbFileName)

  [<Test>]
  static member public WritePage () =
    let dbFileName = "WritePage"
    let dal = new DataAccessLayer (dbFileName, 4096)
    let page = dal.AllocateEmptyPage ()
    let buffer = Array.zeroCreate<byte> (4096)

    page.SetNum (dal.FreeList.NextPage ())
    page.AddString ("data")

    task { do! dal.WritePage (page) } |> Async.AwaitTask |> Async.RunSynchronously

    dal.Close ()
    File.Delete (dbFileName)

  [<Test>]
  static member public ReadPage () =
    let dbFileName = "ReadPage"
    let dal = new DataAccessLayer (dbFileName, 4096)
    let page = dal.AllocateEmptyPage ()

    page.SetNum (dal.FreeList.NextPage ())
    page.AddString ("data")

    task {
      do! dal.WritePage (page)
      return! dal.ReadPage (page.Num)
    }
    |> Async.AwaitTask
    |> Async.RunSynchronously
    |> _.Data
    |> Encoding.Default.GetString
    |> fun str -> str.Substring (0, 4)
    |> fun str -> Assert.AreEqual ("data", str)

    dal.Close ()
    File.Delete (dbFileName)
