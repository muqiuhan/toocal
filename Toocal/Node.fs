module Toocal.Core.DataAccessLayer.Node

open Toocal.Core.DataAccessLayer.Dal
open Toocal.Core.DataAccessLayer.Page
open System

type Node = {
  dal: Dal
  mutable page_num: PageNum
  items: Collections.Generic.List<Item>
  children: Collections.Generic.List<PageNum>
} with

  member public this.is_leaf () = this.children.Count = 0

  member public this.serialize (buffer: Byte[]) =
    let is_leaf = this.is_leaf()
    let mutable left_pos = 0
    let mutable right_pos = buffer.Length - 1
    let bit_set_var = if is_leaf then 1 else 0

    // Serialize page header: isLeaf, key-value pairs count and node num
    buffer[left_pos] <- bit_set_var |> byte
    left_pos <- left_pos + 1

    let key_value_pairs_count =
      (this.items.Count |> uint16 |> BitConverter.GetBytes)

    Array.blit
      key_value_pairs_count
      0
      buffer
      left_pos
      key_value_pairs_count.Length

    left_pos <- left_pos + 2

    // We use slotted pages for storing data in the page. It means the actual keys and values (the cells) are appended
    // to right of the page whereas offsets have a fixed size and are appended from the left.
    // It's easier to preserve the logical order (alphabetical in the case of b-tree) using the metadata and performing
    // pointer arithmetic. Using the data itself is harder as it varies by size.
    //
    // Page structure is:
    // ----------------------------------------------------------------------------------
    // |  Page  | key-value /  child node    key-value 		      |    key-value		 |
    // | Header |   offset /	 pointer	  offset         .... |      data      ..... |
    // ----------------------------------------------------------------------------------

    for i = 0 to this.items.Count - 1 do
      let item = this.items[i]

      if not is_leaf then
        let child = this.children[i]

        // Write the child page as a fixed size of 8 bytes
        let child = (child |> uint64 |> BitConverter.GetBytes)
        Array.blit child 0 buffer left_pos child.Length
        left_pos <- left_pos + Page.SIZE

      let offset =
        right_pos - item.key.Length - item.value.Length - 2
        |> uint16
        |> BitConverter.GetBytes

      Array.blit offset 0 buffer left_pos offset.Length
      left_pos <- left_pos + 2

      right_pos <- right_pos - item.value.Length
      Array.blit item.value 0 buffer right_pos item.value.Length

      right_pos <- right_pos - 1
      buffer[right_pos] <- item.value.Length |> byte

      right_pos <- right_pos - item.key.Length
      Array.blit item.key 0 buffer right_pos item.key.Length

      right_pos <- right_pos - 1
      buffer[right_pos] <- item.key.Length |> byte

    if not is_leaf then
      // Write the last child
      let last_child =
        this.children[this.children.Count - 1]
        |> uint64
        |> BitConverter.GetBytes

      Array.blit last_child 0 buffer left_pos last_child.Length

    buffer

  member public this.deserialize (buffer: byte[]) =
    let mutable left_pos = 0
    let is_leaf = buffer[0] |> uint16
    let items_count = buffer[1..3] |> BitConverter.ToUInt16
    left_pos <- left_pos + 3

    // Read body
    for i = 0 to items_count - 1us |> int do
      if is_leaf = 0us then
        this.page_num <- buffer[left_pos..] |> BitConverter.ToUInt64
        left_pos <- left_pos + Page.SIZE
        this.children.Add (this.page_num)

        // Read offset
        let mutable offset = buffer[left_pos..] |> BitConverter.ToUInt16 |> int
        left_pos <- left_pos + 2

        let klen = buffer[left_pos..] |> BitConverter.ToUInt16 |> int
        left_pos <- left_pos + 1

        let key = buffer[offset .. offset + klen]
        offset <- offset + klen

        let vlen = buffer[left_pos..] |> BitConverter.ToUInt16 |> int
        left_pos <- left_pos + 1

        let value = buffer[offset .. offset + vlen]
        offset <- offset + vlen

        this.items.Add ({ key = key; value = value })

    if is_leaf = 0us then
      // Read the last child nod
      this.children.Add (buffer[left_pos..] |> BitConverter.ToUInt64)

and Item = { key: Byte[]; value: Byte[] }
