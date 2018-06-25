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
   this->kernel_mem_pool = _kernel_mem_pool;
   this->process_mem_pool = _process_mem_pool;
   this->shared_size = _shared_size;
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
   page_directory[0] = page_table;
   page_directory[0] |= 3; // attribute set to: supervisor level, read/write, present(011 in binary)

   // fill in the rest 1023 entries 
   for(i = 1; i < 1024; i++){
      page_directory[i] = 0 | 2; // not present (just use the address 0)
   }

   Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   assert(false);
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   assert(false);
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
  assert(false);
  Console::puts("handled page fault\n");
}

