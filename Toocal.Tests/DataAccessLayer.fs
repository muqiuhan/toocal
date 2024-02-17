module Toocal.Core.DataAccessLayer.Test

open System
open NUnit.Framework
open Toocal.Core.DataAccessLayer.Dal
open Toocal.Core.DataAccessLayer.Freelist
open Toocal.Core.DataAccessLayer.Page

[<Test>]
let test () =
  IO.File.Delete ("db.db")

  let ``Initialize an data access layer and create a new page then commit it.`` =
    use dal = Dal.make("db.db", Environment.SystemPageSize)
    let page = dal.alloc_empty_page(dal.freelist.next_page())
    let data = Text.Encoding.UTF8.GetBytes ("data")
    Array.Copy (data, page.data, data.Length)
    dal.write_page(page)
    dal.write_freelist()

  let ``We expect the freelist state was saved`` =
    use dal = Dal.make("db.db", Environment.SystemPageSize)
    let page = dal.alloc_empty_page(dal.freelist.next_page())
    let data = Text.Encoding.UTF8.GetBytes ("data2")
    Array.Copy (data, page.data, data.Length)
    dal.write_page(page)

    let page_num = dal.freelist.next_page()
    dal.freelist.release_page(page_num)

    dal.write_freelist()

  ()
