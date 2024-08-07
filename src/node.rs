use crate::{data_access_layer::DataAccessLayer, page::PageNum};

pub struct Item {
    pub key: Vec<u8>,
    pub value: Vec<u8>,
}

pub struct Node {
    pub dal: DataAccessLayer,
    pub page_num: PageNum,
    pub items: Vec<Item>,
    pub children: Vec<PageNum>,
}

impl Node {
    pub fn new(dal: DataAccessLayer) -> Self {
        Self {
            dal,
            page_num: 0,
            items: Vec::new(),
            children: Vec::new(),
        }
    }

    pub fn is_leaf(&self) -> bool {
        self.children.len() == 0
    }
}

impl Item {
    pub fn new(key: Vec<u8>, value: Vec<u8>) -> Self {
        Self { key, value }
    }
}
