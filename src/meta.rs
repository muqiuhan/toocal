use std::io::Error;

use crate::page::{PageNum, PAGE_NUM_SIZE};

pub const META_PAGE_NUM: PageNum = 0;

pub struct Meta {
    pub free_list_page_num: PageNum,
}

impl Meta {
    pub fn new() -> Self {
        Self {
            free_list_page_num: 1,
        }
    }

    pub fn serialize(&self, buf: &mut Vec<u8>) {
        buf[0..PAGE_NUM_SIZE].copy_from_slice(&self.free_list_page_num.to_le_bytes())
    }

    pub fn deserialize(buf: &Vec<u8>) -> Result<Self, Error> {
        Ok(Self {
            free_list_page_num: PageNum::from_le_bytes(
                buf[0..PAGE_NUM_SIZE]
                    .try_into()
                    .expect("deserialize error, cannot read free_list_page_num from buf"),
            ),
        })
    }
}
