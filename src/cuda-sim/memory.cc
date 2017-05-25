// Copyright (c) 2009-2011, Tor M. Aamodt, Wilson W.L. Fung,
// The University of British Columbia
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
// Neither the name of The University of British Columbia nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "memory.h"
#include <stdlib.h>
#include "../debug.h"

//added by kh(052515)
#include "ptx_sim.h"
extern int st_impl_called;

#include "../gpgpu-sim/gpu-sim.h"
extern gpgpu_sim_config g_the_gpu_config;
#include "../gpgpu-sim/hpcl_comp_config.h"
extern hpcl_comp_config g_hpcl_comp_config;

//added by kh(073116)
#include "hpcl_cuda_comp_proc.h"
#include "hpcl_cuda_comp_dsc.h"
hpcl_cuda_comp_proc* g_hpcl_cuda_comp_proc = new hpcl_cuda_comp_proc;

#include "ptx_ir.h"

//added by kh(081516)
extern unsigned long long m_cudasim_tot_mem_access_no;
extern unsigned long long m_cudasim_tot_approx_mem_access_no;
///




template<unsigned BSIZE> memory_space_impl<BSIZE>::memory_space_impl( std::string name, unsigned hash_size )
{
   m_name = name;
   MEM_MAP_RESIZE(hash_size);

   m_log2_block_size = -1;
   for( unsigned n=0, mask=1; mask != 0; mask <<= 1, n++ ) {
      if( BSIZE & mask ) {
         assert( m_log2_block_size == (unsigned)-1 );
         m_log2_block_size = n; 
      }
   }
   assert( m_log2_block_size != (unsigned)-1 );
}

template<unsigned BSIZE> void memory_space_impl<BSIZE>::write( mem_addr_t addr, size_t length, const void *data, class ptx_thread_info *thd, const ptx_instruction *pI)
{
   mem_addr_t index = addr >> m_log2_block_size;
   if ( (addr+length) <= (index+1)*BSIZE ) {
      // fast route for intra-block access 
      unsigned offset = addr & (BSIZE-1);
      unsigned nbytes = length;
      m_data[index].write(offset,nbytes,(const unsigned char*)data);
   } else {
      // slow route for inter-block access
      unsigned nbytes_remain = length;
      unsigned src_offset = 0; 
      mem_addr_t current_addr = addr; 

      while (nbytes_remain > 0) {
         unsigned offset = current_addr & (BSIZE-1);
         mem_addr_t page = current_addr >> m_log2_block_size; 
         mem_addr_t access_limit = offset + nbytes_remain; 
         if (access_limit > BSIZE) {
            access_limit = BSIZE;
         } 
         
         size_t tx_bytes = access_limit - offset; 
         m_data[page].write(offset, tx_bytes, &((const unsigned char*)data)[src_offset]);

         // advance pointers 
         src_offset += tx_bytes; 
         current_addr += tx_bytes; 
         nbytes_remain -= tx_bytes; 
      }
      assert(nbytes_remain == 0); 
   }
   if( !m_watchpoints.empty() ) {
      std::map<unsigned,mem_addr_t>::iterator i;
      for( i=m_watchpoints.begin(); i!=m_watchpoints.end(); i++ ) {
         mem_addr_t wa = i->second;
         if( ((addr<=wa) && ((addr+length)>wa)) || ((addr>wa) && (addr < (wa+4))) ) 
            hit_watchpoint(i->first,thd,pI);
      }
   }
}

template<unsigned BSIZE> void memory_space_impl<BSIZE>::read_single_block( mem_addr_t blk_idx, mem_addr_t addr, size_t length, void *data) const
{
   if ((addr + length) > (blk_idx + 1) * BSIZE) {
      printf("GPGPU-Sim PTX: ERROR * access to memory \'%s\' is unaligned : addr=0x%x, length=%zu\n",
             m_name.c_str(), addr, length);
      printf("GPGPU-Sim PTX: (addr+length)=0x%lx > 0x%x=(index+1)*BSIZE, index=0x%x, BSIZE=0x%x\n",
             (addr+length),(blk_idx+1)*BSIZE, blk_idx, BSIZE);
      throw 1;
   }
   typename map_t::const_iterator i = m_data.find(blk_idx);
   if( i == m_data.end() ) {
      for( size_t n=0; n < length; n++ ) 
         ((unsigned char*)data)[n] = (unsigned char) 0;
      //printf("GPGPU-Sim PTX:  WARNING reading %zu bytes from unititialized memory at address 0x%x in space %s\n", length, addr, m_name.c_str() );
   } else {
      unsigned offset = addr & (BSIZE-1);
      unsigned nbytes = length;
/*
      //added by kh(073016)
      std::vector<unsigned char> tmp_data;
      tmp_data.resize(BSIZE);
      i->second.read(0, BSIZE, &tmp_data[0]);
      //printf("BSIZE = %u\n", BSIZE);
      printf("Org_Data (%u) = ", BSIZE);
      for(int i = 0; i < length; i++) {
        printf("%02x ", tmp_data[i]);
      }
      printf("\n");
      ////
*/
      i->second.read(offset,nbytes,(unsigned char*)data);
   }
}

template<unsigned BSIZE> void memory_space_impl<BSIZE>::read( mem_addr_t addr, size_t length, void *data, ptx_thread_info *thread, bool flag ) const
{
   mem_addr_t index = addr >> m_log2_block_size;
   if ((addr+length) <= (index+1)*BSIZE ) {
      // fast route for intra-block access 
      read_single_block(index, addr, length, data); 
   } else {
      // slow route for inter-block access 
      unsigned nbytes_remain = length;
      unsigned dst_offset = 0; 
      mem_addr_t current_addr = addr; 

      while (nbytes_remain > 0) {
         unsigned offset = current_addr & (BSIZE-1);
         mem_addr_t page = current_addr >> m_log2_block_size; 
         mem_addr_t access_limit = offset + nbytes_remain; 
         if (access_limit > BSIZE) {
            access_limit = BSIZE;
         } 
         
         size_t tx_bytes = access_limit - offset; 
         read_single_block(page, current_addr, tx_bytes, &((unsigned char*)data)[dst_offset]); 

         // advance pointers 
         dst_offset += tx_bytes; 
         current_addr += tx_bytes; 
         nbytes_remain -= tx_bytes; 
      }
      assert(nbytes_remain == 0); 
   }


   //added by kh(073116)
   if(g_hpcl_comp_config.hpcl_cudasim_approx_en == 1 && flag == false) {

     std::string glob_mem_name("global");
     if(glob_mem_name.compare(m_name) == 0) {


       //
       //printf("length = %u\n", length);
       assert(length == 4);

       //assert(thread);
       //printf("thread->get_pc() : %u thread->get_inst()->get_PC() : %u\n", thread->get_pc(), thread->get_inst()->get_PC());
       const struct memory_config *m_memory_config = &g_the_gpu_config.get_memory_config();
       new_addr_type block_addr = m_memory_config->m_L2_config.block_addr(addr);
       if(block_addr > 0) {	//const cache mem usually has zero addr, skip this.

	   //Step1: copy data at cache block level
	   std::vector<unsigned char> cache_block(128,0);
	   read(block_addr, 128, &cache_block[0], thread, true);
	   /*
	   printf("Org_Data (128) = ");
	   for(int i = 0; i < 128; i++) {
	     printf("%02x ", cache_block[i]);
	   }
	   printf("\n");
	   */

	   //Store the data to mf object.
	   //core_t * thread->get_core()->

	   hpcl_cuda_mem_fetch* mf = NULL;

	   if(thread) {
	     mf = new hpcl_cuda_mem_fetch(thread->get_pc(), thread->get_hw_sid(), thread->get_hw_wid());
	   } else {	//at the end of benchmark, thread is null.
	     mf = new hpcl_cuda_mem_fetch(0, 0, 0);
	   }

	   mf->set_real_data(&cache_block);
	   //added by kh(081516)
	   mf->set_access_addr(addr);
	   ///

	   //Step2: Perform compression/approximation
	   //Only approximated data returns. replace cache_block with approx_cache_block
	   enum hpcl_cuda_mem_fetch::COMP_DATA_TYPE type = hpcl_cuda_mem_fetch::NO_DATA_TYPE;
	   //std::vector<unsigned char> approx_cache_block(128,0);
	   std::vector<unsigned char> approx_cache_block;
	   g_hpcl_cuda_comp_proc->run(mf, approx_cache_block, type);

	   //added by kh(081516)
	   m_cudasim_tot_mem_access_no++;
	   ///

	   if(approx_cache_block.size() > 0) {
	     /*
	     printf("Org_Data (128) = ");
	     for(int i = 0; i < 128; i++) {
	       printf("%02x ", cache_block[i]);
	     }
	     printf("\n");

	     printf("Approx_Data (128, %d) = ", type);
	     for(int i = 0; i < 128; i++) {
	       printf("%02x ", approx_cache_block[i]);
	     }
	     printf("\n");
	     */
	     cache_block = approx_cache_block;

	     //added by kh(081516)
	     m_cudasim_tot_approx_mem_access_no++;
	     ///
	   }


	   //Step3: extract byte or word data from cache_block by length
	   //Replace data with the extracted data
	   int offset = addr - block_addr;
	   assert(offset >= 0);
	   std::vector<unsigned char> data_tmp(length);
	   bool is_zero = true;
	   for(int i = offset; i < (offset+length); i++) {
	     *((unsigned char*)(data+i-offset)) = cache_block[i];

	     /*
	     //added by kh(091416)
	     //To check whether or not the approximated word is zero
	     if(i == (offset+length-1)) {
	       if(!(cache_block[i] == 0 || cache_block[i] == 128)) {
		 is_zero = false;
	       }
	     } else {
	       if(cache_block[i] != 0) {
		 is_zero = false;
	       }
	     }
	     ///
	     */
	   }

	   //added by kh(091416)
	   /*
	   if(is_zero == true) {

	     printf("mf %u ----\n", mf->get_request_uid());

	     for(int i = offset; i < (offset+length); i++) {
	       printf("%02x", cache_block[i]);
	     }
	     assert(0);
	   }
	   */

	   delete mf;

	   //Unit Test: data copying from cache block to data without approximation
	   /*
	   int offset = addr - block_addr;
	   assert(offset >= 0);
	   std::vector<unsigned char> data_tmp(length);
	   for(int i = offset; i < (offset+length); i++) {
	     data_tmp[i-offset] = cache_block[i];
	   }

	   //check if data is same as data_tmp
	   bool is_same = true;
	   for(int i = 0; i < length; i++) {
	     if(data_tmp[i] != *((unsigned char*)(data+i))) {
	       is_same = false;
	       break;
	     }
	   }
	   if(is_same == false) {
	     for(int i = 0; i < length; i++) printf("data[%d] = %02x\n", i, *((unsigned char*)(data+i)));
	     for(int i = 0; i < length; i++) printf("data_tmp[%d] = %02x\n", i, data_tmp[i]);
	   }
	   assert(is_same == true);
	   */

       }
    }
   }
   ///




}

template<unsigned BSIZE> void memory_space_impl<BSIZE>::print( const char *format, FILE *fout ) const
{
   typename map_t::const_iterator i_page;
   for (i_page = m_data.begin(); i_page != m_data.end(); ++i_page) {
      fprintf(fout, "%s - %#x:", m_name.c_str(), i_page->first);
      i_page->second.print(format, fout);
   }
}

template<unsigned BSIZE> void memory_space_impl<BSIZE>::set_watch( addr_t addr, unsigned watchpoint ) 
{
   m_watchpoints[watchpoint]=addr;
}

template class memory_space_impl<32>;
template class memory_space_impl<64>;
template class memory_space_impl<8192>;
template class memory_space_impl<16*1024>;

void g_print_memory_space(memory_space *mem, const char *format = "%08x", FILE *fout = stdout) 
{
    mem->print(format,fout);
}

#ifdef UNIT_TEST

int main(int argc, char *argv[] )
{
   int errors_found=0;
   memory_space *mem = new memory_space_impl<32>("test",4);
   // write address to [address]
   for( mem_addr_t addr=0; addr < 16*1024; addr+=4) 
      mem->write(addr,4,&addr,NULL,NULL);

   for( mem_addr_t addr=0; addr < 16*1024; addr+=4) {
      unsigned tmp=0;
      mem->read(addr,4,&tmp);
      if( tmp != addr ) {
         errors_found=1;
         printf("ERROR ** mem[0x%x] = 0x%x, expected 0x%x\n", addr, tmp, addr );
      }
   }

   for( mem_addr_t addr=0; addr < 16*1024; addr+=1) {
      unsigned char val = (addr + 128) % 256;
      mem->write(addr,1,&val,NULL,NULL);
   }

   for( mem_addr_t addr=0; addr < 16*1024; addr+=1) {
      unsigned tmp=0;
      mem->read(addr,1,&tmp);
      unsigned char val = (addr + 128) % 256;
      if( tmp != val ) {
         errors_found=1;
         printf("ERROR ** mem[0x%x] = 0x%x, expected 0x%x\n", addr, tmp, (unsigned)val );
      }
   }

   if( errors_found ) {
      printf("SUMMARY:  ERRORS FOUND\n");
   } else {
      printf("SUMMARY: UNIT TEST PASSED\n");
   }
}

#endif
