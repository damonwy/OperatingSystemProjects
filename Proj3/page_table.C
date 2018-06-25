#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;


void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = _shared_size;
   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
   // Set up the Page Directory
   unsigned long directory_frame_id = kernel_mem_pool->get_frames(1);
   this->page_directory = (unsigned long *)(directory_frame_id * Machine::PAGE_SIZE);

   // Set up the Page Table
   unsigned long table_frame_id = kernel_mem_pool->get_frames(1);
   unsigned long *page_table = (unsigned long *)(table_frame_id * Machine::PAGE_SIZE);

   // Fill in the Page Table
   unsigned long address = 0; // holds the physical address of where a page is
   unsigned int i;
   // Map the first 4MB of memory
   for (i = 0; i < 1024; i++){
      page_table[i] = address | 3; // attribute set to : supervisor level, read/write, present(011 in binary)
      address += 4096; // =4kb
   }

   // Fill in the Page Directory Entries
   // fill the first entry of the page directory
   page_directory[0] = (unsigned long)page_table;
   page_directory[0] |= 3; // attribute set to: supervisor level, read/write, present(011 in binary)

   // fill in the rest 1023 entries 
   for(i = 1; i < 1024; i++){
      page_directory[i] = 0 | 2; // not present (just use the address 0)
   }

   Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{ 
   current_page_table = this;
   // Start up Paging
   // write_cr3, read_cr3, write_cr0, and read_cr0 all come from the assembly functions
   write_cr3((unsigned long)page_directory); // put that page directory address into CR3
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{ 
   unsigned long cr0 = read_cr0();
   cr0 = cr0 | 0x8000000;
   write_cr0(cr0);
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
   unsigned long error_address = read_cr2();
   unsigned long table_id = error_address >> 12;
   unsigned long directory_id = table_id >> 10;

   table_id &= 0x3FF;
   directory_id &= 0x3FF;

   // Two-level page table. A page fault can be caused by:
   // Invalid entry in the page directory
   if(current_page_table->page_directory[directory_id] & 1 == false)
   {
      // Page not present
      unsigned long table_frame_id = kernel_mem_pool->get_frames(1);
      unsigned long * page_table = (unsigned long *)(table_frame_id * Machine::PAGE_SIZE);
      for(int i = 1; i < 1024; i++){
        page_table[i] = 0 | 2; // not present (just use the address 0)
      }
      current_page_table->page_directory[directory_id] = (unsigned long) page_table;
      current_page_table->page_directory[directory_id] |= 3;
   }

   unsigned long *pt = (unsigned long *)current_page_table->page_directory[directory_id];

   // Invalid entry in a page table page
   if(pt[table_id] & 1 == false) {
      unsigned long table_frame_id = process_mem_pool->get_frames(1);
      pt[table_id] =  (unsigned long)(Machine::PAGE_SIZE * table_frame_id);
      pt[table_id] |= 3; // Mark it as "present"
   }
  Console::puts("handled page fault\n");
}

