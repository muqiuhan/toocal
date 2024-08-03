use data_access_layer::DataAccessLayer;

mod data_access_layer;
mod free_list;
mod page;
mod utils;

fn main() {
    let db_file = String::from("db.db");
    // initialize db

    let mut dal = DataAccessLayer::new(db_file.clone(), utils::page_size::get())
        .expect(format!("cannot initialize the database from file: {}", &db_file).as_str());

    // create a new page
    let page = &mut dal.allocate_empty_page();
    let page_num = &dal.free_list.get_next_page();

    page.put_str("data");
    page.num = *page_num;

    // commit it
    dal.write_page(page.to_owned())
        .expect("cannot write the page to database");

    dal.close().expect("cannot close the database");
}
