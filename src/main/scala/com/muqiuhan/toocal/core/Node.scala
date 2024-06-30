package com.muqiuhan.toocal.core

import java.nio.ByteBuffer
import com.muqiuhan.toocal.errors.Error

import java.util
import scala.compiletime.ops.boolean
import scala.util.boundary
import boundary.break
import scala.annotation.tailrec

sealed case class Item(
    key: Array[Byte],
    value: Array[Byte]
):
    inline def size(): Int = key.length + value.length + Page.SIZE
end Item

class Node(dal: DataAccessLayer):
    var pageNumber: PageNumber = -1
    var items                  = new collection.mutable.ArrayBuffer[Item]()
    var children               = new collection.mutable.ArrayBuffer[PageNumber]()

    inline def isLeaf: Boolean = children.isEmpty

    inline def getNode(pageNumber: PageNumber): Node = dal.getNode(pageNumber).fold(_.raise(), identity)
    inline def writeNode(node: Node): Node           = dal.writeNode(node).fold(_.raise(), identity)
    inline def writeNodes(nodes: Node*): Unit        = nodes.foreach(writeNode)
    inline def indexItemSize(index: Int): Int        = items(index).key.length + items(index).value.length + Page.SIZE
    inline def isOverPopulated: Boolean              = dal.isOverPopulated(this)
    inline def isUnderPopulated: Boolean             = dal.isUnderPopulated(this)

    private def _findWithKey(key: Array[Byte]): (Boolean, Int) =
        boundary:
            for i <- items.indices do
                util.Arrays.compare(items(i).key, key) match
                    case 0 => break((true, i))
                    case 1 => break((false, i))
                    case _ => ()
            end for
            (false, items.length)
    end _findWithKey

    inline def findWithKey(key: Array[Byte]): Either[Error, (Int, Node)] =
        Node.findKey(this, key) match
            case (_, None)           => Left(Error.FindNodeWithKeyError(key))
            case (index, Some(node)) => Right((index, node))
        end match
    end findWithKey

    inline def nodeSize: Int =
        items.foldLeft(Node.HEADER_SIZE)((size: Int, item: Item) =>
            size + item.size()
        ) + Page.SIZE

    inline def addItem(item: Item, insertionIndex: Int): Int =
        items.insert(insertionIndex, item)
        insertionIndex

    def serialize(buffer: ByteBuffer): Unit =
        val isLeafFlag = isLeaf

        buffer.put((if isLeafFlag then 1 else 0).toByte)
        buffer.putInt(items.length)

        for i <- items.indices do
            val item = items(i)
            if !isLeafFlag then buffer.putLong(children(i))

            buffer
                .putInt(item.key.length)
                .putInt(item.value.length)
                .put(item.key)
                .put(item.value)
        end for

        if !isLeafFlag then buffer.putLong(children.last)
    end serialize

    def deserialize(buffer: ByteBuffer): Unit =
        val isLeafFlag = buffer.get() == 0

        for i <- 0 until buffer.getInt() do
            if !isLeafFlag then children.addOne(buffer.getLong())

            val key   = new Array[Byte](buffer.getInt())
            val value = new Array[Byte](buffer.getInt())

            buffer.get(key).get(value)
            items.addOne(Item(key, value))
        end for
    end deserialize
end Node

object Node:
    val HEADER_SIZE: Int = 3

    @tailrec
    def findKey(node: Node, key: Array[Byte]): (Int, Option[Node]) =
        val (wasFound, index) = node._findWithKey(key)

        if wasFound then return (index, Some(node))
        if node.isLeaf then return (-1, None)

        findKey(node.getNode(node.children(index)), key)
    end findKey
end Node

extension (self: DataAccessLayer)
    /** Get the corresponding page and deserialize it to node.
      * 
      * @param pageNumber The page number corresponding to Node.
      * @return If failure returns Error.DataAccessLayerCannotGetNode
      */
    def getNode(pageNumber: PageNumber): Either[Error, Node] =
        self.operatingAtPage(
            pageNumber,
            () =>
                self.readPage(pageNumber).flatMap(page =>
                    val node = new Node(self)
                    node.deserialize(page.data)
                    node.pageNumber = pageNumber
                    Right(node)
                ).orElse(Left(Error.DataAccessLayerCannotGetNode))
        )
    end getNode

    /** Serialize and write the node to the corresponding page.
      * 
      * @param node The page number corresponding to Node.
      * @return If failure returns Error.DataAccessLayerCannotWriteNode
      */
    def writeNode(node: Node): Either[Error, Node] =
        if node.pageNumber == 0 then
            node.pageNumber = self.freelist.getNextPage

        val page = self.allocateEmptyPage(node.pageNumber)
        node.serialize(page.data)

        self.operatingAtPage(
            self.meta.freelistPage,
            () =>
                self.writePage(page) match
                    case Left(_)      => Left(Error.DataAccessLayerCannotWriteNode)
                    case Right(value) => Right(node)
        )
    end writeNode

    def getSplitIndex(node: Node): Int =
        var size = Node.HEADER_SIZE

        boundary:
            for i <- node.items.indices do
                size += node.indexItemSize(i)
                if size > self.minThreshold && i < node.items.length - 1 then
                    break(i + 1)
            end for

        -1
    end getSplitIndex

    /** Delete a Node and release the page where it is located directly from the freelist.
      * 
      * @param pageNumber The page number of node.
      */
    inline def deleteNode(pageNumber: PageNumber): Unit = self.freelist.releasePage(pageNumber)
    inline def isOverPopulated(node: Node): Boolean  = node.nodeSize > self.maxThreshold
    inline def isUnderPopulated(node: Node): Boolean = node.nodeSize < self.maxThreshold
end extension
