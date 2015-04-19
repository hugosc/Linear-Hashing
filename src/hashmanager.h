#include <cstdio>
#include "page.h"
#include <unistd.h>

namespace lhash {

	int chars_to_int (char* c) {
		return (c[0] << 24)+(c[1] << 16)+(c[2] << 8)+c[3];
	}

	void int_to_chars (int n, char* c) {
		char ones = ~0x00;
		c[0] = (n >> 24) & ones;
		c[1] = (n >> 16) & ones;
		c[2] = (n >> 8) & ones;
		c[3] = n & ones;

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
			int bucket_pos(int);
			void load_page_to_buffer(int);
			void write_page_to_buffer(page*);
			void save_buffer_to_file(int);
			page* page_from_buffer();
			inline fheader_page* load_fheader_page();
			inline pointer_page* load_pointer_page();
			inline content_page* load_content_page();
			inline void write_fheader_page(fheader_page*);
			inline void write_pointer_page(pointer_page*);
			inline void write_content_page(content_page*);
			content_page* page_with_key(int);
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

	content_page* page_with_key (int key) {
		int page_pos = bucket_pos(int key);
		int ppage_pos;
		load_page_to_buffer(page_pos);
		content_page* curr_page = dynamic_cast<content_page*>(page_from_buffer());
		if curr_page->has_key(key); return curr_page;
		delete curr_page;
		page_pos += page_size;
		load_page_to_buffer(page_pos);
		curr_page = content_page* curr_page = dynamic_cast<content_page*>(page_from_buffer());
		if curr_page->has_key(key); return curr_page;
		delete curr_page;
		ppage_pos = page_pos + page_size;
		bool still_has_pages = true;
		do {
			load_page_to_buffer(ppage_pos);
			pointer_page* ppage = dynamic_cast<pointer_page*>(page_from_buffer());
			for (int i=0 ; i<n_pointers-1 ; i++) {
				if (ppage->has_ptr(i)) {
					curr_page = ppage->get_ptr(i);
					load_page_to_buffer(page_pos);
					content_page* curr_page = dynamic_cast<content_page*>(page_from_buffer());
					if curr_page->has_key(key) {
						delete ppage;
						return curr_page;
					}
					delete curr_page;
				}
			}
			if (!ppage->has_ptr(n_pointers-1)) still_has_pages = false;
			delete ppage;
		}
		while (still_has_pages)
		return nullptr;
	}


	int hash_manager::bucket_pos(int search_key){
		bucketh = hash1(search_key);

		if( bucketh < n_buckets){
			return (bucketh*(3*page_size))+page_size;
		}else{
			bucketh = hash0(search_key);
			return (bucketh*(3*page_size))+page_size;
		}
		return 0;
	}

	void hash_manager::insert_data(int search_key, int rid) {
		//TODO
	}

	void hash_manager::remove_data(int search_key) {
		content_page* p = page_with_key(search_key);
		if(page != nullptr){
			for(int i = 0; i < n_data; i++){
				if (p->has_data(i)){
					if(p->get_data(i).first == search_key){
						p->remove_data(i);
						if (p->is_empty() && (p->get_parent() != -1)){
							p->deleted_flag();
						}
					}
				}
			}
		}
	}

	int hash_manager::find_data(int search_key) {
		content_page* page = page_with_key(search_key);
		if(page != nullptr){
			for(int i = 0; i < n_data; i++){
				if (page->has_data(i)){
					if(page->get_data(i).first == search_key){
						return page->get_data(i).second;
					}
				}
			}
		}
		return 0;
	}

	int hash_manager::hash0 (int search_key) {
		return return search_key%((2^(level+1))*n_buckets);
	}

	int hash_manager::hash1 (int search_key) {
		return search_key%((2^level)*n_buckets);
	}

	void hash_manager::load_page_to_buffer(int byte_pos) {
		fseek(file,byte_pos,SEEK_SET);
		fread(buffer,page_size,file);
	}

	void hash_manager::save_buffer_to_file(int byte_pos) {
		fseek(file,byte_pos,SEEK_SET);
		fwrite(buffer,page_size,1,file);
	}

	//load page info into buffer, deletes page.
	void hash_manager::write_page_to_buffer(page* p) {
		int_to_chars(p->type,buffer);
		int_to_chars(p->get_parent(),&buffer[4]);
		int_to_chars(p->get_disc_writes(),&buffer[8]);
		switch (p->type) {
			case fheader_page_t :
				write_fheader_page(static_cast<fheader_page*>(p));
				break;
			case pointer_page_t :
				write_pointer_page(static_cast<pointer_page*>(p));
				break;
			case content_page_t :
				write_content_page(static_cast<content_page*>(p));
				break;
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

	inline void hash_manager::write_fheader_page (fheader_page* p) {
		int_to_chars(p->get_orig_size(),&buffer[12]);
		int_to_chars(p->get_n_buckets(),&buffer[16]);
		int_to_chars(p->get_n_pages(),&buffer[20]);
		int_to_chars(p->get_level(),&buffer[24]);
		int_to_chars(p->get_next(),&buffer[28]);
	}

	inline pointer_page* hash_manager::load_pointer_page() {
		pointer_page* new_page = new pointer_page();
		std::bitset<n_pointers> ptr_bitmap(chars_to_int(&buffer[12]));
		int j = 0;
		for (int i=16 ; i<page_size-3 ; i+=4) {
			if (ptr_bitmap[j++]) new_page->add_ptr(chars_to_int(&buffer[i]))
		}
	}

	inline void hash_manager::write_pointer_page(pointer_page* p) {
		int_to_chars(p->get_bitmap(),&buffer[12]);
		int write_pos = 16;
		for (int i=0 ; i<n_pointers ; ++i) {
			if (p->has_ptr(i)) {
				int_to_chars(p->get_ptr(i),&buffer[write_pos]);
				write_pos += 4;
			}
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

	inline void hash_manager::write_content_page (content_page* p) {
		int_to_chars(p->get_bitmap(),&buffer[12]);
		int write_pos = 16;
		for (int i=0 ; i<n_data ; ++i) {
			if (p->has_ptr(i)) {
				int_to_chars(p->get_data(i).first,&buffer[write_pos]);
				int_to_chars(p->get_data(i).second,&buffer[write_pos+4]);
				write_pos += 8;
			}
		}
	}

	//needs to allocate page and load its info from the buffer
	page* hash_manager::page_from_buffer() {
		switch (buffer[0]) {
			case fheader_page_t :
				return static_cast<page*>(load_header_page());
			case pointer_page_t :
				return static_cast<page*>(load_pointer_page());
			case content_page_t :
				return static_cast<page*>(load_content_page());
		}
		return nullptr;
	}
}
