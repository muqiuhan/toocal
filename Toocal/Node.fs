module Toocal.Core.DataAccessLayer.Node

open Toocal.Core.DataAccessLayer.Dal
open Toocal.Core.DataAccessLayer.Page
open System

type Node = {
  dal: Dal
  page_num: PageNum
  items: array<Item>
  children: array<PageNum>
} with

  member public this.is_leaf () = this.children.Length = 0

  member public this.serialize (buffer: array<byte>) =
    let is_leaf = this.is_leaf()
    let mutable left_pos = 0
    let mutable right_pos = buffer.Length - 1
    let bit_set_var = if is_leaf then 1 else 0

    // Serialize page header: isLeaf, key-value pairs count and node num
    buffer[left_pos] <- bit_set_var |> byte
    left_pos <- left_pos + 1

    let key_value_pairs_count =
      (this.items.Length |> uint16 |> BitConverter.GetBytes)

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

    for i = 0 to this.items.Length - 1 do
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
        this.children |> Array.last |> uint64 |> BitConverter.GetBytes

      Array.blit last_child 0 buffer left_pos last_child.Length

    buffer

and Item = { key: array<byte>; value: array<byte> }
