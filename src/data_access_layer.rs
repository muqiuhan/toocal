use std::{
    fs::{File, OpenOptions},
    io::{Error, Write},
    os::unix::fs::FileExt,
    path::PathBuf,
};

use crate::{
    free_list::FreeList,
    meta::{Meta, META_PAGE_NUM},
    node::Node,
    options::DEFAULT_FILL_PERCENT,
    page::{Page, PageNum},
};

pub struct DataAccessLayer {
    db_file_path: String,
    db_file: File,
    page_size: usize,
    min_fill_percent: f32,
    max_fill_percent: f32,

    pub free_list: FreeList,
    pub meta: Meta,
}

impl DataAccessLayer {
    pub fn new(db_file_path: String, page_size: usize) -> Result<Self, Error> {
        let db_file_exsits = PathBuf::from(&db_file_path).exists();
        info!("database {}: initializing", &db_file_path);

        let mut dal = Self {
            db_file: OpenOptions::new()
                .write(true)
                .read(true)
                .create(!db_file_exsits)
                .open(&db_file_path)?,
            db_file_path: db_file_path.clone(),
            page_size,
            free_list: FreeList::new(),
            meta: Meta::new(),
            min_fill_percent: DEFAULT_FILL_PERCENT.min_fill_percent,
            max_fill_percent: DEFAULT_FILL_PERCENT.max_fill_percent,
        };

        if db_file_exsits {
            info!("database {}: loading...", &db_file_path);
            dal.read_meta()?;
            dal.read_free_list()?;

            Ok(dal)
        } else {
            info!("database {}: creating...", &db_file_path);
            dal.meta.free_list_page_num = dal.free_list.get_next_page();
            dal.write_free_list()?;
            dal.write_meta(&dal.meta)?;

            Ok(dal)
        }
    }

    pub fn close(&mut self) -> Result<(), Error> {
        info!("database {}: close", self.db_file_path);
        self.db_file
            .flush()
            .expect(format!("could not flush file: {}", self.db_file_path).as_str());
        Ok(())
    }

    pub fn allocate_empty_page(&self) -> Page {
        info!("database {}: allocate empty page", self.db_file_path);
        Page::from_data(Page::initialize_raw_data(self.page_size))
    }

    pub fn allocate_empty_page_with_num(&self, page_num: PageNum) -> Page {
        info!(
            "database {}: allocate empty page {}",
            self.db_file_path, page_num
        );

        Page::new(page_num, Page::initialize_raw_data(self.page_size))
    }

    pub fn read_page(&self, page_num: PageNum) -> Result<Page, Error> {
        info!("database {}: read page {}", self.db_file_path, page_num);

        let page_size = self.page_size;
        let mut page_data = Page::initialize_raw_data(page_size);

        let readed_bytes = self
            .db_file
            .read_at(&mut page_data, page_num * page_size as u64)?;

        if readed_bytes != page_size {
            panic!(
                "The complete page was not read successfully, page size: {}, readed: {}",
                page_size, readed_bytes
            );
        }

        Ok(Page::from_data(page_data))
    }

    pub fn write_page(&self, page: &Page) -> Result<(), Error> {
        info!("database {}: write page {}", self.db_file_path, page.num);
        let page_size = self.page_size as u64;

        Ok(self
            .db_file
            .write_all_at(&page.data, &page.num * page_size)?)
    }

    pub fn read_meta(&mut self) -> Result<(), Error> {
        info!(
            "database {}: read meta page from page {}",
            self.db_file_path, META_PAGE_NUM
        );

        let page = self.read_page(META_PAGE_NUM)?;
        self.meta = Meta::deserialize(&page.data)?;
        Ok(())
    }

    pub fn write_meta(&self, meta: &Meta) -> Result<Page, Error> {
        info!(
            "database {}: write meta page to page {}",
            self.db_file_path, META_PAGE_NUM
        );

        let mut page = self.allocate_empty_page_with_num(META_PAGE_NUM);
        meta.serialize(&mut page.data);
        self.write_page(&page)?;
        Ok(page)
    }

    pub fn read_free_list(&mut self) -> Result<(), Error> {
        info!(
            "database {}: read free list from page {}",
            self.db_file_path, self.meta.free_list_page_num
        );

        let page = self.read_page(self.meta.free_list_page_num)?;
        self.free_list = FreeList::deserialize(&page.data);
        Ok(())
    }

    pub fn write_free_list(&self) -> Result<Page, Error> {
        info!(
            "database {}: write free list to page {}",
            self.db_file_path, self.meta.free_list_page_num
        );

        let mut page = self.allocate_empty_page_with_num(self.meta.free_list_page_num);
        self.free_list.serialize(&mut page.data);
        self.write_page(&page)?;
        Ok(page)
    }

    pub fn get_node(&self, page_num: PageNum) -> Result<Node, Error> {
        info!(
            "database {}: get node from page {}",
            self.db_file_path, page_num
        );

        let page = self.read_page(page_num)?;
        let mut node = Node::new(self);
        node.deserialize(&page.data);
        node.page_num = page.num;
        Ok(node)
    }

    pub fn write_node(&mut self, node: &mut Node) -> Result<(), Error> {
        info!(
            "database {}: write node to page {}",
            self.db_file_path, node.page_num
        );

        let mut page = self.allocate_empty_page();
        if node.page_num == 0 {
            page.num = self.free_list.get_next_page();
            node.page_num = page.num;
        } else {
            page.num = node.page_num;
        }

        node.serialize(&mut page.data)?;
        self.write_page(&page)?;
        Ok(())
    }

    #[inline]
    pub fn delete_node(&mut self, page_num: PageNum) {
        self.free_list.release_page(page_num);
    }

    #[inline]
    pub fn max_threshold(&self) -> f32 {
        self.max_fill_percent * (self.page_size as f32)
    }

    #[inline]
    pub fn is_over_populated(&self, node: &Node) -> bool {
        (node.node_size() as f32) > self.max_threshold()
    }

    #[inline]
    pub fn min_threshold(&self) -> f32 {
        self.min_fill_percent * (self.page_size as f32)
    }

    #[inline]
    pub fn is_under_populated(&self, node: &Node) -> bool {
        (node.node_size() as f32) < self.min_threshold()
    }
}
