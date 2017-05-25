/*
 * hpcl_comp_pl_proc.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#include <vector>
#include "hpcl_cuda_comp_dsc.h"

hpcl_cuda_comp_dsc::hpcl_cuda_comp_dsc() {
}

void hpcl_cuda_comp_dsc::run(hpcl_cuda_mem_fetch* mf)
{
#ifdef old
  mem_fetch* mf = m_input->get_mem_fetch();
  m_output->set_mem_fetch(mf);

  if(!mf) return;

  if(mf->get_real_data_size() == 0)	return;
#endif

  unsigned DSC_MIN_DS_SIZE = g_hpcl_comp_config.hpcl_dsc_min_ds_size; //16; //2; //16;

  std::vector<std::vector<unsigned char>* > cache_data;

  unsigned data_type_no = mf->get_trans_data_no() + 1;
  cache_data.resize(data_type_no, NULL);
  cache_data[0] = &mf->get_real_data_ptr();
  for(int i = 1; i < data_type_no; i++) cache_data[i] = &mf->get_trans_data_ptr(i-1);


  //unsigned data_type_no = cache_data.size();
  std::vector<std::vector<unsigned> > enc_status;
  std::vector<unsigned> comp_data_bits;
  std::vector<unsigned> comp_data_bits_only;

  enc_status.resize(data_type_no);
  comp_data_bits.resize(data_type_no, 0);
  comp_data_bits_only.resize(data_type_no, 0);

  std::vector<unsigned> dsc_res_candi;
  dsc_res_candi.push_back(DSC_MIN_DS_SIZE);
  unsigned ds_size = DSC_MIN_DS_SIZE*2;
  while(ds_size <= cache_data[0]->size()) {
    //added by kh(073016)
    if(ds_size > g_hpcl_comp_config.hpcl_dsc_max_ds_size) {
      break;
    }
    ///
    dsc_res_candi.push_back(ds_size);
    ds_size = ds_size*2;
  }

  unsigned min_index = 0;
  unsigned min_comp_data_bits = 0;
  unsigned min_ds_size = 0;
  unsigned min_comp_data_bits_only = 0;
  unsigned candi_res_no = dsc_res_candi.size();

  //added by kh(080316)
  unsigned start_data_type_index = 0;
  if(g_hpcl_comp_config.hpcl_remap_data_use_only_en == 1) {
    start_data_type_index = 1;
  }
  ///

  //deleted by kh(080316)
  //for(unsigned m = 0; m < data_type_no; m++) {
  //added by kh(080316)
  for(unsigned m = start_data_type_index; m < data_type_no; m++) {

    //static resolution for all segments
    //#ifdef DATA_SEG_COMP_STATIC_RES
    unsigned min_rec_comp_data_bits = 0;
    unsigned min_rec_comp_data_seg = 0;
    unsigned min_rec_comp_data_bits_only = 0;

    std::vector<std::vector<unsigned> > rec_enc_status;
    std::vector<unsigned> rec_comp_data_bits;
    std::vector<unsigned> rec_comp_data_bits_only;
    rec_enc_status.resize(candi_res_no);
    rec_comp_data_bits.resize(candi_res_no,0);
    rec_comp_data_bits_only.resize(candi_res_no,0);

    for(unsigned i = 0; i < candi_res_no; i++) {
      run_ds_comp(mf, dsc_res_candi[i], m, *cache_data[m], rec_enc_status[i], rec_comp_data_bits[i], rec_comp_data_bits_only[i]);
//	DATA_SEG_COMP_DEBUG_PRINT("DSM : mf %u res %u data_type %d org_data_bits %u comp_bits %u\n",
//	  mf->get_request_uid(), dsc_res_candi[i], m, mf->get_real_data_size()*8, rec_comp_data_bits[i]);
    }

    min_rec_comp_data_bits = rec_comp_data_bits[0];
    min_rec_comp_data_seg = dsc_res_candi[0];
    min_rec_comp_data_bits_only = rec_comp_data_bits_only[0];
    for(int i = 1; i < rec_comp_data_bits.size(); i++) {
      if(rec_comp_data_bits[i] < min_rec_comp_data_bits) {
	min_rec_comp_data_bits = rec_comp_data_bits[i];
	min_rec_comp_data_seg = dsc_res_candi[i];
	min_rec_comp_data_bits_only = rec_comp_data_bits_only[i];
      }
    }


    if(min_comp_data_bits == 0 || (min_comp_data_bits > min_rec_comp_data_bits)) {
      min_comp_data_bits = min_rec_comp_data_bits;
      min_ds_size = min_rec_comp_data_seg;
      min_index = m;
      min_comp_data_bits_only = min_rec_comp_data_bits_only;
    }


  }

  //DATA_SEG_COMP_DEBUG_PRINT("min_comp_data_bits %d, min_ds_size %u, data_source %u\n", min_comp_data_bits, min_ds_size, min_index);


  if(min_comp_data_bits > cache_data[0]->size()*8) { //if the compressed data is larger than original cache data
    mf->set_comp_data_bits(cache_data[0]->size()*8+1);	//1-bit indicates compressed or not
//    mf->set_dsc_comp_res(0);
//    mf->set_dsc_comp_data_type(mem_fetch::NO_DATA_TYPE);
//    mf->set_comp_algo_type(mem_fetch::NO_COMP);
//
//
//    mf->set_dsc_compressed_bits_only(1);
    DATA_SEG_COMP_DEBUG_PRINT("correct min_comp_data_bits %d\n", cache_data[0]->size()*8+1);
    DATA_SEG_COMP_DEBUG_PRINT("compressed_bits_only %u\n", mf->get_dsc_compressed_bits_only());
    //std::cout << "comp_data_type = " << mf->get_dsc_comp_data_type() << std::endl;
    //assert(0);
  } else {

    mf->set_comp_data_bits(min_comp_data_bits);
//    mf->set_dsc_comp_res(min_ds_size);
//    mf->set_comp_algo_type(mem_fetch::DSM_COMP);
//
    if(min_index == 0) 		mf->set_dsc_comp_data_type(hpcl_cuda_mem_fetch::ORG_DATA);
    else 			mf->set_dsc_comp_data_type(mf->get_remapped_data_type(min_index-1));


/*
    printf("----- mf %u starts ---- \n", mf->get_request_uid());
    mf->print_data_all(min_ds_size);
    printf("DSM - Final: Select ");
    mf->print_dsc_comp_data_type(mf->get_dsc_comp_data_type());
    printf("min_comp_data_bits %u, org_data_bits %u\n", min_comp_data_bits, mf->get_real_data_size()*8);
    printf("----------------------- \n\n");
*/


//    mf->set_dsc_compressed_bits_only(min_comp_data_bits_only);
    DATA_SEG_COMP_DEBUG_PRINT("compressed_bits_only %u\n", mf->get_dsc_compressed_bits_only());
  }
  DATA_SEG_COMP_DEBUG_PRINT("DSM (Final): mf %u res %u data_type %d org_data_bits %u comp_bits %u comp_bits_only %u\n", mf->get_request_uid(), mf->get_dsc_comp_res(), mf->get_dsc_comp_data_type(), mf->get_real_data_size()*8, mf->get_comp_data_bits(), mf->get_dsc_compressed_bits_only());
  DATA_SEG_COMP_DEBUG_PRINT("\n");



}


void hpcl_cuda_comp_dsc::run_ds_comp(hpcl_cuda_mem_fetch* mf, unsigned res,  unsigned data_type, std::vector<unsigned char>& cache_data,std::vector<unsigned>& enc_status, unsigned& comp_bits, unsigned& comp_data_bits_only)
{
  int cache_size = cache_data.size();
  int ds_no = cache_size/res;

//  #ifdef DATA_SEG_COMP_DEBUG_PRINT
//  DATA_SEG_COMP_DEBUG_PRINT("mf %u, Data_Type = %u, Res = %u ", mf->get_request_uid(), data_type, res);
//  DATA_SEG_COMP_DEBUG_PRINT("Data(%03d) = ", cache_size);
//  for(int i = 0; i < cache_size; i++) {
//      DATA_SEG_COMP_DEBUG_PRINT("%02x", cache_data[i]);
//  }
//  DATA_SEG_COMP_DEBUG_PRINT("\n");
//  #endif

  enc_status.resize(ds_no, 0);
  comp_bits = 0;
  comp_data_bits_only = 0;

  unsigned dsc_comp_header_bits = 0; //1-bit compressed or not, 2-bit resolution size, 8-bit encoding status
  dsc_comp_header_bits += 1; //1-bit compressed or not
  dsc_comp_header_bits += 3; //3-bit data remapping type
  dsc_comp_header_bits += 3; //3-bit resolution ( 2 ~ 128 )
  dsc_comp_header_bits += 3; //3-bit # of ds = 2 ^ (3bit)
  //dsc_comp_header_bits += (ds_no == 1)? 1 : (unsigned) log2((double)ds_no);	// based on the resolution and cache size
  //dsc_comp_header_bits += ds_no;
  comp_bits += dsc_comp_header_bits;
  comp_data_bits_only += dsc_comp_header_bits;

  //DATA_SEG_COMP_DEBUG_PRINT("\tStep1: comp_bits = %u\n", comp_bits);
  for(int i = 0; i < ds_no; i++)
  {
    bool is_all_byte_same = true;

    int index = i * res;
    unsigned char byte_data = cache_data[index];
    for(int j = 1; j < res; j++)
    {
      //DATA_SEG_COMP_DEBUG_PRINT("\tbyte_data %u, cache_data[%d] %u, ", byte_data, index+j, cache_data[index+j]);
      if(byte_data != cache_data[index+j]) {
	is_all_byte_same = false;
	//DATA_SEG_COMP_DEBUG_PRINT("diff!\n");
	break;
      } else {
	  //DATA_SEG_COMP_DEBUG_PRINT("same!\n");
      }
    }

    bool is_compressed = false;
    if(is_all_byte_same == true) {
      //DATA_SEG_COMP_DEBUG_PRINT("\tDS%d has all bytes same\n", i);
      unsigned char high_nibble = (byte_data & 0xf0) >> 4;
      unsigned char low_nibble = (byte_data & 0x0f);

      //DATA_SEG_COMP_DEBUG_PRINT("\thigh_nibble %u low_nibble %u\n", high_nibble, low_nibble);
      if(high_nibble == low_nibble) {
	is_compressed = true;
      }
    } else {
      //DATA_SEG_COMP_DEBUG_PRINT("\tDS%d does not have all bytes same\n", i);
    }

    if(is_compressed == true) {
      enc_status[i] = 1;
      comp_bits += (4+1);		//1: encoding status bit. 4: encoding bits
      comp_data_bits_only += (4+1);
      DATA_SEG_COMP_DEBUG_PRINT("\tDS%d is compressed\n", i);
    } else {
      enc_status[i] = 0;
      comp_bits += (res*8+1);
      DATA_SEG_COMP_DEBUG_PRINT("\tDS%d is not compressed\n", i);
    }
  }
  DATA_SEG_COMP_DEBUG_PRINT("\tStep2: comp_bits = %u, comp_data_bits_only = %u\n", comp_bits, comp_data_bits_only);
  /*
  if(comp_data_bits_only > 0) {
    comp_data_bits_only += 1;	//1-bit compressed or not
  }
   */
  //added by kh(072016)
  //#ifdef DATA_REMAP_OP
  if(g_hpcl_comp_config.hpcl_data_remap_op_en == 1) {
    //If the data is remapped.
    if(data_type > 0) {
      comp_bits += mf->get_remap_op_bits(data_type-1);
      comp_data_bits_only += mf->get_remap_op_bits(data_type-1);
    }
    ///
  }
  //#endif

  DATA_SEG_COMP_DEBUG_PRINT("\tStep3: comp_bits = %u, comp_data_bits_only = %u\n", comp_bits, comp_data_bits_only);
}

