module Toocal.Tests.Core.FreeList

open Toocal.Core.FreeList
open NUnit.Framework

[<TestFixture>]
type FreeListTests () =

  [<Test>]
  static member public NextPage () =
    let freelist = new FreeList ()

    Assert.AreEqual (1UL, freelist.NextPage ())
    Assert.AreEqual (1UL, freelist.MaxPage)
    Assert.AreEqual (2UL, freelist.NextPage ())
    Assert.AreEqual (2UL, freelist.MaxPage)

    Assert.AreEqual (0UL, freelist.ReleasedPage.Count)

  [<Test>]
  static member public ReleasePage () =
    let freelist = new FreeList ()

    Assert.AreEqual (1UL, freelist.NextPage ())
    Assert.AreEqual (1UL, freelist.MaxPage)
    Assert.AreEqual (2UL, freelist.NextPage ())
    Assert.AreEqual (2UL, freelist.MaxPage)

    Assert.AreEqual (0UL, freelist.ReleasedPage.Count)
    freelist.ReleasePage (2UL)
    Assert.AreEqual (1UL, freelist.ReleasedPage.Count)
    Assert.AreEqual (2UL, freelist.ReleasedPage.Peek ())
    Assert.AreEqual (2UL, freelist.MaxPage)

    Assert.AreEqual (2UL, freelist.NextPage ())
    Assert.AreEqual (2UL, freelist.MaxPage)
    Assert.AreEqual (0UL, freelist.ReleasedPage.Count)
