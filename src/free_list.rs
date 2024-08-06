use crate::{meta::META_PAGE_NUM, page::{PageNum, PAGE_NUM_SIZE}};

#[derive(Clone)]
pub struct FreeList {
    max_page: PageNum,
    released_pages: Vec<PageNum>,
}

impl FreeList {
    pub fn new() -> Self {
        Self {
            max_page: META_PAGE_NUM,
            released_pages: Vec::new(),
        }
    }

    pub fn get_next_page(&mut self) -> PageNum {
        if !self.released_pages.is_empty() {
            self.released_pages
                .pop()
                .expect("This error is theoretically impossible")
        } else {
            self.max_page += 1;
            self.max_page
        }
    }

    pub fn release_page(&mut self, page: PageNum) {
        self.released_pages.push(page)
    }

    pub fn serialize(&self, buf: &mut Vec<u8>) {
        let mut pos = 0;
        buf[pos..pos + 2].copy_from_slice(&(self.max_page as u16).to_le_bytes());
        pos += 2;
        buf[pos..pos + 2].copy_from_slice(&(self.released_pages.len() as u16).to_le_bytes());
        pos += 2;

        self.released_pages.iter().for_each(|released_page| {
            buf[pos..pos + PAGE_NUM_SIZE].copy_from_slice(&released_page.to_le_bytes());
            pos += PAGE_NUM_SIZE;
        });
    }

    pub fn deserialize(buf: &Vec<u8>) -> Self {
        let mut pos = 0;
        let max_page = u16::from_le_bytes(
            buf[pos..pos + 2]
                .try_into()
                .expect("deserialize error, cannot read max_page from buf"),
        ) as u64;

        pos += 2;

        let mut released_pages_len = u16::from_le_bytes(
            buf[pos..pos + 2]
                .try_into()
                .expect("deserialize error, cannot read released_pages_len from buf"),
        ) as usize;

        pos += 2;

        let mut released_pages = Vec::with_capacity(released_pages_len);
        while released_pages_len != 0 {
            released_pages.push(PageNum::from_le_bytes(
                buf[pos..pos + PAGE_NUM_SIZE]
                    .try_into()
                    .expect("deserialize error, cannot read released_page from buf"),
            ));
            released_pages_len -= 1;
            pos += PAGE_NUM_SIZE;
        }

        Self {
            max_page,
            released_pages,
        }
    }
}
