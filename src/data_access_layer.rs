use std::{
    fs::{File, OpenOptions},
    io::{Error, Read, Write},
    os::{fd::AsFd, unix::fs::FileExt},
    path::{Path, PathBuf},
};

use crate::{
    free_list::FreeList,
    meta::{Meta, META_PAGE_NUM},
    page::{Page, PageNum},
};

pub struct DataAccessLayer {
    db_file_path: String,
    db_file: File,
    page_size: usize,

    pub free_list: FreeList,
    pub meta: Meta,
}

impl DataAccessLayer {
    pub fn new(db_file_path: String, page_size: usize) -> Result<Self, Error> {
        let db_file_exsits = PathBuf::from(&db_file_path).exists();
        info!("initialize database from {}", &db_file_path);

        let mut dal = Self {
            db_file: OpenOptions::new()
                .append(true)
                .read(true)
                .create(!db_file_exsits)
                .open(&db_file_path)?,
            db_file_path: db_file_path.clone(),
            page_size,
            free_list: FreeList::new(),
            meta: Meta::new(),
        };

        if db_file_exsits {
            info!("loading database from {}", &db_file_path);
            dal.read_meta()?;
            dal.read_free_list()?;

            Ok(dal)
        } else {
            info!("creating database {}", &db_file_path);
            dal.meta.free_list_page_num = dal.free_list.get_next_page();
            dal.write_free_list()?;
            dal.write_meta(&dal.meta)?;

            Ok(dal)
        }
    }

    pub fn close(&mut self) -> Result<(), Error> {
        info!("close database {}", self.db_file_path);
        self.db_file
            .flush()
            .expect(format!("could not flush file: {}", self.db_file_path).as_str());
        Ok(())
    }

    pub fn allocate_empty_page(&self) -> Page {
        info!("allocate empty page for database {}", self.db_file_path);
        Page::from_data(Page::initialize_raw_data(self.page_size))
    }

    pub fn allocate_empty_page_with_num(&self, page_num: PageNum) -> Page {
        info!(
            "allocate empty page with page num {} for database {}",
            page_num, self.db_file_path
        );

        Page::new(page_num, Page::initialize_raw_data(self.page_size))
    }

    pub fn read_page(&self, page_num: PageNum) -> Result<Page, Error> {
        info!("read page {} from database {}", page_num, self.db_file_path);

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
        info!("write page {} to database {}", page.num, self.db_file_path);
        let page_size = self.page_size as u64;

        Ok(self
            .db_file
            .write_all_at(&page.data, &page.num * page_size)?)
    }

    pub fn read_meta(&self) -> Result<Meta, Error> {
        info!(
            "read meta page(page {}) from database {}",
            META_PAGE_NUM, self.db_file_path
        );

        let page = self.read_page(META_PAGE_NUM)?;
        Ok(Meta::deserialize(&page.data)?)
    }

    pub fn write_meta(&self, meta: &Meta) -> Result<Page, Error> {
        info!(
            "write meta page(page {}) to database {}",
            META_PAGE_NUM, self.db_file_path
        );

        let mut page = self.allocate_empty_page_with_num(META_PAGE_NUM);
        meta.serialize(&mut page.data);
        self.write_page(&page)?;
        Ok(page)
    }

    pub fn read_free_list(&self) -> Result<FreeList, Error> {
        info!(
            "read free list(page {}) from database {}",
            self.meta.free_list_page_num, self.db_file_path
        );

        let page = self.read_page(self.meta.free_list_page_num)?;
        Ok(FreeList::deserialize(&page.data))
    }

    pub fn write_free_list(&self) -> Result<Page, Error> {
        info!(
            "write free list(page {}) to database {}",
            self.meta.free_list_page_num, self.db_file_path
        );

        let mut page = self.allocate_empty_page_with_num(self.meta.free_list_page_num);
        self.free_list.serialize(&mut page.data);
        self.write_page(&page)?;
        Ok(page)
    }
}
