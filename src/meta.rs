use std::io::Error;

use crate::page::{PageNum, PAGE_NUM_SIZE};

pub const META_PAGE_NUM: PageNum = 0;

pub struct Meta {
    pub root: PageNum,
    pub free_list_page_num: PageNum,
}

impl Meta {
    pub fn new() -> Self {
        Self {
            root: 2,
            free_list_page_num: 1,
        }
    }

    pub fn serialize(&self, buf: &mut Vec<u8>) {
        let mut pos = 0;
        buf[pos..PAGE_NUM_SIZE].copy_from_slice(&self.root.to_le_bytes());
        pos += PAGE_NUM_SIZE;
        buf[pos..pos + PAGE_NUM_SIZE].copy_from_slice(&self.free_list_page_num.to_le_bytes());
    }

    pub fn deserialize(buf: &Vec<u8>) -> Result<Self, Error> {
        let mut pos = 0;
        let root = PageNum::from_le_bytes(
            buf[pos..PAGE_NUM_SIZE]
                .try_into()
                .expect("deserialize error, cannot read root from buf"),
        );

        pos += PAGE_NUM_SIZE;

        let free_list_page_num = PageNum::from_le_bytes(
            buf[pos..pos + PAGE_NUM_SIZE]
                .try_into()
                .expect("deserialize error, cannot read free_list_page_num from buf"),
        );

        Ok(Self {
            root,
            free_list_page_num,
        })
    }
}
