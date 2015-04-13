#include <array>
#include <bitset>


//base class which has the page header already defined, since it is common to all page types
class page {
	protected :
		int parent_pointer;
		int disc_writes;

	public :
		const char type;
		static const char fheader_page = 0;
		static const char pointer_page = 1;
		static const char content_page = 2;
		page (char t) : type(t), parent_pointer(-1), disc_writes(0) {};
		page (char t, int parent_ptr, int dw) : type(t), parent_pointer(parent_ptr), disc_writes(dw) {}
		inline void set_parent_ptr (int);
		inline int get_parent_ptr (int);
		inline void incr_disc_writes ();
}

inline void page::set_parent_ptr(int ptr) { parent_pointer = ptr; }
inline int page::get_parent_pointer() { return parent_pointer; }
inline int incr_disc_writes() { return ++disc_writes; }

//-----------end page class-----------------

//file header page. Contains the info about the hash table in the file
class fheader_page : public page {
	public :
		header_page() : page(page::fheader_page) {}
}

//----------end fheader_page class----------


//pointer page. Stores pointers to overflow pages from the buckets
class pointer_page : public page {
	private :
		std::bitset<n_pointers>   ptr_bitmap;
		std::array<int,n_pointers> ptr_array;
		inline int available_slot();
	public :
		static const n_pointers = 29;
		pointer_page() : page(page::pointer_page) {}
		pointer_page(std::array<int,n_pointers>) ptrs, int bitmap) : page(page::pointer_page) , ptr_bitmap(bitmap), ptr_array(ptrs) {}
		inline void add_ptr(int);
		inline int get_ptr(int);
		inline bool is_full();
}

//get rightmost available slot
inline int available_slot() {
	auto bitmap = ptr_bitmap.to_ulong();
	return (~bitmap & (bitmap+1))-1;
}

//insert pointer in available slot
inline void pointer_page::add_ptr(int ptr) {
	if (ptr_bitmap.any()) {
		int i = available_slot();
		ptr_array[i] = ptr;
		ptr_bitmap.set(i);
	}
}

inline int page_pointer::get_pointer (int i) { return ptr_array[i]; }
inline bool page_pointer::is_full() { return ptr_bitmap.any(); }
//----------end pointer_page class----------

//content page. Stores the actual index data, i.e. a lot of <key,rid>'s
class content_page : public page {
	public :
		content_page() : page(page::content_page) {}
}
//----------end content_page class----------
