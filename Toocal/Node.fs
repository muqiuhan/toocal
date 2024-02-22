module Toocal.Core.DataAccessLayer.Node

open Toocal.Core.DataAccessLayer.Page
open System

type Node () =
  let mutable _PageNum: PageNum = 0UL
  let _Items = new Collections.Generic.List<Item> ()
  let _Children = new Collections.Generic.List<PageNum> ()

  member public this.PageNum
    with get () = _PageNum
    and set (pageNum: PageNum) = _PageNum <- pageNum

  member public this.isLeaf () = _Children.Count = 0

  /// Serialize page header: isLeaf, key-value pairs count and node num
  member private this.SerializeHeader
    (
      isLeaf: bool,
      left: int,
      buffer: Byte[]
    )
    =
    let bitSetVar = if isLeaf then 1 else 0
    let mutable left = left
    buffer[left] <- bitSetVar |> byte
    left <- left + 1

    let keyValuePairsCount = (_Items.Count |> uint16 |> BitConverter.GetBytes)

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
  member private this.SerializeBody
    (
      isLeaf: bool,
      left: int,
      right: int,
      buffer: Byte[]
    )
    =
    let mutable left = left
    let mutable right = right

    for i = 0 to _Items.Count - 1 do
      let item = _Items[i]

      if not isLeaf then
        let child = _Children[i]
        // Write the child page as a fixed size of 8 bytes
        let child = (child |> uint64 |> BitConverter.GetBytes)
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
  member private this.SerializeLastChild
    (
      isLeaf: bool,
      left: int,
      buffer: Byte[]
    )
    =
    if not isLeaf then
      let last_child =
        _Children[_Children.Count - 1] |> uint64 |> BitConverter.GetBytes

      Array.blit last_child 0 buffer left last_child.Length

  member public this.Serialize (buffer: Byte[]) =
    let isLeaf = this.isLeaf()
    let mutable left = 0
    let mutable right = buffer.Length - 1

    left <- this.SerializeHeader (isLeaf, left, buffer)

    let (new_left, new_right) = this.SerializeBody (isLeaf, left, right, buffer)

    left <- new_left
    right <- new_right

    this.SerializeLastChild (isLeaf, left, buffer)

    buffer

  member public this.Deserialize (buffer: byte[]) =
    let mutable left = 0
    let isLeaf = buffer[0] |> uint16
    let itemsCount = buffer[1..3] |> BitConverter.ToUInt16
    left <- left + 3

    // Read body
    for i = 0 to itemsCount - 1us |> int do
      if isLeaf = 0us then
        _PageNum <- buffer[left..] |> BitConverter.ToUInt64
        left <- left + Page.SIZE
        _Children.Add (_PageNum)

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

        _Items.Add ({ key = key; value = value })

    if isLeaf = 0us then
      // Read the last child nod
      _Children.Add (buffer[left..] |> BitConverter.ToUInt64)

and Item = { key: Byte[]; value: Byte[] }
