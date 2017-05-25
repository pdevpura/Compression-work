
#ifndef HPCL_DATA_ANAL_H_
#define HPCL_DATA_ANAL_H_

//#include "gpu-sim.h"
//#include "../cuda-sim/memory.h"
#include "hpcl_stat.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <map>

//added by kh(091916)
//#include "../cuda-sim/hpcl_cuda_comp.h"
///

class hpcl_data_anal
{
public:
  hpcl_data_anal ();
  virtual
  ~hpcl_data_anal ();


	enum sample_type {
		CHAR_TYPE_DATA_NO = 0,
		NONCHAR_TYPE_DATA_NO,
	};
	void create(double char_type_threshold);
	//void add_sample(enum sample_type type, double val, int id=-1);
	void display(std::ostream & os = std::cout) const;		
		

	int analyze_data_type(std::vector<unsigned char>& data, bool is_char, double char_type_pct);
	//double get_char_type_pct(std::vector<unsigned char>& data);

	enum int_data_redundancy_type {
		ZERO_VAL_PATTERNS = 0,
		NARROW_VAL_PATTERNS,
		REPEAT_VAL_PATTERNS,
		SIMILARITY_VAL_1_25_BYTE_PATTERNS,
		SIMILARITY_VAL_26_50_BYTE_PATTERNS,
		SIMILARITY_VAL_51_75_BYTE_PATTERNS,
		SIMILARITY_VAL_76_100_BYTE_PATTERNS,
		NO_PATTERNS,
	};
	enum int_data_redundancy_type get_int_redundancy_type_pct(std::vector<unsigned char>& data);
	/*
	bool is_all_zero(std::vector<unsigned int>& word);
	bool is_narrow_val(std::vector<unsigned int>& word);
	bool is_repeat_val(std::vector<unsigned int>& word);
	enum int_data_redundancy_type is_sim_val(std::vector<unsigned char>& data);
	*/
	void decompose_data(std::vector<unsigned char>& data, std::vector<unsigned int>& word_list);

private:
	//type of cache block
	hpcl_stat* m_char_type_no;
	hpcl_stat* m_nonchar_type_no;
	double m_char_type_threshold;


	hpcl_stat* m_zero_val_pat_no;
	hpcl_stat* m_narrow_val_pat_no;
	hpcl_stat* m_repeat_val_pat_no;
	hpcl_stat* m_similar_val_1_25_byte_pat_no;
	hpcl_stat* m_similar_val_26_50_byte_pat_no;
	hpcl_stat* m_similar_val_51_75_byte_pat_no;
	hpcl_stat* m_similar_val_76_100_byte_pat_no;
	hpcl_stat* m_no_pattern_no;


	//added by kh(030217)
	std::vector<hpcl_stat*> m_char_data_dist;		//16 classes of characters according to the high nibble.
	std::vector<hpcl_stat*> m_char_type_pct_dist;	//10 classes; 0-9, 10-19, ......


};

#endif /* HPCL_DATA_ANAL_H_ */
