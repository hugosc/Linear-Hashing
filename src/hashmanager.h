#include <cstdio>
#include <cassert>
#include "page.h"

class hash_manager {
	private :
		std::string file_name;
		FILE* file;
		char buffer[page::page_size];
		int original_size;
		int n_buckets;
		int n_pages;
		int level;
		int next;
		//Take f modulo n*2^level to be the hash function
		std::function<int (int)> func;
		int hash0(int);
		int hash1(int);
		void load_page_to_buffer(int);
		void write_page_to_buffer(page*);
		void save_buffer_to_file(int);
		page* page_from_buffer();
		inline fheader_page* load_fheader_page();
		inline pointer_page* load_pointer_page();
		inline content_page* load_content_page();
	public :
		hash_manager(std::string, std::function<int (int)>);
		void insert_data(int,int);
		void remove_data(int);
		int find_data(int);
}

hash_manager::hash_manager(std::string fname, std::function<int (int)> f = [](int x){return x;}) : file_name(fname), func(f) {
	file = fopen(file_name.c_string(),"r+");
	assert("File needs to exist",file);
	load_page_to_buffer(0);
	fheader_page* file_header = dynamic_cast<fheader_page*>(page_from_buffer());
	//Load page info into private variables
	//TODO


	delete fheader_page;
}

void hash_manager::insert_data(int search_key, int rid) {
	//TODO
}

void hash_manager::remove_data(int search_key) {
	//TODO
}

int hash_manager::find_data(int search_key) {
	//TODO
	return 0;
}

int hash_manager::hash0 (int search_key) {
	//TODO
	return 0;
}

int hash_manager::hash1 (int search_key) {
	//TODO
	return 0;
}

void hash_manager::load_page_to_buffer(int byte_pos) {
	fseek(file,byte_pos,SEEK_SET);
	fread(buffer,page::page_size,file);
}

void hash_manager::save_buffer_to_file(int byte_pos) {
	fseek(file,byte_pos,SEEK_SET);
	fwrite(buffer,page::page_file,1,file);
}

//load page info into buffer, deletes page.
void hash_manager::write_page_to_buffer(page* p) {
	//TODO
}

inline fheader_page* hash_manager::load_fheader_page() {
	//TODO
	return nullptr;
}

inline pointer_page* hash_manager::load_pointer_page() {
	pointer_page* new_page = new pointer_page();
}

inline content_page* hash_manager::load_content_page() {
	//TODO
	return nullptr;
}

//needs to allocate page and load its info from the buffer
page* hash_manager::page_from_buffer() {
	switch (buffer[0]) {
		case page::fheader_page :
			return static_cast<page*>(load_header_page());
		case page::pointer_page :
			return static_cast<page*>(load_pointer_page());
		case page::content_page :
			return static_cast<page*>(load_content_page());
	}
	return nullptr;
}
