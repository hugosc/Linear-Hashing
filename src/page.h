#include <array>
#include <bitset>
#include <iostream>

namespace lhash {
	//namespace constants
	extern const char fheader_page_t = 0;
	extern const char pointer_page_t = 1;
	extern const char content_page_t = 2;
	extern const int page_size = 128;
	extern const int n_pointers = 28;
	extern const int n_data = 14;

	//base class which has the page header already defined, since it is common to all page types
	class page {
		protected :
			int parent_pointer;
			int disc_writes;

		public :
			const char type;
			page (char t) : type(t), parent_pointer(-1), disc_writes(0) {};
			page (char t, int parent_ptr, int dw) : type(t), parent_pointer(parent_ptr), disc_writes(dw) {}
			virtual ~page() {}
			inline void set_parent_ptr (int);
			inline int get_parent_ptr ();
			inline void incr_disc_writes ();
			inline int get_disc_writes ();
			inline void deleted_flag();
	};

	inline void page::set_parent_ptr(int ptr) { parent_pointer = ptr; }
	inline int page::get_parent_ptr() { return parent_pointer; }
	inline void page::incr_disc_writes() { ++disc_writes; }
	inline int page::get_disc_writes() { return disc_writes; }
	inline void page::deleted_flag(){ disc_writes = -1; }

	//-----------end page class-----------------

	//file header page. Contains the info about the hash table in the file
	class fheader_page : public page {
		private :
			int original_size;
			int n_buckets;
			int n_pages;
			int level;
			int next;

		public :
			fheader_page() : page(fheader_page_t) {}
			inline void set_orig_size(int);
			inline void set_n_buckets(int);
			inline void set_n_pages(int);
			inline void set_level(int);
			inline void set_next(int);
			inline int get_orig_size();
			inline int get_n_buckets();
			inline int get_n_pages();
			inline int get_level();
			inline int get_next();
	};

	inline void fheader_page::set_orig_size(int orig_size) {original_size = orig_size;}
	inline void fheader_page::set_n_buckets(int buckets) { n_buckets = buckets; }
	inline void fheader_page::set_n_pages(int pages) { n_pages = pages; }
	inline void fheader_page::set_level(int l) {level = l; }
	inline void fheader_page::set_next(int n) {next = n; }
	inline void fheader_page::ger_orig_size() {return original_size;}
	inline int fheader_page::get_n_buckets() {return n_buckets;}
	inline int fheader_page::get_n_pages() {return n_pages;}
	inline int fheader_page::get_level() {return level;}
	inline int fheader_page::get_next() {return next;}
	//----------end fheader_page class----------


	//pointer page. Stores pointers to overflow pages from the buckets
	class pointer_page : public page {
		public :
			pointer_page() : page(pointer_page_t) {}
			pointer_page(std::array<int,n_pointers> ptrs, int bitmap) : page(page::pointer_page) , ptr_bitmap(bitmap), ptr_array(ptrs) {}
			inline bool has_ptr(int);
			inline void add_ptr(int);
			inline void remove_ptr(int);
			inline int get_ptr(int);
			inline bool is_full();
			inline int get_bitmap();
			inline bool is_empty();
		private :
			std::bitset<n_pointers>   ptr_bitmap;
			std::array<int,n_pointers> ptr_array;
			inline int available_slot();
	};

	//get leftmost available slot
	inline int pointer_page::available_slot() {
		auto bmp = ptr_bitmap.to_ulong(); int i=0;
		while (bmp%2 == 1) {
			bmp = bmp >> 1;
			++i;
		}
		return i;
	}

	inline bool pointer_page::has_ptr(int pos) {
		return ptr_bitmap[pos];
	}

	//insert pointer in available slot
	inline void pointer_page::add_ptr(int ptr) {
		if (!ptr_bitmap.all()) {
			int i = available_slot();
			ptr_array[i] = ptr;
			ptr_bitmap.set(i);
		}
	}

	inline void pointer_page::remove_ptr(int pos) {
		if (pos < n_pointers) ptr_bitmap.reset(pos);
	}

	inline int pointer_page::get_bitmap() { return static_cast<int>(ptr_bitmap.to_ulong()); }
	inline int pointer_page::get_ptr (int i) { return ptr_array[i]; }
	inline bool pointer_page::is_full() { return ptr_bitmap.all(); }
	inline bool pointer_page::is_empty(){ return ptr_bitmap.none(); }
	//----------end pointer_page class----------

	//content page. Stores the actual index data, i.e. a lot of <key,rid>'s
	class content_page : public page {
		private :
			std::bitset<n_data>   data_bitmap;
			std::array<int,n_data> data_array;
			inline int available_slot();
		public :
			content_page() : page(content_page_t) {}
			inline int has_data(int);
			inline void add_data(std::pair<int,int>);
			inline void remove_data(int);
			inline std::pair <int,int> get_data(int);
			inline bool is_full();
			inline int get_bitmap();
			inline bool is_empty();

	};

	inline int content_page::available_slot(){
		auto bmp = data_bitmap.to_ulong();
		int i = 0;
		while(bmp%2 == 1){
			bmp = bmp >> 1;
			++i;
		}
		return i;
	}

	inline bool content_page::has_data(int pos) {
		return data_bitmap[pos];
	}

	inline void content_page::add_data(std::pair<int,int> p){
		if(!data_bitmap.all()){
			int i = available_slot();
			data_array[i] = p;
			data_bitmap.set(i);
		}
	}

	inline void content_page::remove_data(int pos) {
		if (pos < n_data) data_bitmap.reset(pos);
	}

	inline int content_page::get_bitmap() { return static_cast<int>(data_bitmap.to_ulong()); }
	inline std:pair<int,int> content_page::get_data(int i) { return data_array[i]; }
	inline bool content_page::is_full() { return data_bitmap.all(); }
	inline bool content_page::is_empty(){ return ptr_bitmap.none(); }
	//----------end content_page class----------
}
