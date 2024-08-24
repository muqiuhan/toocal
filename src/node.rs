use core::str;
use std::{
    borrow::{Borrow, BorrowMut},
    io::Error,
    ops::Deref,
};

use crate::{
    data_access_layer::DataAccessLayer,
    page::{PageNum, PAGE_NUM_SIZE},
};

#[derive(Clone)]
pub struct Item {
    pub key: Vec<u8>,
    pub value: Vec<u8>,
}

pub struct Node<'dal> {
    pub dal: &'dal mut DataAccessLayer,
    pub page_num: PageNum,
    pub items: Vec<Item>,
    pub children: Vec<PageNum>,
}

pub const NODE_HEADER_SIZE: usize = 3;

impl Item {
    pub fn new(key: Vec<u8>, value: Vec<u8>) -> Self {
        Self { key, value }
    }

    #[inline]
    pub fn size(&self) -> usize {
        self.key.len() + self.value.len() + PAGE_NUM_SIZE
    }
}

enum SplitMethod<'dal> {
    LEAF(Node<'dal>),
    ELSE(Node<'dal>),
}

impl<'dal> Node<'dal> {
    pub fn new(dal: &'dal mut DataAccessLayer) -> Self {
        Self {
            dal,
            page_num: 0,
            items: Vec::new(),
            children: Vec::new(),
        }
    }

    #[inline]
    pub fn item_size(&self, index: usize) -> usize {
        self.items[index].size()
    }

    pub fn node_size(&self) -> usize {
        self.items
            .iter()
            .fold(NODE_HEADER_SIZE, |size, item| size + item.size())
            + PAGE_NUM_SIZE
    }

    #[inline]
    pub fn is_leaf(&self) -> bool {
        self.children.len() == 0
    }

    pub fn add_item(&mut self, item: Item, index: usize) -> usize {
        if index == self.items.len() {
            self.items.push(item);
            return index;
        }

        self.items.insert(index, item);
        index
    }

    pub fn serialize(&self, buf: &mut Vec<u8>) -> Result<(), Error> {
        let mut left = 0;
        let mut right = 0;
        let is_leaf = self.is_leaf();

        buf[left] = if self.is_leaf() { 0u8 } else { 1u8 };
        left += 1;

        buf[left..left + 2].copy_from_slice(&(self.items.len() as u16).to_le_bytes());
        left += 2;

        let mut child_index = 0;
        self.items.iter().for_each(|item| {
            if !is_leaf {
                buf[left..left + PAGE_NUM_SIZE].copy_from_slice(&(self.children[0]).to_le_bytes());
                left += PAGE_NUM_SIZE;
            }

            buf[left..left + 2].copy_from_slice(
                &((right - item.key.len() - item.value.len() - 2) as u16).to_le_bytes(),
            );
            left += 2;

            right -= item.value.len();
            buf[right..].copy_from_slice(&item.value);

            right -= 1;
            buf[right] = item.value.len() as u8;

            right -= item.key.len();
            buf[right..].copy_from_slice(&item.key);

            right -= 1;
            buf[right] = item.key.len() as u8;

            child_index += 1;
        });

        if !is_leaf {
            buf[left..left + PAGE_NUM_SIZE].copy_from_slice(
                &(self
                    .children
                    .last()
                    .expect("serialize error: no last child node"))
                .to_le_bytes(),
            );
        }

        Ok(())
    }

    pub fn deserialize(&mut self, buf: &Vec<u8>) {
        let mut left = 0;
        let is_leaf = buf[0] as u16 == 1;

        left += 3;
        for i in 0..(u16::from_le_bytes(
            buf[1..3]
                .try_into()
                .expect("deserialize error: cannot read the items count"),
        )) {
            if !is_leaf {
                self.children.push(PageNum::from_le_bytes(
                    buf[left..left + PAGE_NUM_SIZE]
                        .try_into()
                        .expect(format!("deserialize error: cannot read the child {}", i).as_str()),
                ));
                left += PAGE_NUM_SIZE;
            }

            let mut offset = u16::from_le_bytes(
                buf[left..left + 2]
                    .try_into()
                    .expect("deserialize error: cannot read the offset"),
            ) as usize;

            left += 2;

            let key_len = buf[offset] as usize;
            offset += 1;

            let key = (buf[offset..offset + key_len])
                .iter()
                .cloned()
                .collect::<Vec<u8>>();
            offset += key_len;

            let value_len = buf[offset] as usize;
            offset += 1;

            let value = (buf[offset..offset + value_len])
                .iter()
                .cloned()
                .collect::<Vec<u8>>();

            self.items.push(Item::new(key, value));
        }

        if !is_leaf {
            self.children.push(PageNum::from_le_bytes(
                buf[left..left + PAGE_NUM_SIZE]
                    .try_into()
                    .expect("deserialize error: cannot read the last child"),
            ));
        }
    }

    fn __find_key_in_node(&self, key: &Vec<u8>) -> Result<(bool, usize), Error> {
        for (index, item) in self.items.iter().enumerate() {
            match item.key.cmp(key) {
                std::cmp::Ordering::Less => continue,
                std::cmp::Ordering::Equal => return Ok((true, index)),
                std::cmp::Ordering::Greater => return Ok((false, index)),
            }
        }

        Ok((false, self.items.len()))
    }

    /// TODO: &'dal Node<'dal>
    fn __find_key_helper(
        node: Node<'dal>,
        key: &'dal Vec<u8>,
    ) -> Result<(usize, Option<Node<'dal>>), Error> {
        let (was_found, index) = node.__find_key_in_node(key)?;

        if was_found {
            return Ok((index, Some(node)));
        }

        if node.is_leaf() {
            return Ok((0, None));
        }

        return Self::__find_key_helper(node.dal.get_node(node.children[index])?, key);
    }

    /// TODO: &self
    pub fn find(self, key: &'dal Vec<u8>) -> Result<(usize, Option<Node>), Error> {
        Self::__find_key_helper(self, key)
    }

    #[inline]
    pub fn is_over_populated(&self) -> bool {
        self.dal.is_over_populated(self)
    }

    #[inline]
    pub fn is_under_populated(&self) -> bool {
        self.dal.is_under_populated(self)
    }

    fn __split_new_node(&mut self, node_to_split: &mut Node, split_index: usize) -> SplitMethod {
        if node_to_split.is_leaf() {
            return SplitMethod::LEAF(
                self.dal
                    .new_node(node_to_split.items[split_index + 1..].to_vec(), vec![]),
            );
        } else {
            return SplitMethod::ELSE(self.dal.new_node(
                node_to_split.items[split_index + 1..].to_vec(),
                node_to_split.children[split_index + 1..].to_vec(),
            ));
        }
    }

    fn __handle_split_new_node(&mut self, node_to_split: &mut Node, index: usize) -> Node {
        let split_index = node_to_split
            .dal
            .get_split_index(node_to_split)
            .expect("node split: cannot get the split index");

        match self.__split_new_node(node_to_split, split_index) {
            SplitMethod::LEAF(node) => {
                node_to_split.items = node_to_split.items[..split_index].to_vec();
                node
            }
            SplitMethod::ELSE(node) => {
                node_to_split.items = node_to_split.items[..split_index].to_vec();
                node_to_split.children = node_to_split.children[..split_index + 1].to_vec();
                node
            }
        }
    }
}
