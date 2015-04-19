#include "linearhashing.h"

class hash_manager::bucket::overflow_iterator {
	private :
		int next;
		content_page curr_page;
	public :
		page_iterator(int pos, int pointed_by) : position(pos), next(pointed_by) {
			curr_page = load_page<content_page>(position);
		}

		page_iterator operator++() {
			if (next == -1) return *this;
			pointer_page ptr_page = load_page<pointer_page>(curr_page.get_parent_pos());
			while (true) {
				while (next < n_pointers-1) {
					if (ptr_page.has_ptr(next)) {
						curr_page = load_page<content_page>(ptr_page.get_ptr(next));
						++next;
						return *this;
					}
					++next;
				}
				if (peek_type(ptr_page.get_ptr(next)) == content_page_t) {
					curr_page = content_page(buffer);
					next = -1;
					return *this;
				} else {
					ptr_page = pointer_page(buffer);
					next = 0;
				}
			}
		}

		page_iterator operator--() {
			//TODO
			return *this;
		}

		content_page operator* () {
			return curr_page;
		}
		bool not_end() {
			return (next != -1);
		}
}

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

}

bool hash_manager::bucket::add_data_entry(int,int) {

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
	return -1;
}

int hash_manager::bucket::remove_overflow_data(int key) {
	for (auto i=begin() ; i.not_end() ; ++i) {
		(if (*i).find_pos(key) != -1) {
			(*i).remove_data(i);
			if ((*i).is_empty() && ((*i).get_parent() != -1)){
				remove_overflow_page((*i));
				return 1;
			}
			return 0;
		}
	}
	return -1;
}

int hash_manager::bucket search_static_pages(int key) {
	int rid = load_page<content_page>(pos_in_file).find_rid(key);
	if (rid  == -1) rid = load_page<content_page>(pos_in_file+page_size).find(key);
	return rid;
}

int hash_manager::bucket search_overflow_pages(int key) {
	for (auto i=begin() ; i.not_end() ; ++i) {
		int rid = (*i).find(key);
		if (rid !=-1) return rid;
	}
	return -1;
}

bucket hash_manager::bucket split_bucket() {

}
