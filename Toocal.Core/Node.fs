module Toocal.Core.Node

open System
open DataAccessLayer
open Page

type Item =
  struct
    val Key : Byte[]
    val Value : Byte[]

    new (key, value) = { Key = key ; Value = value }
  end

type DataAccessLayer with

  member public this.GetNode (pageNum : PageNum) =
    let node = Node this

    task {
      let! page = this.ReadPage pageNum
      node.Deserialize page.Data
      node.SetPageNum page.Num
      return node
    }

  member public this.WriteNode (node : Node) =
    let page = this.AllocateEmptyPage ()

    if node.PageNum = 0UL then
      page.SetNum (this.FreeList.NextPage ())
      node.SetPageNum page.Num
    else
      page.SetNum node.PageNum

    node.Serialize page.Data

    task {
      do! this.WritePage page
      return node
    }

  member inline public this.DeleteNode (pageNum : PageNum) = this.FreeList.ReleasePage pageNum

and Node =
  struct
    val mutable PageNum : PageNum
    val Items : Collections.Generic.List<Item>
    val Children : Collections.Generic.List<PageNum>
    val Dal : DataAccessLayer

    new (dal : DataAccessLayer) =
      {
        PageNum = 0UL
        Items = Collections.Generic.List<Item> ()
        Children = Collections.Generic.List<PageNum> ()
        Dal = dal
      }

    member public this.FindWithKey (key : Byte[]) =
      let mutable compare = false

      let index =
        this.Items.FindIndex (fun item ->
          match Array.compareWith (fun (k1 : Byte) k2 -> k1.CompareTo k2) item.Key key with
          | 0 ->
            compare <- true
            true
          | 1 ->
            compare <- false
            true
          | _ -> false)

      if index = -1 then false, this.Items.Count else compare, index

    member public this.FindKey (key : Byte[]) =
      // Byrefs cannot be captured by closures or passed to inner functions.
      let this = this
      task { return! Node.FindKeyHelper (this, key) }

    static member private FindKeyHelper (node : Node, key : Byte[]) =
      let wasFound, index = node.FindWithKey key

      if wasFound then
        task { return index, Some node }
      elif node.IsLeaf () then
        task { return -1, None }
      else
        task {
          let! node = node.Dal.GetNode (node.Children[index])
          return! Node.FindKeyHelper (node, key)
        }

    member inline public this.WriteNodes (nodes : Node[]) =
      let resultArray = Array.zeroCreate<Threading.Tasks.Task<Node>> nodes.Length

      for index = 0 to (resultArray.Length - 1) do
        resultArray[index] <- this.Dal.WriteNode (nodes[index])

      resultArray

    member public this.SetPageNum (pageNum : PageNum) = this.PageNum <- pageNum

    member private this.IsLeaf () = this.Children.Count = 0

    member public this.Serialize (buffer : Byte[]) =
      this.SerializeHeader (buffer, this.IsLeaf (), 0, buffer.Length - 1) |> this.SerializeItems

    member public this.Deserialize (buffer : Byte[]) = this.DeserializeHeader (buffer, 0) |> this.DeserializeItems

    member private this.DeserializeHeader (buffer : Byte[], left : int) =
      let mutable left = left
      let isLeaf = buffer[0] |> uint16
      let itemsCount = buffer[1..2] |> BitConverter.ToInt32
      left <- left + 3
      (buffer, isLeaf, itemsCount, left)

    member private this.DeserializeItems (buffer : Byte[], isLeaf : uint16, itemsCount : int, left : int) =
      let mutable left = left

      for i = 0 to itemsCount - 1 do
        if isLeaf = 0us then
          this.Children.Add (buffer[left..] |> BitConverter.ToUInt64)
          left <- left + Page.SIZE

        let mutable offset = buffer[left..] |> BitConverter.ToUInt16
        left <- left + 2

        let keyLen = buffer[int offset] |> uint16
        offset <- offset + 1us
        let key = buffer[int offset .. int (offset + keyLen - 1us)]
        offset <- offset + keyLen

        let valueLen = buffer[int offset] |> uint16
        offset <- offset + 1us
        let value = buffer[int offset .. int (offset + valueLen - 1us)]
        offset <- offset + valueLen

        this.Items.Add (Item (key, value))

      if isLeaf = 0us then this.Children.Add (buffer[left..] |> BitConverter.ToUInt64)

    member private this.SerializeItems (buffer : Byte[], isLeaf : bool, left : int, right : int) =
      let mutable left = left
      let mutable right = right

      for i = 0 to this.Items.Count - 1 do
        let item = this.Items[i]

        if not isLeaf then
          let child = this.Children[i] |> BitConverter.GetBytes
          Array.blit child 0 buffer left child.Length
          left <- Page.SIZE

        let offset = (right - item.Key.Length - item.Value.Length - 2) |> uint16 |> BitConverter.GetBytes

        Array.blit offset 0 buffer left offset.Length
        left <- left + 2

        right <- right - item.Value.Length
        Array.blit item.Value 0 buffer right item.Value.Length
        right <- right - 1
        buffer[right] <- item.Value.Length |> Convert.ToByte

        right <- right - item.Key.Length
        Array.blit item.Key 0 buffer right item.Key.Length
        right <- right - 1
        buffer[right] <- item.Key.Length |> Convert.ToByte

    member private this.SerializeHeader (buffer : Byte[], isLeaf : bool, left : int, right : int) =
      let mutable left = left
      let mutable right = right

      buffer[left] <- if isLeaf then 1uy else 0uy
      left <- left + 1

      // Add the Key-Value pair count
      let keyValuePairCount = this.Items.Count |> uint16 |> BitConverter.GetBytes

      Array.blit keyValuePairCount 0 buffer left keyValuePairCount.Length
      left <- left + 2

      (buffer, isLeaf, left, right)

  end
