#include "linearhashing.h"

class hash_manager::bucket::overflow_iterator {
	private :
		int next;
		content_page curr_page;
	public :
		int position;
		overflow_iterator(int pos, int pointed_by) : position(pos), next(pointed_by) {
			curr_page = load_page<content_page>(position);
		}

		overflow_iterator operator++() {
			if (next == -1) return *this;
			pointer_page ptr_page = load_page<pointer_page>(curr_page.get_parent_ptr());
			while (true) {
				while (next < n_pointers-1) {
					if (ptr_page.has_ptr(next)) {
						position = ptr_page.get_ptr(next);
						curr_page = load_page<content_page>(position);
						++next;
						return *this;
					}
					++next;
				}
				if (peek_type(ptr_page.get_ptr(next)) == content_page_t) {
					position = ptr_page.get_ptr(next);
					curr_page = content_page(buffer);
					next = -1;
					return *this;
				} else {
					ptr_page = pointer_page(buffer);
					next = 0;
				}
			}
		}

		overflow_iterator operator--() {
			//TODO
			return *this;
		}

		content_page operator* () {
			return curr_page;
		}
		bool not_end() {
			return (next != -1);
		}
};

overflow_iterator hash_manager::bucket::begin() {
	pointer_page ppage = load_page<pointer_page>(pos_in_file + 2*page_size);
	while (true) {
		for (int i=0 ; i<n_pointers-1 ; i++) {
			if (ppage.has_ptr(i)) return overflow_iterator(load_page<content_page>(ppage.get_ptr(i)),i+1);
		}
		if (ppage.has_ptr(n_pointers-1) {
			if (peek_type(ppage.get_ptr(n_pointers-1)) == content_page_t) {
				return overflow_iterator(content_page(buffer),-1);
			} else {
				ppage = pointer_page(buffer);
			}
		} else break;
	}
	return overflow_iterator(content_page(),-1);
}

int hash_manager::bucket::add_with_overflow(int pos, std::pair<int,int> data) {
	pointer_page ppage = load_page<pointer_page>(pos);
	int new_pos = n_pages*page_size;
	if (!ppage.is_full()) {
		ppage.add_ptr(new_pos);
		content_page new_page();
		new_page.set_parent_ptr(pos);
		new_page.add_data(data);
		save_page(new_page,new_pos);
		++n_pages;
		return new_pos;
	} else {
		content_page last_page = load_page<content_page>(ppage.get_ptr(n_pointers-1));
		ppage.remove_ptr(n_pointer-1);
		ppage.add_ptr(new_pos);
		pointer_page new_ppage();
		new_ppage.set_parent_ptr(pos);
		save_page(new_ppage,new_pos);
		++n_pages;
		return add_overflow_page(new_pos,data);
	}
}

void hash_manager::bucket::remove_overflow_page(int pos) {
	if (peek_type(pos) == content_page_t) {
		content_page cpage = content_page(buffer);
		cpage.set_deleted_flag();
		write_page(cpage,pos);
		pointer_page ppage = load_page<pointer_page>(cpage.get_parent_ptr());
		for (int i=0 ; i<n_pointers ; ++i) 
			if (ppage.get_ptr(i) == pos) {
				ppage.remove_ptr(i);
				break;
			}
		if (ppage.is_empty()) remove_overflow_page(cpage.get_parent_ptr());
	}
	else {
		if (pos == pos_in_file + 2*page_size) return;
		pointer_page ppage = load_page<pointer_page>(pos);
		ppage.set_deleted_flag();
		write_page(ppage,pos);
		pointer_page prt_ppage = load_page<pointer_page>(ppage.get_parent_ptr());
		for (int i=0 ; i<n_pointers ; ++i) 
			if (prt_ppage.get_ptr(i) == pos) {
				prt_ppage.remove_ptr(i);
				break;
			}
		if (prt_ppage.is_empty()) remove_overflow_page(ppage.get_parent_ptr());
	}
}

bool hash_manager::bucket::add_data_entry(int key,int rid) {
	for (int i=0 ; i<2 ; ++i) {
		content_page cpage = load_page<content_page>(pos_in_file+i*page_size);
		if (!cpage.is_full()) {
			cpage.add_data(std::make_pair(key,rid));
			return false;
		}
	}
	int ptr_pos = pos_in_file + 2*page_size;
	for(auto i=begin() ; i.not_end() . ++i) {
		if (!(*i).is_full()) {
			(*i).add_data(std::make_pair(key,rid));
			return false;
		}
		ptr_pos = (*i).get_parent_ptr();
	}
	add_with_overflow(prt_pos);
	return true;
}

int hash_manager::bucket::remove_static_data(int key) {
	for (int i=0 ; i<2 ; ++i) {
		content_page cpage = load_page<content_page>(pos_in_file+i*page_size);
		int pos = cpage.find_pos(key);
		if (pos != -1) {
			cpage.remove_data(pos);
			if (cpage.is_emtpy()) return 1;
			return 0;
		}
	}
	return -1;
}

int hash_manager::bucket::remove_overflow_data(int key) {
	for (auto i=begin() ; i.not_end() ; ++i) {
		(if (*i).find_pos(key) != -1) {
			(*i).remove_data(i);
			if ((*i).is_empty() && ((*i).get_parent_ptr() != -1)){
				remove_overflow_page(i.position);
				return 1;
			}
			return 0;
		}
	}
	return -1;
}

int hash_manager::bucket::search_static_pages(int key) {
	int rid = load_page<content_page>(pos_in_file).find_rid(key);
	if (rid  == -1) rid = load_page<content_page>(pos_in_file+page_size).find(key);
	return rid;
}

int hash_manager::bucket::search_overflow_pages(int key) {
	for (auto i=begin() ; i.not_end() ; ++i) {
		int rid = (*i).find(key);
		if (rid !=-1) return rid;
	}
	return -1;
}

bucket hash_manager::bucket::split_bucket(int hash_val1, int hash_val2) {
	bucket new_bucket(hash_val2*3*page_size + page_size);
	for (int i=0 ; i<2 ; i++) {
		content_page cpage = load_page<content_page>(pos_in_file + page_size*i);
		for (int j=0 ; j<n_data ; j++) {
			if (cpage.has_data(j)) {
				auto pair = cpage.get_data(j);
				if (hash(pair.first) != hash_val1) {
					cpage.remove_data(j);
					new_bucket.add_data_entry(pair.first,pair.second);
				}
			}
	}
	for (auto i=begin() ; i.not_end() ; ++i) {
		for (int j=0 ; j<n_data ; j++) {
			if ((*i).has_data(j)) {
				auto pair = (*i).get_data(j);
				if (hash(pair.first) != hash_val1) {
					(*i).remove_data(j);
					new_bucket.add_data_entry(pair.first,pair.second);
				}
			}
			save_page(*i,i.position);	
		}
	}
}
