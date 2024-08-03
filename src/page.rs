use std::iter;

pub type PageNum = u64;

#[derive(Clone)]
pub struct Page {
    pub num: PageNum,
    data: Vec<u8>,
    cursor: usize,
}

impl Page {
    /// Defaults to 0 if num is None.
    pub fn new(num: PageNum, data: Vec<u8>) -> Self {
        Self {
            num,
            data,
            cursor: 0,
        }
    }

    pub fn from_data(data: Vec<u8>) -> Self {
        Self::new(0, data)
    }

    pub fn initialize_raw_data(page_size: usize) -> Vec<u8> {
        iter::repeat(0u8).take(page_size).collect()
    }

    pub fn data(&self) -> &Vec<u8> {
        &self.data
    }

    pub fn put_str(&mut self, str: &'static str) {
        self.put_string(String::from(str))
    }

    pub fn put_string(&mut self, str: String) {
        let data = str.as_bytes().to_vec();
        data.iter().for_each(|byte| {
            self.data[self.cursor] = *byte;
            self.cursor += 1;
        })
    }
}
