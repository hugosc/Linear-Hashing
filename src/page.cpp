#include "linearhashing.h"

hash_manager::page::page(char buffer[hash_manager::page_size]) : type(buffer[0]) {
	parent_pointer = chars_to_int(&buffer[1]);
	disc_writes = chars_to_int(&buffer[5]);
}

void hash_manager::page::bufferize(char buffer[hash_manager::page_size]) {
	buffer[0] = type;
	int_to_chars(parent_pointer,&buffer[1]);
	int_to_chars(disc_writes,&buffer[5]);
}

int hash_manager::page::get_parent_ptr() { return parent_pointer; }
int hash_manager::page::incr_disc_writes() { return ++disc_writes; }
void hash_manager::page::set_trash_flag() { disc_writes = -1; }
void hash_manager::page::set_parent_ptr(int ptr) { parent_pointer = ptr; }
bool hash_manager::page::is_trash() { return (disc_writes == -1); }

hash_manager::fheader_page::fheader_page(char buffer[hash_manager::page_size]) : page(buffer) {
	n_buckets = chars_to_int(&buffer[9]);
	n_pages = chars_to_int(&buffer[13]);
	level = chars_to_int(&buffer[17]);
	next = chars_to_int(&buffer[21]);
}

void hash_manager::fheader_page::bufferize(char buffer[hash_manager::page_size]) {
	page::bufferize(buffer);
	int_to_chars(n_buckets,&buffer[9]);
	int_to_chars(n_pages,&buffer[13]);
	int_to_chars(level,&buffer[17]);
	int_to_chars(next,&buffer[21]);
}

void hash_manager::fheader_page::set_orig_size(int orig_size) {original_size = orig_size;}
void hash_manager::fheader_page::set_n_buckets(int buckets) { n_buckets = buckets; }
void hash_manager::fheader_page::set_n_pages(int pages) { n_pages = pages; }
void hash_manager::fheader_page::set_level(int l) {level = l; }
void hash_manager::fheader_page::set_next(int n) {next = n; }
void hash_manager::fheader_page::ger_orig_size() {return original_size;}
int hash_manager::fheader_page::get_n_buckets() {return n_buckets;}
int hash_manager::fheader_page::get_n_pages() {return n_pages;}
int hash_manager::fheader_page::get_level() {return level;}
int hash_manager::fheader_page::get_next() {return next;}

hash_manager::pointer_page::pointer_page(char buffer[hash_manager::page_size]) : page(buffer) {
	std::bitset<n_pointers> ptr_bitmap(chars_to_int(&buffer[9]));
	int j = 0;
	for (int i=13 ; i<page_size-3 ; i+=4) {
		if (ptr_bitmap[j++]) add_ptr(chars_to_int(&buffer[i]))
	}
}

void hash_manager::pointer_page::bufferize(char buffer[hash_manager::page_size]) {
	int_to_chars(static_cast<int>(ptr_bitmap.to_ulong()),&buffer[9]);
	int write_pos = 13;
	for (int i=0 ; i<n_pointers ; ++i) {
		if (ptr_bitmap[i]) {
			int_to_chars(ptr_array[i],&buffer[write_pos]);
			write_pos += 4;
		}
	}
}

int hash_manager::pointer_page::available_slot() {
	auto bmp = ptr_bitmap.to_ulong(); int i=0;
	while (bmp%2 == 1) {
		bmp = bmp >> 1;
		++i;
	}
	return i;
}

bool hash_manager::pointer_page::has_ptr(int pos) {
	return ptr_bitmap[pos];
}

void hash_manager::pointer_page::add_ptr(int ptr) {
	if (!ptr_bitmap.all()) {
		int i = available_slot();
		ptr_array[i] = ptr;
		ptr_bitmap.set(i);
	}
}

void hash_manager::pointer_page::remove_ptr(int pos) {
	if (pos < n_pointers) ptr_bitmap.reset(pos);
}

hash_manager::content_page::content_page(char buffer[hash_manager::page_size]) : page(buffer) {
	std::bitset<n_data> data_bitmap(chars_to_int(&buffer[9]);
	int j = 0;
	for (int i=13 ; i<page_size-7 ; i+=8) {
		if (data_bitmap[j++]) add_data(std::make_pair(chars_to_int(&buffer[i]),chars_to_int(&buffer[i+4])));
	}
}

void hash_manager::content_page::bufferize(char buffer[hash_manager::page_size]) {
	int_to_chars(static_cast<int>(ptr_bitmap.to_ulong()),&buffer[9]);
	int write_pos = 13;
	for (int i=0 ; i<n_data ; ++i) {
		if (data_bitmap[i]) {
			int_to_chars(data_array[i].first,&buffer[write_pos]);
			int_to_chars(data_array[i].second,&buffer[write_pos+4])
			write_pos += 8;
		}
	}
}

int hash_manager::content_page::available_slot(){
	auto bmp = data_bitmap.to_ulong();
	int i = 0;
	while(bmp%2 == 1){
		bmp = bmp >> 1;
		++i;
	}
	return i;
}

bool hash_manager::content_page::has_data(int pos) {
	return data_bitmap[pos];
}

void hash_manager::content_page::add_data(std::pair<int,int> p){
	if(!data_bitmap.all()){
		int i = available_slot();
		data_array[i] = p;
		data_bitmap.set(i);
	}
}

void hash_manager::content_page::remove_data(int pos) {
	if (pos < n_data) data_bitmap.reset(pos);
}

int hash_manager::content_page::get_bitmap() { return static_cast<int>(data_bitmap.to_ulong()); }
std:pair<int,int> hash_manager::content_page::get_data(int i) { return data_array[i]; }
bool hash_manager::content_page::is_full() { return data_bitmap.all(); }

int hash_manager::content_page::find_rid(int search_key) {
	for(int i = 0; i < n_data; i++){
		if (data_bitmap[i] && data_array[i].first == search_key) {
				return data_array[i].second;
			}
		}
	}
	return -1;
}

int hash_manager::content_page::find_pos(int search_key) {
	for(int i = 0; i < n_data; i++){
		if (data_bitmap[i] && data_array[i].first == search_key) {
				return i;
			}
		}
	}
	return -1;
}

bool hash_manager::operator==(const content_page& a, content_page& b) {
	return (a.data_array == b.data_array) && (a.data_bitmap == b.data_bitmap);
}

bool hash_manager::operator==(const content_page& a, content_page& b) {
	return (!a == b);
}
