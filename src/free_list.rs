use crate::page::PageNum;

#[derive(Clone)]
pub struct FreeList {
    max_page: PageNum,
    released_pages: Vec<PageNum>,
}

impl FreeList {
    pub fn new() -> Self {
        Self {
            max_page: 0,
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
}
