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

hash_manager::hash_manager(std::string fname) : file_name(fname) {
	if (access(file_name.c_String(),F_OK) != -1) build_from_file();
		else build_new();
	}
}

void hash_manager::hash_manager build_new() {
	file = fopen(file_name.c_str(),"w+");
	fheader_page hpage;
	hpage.set_orig_size(4);
	hpage.set_n_buckets(4);
	hpage.set_n_pages(13);
	hpage.set_level(0);
	hpage.set_next(0);

	save_page(hpage);
	for (int i=0 ; i<hpage.get_n_buckets ; i++) {
		load_page<content_page>(content_page(),i*3*page_size+page_size);
		load_page<content_page>(content_page(),i*3*page_size+2*page_size);
		load_page<pointer_page>(pointer_page(),i*3*page_size+3*page_size)
	}
	fclose(file);
	build_from_file();
}

void hash_manager::build_from_file() {
	file = fopen(file_name.c_str(),"r+");
	fheader_page hpage = load_page(0);
	n_buckets = hpage.get_n_buckets();
	n_pages = hpage.get_n_pages();
	level = hpage.get_level();
	next = hpage.get_next();
	for (int i=0 ; i<n_buckets ; i++) {
		buckets.push_back(bucket((3*page_size*i) + page_size));
	}
}

int hash_manager::hash0 (int search_key) {
	return search_key%(pow(2,level+1))*original_size);
}

int hash_manager::hash1 (int search_key) {
	return search_key%(pow(2,level)*original_size);
}

int hash_manager::hash(int search_key) {
	bucketh = hash1(search_key);

		if( bucketh < n_buckets){
			return bucketh;
		}else{
			bucketh = hash0(search_key);
			return bucketh;
		}
		return 0;
}

bool hash_manager::add_data(int key,int rid) {
	int hash_val = hash(key);
	bool result = buckets[hash_val].add_data_entry(key,rid);
	if(result == true){
		buckets[next].split_bucket(next, n_buckets);
		n_buckets++;
		if (n_buckets > original_size*pow(2,level)) level++;
		next = (next+1)%(pow(2,level-1)*original_size);
		return true;
	}
	return false;
}

int hash_manager::find_rid(int key) {
	int hash_val = hash(key);
	int rid = buckets[hash_val].search_static_pages(key);
	if (rid == -1)	 rid = buckets[hash_val].search_overflow_pages(key);
	return rid;
}

int hash_manager::delete_data(int key) {
	if (buckets[hash(key)].remove_static_data(key) > -1) return true;
	if (buckets[hash(key)].remove_overflow_data(key) > -1) return true;
	return false;
}
