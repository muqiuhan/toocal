#[macro_use]
extern crate log;
extern crate colog;

use data_access_layer::DataAccessLayer;

mod data_access_layer;
mod free_list;
mod meta;
mod page;
mod utils;

fn main() {
    colog::init();

    let db_file = String::from("db.db");

    {
        // initialize db
        let mut dal = DataAccessLayer::new(db_file.clone(), utils::page_size::get())
            .expect(format!("cannot initialize the database from file: {}", &db_file).as_str());

        // create a new page
        let page = &mut dal.allocate_empty_page();
        let page_num = &dal.free_list.get_next_page();

        page.put_str("data");
        page.num = *page_num;

        // commit it
        dal.write_page(&page)
            .expect("cannot write the page to database");
        
        dal.write_free_list().expect("cannot write free_list page");
        dal.close().expect("cannot close the database");
    }

    {
        // reopen db
        let mut dal = DataAccessLayer::new(db_file.clone(), utils::page_size::get())
            .expect(format!("cannot initialize the database from file: {}", &db_file).as_str());

        let page = &mut dal.allocate_empty_page();
        let page_num = &dal.free_list.get_next_page();

        page.put_str("data2");
        page.num = *page_num;

        let page_num = dal.free_list.get_next_page();
        dal.free_list.release_page(page_num);

        // commit it
        dal.write_page(&page)
            .expect("cannot write the page to database");
        dal.write_free_list().expect("cannot write free_list page");
        dal.close().expect("cannot close the database");
    }
}
