#[macro_use]
extern crate log;
extern crate colog;

use core::str;

use data_access_layer::DataAccessLayer;

mod data_access_layer;
mod free_list;
mod meta;
mod node;
mod page;
mod utils;
mod options;

fn main() {
    colog::init();

    let db_file = String::from("db.db");

    {
        // initialize db
        let mut dal = DataAccessLayer::new(db_file.clone(), utils::page_size::get())
            .expect(format!("cannot initialize the database from file: {}", &db_file).as_str());

        let mut node = dal.get_node(dal.meta.root).unwrap();
        node.dal = &dal;

        let key = "Key1".as_bytes().to_vec();
        let (index, containing_node) = node.find(&key).unwrap();
        let res = &containing_node.unwrap().items[index];

        println!(
            "key is: {}, value is: {}",
            str::from_utf8(&res.key).unwrap(),
            str::from_utf8(&res.value).unwrap()
        );

        dal.close().expect("cannot close the database");
    }
}
