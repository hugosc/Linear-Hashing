#include "linearhashing.h"

template <typename Page_t> hash_manager::load_page(int pos) {
	load_buffer(pos);
	return Page_t(buffer);
}

template <typename Page_t> hash_manager::save_page(const Page_t& p, int pos) {
	p.bufferize(buffer);
	write_buffer(pos);
}

char hash_manager::peek_type(int pos) {
	load_buffer(pos);
	return buffer[0];
}

void hash_manager::load_buffer(int byte_pos) {
	fseek(file,byte_pos,SEEK_SET);
	fread(buffer,page_size,file);
}

void hash_manager::write_buffer(int byte_pos) {
	fseek(file,byte_pos,SEEK_SET);
	fwrite(buffer,page_size,1,file);
}

int hash_manager::chars_to_int(const char* c) {
	return (c[0] << 24)+(c[1] << 16)+(c[2] << 8)+c[3];
}

void hash_manager::int_to_chars(const int n,char* c) {
	char ones = ~0x00;
	c[0] = (n >> 24) & ones;
	c[1] = (n >> 16) & ones;
	c[2] = (n >> 8) & ones;
	c[3] = n & ones;
}

hash_manager::hash_manager(std::string fname) {

}

int hash_manager::hash() {

}

bool hash_manager::add_data(int,int) {

}

int hash_manager::find_rid(int key) {
	int hash_val = hash(key);
	int rid = buckets[hash_val].search_static_pages(key);
	if (rid == -1)	 rid = buckets[hash_val].search_overflow_pages(key);
	return rid;
}

int hash_manager::delete_data(int key) {
	if (buckets[hash(key)].remove_static_data(key) > -1) return true;
	if buckets[hash(key)].remove_static_data(key) > -1) return true;
	return false;
}
