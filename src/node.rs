use std::io::Error;

use crate::{
    data_access_layer::DataAccessLayer,
    page::{PageNum, PAGE_NUM_SIZE},
};

pub struct Item {
    pub key: Vec<u8>,
    pub value: Vec<u8>,
}

pub struct Node<'dal> {
    pub dal: &'dal DataAccessLayer,
    pub page_num: PageNum,
    pub items: Vec<Item>,
    pub children: Vec<PageNum>,
}

impl Item {
    pub fn new(key: Vec<u8>, value: Vec<u8>) -> Self {
        Self { key, value }
    }
}

impl<'dal> Node<'dal> {
    pub fn new(dal: &'dal DataAccessLayer) -> Self {
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
            if is_leaf {
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
}
