module Toocal.Core.DataAccessLayer.Node

open Toocal.Core.DataAccessLayer.Page
open Toocal.Core.ISerializable
open System

[<Interface>]
type INodeSerializable =
  abstract Serialize: buffer: Byte[] -> Byte[]
  abstract SerializeHeader: isLeaf: bool * left: int * buffer: Byte[] -> int

  abstract SerializeBody:
    isLeaf: bool * left: int * right: int * buffer: Byte[] -> int * int

  abstract SerializeLastChild: isLeaf: bool * left: int * buffer: Byte[] -> unit

type Node() =

  let mutable _PageNum: PageNum = 0UL
  let _Items = new Collections.Generic.List<Item>()
  let _Children = new Collections.Generic.List<PageNum>()

  member public this.PageNum
    with get () = _PageNum
    and set (pageNum: PageNum) = _PageNum <- pageNum

  member public this.Items = _Items
  member public this.Children = _Children

  member public this.IsLeaf() = _Children.Count = 0

  interface INodeSerializable with
    /// Serialize page header: isLeaf, key-value pairs count and node num
    member this.SerializeHeader(isLeaf: bool, left: int, buffer: Byte[]) =
      let bitSetVar = if isLeaf then 1 else 0
      let mutable left = left

      buffer[left] <- bitSetVar |> byte
      left <- left + 1

      let keyValuePairsCount = (this.Items.Count |> uint16 |> BitConverter.GetBytes)
      Array.blit keyValuePairsCount 0 buffer left keyValuePairsCount.Length
      left <- left + 2
      left

    /// We use slotted pages for storing data in the page. It means the actual keys and values (the cells) are appended
    /// to right of the page whereas offsets have a fixed size and are appended from the left.
    /// It's easier to preserve the logical order (alphabetical in the case of b-tree) using the metadata and performing
    /// pointer arithmetic. Using the data itself is harder as it varies by size.
    ///
    /// Page structure is:
    /// ----------------------------------------------------------------------------------
    /// |  Page  | key-value /  child node    key-value 		      |    key-value	    	 |
    /// | Header |   offset /	 pointer	  offset         ......   |      data      ..... |
    /// ----------------------------------------------------------------------------------
    member this.SerializeBody(isLeaf: bool, left: int, right: int, buffer: Byte[]) =
      let mutable left = left
      let mutable right = right

      for i = 0 to this.Items.Count - 1 do
        let item = this.Items[i]

        // Write the child page as a fixed size of 8 bytes
        if not isLeaf then
          let child = (this.Children[i] |> uint64 |> BitConverter.GetBytes)
          Array.blit child 0 buffer left child.Length
          left <- left + Page.SIZE

        let offset =
          right - item.key.Length - item.value.Length - 2
          |> uint16
          |> BitConverter.GetBytes

        Array.blit offset 0 buffer left offset.Length
        left <- left + 2
        right <- right - item.value.Length

        Array.blit item.value 0 buffer right item.value.Length
        right <- right - 1

        buffer[right] <- item.value.Length |> byte
        right <- right - item.key.Length

        Array.blit item.key 0 buffer right item.key.Length
        right <- right - 1

        buffer[right] <- item.key.Length |> byte

      (left, right)

    /// Write the last child
    member this.SerializeLastChild(isLeaf: bool, left: int, buffer: Byte[]) =
      if not isLeaf then
        let last_child =
          this.Children[this.Children.Count - 1] |> uint64 |> BitConverter.GetBytes

        Array.blit last_child 0 buffer left last_child.Length

    member this.Serialize(buffer: Byte[]) =
      (this :> ISerializable).Serialize(buffer)
      buffer

  interface Toocal.Core.ISerializable.ISerializable with

    member this.Serialize(buffer: Byte[]) =
      let isLeaf = this.IsLeaf()
      let mutable left = 0
      let mutable right = buffer.Length - 1
      left <- (this :> INodeSerializable).SerializeHeader(isLeaf, left, buffer)

      let (new_left, new_right) =
        (this :> INodeSerializable).SerializeBody(isLeaf, left, right, buffer)

      left <- new_left
      right <- new_right

      (this :> INodeSerializable).SerializeLastChild(isLeaf, left, buffer)

    member this.Deserialize(buffer: byte[]) =
      let mutable left = 0
      let isLeaf = buffer[0] |> uint16
      let itemsCount = buffer[1..3] |> BitConverter.ToUInt16
      left <- left + 3

      // Read body
      for i = 0 to itemsCount - 1us |> int do
        if isLeaf = 0us then
          this.PageNum <- buffer[left..] |> BitConverter.ToUInt64
          left <- left + Page.SIZE
          this.Children.Add(this.PageNum)

          // Read offset
          let mutable offset = buffer[left..] |> BitConverter.ToUInt16 |> int

          left <- left + 2

          let klen = buffer[left..] |> BitConverter.ToUInt16 |> int
          left <- left + 1

          let key = buffer[offset .. offset + klen]
          offset <- offset + klen

          let vlen = buffer[left..] |> BitConverter.ToUInt16 |> int
          left <- left + 1

          let value = buffer[offset .. offset + vlen]
          offset <- offset + vlen

          this.Items.Add({ key = key; value = value })

      if isLeaf = 0us then
        // Read the last child nod
        this.Children.Add(buffer[left..] |> BitConverter.ToUInt64)

  member public this.Serialize(buffer: Byte[]) =
    (this :> INodeSerializable).Serialize(buffer)

  member public this.Deserialize = (this :> ISerializable).Deserialize

and Item = { key: Byte[]; value: Byte[] }

and Dal(Path: String, PageSize: Int32) =

  inherit Dal.Dal(Path, PageSize)

  member public this.GetNode(pageNum: PageNum) =
    let node = new Node()

    node.Deserialize(this.ReadPage(pageNum).Data)
    node.PageNum <- pageNum

    node

  member public this.WriteNode(node: Node) =
    let page = this.AllocEmptyPage()

    if node.PageNum = 0UL then
      page.Num <- this.Freelist.NextPage()
      node.PageNum <- page.Num
    else
      page.Num <- node.PageNum

    page.Data <- node.Serialize page.Data
    this.WritePage page
    node

  member public this.DeleteNode(pageNum: PageNum) = this.Freelist.ReleasePage(pageNum)
