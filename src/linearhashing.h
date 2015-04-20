#include <utility>
#include <array>
#include <string>
#include <bitset>
#include <vector>
#include <unistd.h>
#include <cmath>

class hash_manager {
	public :
		static const char fheader_page_t = 0;
		static const char pointer_page_t = 1;
		static const char content_page_t = 2;
		static const int page_size     = 128;
		static const int n_pointers     = 28;
		static const int n_data         = 14;
		static const int original_size   = 4;

		hash_manager(std::string);
		int hash0(int);
		int hash1(int);
		int hash(int);
		bool add_data(int,int);
		int find_rid(int);
		int delete_data(int);

		class page {
			protected :
				int parent_pointer;
				int disc_writes;

			public :
				char type;
				page (char t) : type(t), parent_pointer(-1), disc_writes(0) {};
				page (const char[page_size]);
				virtual ~page() {}
				void bufferize(char[page_size]) const;
				void set_parent_ptr (int);
				int get_parent_ptr ();
				int incr_disc_writes ();
				void set_trash_flag ();
				bool is_trash();
		};

		class fheader_page : public page {
			private :
				int original_size;
				int n_buckets;
				int n_pages;
				int level;
				int next;

			public :
				fheader_page() : page(fheader_page_t) {}
				fheader_page(const char[page_size]);
				void bufferize(char[page_size]) const;
				void set_orig_size(int);
				void set_n_buckets(int);
				void set_n_pages(int);
				void set_level(int);
				void set_next(int);
				int get_orig_size();
				int get_n_buckets();
				int get_n_pages();
				int get_level();
				int get_next();
		};

		class pointer_page : public page {
			public :
				pointer_page() : page(pointer_page_t) {}
				pointer_page(const char[page_size]);
				void bufferize(char[page_size]) const;
				bool has_ptr(int);
				void add_ptr(int);
				void remove_ptr(int);
				int get_ptr(int);
				bool is_full();
				int get_bitmap();
			private :
				std::bitset<n_pointers>   ptr_bitmap;
				std::array<int,n_pointers> ptr_array;
				int available_slot();
		};

		class content_page : public page {
			private :
				std::bitset<n_data>   data_bitmap;
				std::array<std::pair<int,int>,n_data> data_array;
				int available_slot();
			public :
				content_page() : page(content_page_t) {}
				content_page(const char[page_size]);
				void bufferize(char[page_size]) const;
				bool has_data(int);
				void add_data(std::pair<int,int>);
				void remove_data(int);
				std::pair <int,int> get_data(int);
				bool is_full();
				int get_bitmap();
				int find_rid(int);
				int find_pos(int);
		};

		class bucket {
			private :
				void add_oveflow_page(int);
				void remove_overflow_page(int);
				hash_manager* hash_t;

			public :
				bucket(int pos,hash_manager* ptr) : hash_t(ptr), pos_in_file(pos) {}
				const int pos_in_file;
				class overflow_iterator;

				overflow_iterator begin();
				bool add_data_entry(int,int);
				bool remove_data_entry(int);
				int search_data_entry(int);
				bucket split_bucket(int,int);
				int remove_overflow_data(int);
				int remove_static_data(int);
				int search_overflow_pages(int);
				int search_static_pages(int);
		};

	private :

		std::string file_name;
		FILE* file;
		char buffer[page_size];
		int n_buckets;
		int n_pages;
		int level;
		int next;
		std::vector<bucket> buckets;

		template <typename Page_t> Page_t load_page(int);
		template <typename Page_t> void save_page(const Page_t&,int);

		char peek_type(int);
		void load_buffer(int);
		void write_buffer(int);
		void build_new();
		void build_from_file();

		static int chars_to_int(const char*);
		static void int_to_chars(int,char*);
};
