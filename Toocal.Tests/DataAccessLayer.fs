module Toocal.Core.DataAccessLayer.Test

open System
open NUnit.Framework
open Toocal.Core.DataAccessLayer.Dal
open Toocal.Core.DataAccessLayer.Freelist
open Toocal.Core.DataAccessLayer.Page


[<TestFixture>]
type Test () =
  let dal = new Dal("db.db", Environment.SystemPageSize)

  [<Test>]
  member public _.``Initialize an data access layer and create a new page then commit it.``
    ()
    =
    let page = dal.AllocateEmptyPage(dal.Freelist.NextPage())
    let data = Text.Encoding.UTF8.GetBytes("data")
    Array.Copy(data, page.Data, data.Length)
    dal.WritePage(page)

  [<Test>]
  member public _.``Read previously committed page`` () =
    let page = dal.ReadPage(1UL)

    Assert.AreEqual(
      "data",
      Text.Encoding.UTF8.GetString(
        page.Data |> fun data -> (Array.sub data 0 4)
      )
    )
