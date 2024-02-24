module Toocal.Core.DataAccessLayer.Test

open System
open NUnit.Framework
open Toocal.Core.DataAccessLayer.Dal
open Toocal.Core.DataAccessLayer.Freelist
open Toocal.Core.DataAccessLayer.Page

[<Test>]
let test() =
  IO.File.Delete("db.db")

  let ``Initialize an data access layer and create a new page then commit it.`` =
    use dal = new Dal("db.db", Environment.SystemPageSize)
    let page = dal.AllocEmptyPage()
    page.Num <- dal.Freelist.NextPage()
    let data = Text.Encoding.UTF8.GetBytes("data")
    Array.Copy(data, page.Data, data.Length)
    dal.WritePage(page)
    dal.WriteFreelist()

  let ``We expect the freelist state was saved`` =
    use dal = new Dal("db.db", Environment.SystemPageSize)
    let page = dal.AllocEmptyPage()
    page.Num <- dal.Freelist.NextPage()
    let data = Text.Encoding.UTF8.GetBytes("data2")
    Array.Copy(data, page.Data, data.Length)
    dal.WritePage(page)

    let pageNum = dal.Freelist.NextPage()
    dal.Freelist.ReleasePage(pageNum)

    dal.WriteFreelist()

  ()
