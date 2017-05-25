/*
 * hpcl_cuda_comp.cc
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#include <iostream>
#include <string>
#include <stdio.h>
#include <cassert>

#include "hpcl_cuda_comp.h"
#include "hpcl_cuda_mem_fetch.h"

#include "../option_parser.h"
#include "../gpgpu-sim/hpcl_comp_config.h"
extern hpcl_comp_config g_hpcl_comp_config;
//added by ab(080616)
#include <unordered_map>
#include <utility>


//added by kh(081516)
#include "../gpgpu-sim/hpcl_approx_mem_space.h"
extern hpcl_approx_mem_space* g_hpcl_approx_mem_space;
///
#include "hpcl_cuda_user_define_stmts.h"

hpcl_cuda_comp::hpcl_cuda_comp ()
{
  // TODO Auto-generated constructor stub
}

hpcl_cuda_comp::~hpcl_cuda_comp ()
{
  // TODO Auto-generated destructor stub
}

void hpcl_cuda_comp::approximate_data(std::vector<unsigned char>& data, hpcl_cuda_mem_fetch::COMP_DATA_TYPE type)
{
  unsigned approx_range = g_hpcl_comp_config.hpcl_float_approx_range;	// # of approximable bits
  unsigned interval = 0;
  if(type == hpcl_cuda_mem_fetch::REMAPPED_DATA_2)	interval = 4;
  else if(type == hpcl_cuda_mem_fetch::REMAPPED_DATA_4)	interval = 8;
  else	assert(0);

  /*
  printf("approx_range %u\n", approx_range);
  printf("data before approximation = \n");
  for(int i = 0; i < data.size(); i++) {
    printf("%02x ", data[i]);
    if(i%4 == 3) printf("\n");
  }
  printf("\n");
  */

  if(g_hpcl_comp_config.hpcl_approx_type == hpcl_comp_config::SET_ZERO) {

     //basic approximation
     if(g_hpcl_comp_config.hpcl_approx_err_compen_en == 0) {

       /*
       for(int i = 0; i < data.size(); i = i + interval) {
	 for(unsigned j = 0; j < approx_range; j++) {
	   if(j%2 == 0) {
	     data[i+j/2] = data[i+j/2] & 0xf0;	//approxim
	   } else if(j%2 == 1) {
	     data[i+j/2] = data[i+j/2] & 0x0f;	//approxim
	   }
	 }
       }
       //assert(0);
       */

       //added by kh(091516)
       //approx_range = # of approximable bits
       int last_byte_index = (approx_range/2-1)/4;
       int last_2bit_index = (approx_range/2-1)%4;
       //printf("last_byte_index %d last_2bit_index %d\n", last_byte_index, last_2bit_index);
       for(int i = 0; i < data.size(); i = i + interval) {
	 /*
	 printf("before approx - ");
	 for(int j = 0; j < 4; j++)
	   printf("%02x ", data[i+j]);
	 printf("\n");
	 */

	 for(int j = 0; j < last_byte_index; j++) data[i+j] = 0x00;


	 if(last_2bit_index == 0) {
	   data[i+last_byte_index] = data[i+last_byte_index] & 0xfc;
	 } else if(last_2bit_index == 1) {
	   data[i+last_byte_index] = data[i+last_byte_index] & 0xf0;
	 } else if(last_2bit_index == 2) {
	   data[i+last_byte_index] = data[i+last_byte_index] & 0xc0;
	 } else if(last_2bit_index == 3) {
	   data[i+last_byte_index] = data[i+last_byte_index] & 0x00;
	 }

	 /*
	 printf("after approx - ");
	 for(int j = 0; j < 4; j++)
	   printf("%02x ", data[i+j]);
	 printf("\n");
	 */
       }
       ///

     } else {
     ////

       //added by kh(091416)
       for(int i = 0; i < data.size(); i = i + interval) {
	 if(approx_range == 2) {

	   bool need_err_compensation = false;
	   unsigned char nib = data[i+0] & 0x03;
	   if(nib >= 0x02) {
	     need_err_compensation = true;
	     //printf("data[i+0] = %02x\n", data[i+0]);
	   }

	    #ifdef ERROR_COMPEN_DEBUG
	    printf("before err compensation\n");
	    printf("%02x ", data[i+0]);
	    printf("%02x ", data[i+1]);
	    printf("%02x ", data[i+2] & 0x7f);
	    printf("\n");
	    #endif


	   data[i+0] = data[i+0] & 0xfc;	//approxim
	   if(need_err_compensation == true) {
	     unsigned int tmp = data[i+0];
	     tmp |= (data[i+1] << 8);
	     tmp |= ((data[i+2] & 0x7f) << 16);
	     unsigned int extra = (1 << approx_range);
	     tmp += extra;

	     data[i+0] = (unsigned char)(tmp & 0xff);
	     data[i+1] = (unsigned char)((tmp >> 8) & 0xff);
	     data[i+2] &= 0x80;
	     data[i+2] |= (unsigned char)((tmp >> 16) & 0x7f);

	     #ifdef ERROR_COMPEN_DEBUG
	     printf("after err compensation\n");
	     printf("%02x ", data[i+0]);
	     printf("%02x ", data[i+1]);
	     printf("%02x ", data[i+2] & 0x7f);
	     printf("\n");
	     assert(0);
	     #endif
	   }

	 } else if(approx_range == 4) {

	   bool need_err_compensation = false;
	   unsigned char nib = data[i+0] & 0x0f;
	   if(nib >= 0x08) {
	     need_err_compensation = true;
	     //printf("data[i+0] = %02x\n", data[i+0]);
	   }

	    #ifdef ERROR_COMPEN_DEBUG
	    printf("before err compensation\n");
	    printf("%02x ", data[i+0]);
	    printf("%02x ", data[i+1]);
	    printf("%02x ", data[i+2] & 0x7f);
	    printf("\n");
	    #endif

	   data[i+0] = data[i+0] & 0xf0;	//approxim
	   if(need_err_compensation == true) {
	     unsigned int tmp = data[i+0];
	     tmp |= (data[i+1] << 8);
	     tmp |= ((data[i+2] & 0x7f) << 16);
	     unsigned int extra = (1 << 4);
	     tmp += extra;

	     data[i+0] = (unsigned char)(tmp & 0xff);
	     data[i+1] = (unsigned char)((tmp >> 8) & 0xff);
	     data[i+2] &= 0x80;
	     data[i+2] |= (unsigned char)((tmp >> 16) & 0x7f);

	     #ifdef ERROR_COMPEN_DEBUG
	     printf("after err compensation\n");
	     printf("%02x ", data[i+0]);
	     printf("%02x ", data[i+1]);
	     printf("%02x ", data[i+2] & 0x7f);
	     printf("\n");
	     assert(0);
	     #endif
	   }
	 }
	 else if(approx_range == 6) {
	   bool need_err_compensation = false;
	   unsigned char high_nib = data[i+0] & 0x30;
	   high_nib = (high_nib >> 4);
	   if(high_nib >= 0x02) {
	     need_err_compensation = true;
	     //printf("data[i+0] = %02x\n", data[i+0]);
	   }

	   #ifdef ERROR_COMPEN_DEBUG
	   printf("before err compensation\n");
	   printf("%02x ", data[i+0]);
	   printf("%02x ", data[i+1]);
	   printf("%02x ", data[i+2] & 0x7f);
	   printf("\n");
	   #endif

	   data[i+0] = data[i+0] & 0xc0;	//approxim
	   if(need_err_compensation == true) {
	     unsigned int tmp = data[i+0];
	     tmp |= (data[i+1] << 8);
	     tmp |= ((data[i+2] & 0x7f) << 16);
	     unsigned int extra = (1 << approx_range);
	     tmp += extra;

	     data[i+0] = (unsigned char)(tmp & 0xff);
	     data[i+1] = (unsigned char)((tmp >> 8) & 0xff);
	     data[i+2] &= 0x80;
	     data[i+2] |= (unsigned char)((tmp >> 16) & 0x7f);

	     #ifdef ERROR_COMPEN_DEBUG
	     printf("after err compensation\n");
	     printf("%02x ", data[i+0]);
	     printf("%02x ", data[i+1]);
	     printf("%02x ", data[i+2] & 0x7f);
	     printf("\n");
	     assert(0);
	     #endif
	   }
	 }
	 else if(approx_range == 8) {
	   bool need_err_compensation = false;
	   unsigned char high_nib = data[i+0] & 0xf0;
	   unsigned char low_nib = data[i+0] & 0x0f;
	   high_nib = (high_nib >> 4);
	   if(high_nib >= 0x08) {
	     need_err_compensation = true;
	     //printf("data[i+0] = %02x\n", data[i+0]);
	   }

	   #ifdef ERROR_COMPEN_DEBUG
	   printf("before err compensation\n");
	   printf("%02x ", data[i+0]);
	   printf("%02x ", data[i+1]);
	   printf("%02x ", data[i+2] & 0x7f);
	   printf("\n");
	   #endif

	   data[i+0] = data[i+0] & 0x00;	//approxim
	   if(need_err_compensation == true) {
	     unsigned int tmp = data[i+0];
	     tmp |= (data[i+1] << 8);
	     tmp |= ((data[i+2] & 0x7f) << 16);
	     unsigned int extra = (1 << 8);
	     tmp += extra;

	     data[i+0] = (unsigned char)(tmp & 0xff);
	     data[i+1] = (unsigned char)((tmp >> 8) & 0xff);
	     data[i+2] &= 0x80;
	     data[i+2] |= (unsigned char)((tmp >> 16) & 0x7f);

	     #ifdef ERROR_COMPEN_DEBUG
	     printf("after err compensation\n");
	     printf("%02x ", data[i+0]);
	     printf("%02x ", data[i+1]);
	     printf("%02x ", data[i+2] & 0x7f);
	     printf("\n");
	     assert(0);
	     #endif
	   }
	 }
	 else if(approx_range == 10) {
	   bool need_err_compensation = false;
	   unsigned char low_nib = data[i+1] & 0x03;
	   if(low_nib >= 0x02) {
	     need_err_compensation = true;
	   }

	   #ifdef ERROR_COMPEN_DEBUG
	   printf("before err compensation\n");
	   printf("%02x ", data[i+0]);
	   printf("%02x ", data[i+1]);
	   printf("%02x ", data[i+2] & 0x7f);
	   printf("\n");
	   #endif

	   data[i+0] = data[i+0] & 0x00;	//approxim
	   data[i+1] = data[i+1] & 0xfc;	//approxim
	   if(need_err_compensation == true) {
	     unsigned int tmp = data[i+0];
	     tmp |= (data[i+1] << 8);
	     tmp |= ((data[i+2] & 0x7f) << 16);
	     unsigned int extra = (1 << approx_range);
	     tmp += extra;

	     data[i+0] = (unsigned char)(tmp & 0xff);
	     data[i+1] = (unsigned char)((tmp >> 8) & 0xff);
	     data[i+2] &= 0x80;
	     data[i+2] |= (unsigned char)((tmp >> 16) & 0x7f);

	     #ifdef ERROR_COMPEN_DEBUG
	     printf("after err compensation\n");
	     printf("%02x ", data[i+0]);
	     printf("%02x ", data[i+1]);
	     printf("%02x ", data[i+2] & 0x7f);
	     printf("\n");
	     assert(0);
	     #endif
	   }
	 }
	 else if(approx_range == 12) {
	   bool need_err_compensation = false;
	   unsigned char low_nib = data[i+1] & 0x0f;
	   //if((nib == 0x08) || (nib == 0x0c) || (nib == 0x0e) || (nib == 0x0f)) {
	   //if(low_nib == 0x0f && data[i+0] == 0xff ) {
	   if(low_nib >= 0x08) {
	     need_err_compensation = true;
	   }

	   #ifdef ERROR_COMPEN_DEBUG
	   printf("before err compensation\n");
	   printf("%02x ", data[i+0]);
	   printf("%02x ", data[i+1]);
	   printf("%02x ", data[i+2] & 0x7f);
	   printf("\n");
	   #endif

	   data[i+0] = data[i+0] & 0x00;	//approxim
	   data[i+1] = data[i+1] & 0xf0;	//approxim
	   if(need_err_compensation == true) {
	     unsigned int tmp = data[i+0];
	     tmp |= (data[i+1] << 8);
	     tmp |= ((data[i+2] & 0x7f) << 16);
	     unsigned int extra = (1 << 12);
	     tmp += extra;

	     data[i+0] = (unsigned char)(tmp & 0xff);
	     data[i+1] = (unsigned char)((tmp >> 8) & 0xff);
	     data[i+2] &= 0x80;
	     data[i+2] |= (unsigned char)((tmp >> 16) & 0x7f);

	     #ifdef ERROR_COMPEN_DEBUG
	     printf("after err compensation\n");
	     printf("%02x ", data[i+0]);
	     printf("%02x ", data[i+1]);
	     printf("%02x ", data[i+2] & 0x7f);
	     printf("\n");
	     assert(0);
	     #endif
	   }
	 }
	 else if(approx_range == 14) {
	   bool need_err_compensation = false;
	   unsigned char nib = data[i+1] & 0x30;
	   nib = (nib >> 4);
	   if(nib >= 0x02) {
	     need_err_compensation = true;
	   }

	   #ifdef ERROR_COMPEN_DEBUG
	   printf("before err compensation\n");
	   printf("%02x ", data[i+0]);
	   printf("%02x ", data[i+1]);
	   printf("%02x ", data[i+2] & 0x7f);
	   printf("\n");
	   #endif

	   data[i+0] = data[i+0] & 0x00;	//approxim
	   data[i+1] = data[i+1] & 0xc0;	//approxim
	   if(need_err_compensation == true) {
	     unsigned int tmp = data[i+0];
	     tmp |= (data[i+1] << 8);
	     tmp |= ((data[i+2] & 0x7f) << 16);
	     unsigned int extra = (1 << approx_range);
	     tmp += extra;

	     data[i+0] = (unsigned char)(tmp & 0xff);
	     data[i+1] = (unsigned char)((tmp >> 8) & 0xff);
	     data[i+2] &= 0x80;
	     data[i+2] |= (unsigned char)((tmp >> 16) & 0x7f);

	     #ifdef ERROR_COMPEN_DEBUG
	     printf("after err compensation\n");
	     printf("%02x ", data[i+0]);
	     printf("%02x ", data[i+1]);
	     printf("%02x ", data[i+2] & 0x7f);
	     printf("\n");
	     assert(0);
	     #endif
	   }
	 }
	 else if(approx_range == 16) {
	   bool need_err_compensation = false;
	   unsigned char nib = data[i+1] & 0xf0;
	   nib = (nib >> 4);
	   //if((nib == 0x08) || (nib == 0x0c) || (nib == 0x0e) || (nib == 0x0f)) {
	   if(nib >= 0x08) {
	     need_err_compensation = true;
	   }

	   #ifdef ERROR_COMPEN_DEBUG
	   printf("before err compensation\n");
	   printf("%02x ", data[i+0]);
	   printf("%02x ", data[i+1]);
	   printf("%02x ", data[i+2] & 0x7f);
	   printf("\n");
	   #endif

	   data[i+0] = data[i+0] & 0x00;	//approxim
	   data[i+1] = data[i+1] & 0x00;	//approxim
	   if(need_err_compensation == true) {
	     unsigned int tmp = data[i+0];
	     tmp |= (data[i+1] << 8);
	     tmp |= ((data[i+2] & 0x7f) << 16);
	     unsigned int extra = (1 << 16);
	     tmp += extra;

	     data[i+0] = (unsigned char)(tmp & 0xff);
	     data[i+1] = (unsigned char)((tmp >> 8) & 0xff);
	     data[i+2] &= 0x80;
	     data[i+2] |= (unsigned char)((tmp >> 16) & 0x7f);

	     #ifdef ERROR_COMPEN_DEBUG
	     printf("after err compensation\n");
	     printf("%02x ", data[i+0]);
	     printf("%02x ", data[i+1]);
	     printf("%02x ", data[i+2] & 0x7f);
	     printf("\n");
	     assert(0);
	     #endif
	   }
	 }
	 else if(approx_range == 18) {
	   bool need_err_compensation = false;
	   unsigned char nib = data[i+2] & 0x03;
	   if(nib >= 0x02) {
	     need_err_compensation = true;
	   }

	   #ifdef ERROR_COMPEN_DEBUG
	   printf("before err compensation\n");
	   printf("%02x ", data[i+0]);
	   printf("%02x ", data[i+1]);
	   printf("%02x ", data[i+2] & 0x7f);
	   printf("\n");
	   #endif

	   data[i+0] = data[i+0] & 0x00;	//approxim
	   data[i+1] = data[i+1] & 0x00;	//approxim
	   data[i+2] = data[i+2] & 0xfc;	//approxim
	   if(need_err_compensation == true) {
	     unsigned int tmp = data[i+0];
	     tmp |= (data[i+1] << 8);
	     tmp |= ((data[i+2] & 0x7f) << 16);
	     unsigned int extra = (1 << approx_range);
	     tmp += extra;

	     data[i+0] = (unsigned char)(tmp & 0xff);
	     data[i+1] = (unsigned char)((tmp >> 8) & 0xff);
	     data[i+2] &= 0x80;
	     data[i+2] |= (unsigned char)((tmp >> 16) & 0x7f);

	     #ifdef ERROR_COMPEN_DEBUG
	     printf("after err compensation\n");
	     printf("%02x ", data[i+0]);
	     printf("%02x ", data[i+1]);
	     printf("%02x ", data[i+2] & 0x7f);
	     printf("\n");
	     assert(0);
	     #endif
	   }
	 }
	 else if(approx_range == 20) {
	   bool need_err_compensation = false;
	   unsigned char nib = data[i+2] & 0x0f;
	   //if((nib == 0x08) || (nib == 0x0c) || (nib == 0x0e) || (nib == 0x0f)) {
	   if(nib >= 0x08) {
	     need_err_compensation = true;
	   }

	   #ifdef ERROR_COMPEN_DEBUG
	   printf("before err compensation\n");
	   printf("%02x ", data[i+0]);
	   printf("%02x ", data[i+1]);
	   printf("%02x ", data[i+2] & 0x7f);
	   printf("\n");
	   #endif

	   data[i+0] = data[i+0] & 0x00;	//approxim
	   data[i+1] = data[i+1] & 0x00;	//approxim
	   data[i+2] = data[i+2] & 0xf0;	//approxim
	   if(need_err_compensation == true) {
	     unsigned int tmp = data[i+0];
	     tmp |= (data[i+1] << 8);
	     tmp |= ((data[i+2] & 0x7f) << 16);
	     unsigned int extra = (1 << 20);
	     tmp += extra;

	     data[i+0] = (unsigned char)(tmp & 0xff);
	     data[i+1] = (unsigned char)((tmp >> 8) & 0xff);
	     data[i+2] &= 0x80;
	     data[i+2] |= (unsigned char)((tmp >> 16) & 0x7f);

	     #ifdef ERROR_COMPEN_DEBUG
	     printf("after err compensation\n");
	     printf("%02x ", data[i+0]);
	     printf("%02x ", data[i+1]);
	     printf("%02x ", data[i+2] & 0x7f);
	     printf("\n");
	     assert(0);
	     #endif
	   }
	 }
       }

     }
     ///



  } else { //added by ab(080616)

    for(unsigned j = 0; j < approx_range; j++) {

      unsigned approx_val =0;

      //Step1: extract nibles
      std::vector<unsigned> nib_track;
      for(int i = 0; i < data.size(); i = i + interval) {
	if(j%2==0) {
	  //nib_track.push_back((data[i+j/2]& 0xf0)>>4);
	  nib_track.push_back((data[i+j/2]& 0x0f));
	} else {
	  //nib_track.push_back(data[i+j/2] & 0x0f);
	  nib_track.push_back((data[i+j/2] & 0xf0)>>4);
	}
      }

      /*
      printf("approx_range = %u, iter %u\n", approx_range, j);
      for(int i = 0; i < nib_track.size(); i++) {
	printf("nib %d : %02x\n", i, nib_track[i]);
      }
      */

      //Step2: calculate approximate value
      if(g_hpcl_comp_config.hpcl_approx_type == hpcl_comp_config::SET_AVG) {
	// a. Take  Average
	for(unsigned k=0;k<nib_track.size();k++) {
	  approx_val+=nib_track[k];
	}
	approx_val /= nib_track.size();
	approx_val &= 0x0f;

	/*
	printf("approx_type = SET_AVG\n");
	printf("approx_val = %02x\n", approx_val);
	*/

      } else if(g_hpcl_comp_config.hpcl_approx_type == hpcl_comp_config::SET_FREQ) {

	// b. Take Max frequent
	std::unordered_map<int,int> freq_check;
	int max_c=0,max_val=0;
	for(unsigned k=0;k<nib_track.size();k++) {
	  //if(freq_check.find(nib_track[k])!=freq_check.end()) {	//bug
	  if(freq_check.find(nib_track[k]) == freq_check.end()) {
	    freq_check.insert(std::make_pair(nib_track[k],1));
	  } else {
	    freq_check[nib_track[k]]++;
	  }
	  if(freq_check[nib_track[k]]>max_c) {
	    max_c = freq_check[nib_track[k]];
	    max_val = nib_track[k];
	  }
	}
	approx_val = max_val;
	approx_val &= 0x0f;
	freq_check.clear();

	/*
	printf("approx_type = SET_FREQ\n");
	printf("approx_val = %02x\n", approx_val);
	*/
      }

      //clear extracted nibbles
      nib_track.clear();


      //Step2: Replace real data with approx data
      for(int i = 0; i < data.size(); i = i + interval) {
	if(j%2==0) {
	  data[i+j/2] &= 0xf0;
	  data[i+j/2] |= (approx_val);
	} else {
	  data[i+j/2] &= 0x0f;
	  data[i+j/2] |= (approx_val << 4);
	}
      }

    }

  }

  /*
  printf("\ndata after approximation = \n");
  for(int i = 0; i < data.size(); i++) {
    printf("%02x ", data[i]);
    if(i%4 == 3) printf("\n");
  }
  printf("\n");
  assert(0);
  */

}




void hpcl_cuda_comp::remap_data_to_type0(void* data, unsigned remap_res, std::vector<unsigned char>* remapped_data)
{
  hpcl_cuda_mem_fetch* mf = (hpcl_cuda_mem_fetch*)data;

  unsigned data_size = mf->get_real_data_size();
  remapped_data->resize(data_size, 0);
  //mf->config_trans_data(data_size, type_index);

  #ifdef TRASFORM_CORRECTNESS_TEST
  printf("transform_cache_data::Org_Data - ");
  for(unsigned i = 0; i < data_size; i++) {
    printf("%02x ", mf->get_real_data(i));
  }
  printf("\n");
  #endif

  //printf("haha1\n");
  //unsigned trans_res = 4;
  //added by kh(071316)
  assert(data_size%remap_res == 0);
  std::vector< std::vector<unsigned char> > bins;
  for(unsigned i = 0; i < remap_res; i++) {
    std::vector<unsigned char> tmp;
    //tmp.resize(data_size/trans_res,0);
    bins.push_back(tmp);
  }

  //printf("haha2\n");
  unsigned index = 0;
  for(unsigned i = 0; i < data_size; i++) {
    index = i%remap_res;
    bins[index].push_back(mf->get_real_data(i));
  }

  //printf("haha3\n");
  unsigned ptr = 0;
  //for(unsigned i = 0; i < bins.size(); i++) {
  for(int i = bins.size()-1; i >= 0; i--) {
    //printf("bins[i].size() %u\n", bins[i].size());
    for(unsigned j = 0; j < bins[i].size(); j++) {
      //printf("ptr %u, bins[i][j] 0x%2x\n", ptr, bins[i][j]);
      //mf->set_trans_data(ptr, bins[i][j], type_index);
      remapped_data->at(ptr) = bins[i][j];
      ptr++;
    }
  }

  #ifdef TRASFORM_CORRECTNESS_TEST
  printf("transform_cache_data::Trans_Data - ");
  for(unsigned i = 0; i < data_size; i++) {
    printf("%02x ", mf->get_trans_data(i));
  }
  printf("\n");
  #endif
}


int hpcl_cuda_comp::check_csn_pattern(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size)
{
  //Step1: CSN-checking.
  bool is_CSN_pattern = true;
  unsigned char high_nib = remapped_data_type[index] & 0x0f;
  unsigned char low_nib = (remapped_data_type[index] >> 4) & 0x0f;
  if(high_nib == low_nib) {
    //printf("\thigh_nib: %02x low_nib: %02x\n", high_nib, low_nib);
    for(int j = 1; j < subgroup_size; j++) {
      unsigned char hn = remapped_data_type[index+j] & 0x0f;
      unsigned char ln = (remapped_data_type[index+j] >> 4) & 0x0f;
      //printf("\thn: %02x ln: %02x\n", hn, ln);
      if(hn != high_nib || ln != high_nib) {
	  is_CSN_pattern = false;
	  break;
      }
    }
  } else {
    is_CSN_pattern = false;
  }

  ///
  if(is_CSN_pattern == true) {
    //printf("csn_segment: yes\n");
    return 1;
  } else {
    //printf("csn_segment: no\n");
    return 0;
  }
}

int hpcl_cuda_comp::complement_data(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size)
{
  //Step2: If subgroup is not CSN pattern, complement nibbles with more than 7
  int complement_use = 0;

  bool is_all_nibble_larger_than_7 = true;
  for(int j = 0; j < subgroup_size; j++) {
    unsigned char byte_val = remapped_data_type[index+j];
    if((byte_val & 0x0f) < 0x07)	{
      is_all_nibble_larger_than_7 = false;
      break;
    }

    if(((byte_val >> 4) & 0x0f) < 0x07)	{
      is_all_nibble_larger_than_7 = false;
      break;
    }
  }

  if(is_all_nibble_larger_than_7 == true) {
    complement_use = 0;
  } else {
    for(int j = 0; j < subgroup_size; j++) {
      unsigned char byte_val = remapped_data_type[index+j];
      unsigned char new_byte_val = 0;

      //printf("byte_val1 0x%02x, byte_val2 0x%02x comp %d\n", byte_val & 0x0f, 0x07, ((byte_val & 0x0f) > 0x07));
      if((byte_val & 0x0f) > 0x07) {
	new_byte_val = (~byte_val & 0x0f);
      } else {
	new_byte_val = (byte_val & 0x0f);
      }
      byte_val = byte_val >> 4;
      if((byte_val & 0x0f) > 0x07) {
	new_byte_val = new_byte_val | ((~byte_val & 0x0f) << 4);
      } else {
	new_byte_val = new_byte_val | ((byte_val & 0x0f) << 4 );
      }

      //printf("byte_val 0x%02x, new_byte_val 0x%02x\n", remapped_data_type1_1[index+j], new_byte_val);
      if(remapped_data_type[index+j] != new_byte_val) {
	remapped_data_type[index+j] = new_byte_val;
	complement_use = 1;
      }
    }
  }

  return complement_use;
}

int hpcl_cuda_comp::check_float_number(std::vector<unsigned char>& remapped_data_type, int group_size)
{
  //check if the first group (32B for 128B cache block) has CSN pattern
  int is_csn_segment1 = check_csn_pattern(remapped_data_type, 0, group_size/2);
  int is_csn_segment2 = check_csn_pattern(remapped_data_type, group_size/2, group_size/2);

  unsigned char byte_val1 = remapped_data_type[0] & 0x0f;
  unsigned char byte_val2 = remapped_data_type[group_size/2] & 0x0f;

  if(is_csn_segment1 == 1 && is_csn_segment2 == 1 && byte_val1 != 0) {
    if(byte_val1 != byte_val2)	return 1;
    else			return 2;
  } else {
    return 0;
  }
}

int hpcl_cuda_comp::compute_delta(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size, int& out_max_delta_val, unsigned char& out_min_nib_val)
{
  //Step3: If subgroup is not CSN pattern, compute delta value
  //Compute delta value
  int delta_val_use = 1;
  int MAX_DELTA = 3;

  //Find a minimum value
  unsigned char min_nib_val = (remapped_data_type[index] & 0x0f);
  unsigned char min_tmp_nib_val = (remapped_data_type[index] >> 4 & 0x0f);
  if(min_nib_val > min_tmp_nib_val) min_nib_val = min_tmp_nib_val;
  for(int j = 1; j < subgroup_size; j++) {
    unsigned char byte_val = remapped_data_type[index+j];
    unsigned char high_nib = byte_val & 0x0f;
    unsigned char low_nib = (byte_val >> 4) & 0x0f;
    if(high_nib < min_nib_val) {
      min_nib_val = high_nib;
    }
    if(low_nib < min_nib_val) {
      min_nib_val = low_nib;
    }
  }

  int max_delta_value = -1;
  for(int j = 0; j < subgroup_size; j++) {
    unsigned char high_nib = (remapped_data_type[index+j] & 0x0f);
    unsigned char low_nib = ((remapped_data_type[index+j] >> 4) & 0x0f);

    int delta_val1 = high_nib-min_nib_val;
    int delta_val2 = low_nib-min_nib_val;
    assert(delta_val1 >= 0);
    assert(delta_val2 >= 0);
    if(delta_val1 > MAX_DELTA || delta_val2 > MAX_DELTA) {
      delta_val_use = 0;
      max_delta_value = (delta_val1 > delta_val2)? delta_val1 : delta_val2;
    } else {
      if(max_delta_value < delta_val1)	max_delta_value = delta_val1;
      if(max_delta_value < delta_val2)	max_delta_value = delta_val2;
    }
  }
  if(delta_val_use == 1) {
    if(max_delta_value == 0) {	//all nibbles are same to each other.
      delta_val_use = 0;
    }
  }
  out_max_delta_val = max_delta_value;
  out_min_nib_val = min_nib_val;


  /*
  //There is no CSN pattern in the complement-only mode
  if(delta_val_use == 0 && complement_use == 1 && max_delta_value > 3) {
    complement_use = 0;
  }
  */
  ///
  return delta_val_use;
}

int hpcl_cuda_comp::compute_masking(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size, unsigned char& out_max_nib_val)
{
  //Step4: If subgroup is not CSN pattern, check masking
  int masking_use = 1;
  unsigned char max_nib_val = (remapped_data_type[index] & 0x0f);
  unsigned char max_tmp_nib_val = ((remapped_data_type[index] >> 4) & 0x0f);
  if(max_nib_val < max_tmp_nib_val) max_nib_val = max_tmp_nib_val;
  for(int j = 1; j < subgroup_size; j++) {
    unsigned char byte_val = remapped_data_type[index+j];
    unsigned char low_nib = byte_val & 0x0f;
    unsigned char high_nib = (byte_val >> 4) & 0x0f;
    if(high_nib > max_nib_val) {
      max_nib_val = high_nib;
    }
    if(low_nib > max_nib_val) {
      max_nib_val = low_nib;
    }
  }
  //printf("max_nib_val 0x%02x\n", max_nib_val);
  for(int j = 0; j < subgroup_size; j++) {
    unsigned char low_nib = (remapped_data_type[index+j] & 0x0f);
    unsigned char high_nib = (remapped_data_type[index+j] >> 4) & 0x0f;

    if(low_nib != 0 && low_nib != max_nib_val) {
      masking_use = 0;
      //printf("max_nib_val 0x%02x, low_nib \n", max_nib_val);
      break;
    }
    if(high_nib != 0 && high_nib != max_nib_val) {
      masking_use = 0;
      break;
    }
  }

  out_max_nib_val = max_nib_val;


  return masking_use;
}

int hpcl_cuda_comp::compute_neighbor_delta(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size, unsigned char& first_nib_val)
{
  //Step5: If subgroup is not CSN pattern, check constant neighboring delta
  int neighbor_const_delta_use = 1;
  unsigned char low_nib_val = (remapped_data_type[index] & 0x0f);
  unsigned char high_nib_val = ((remapped_data_type[index] >> 4) & 0x0f);
  unsigned char diff = (high_nib_val-low_nib_val)&0x0f;
  unsigned char last_nib_val = low_nib_val;

  for(int j = 1; j < subgroup_size; j++) {
    unsigned char byte_val = remapped_data_type[index+j];
    unsigned char low_nib = byte_val & 0x0f;
    unsigned char high_nib = (byte_val >> 4) & 0x0f;

    if(diff != ((last_nib_val - high_nib)&0x0f)) {
      neighbor_const_delta_use = 0;
      break;
    } else {
      last_nib_val = high_nib;
    }

    if(diff != ((last_nib_val - low_nib)&0x0f)) {
      neighbor_const_delta_use = 0;
      break;
    } else {
      last_nib_val = low_nib;
    }
  }

  first_nib_val = low_nib_val;

  return neighbor_const_delta_use;
}

//void hpcl_cuda_comp::remap_data_to_type1(void* data, unsigned remap_res, std::vector<unsigned char>* remapped_data_type1, unsigned& added_bits_for_remap, std::vector<enum hpcl_cuda_mem_fetch::REMAP_OP_TYPE>& remap_op_types, unsigned& approx_op, unsigned& approx_except)
void hpcl_cuda_comp::remap_data_to_type1(void* data, unsigned remap_res, std::vector<unsigned char>* remapped_data_type1, unsigned& added_bits_for_remap, unsigned& approx_op)
{
  hpcl_cuda_mem_fetch* mf = (hpcl_cuda_mem_fetch*) data;
  unsigned data_size = mf->get_trans_data_size();

  #ifdef DATA_REMAP_DEBUG
  printf("----- MF %u's data -----\n", mf->get_request_uid());
  printf("Original Data - ");
  for(unsigned i = 0; i < data_size; i++) {
    printf("%02x ", mf->get_real_data(i));
  }
  printf("\n");
  #endif

  //mf->config_trans_data(data_size, type_index);
  remapped_data_type1->resize(data_size, 0);

  std::vector<unsigned char> remapped_data_type0;
  remapped_data_type0.resize(data_size, 0);
  remap_data_to_type0(data, remap_res, &remapped_data_type0);

  #ifdef DATA_REMAP_DEBUG
  printf("Remapped Data (type 0) Remap Res %u - ", remap_res);
  for(unsigned i = 0; i < data_size; i++) {
    printf("%02x ", remapped_data_type0[i]);
  }
  printf("\n");
  #endif

  //added by kh(071316)
  unsigned remap_unit_size = data_size / remap_res;

  if(g_hpcl_comp_config.hpcl_data_remap_level == 0) {	//nibble-level remapping

    unsigned data_seg_no = data_size/remap_unit_size;
    for(unsigned i = 0; i < data_seg_no; i++) {
      //DATA_REMAP_DEBUG_PRINT("data segment %u\n", i);
      for(unsigned j = 0; j < (remap_unit_size*2); j=j+2) {
	unsigned index1 = i * remap_unit_size + j % remap_unit_size;
	unsigned index2 = i * remap_unit_size + (j+1) % remap_unit_size;
	//DATA_REMAP_DEBUG_PRINT("\tindex1 %u index2 %u, ", index1, index2);
	//unsigned char trans_data1 = mf->get_trans_data(index1, src_type_index);
	//unsigned char trans_data2 = mf->get_trans_data(index2, src_type_index);
	unsigned char trans_data1 = remapped_data_type0[index1];
	unsigned char trans_data2 = remapped_data_type0[index2];

	if(j < remap_unit_size) {
	  //DATA_REMAP_DEBUG_PRINT("\ttrans_data1 %02x, trans_data2 %02x, ", trans_data1, trans_data2);
	  trans_data1 = (trans_data1 & 0xf0)>>4;
	  trans_data2 = (trans_data2 & 0xf0);
	  trans_data1 = (trans_data1 | trans_data2);
	  //DATA_REMAP_DEBUG_PRINT("\tretrans_data %02x\n", trans_data1);
	  //mf->set_trans_data(i*remap_unit_size+j/2, trans_data1, type_index);
	  remapped_data_type1->at(i*remap_unit_size+j/2) = trans_data1;
	} else {
	  //DATA_REMAP_DEBUG_PRINT("\ttrans_data1 %02x, trans_data2 %02x, ", trans_data1, trans_data2);
	  trans_data1 = (trans_data1 & 0x0f);
	  trans_data2 = (trans_data2 & 0x0f)<<4;
	  trans_data1 = (trans_data1 | trans_data2);
	  //DATA_REMAP_DEBUG_PRINT("\tretrans_data %02x\n", trans_data1);
	  //mf->set_trans_data(i*remap_unit_size+j/2, trans_data1, type_index);
	  remapped_data_type1->at(i*remap_unit_size+j/2) = trans_data1;
	}
      }
    }

  } else if(g_hpcl_comp_config.hpcl_data_remap_level == 1) {	//bit-level remapping

    //Bit-level remapping
    unsigned data_seg_no = data_size/remap_unit_size;
    for(unsigned i = 0; i < data_seg_no; i++) {
      //DATA_REMAP_DEBUG_PRINT("data segment %u\n", i);
      for(unsigned j = 0; j < (remap_unit_size*8); j=j+8) {

	int bit_pos = 7 - j / remap_unit_size;
	int index = i * remap_unit_size + j % remap_unit_size;
	unsigned char remapped_byte = 0;
	for(unsigned k = 0; k < 8; k++) {
	  unsigned byte_index = index + k;
	  unsigned char bit_value = ((remapped_data_type0[byte_index] >> bit_pos) & 0x01);
	  unsigned char prev_remapped_byte = remapped_byte;
	  remapped_byte = (remapped_byte << 1) | bit_value;
	  //DATA_REMAP_DEBUG_PRINT("\tbit_pos %d index %u prev_remapped_byte %02x, remapped_byte %02x bit_value %2x\n", bit_pos, byte_index, prev_remapped_byte, remapped_byte, bit_value);
	}

	//DATA_REMAP_DEBUG_PRINT("\n");
	remapped_data_type1->at(i*remap_unit_size+j/8) = remapped_byte;
      }
    }

  }

  #ifdef DATA_REMAP_DEBUG
  printf("Remapped Data (type 1)  Remap Res %u - ", remap_res);
  for(unsigned i = 0; i < data_size; i++) {
    printf("%02x ", remapped_data_type1->at(i));
  }
  printf("\n");
  #endif


  //added by kh(072216)
  //reset remap_op_types
  //remap_op_types.clear();

  if(g_hpcl_comp_config.hpcl_data_remap_op_en == 1)
  {

  unsigned group_size = data_size/remap_res;
  //unsigned subgroup_size = group_size/2;
  unsigned subgroup_size = 8;

  std::vector<unsigned char> remapped_data_type1_1 = *remapped_data_type1;
  std::vector<unsigned char> remapped_data_type1_2 = *remapped_data_type1;
  std::vector<unsigned char> remapped_data_type1_3 = *remapped_data_type1;
  int flipped_subgroup_no = 0;
  //for(int m = 0; m < remap_res; m++) {
  //  int index = m*group_size;

  added_bits_for_remap = data_size/subgroup_size;	//to indicate whether or not subgroup is modified

  if(g_hpcl_comp_config.hpcl_data_remap_op_range == 1) {
    added_bits_for_remap = group_size/subgroup_size;
    assert(remap_res == 4);
  }


  for(int i = 0; i < data_size/subgroup_size; i++) {

      if(g_hpcl_comp_config.hpcl_data_remap_op_range == 1) {
	if(i >= group_size/subgroup_size)	break;
      }


      int index = i*subgroup_size;

      int csn_pattern = check_csn_pattern(remapped_data_type1_1, index, subgroup_size);
      if(csn_pattern == 1)	continue;

      //The complemented data is stored in remapped_data_type1_1.
      int complement_use = complement_data(remapped_data_type1_1, index, subgroup_size);
      unsigned char min_nib_val = 0;
      int max_delta_value = 0;
      int delta_val_use = compute_delta(remapped_data_type1_1, index, subgroup_size, max_delta_value, min_nib_val);

      //There is no CSN pattern in the complement-only mode
      if(delta_val_use == 0 && complement_use == 1 && max_delta_value > 3) {
	complement_use = 0;
      }
      unsigned char max_nib_val = 0;
      int masking_use = compute_masking(remapped_data_type1_2, index, subgroup_size, max_nib_val);
      unsigned char low_nib_val = 0;
      int neighbor_const_delta_use = compute_neighbor_delta(remapped_data_type1_3, index, subgroup_size, low_nib_val);

      int op_bits = 2;

      if(masking_use == 1) {

	added_bits_for_remap += ((subgroup_size*2)+op_bits+2);	//2: op, 2: # of ops

	unsigned char val = max_nib_val | (max_nib_val << 4);
	/*
	printf("masking use is applied\n");
	printf("Before - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");
	*/
	for(int j = 0; j < subgroup_size; j++)	remapped_data_type1->at(index+j) = val;
	/*
	printf("After - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");
	*/
	//remap_op_types.push_back(hpcl_cuda_mem_fetch::MASK_OP);


      }
      #ifdef old  //don't use this due to minimal impact
      else if(neighbor_const_delta_use == 1) {

	added_bits_for_remap += (4+op_bits+2);	//4: delta value, 2: op, 2: # of ops

	/*
	printf("neighbor_const_delta_use is applied\n");
	printf("Before - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");
	*/

	unsigned char val = low_nib_val | (low_nib_val << 4);

	for(int j = 0; j < subgroup_size; j++)	remapped_data_type1->at(index+j) = val;
	remap_op_types.push_back(hpcl_cuda_mem_fetch::NEIGHBOR_CONST_DELTA_OP);

	/*
	printf("After - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");

	assert(0);
	*/
      }
      #endif
      else if(complement_use == 1 && delta_val_use == 0) {
	added_bits_for_remap += ((subgroup_size*2)+op_bits+2);

	/*
	printf("complement use only is applied\n");
	printf("max_delta_value %d\n", max_delta_value);

	printf("Before - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");
	*/

	for(int j = 0; j < subgroup_size; j++)	remapped_data_type1->at(index+j) = remapped_data_type1_1[index+j];

	//remap_op_types.push_back(hpcl_cuda_mem_fetch::COMP_OP);

	/*
	printf("After - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");
	assert(0);
	*/

      }
      else if(complement_use == 0 && delta_val_use == 1) {
	//added by kh(072616)
	if(max_delta_value == 1) {
	  added_bits_for_remap += ((subgroup_size*2)+op_bits+1+2);	//1: max_delta_value indicator
	  //remap_op_types.push_back(hpcl_cuda_mem_fetch::DELTA_UPTO_1_OP);
	} else if(max_delta_value > 1) {
	  added_bits_for_remap += ((subgroup_size*2)*2+op_bits+1+2);	//1: max_delta_value indicator
	  //remap_op_types.push_back(hpcl_cuda_mem_fetch::DELTA_UPTO_3_OP);
	}
	///

	/*
	printf("delta val use only is applied\n");
	printf("Before - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");
	*/

	unsigned char val = min_nib_val | (min_nib_val << 4);
	for(int j = 0; j < subgroup_size; j++)	remapped_data_type1->at(index+j) = val;

	/*
	printf("After - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");
	*/

      } else if(complement_use == 1 && delta_val_use == 1) {


	if(max_delta_value == 1) {
	  added_bits_for_remap += ((subgroup_size*2)*2+op_bits+1+2);	//1: max_delta_value indicator
	  //remap_op_types.push_back(hpcl_cuda_mem_fetch::COMP_DELTA_UPTO_1_OP);
	} else if(max_delta_value > 1) {
	  added_bits_for_remap += ((subgroup_size*2)*3+op_bits+1+2);	//1: max_delta_value indicator
	  //remap_op_types.push_back(hpcl_cuda_mem_fetch::COMP_DELTA_UPTO_3_OP);
	}

	/*
	if(max_delta_value > 1) {
	printf("both complement and delta val use is applied\n");
	printf("Before - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");
	}
	*/
	unsigned char val = min_nib_val | (min_nib_val << 4);
	for(int j = 0; j < subgroup_size; j++)	remapped_data_type1->at(index+j) = val;
	/*
	if(max_delta_value > 1) {
	printf("After - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");
	}
	*/
      }
      else {
	 //no operation is possible
      }
  }
  //}

  #ifdef DATA_REMAP_DEBUG
  printf("New Remapped Data (type 1)  Remap Res %u - ", remap_res);
  for(unsigned i = 0; i < data_size; i++) {
    printf("%02x ", remapped_data_type1->at(i));
  }
  printf("\n");
  #endif

  }


  //unsigned group_size = data_size/remap_res;
  approx_op = 0;	//reset approx_op
  //approx_except = 0;	//reset approx_except
  if(g_hpcl_comp_config.hpcl_cudasim_approx_en == 1 && g_hpcl_comp_config.hpcl_float_approx_range > 0)
  {
      if(g_hpcl_comp_config.hpcl_float_det_type == hpcl_comp_config::HEURISTIC)
      {
	std::vector<unsigned char>& approx_remapped_data_type1 = *remapped_data_type1;
	unsigned group_size = approx_remapped_data_type1.size()/remap_res;
	int is_float_num = check_float_number(approx_remapped_data_type1, group_size);
	if(is_float_num == 1) {

	  //printf("Float number is checked\n");
	  /*
	  //for(int i = 0; i < group_size; i++) {
	  for(int i = 0; i < approx_remapped_data_type1.size(); i++) {
	    printf("%02x", approx_remapped_data_type1[i]);
	    if(i == group_size)	printf(" / ");
	  }
	  printf("\n");
	  */
	  for(int i = 1; i <= g_hpcl_comp_config.hpcl_float_approx_range; i++) {
	    int index = approx_remapped_data_type1.size() - (i * group_size/2);
	    bool is_subgroup_zeros = true;
	    for(int j = 0; j < group_size/2; j++) {
	      if(approx_remapped_data_type1[index+j] != 0) {
		is_subgroup_zeros = false;
		break;
	      }
	    }
	    if(is_subgroup_zeros == false) {

	      /*
	      printf("Before Approx..\n");
	      for(int j = 0; j < group_size/2; j++)	printf("%02x", approx_remapped_data_type1[index+j]);
	      printf("\n");
	      */

	      for(int j = 0; j < group_size/2; j++)	approx_remapped_data_type1[index+j] = 0;

	      /*
	      printf("After Approx..\n");
	      for(int j = 0; j < group_size/2; j++) printf("%02x", approx_remapped_data_type1[index+j]);
	      printf("\n");
	      */

	      approx_op = 1;	//set approx_op


	      //printf("mf %u | approximable float number\n", mf->get_request_uid());


	    } else {
	      //printf("subgroup zero!!!\n");
	    }
	  }

	} else if(is_float_num == 2) {

	  /*
	  printf("Is this Float number? mf %u\n", mf->get_request_uid());
	  for(int i = 0; i < approx_remapped_data_type1.size(); i++) {
	    if(i == group_size)	printf(" / ");
	    printf("%02x", approx_remapped_data_type1[i]);
	  }
	  printf("\n");

	  printf("After the first nibble-level remapping\n");
	  mf->print_data(remapped_data_type0, 4);
	  ///
	  */

	  //mf->print_data_all(4);
	  //approx_except = 1;
	}
      } else if(g_hpcl_comp_config.hpcl_float_det_type == hpcl_comp_config::MEMSPACE) {

	mem_addr_t addr = mf->get_access_addr();


	if(g_hpcl_approx_mem_space->is_in_approximable_mem_space(addr) == true) {

	  std::vector<unsigned char>& approx_remapped_data_type1 = *remapped_data_type1;
	  unsigned group_size = approx_remapped_data_type1.size()/remap_res;
	  for(int i = 1; i <= g_hpcl_comp_config.hpcl_float_approx_range; i++) {
	    int index = approx_remapped_data_type1.size() - (i * group_size/2);
	    bool is_subgroup_zeros = true;
	    for(int j = 0; j < group_size/2; j++) {
	      if(approx_remapped_data_type1[index+j] != 0) {
		is_subgroup_zeros = false;
		break;
	      }
	    }
	    if(is_subgroup_zeros == false) {

	      /*
	      printf("Before Approx..\n");
	      for(int j = 0; j < group_size/2; j++)	printf("%02x", approx_remapped_data_type1[index+j]);
	      printf("\n");
	      */

	      for(int j = 0; j < group_size/2; j++)	approx_remapped_data_type1[index+j] = 0;

	      /*
	      printf("After Approx..\n");
	      for(int j = 0; j < group_size/2; j++) printf("%02x", approx_remapped_data_type1[index+j]);
	      printf("\n");
	      */

	      approx_op = 1;	//set approx_op


	      //printf("mf %u | approximable float number\n", mf->get_request_uid());


	    } else {
	      //printf("subgroup zero!!!\n");
	    }
	  }

	}

      }













  }
  ///

}

void hpcl_cuda_comp::remap_data(void* data)
{
  hpcl_cuda_mem_fetch* mf = (hpcl_cuda_mem_fetch*) data;

  #ifdef DATA_REMAP_DEBUG
  unsigned data_size = mf->get_real_data_size();
  printf("----- MF %u's data SM %u WID %u PC %u Time %llu-----\n", mf->get_request_uid(), mf->get_tpc(), mf->get_wid(), mf->get_pc(), (gpu_sim_cycle+gpu_tot_sim_cycle));
  printf("Original Data - ");
  std::string val;
  std::vector<int> delta_val;
  std::vector<int> hex_val;
  for(int i = data_size-1; i >= 0; i--) {
    printf("%02x", mf->get_real_data(i));
    char str_hex[20];
    sprintf(str_hex, "%02x", mf->get_real_data(i));
    val += std::string(str_hex);
    if(i%4 == 0) {
      printf(" ");
      //std::cout << "val - " << val << std::endl;
      long i_hex = std::stol (val, 0, 16);
      if(hex_val.size() > 0) {
	delta_val.push_back(i_hex - hex_val[hex_val.size()-1]);
      }
      hex_val.push_back(i_hex);
      //std::cout << "i_hex  - " << i_hex << std::endl;
      val.clear();
    }
  }
  printf("\n");

  printf("Delta Val - ");
  for(int i = 0; i < delta_val.size(); i++) {
    printf("%02x ", delta_val[i]);
  }
  printf("\n");
  #endif

  unsigned remap_func_no = g_hpcl_comp_config.hpcl_data_remap_function.size();
  for(int i = 0; i < remap_func_no; i++) {
    unsigned func_index = g_hpcl_comp_config.hpcl_data_remap_function[i];

    std::vector<unsigned char>* remapped_data = mf->config_remapped_data(i, mf->get_real_data_size());
    unsigned remap_op_bits = 0;
    unsigned approx_op = 0;
    unsigned approx_exception = 0;
    //std::vector<enum hpcl_cuda_mem_fetch::REMAP_OP_TYPE> remap_op_types;
    if(func_index == 1) {
	remap_data_to_type0(mf, 4, remapped_data);
	mf->set_remapped_data_type(i, hpcl_cuda_mem_fetch::REMAPPED_DATA_1);
    } else if(func_index == 2) {
	//remap_data_to_type1(mf, 4, remapped_data, remap_op_bits, remap_op_types, approx_op, approx_exception);
	remap_data_to_type1(mf, 4, remapped_data, remap_op_bits, approx_op);
	mf->set_remapped_data_type(i, hpcl_cuda_mem_fetch::REMAPPED_DATA_2);
	mf->set_remap_op_bits(i, remap_op_bits);
//	mf->set_remap_op_type(i, remap_op_types);
	mf->set_approx_op(i, approx_op);
//	mf->set_approx_exception(i, approx_exception);
    } else if(func_index == 3) {
	remap_data_to_type0(mf, 8, remapped_data);
	mf->set_remapped_data_type(i, hpcl_cuda_mem_fetch::REMAPPED_DATA_3);
    } else if(func_index == 4) {
	//remap_data_to_type1(mf, 8, remapped_data, remap_op_bits, remap_op_types, approx_op, approx_exception);
	remap_data_to_type1(mf, 8, remapped_data, remap_op_bits, approx_op);
	mf->set_remapped_data_type(i, hpcl_cuda_mem_fetch::REMAPPED_DATA_4);
	mf->set_remap_op_bits(i, remap_op_bits);
//	mf->set_remap_op_type(i, remap_op_types);
	mf->set_approx_op(i, approx_op);
//	mf->set_approx_exception(i, approx_exception);
    } else if(func_index == 5) {
	remap_data_to_type0(mf, 2, remapped_data);
	mf->set_remapped_data_type(i, hpcl_cuda_mem_fetch::REMAPPED_DATA_5);
    } else if(func_index == 6) {
	//remap_data_to_type1(mf, 2, remapped_data, remap_op_bits, remap_op_types, approx_op, approx_exception);
	remap_data_to_type1(mf, 2, remapped_data, remap_op_bits, approx_op);
	mf->set_remapped_data_type(i, hpcl_cuda_mem_fetch::REMAPPED_DATA_6);
	mf->set_remap_op_bits(i, remap_op_bits);
//	mf->set_remap_op_type(i, remap_op_types);
	mf->set_approx_op(i, approx_op);
//	mf->set_approx_exception(i, approx_exception);
    } else {
	printf("Error: func_index %u\n", func_index);
	assert(0);
    }
  }
}

int hpcl_cuda_comp::compare_data(void* data1, void* data2)
{
  hpcl_cuda_mem_fetch* mf1 = (hpcl_cuda_mem_fetch*) data1;
  hpcl_cuda_mem_fetch* mf2 = (hpcl_cuda_mem_fetch*) data2;

  unsigned data_size1 = mf1->get_real_data_size();
  unsigned data_size2 = mf2->get_real_data_size();

  if(data_size1 != data_size2)	return -1;

  for(unsigned i = 0; i < data_size1; i++) {
    if(mf1->get_real_data(i) != mf2->get_real_data(i))	return -1;
  }

  return 0;
}




