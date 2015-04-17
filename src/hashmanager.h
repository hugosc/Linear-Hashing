#include <cstdio>
#include "page.h"
#include <unistd.h>

namespace lhash {

	int chars_to_int (char* c) {
		return (c[0] << 24)+(c[1] << 16)+(c[2] << 8)+c[3];
	}

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
			void buid_from_file();
			void build_new();
		public :
			hash_manager(std::string, std::function<int (int)>);
			~hash_manager();
			void insert_data(int,int);
			void remove_data(int);
			int find_data(int);
	};

	hash_manager::hash_manager(std::string fname, std::function<int (int)> f = [](int x){return x;}) : file_name(fname), func(f) {
		//if file exists
		if (access(file_name.c_String(),F_OK) != -1) build_from_file();
		else build_new();
	}

	hash_manager::~hash_manager() {
		fclose(file);
	}
	void hash_manager::hash_manager build_new() {
		file = fopen(file_name.c_str(),"w+");
		//TODO
	}

	void hash_manager::buid_from_file() {
		file = fopen(file_name.c_str(),"r+");
		load_page_to_buffer(0);
		fheader_page* header_page = dynamic_cast<fheader_page*>(page_from_buffer());
		original_size = header_page->get_orig_size();
		n_buckets = header_page->get_n_buckets();
		n_pages = header_page->get_n_pages();
		level = header_page->get_level();
		next = header_page->get_next();
		delete header_page;
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
		fheader_page* new_page = new fheader_page();
		new_page->set_orig_size(chars_to_int(&buffer[12]));
		new_page->set_n_buckets(chars_to_int(&buffer[16]));
		new_page->set_n_pages(chars_to_int(&buffer[20]));
		new_page->set_level(chars_to_int(&buffer[24]));
		new_page->set_next(chars_to_int(&buffer[28]));
		return new_page;
	}

	inline pointer_page* hash_manager::load_pointer_page() {
		pointer_page* new_page = new pointer_page();
		std::bitset<n_pointers> ptr_bitmap(chars_to_int(&buffer[12]));
		int j = 0;
		for (int i=16 ; i<page_size-3 ; i+=4) {
			if (ptr_bitmap[j++]) new_page->add_ptr(chars_to_int(&buffer[i]))
		}
	}

	inline content_page* hash_manager::load_content_page() {
		content_page* new_page = new content_page();
		std::bitset<n_data> data_bitmap(chars_to_int(&buffer[12]);
		int j = 0;
		for (int i=6 ; i<page_size-7 ; i+=8) {
			if (data_bitmap[j++]) new_page->add_data(std::make_pair(chars_to_int(&buffer[i]),chars_to_int(&buffer[i+4])));
		}
		return new_page;
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
}