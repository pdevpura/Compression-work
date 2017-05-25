
// added by abpd

#ifndef HPCL_COMP_FPH_PL_PROC_H_
#define HPCL_COMP_FPH_PL_PROC_H_

#include "hpcl_comp_pl_data.h"
#include "hpcl_comp_data.h"
#include <vector>
#include <cassert>
#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <unordered_set>

#define BYTES_LEN 16 // for 8 byte data
#define SIGN 1
#define EXP 11
#define MH 20
#define ML 32

using namespace std;

#define ABPD_DEBUG 0

template<class K>
class hpcl_comp_fph_pl_proc {
private:
  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;

  int m_pl_index;

public:
  hpcl_comp_fph_pl_proc();
  ~hpcl_comp_fph_pl_proc() {
    if(m_input)	 	delete m_input;
    if(m_output)	delete m_output;
      };
  void set_pl_index(int pl_index);
  void set_output(hpcl_comp_pl_data* output);
  hpcl_comp_pl_data* get_output();
  hpcl_comp_pl_data* get_input();
  void reset_output();

private:
  int m_type;
  void decompose_data(std::vector<unsigned char>& cache_data, string& packet);
  string int_to_hex( K i );
 


public:
  void run(int state,long long& rep_count,long long& t_nibles,unordered_map<string,long long>& exp_track,unordered_map<string,long long>& mh_track,unordered_map<string,long long>& ml_track,unordered_map<string,string>& exp_tracker,unordered_map<string,string>& mh_tracker,unordered_map<string,string>& ml_tracker);
  mem_fetch* get_mf_input();
  void set_mf_input(mem_fetch *mf);
  bool fetch_data;
  bool execute_data;
  void set_pl_type(int type);
  int get_pl_type();
  enum 
  {
	  NONE=0,
	  DUMMY,
	  COMP,
	  GET_OUTPUT,
  };

  string do_encode(vector<string> input,float& orig_bytes,float& encode_bytes,unordered_map<string,string> exp_tracker,
				                                                                  unordered_map<string,string> mh_tracker,unordered_map<string,string> ml_tracker,long long& rep_count);
  // split and count for each
  void split_raw_value(unordered_map<string,long long>& exp_track,unordered_map<string,long long>& mh_track,
				                                                  unordered_map<string,long long>& ml_track,vector<string> raw_binary_data);
  vector<string> convert_binary(string raw_data);
  string get_bin_str(unsigned int val);
  unsigned int get_dec(string raw);
};

template <class K>
hpcl_comp_fph_pl_proc<K>::hpcl_comp_fph_pl_proc() {
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_pl_index = -1;
  m_type=NONE;
}
template <class K>
void hpcl_comp_fph_pl_proc<K>::set_pl_type(int type) {
  m_type = type;
}
template <class K>
int hpcl_comp_fph_pl_proc<K>::get_pl_type() {
  return m_type;
}

template <class K>
void hpcl_comp_fph_pl_proc<K>::set_pl_index(int pl_index) {
  m_pl_index = pl_index;
}

template <class K>
void hpcl_comp_fph_pl_proc<K>::set_output(hpcl_comp_pl_data* output) {
  m_output = output;
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_fph_pl_proc<K>::get_output() {
  return m_output;
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_fph_pl_proc<K>::get_input() {
  return m_input;
}
template<class K>
string hpcl_comp_fph_pl_proc<K>::int_to_hex( K i )
{
  stringstream stream;
  stream << std::setfill ('0') << std::setw(sizeof(K)*2) 
	  << std::hex << i;
  return stream.str();
}
template <class K>
void hpcl_comp_fph_pl_proc<K>::decompose_data(vector<unsigned char>& cache_data, string& packet)
{
  for(unsigned i = 0; i < cache_data.size(); i=i+sizeof(K)) {
    K word_candi = 0;
    for(int j = sizeof(K)-1; j >= 0; j--) {
	         K tmp = cache_data[i+j];
      tmp = (tmp << (8*j));

      word_candi += tmp;
    }
    packet+=int_to_hex(word_candi);
  }
}

// added by abpd
template <class K>
mem_fetch* hpcl_comp_fph_pl_proc<K>::get_mf_input() {
	mem_fetch* mf = m_input->get_mem_fetch();

  return mf;
}
template <class K>
void hpcl_comp_fph_pl_proc<K>::set_mf_input(mem_fetch *mf) {

	m_input->set_mem_fetch(mf);
}

template <class K>
void hpcl_comp_fph_pl_proc<K>::reset_output() {
  m_output->clean();
}

template<class K>
string hpcl_comp_fph_pl_proc<K>::do_encode(vector<string> input,float& orig_bytes,float& encode_bytes,unordered_map<string,string> exp_tracker,
				            unordered_map<string,string> mh_tracker,unordered_map<string,string> ml_tracker,long long& rep_count)
{
		 string output;
		  // input refers to single packet, i.e 128 bytes packet split into 8 bytes words
		for(int i=0;i<input.size();i++)
		{
		        string cur = input[i];
				string exp = cur.substr(1,EXP);
				string mh = cur.substr(EXP+1,MH);
				string ml = cur.substr(1+EXP+MH);
				orig_bytes+=(cur.size()/8.0);
				string exp_code = exp_tracker.find(exp)!=exp_tracker.end() ? exp_tracker[exp] : exp;
				string mh_code = mh_tracker.find(mh)!=mh_tracker.end() ? mh_tracker[mh] : mh;
				string ml_code = ml_tracker.find(ml)!=ml_tracker.end() ? ml_tracker[ml] : ml;
				output = output + cur.substr(0,1) + exp_code + mh_code + ml_code;
				
					#ifdef COMP_DEBUG
					string out_word;
					out_word=cur.substr(0,1) + "#"+exp_code +"#"+ mh_code +"#"+ ml_code;
					int word_sz = out_word.size()-3;
					cout<<i<<" INPUT WORD: "<<cur<<" COMP WORD: "<<out_word<<" Bits size: "<<word_sz <<endl;
					#endif


		}
		encode_bytes+=(output.size()/8.0);
		return output;
}

// to convert hex to decimal

template<class K>
unsigned int hpcl_comp_fph_pl_proc<K>::get_dec(string raw)
{
	 stringstream convert(raw);
	 unsigned int ret;
	 convert >> std::hex >> ret;
	 return ret;
}
// function to get binary string from dec

template<class K>
string hpcl_comp_fph_pl_proc<K>::get_bin_str(unsigned int val)
{
	string res;
	int count=4; // each 4 bit only , need to change for generic code
	while(count>0)
	{
		res.insert(res.begin(),(val&1)+'0');
		val>>=1;
		count--;
	}
	return res;
}

// function to convert hexadecimal to binary // returns output in form of string(0,1)

template<class K>
vector<string> hpcl_comp_fph_pl_proc<K>::convert_binary(string raw_data)
{
	vector<string> bin_data;
    for(int i=0;i<raw_data.size();i+=BYTES_LEN)
	{
		string cur = raw_data.substr(i,BYTES_LEN);
		string res;
		for(int j = 0; j<cur.size();j++)
		{
			string tmp;
			tmp.push_back(cur[j]);
			unsigned int val = get_dec(tmp);
			res+=get_bin_str(val);
		}
		bin_data.push_back(res);
	}
	return bin_data;
}
// split and count for each
template<class K>
void hpcl_comp_fph_pl_proc<K>::split_raw_value(unordered_map<string,long long>& exp_track,unordered_map<string,long long>& mh_track,
										unordered_map<string,long long>& ml_track,vector<string> raw_binary_data)
{
	for(int i=0;i<raw_binary_data.size();i++)
	{
		string cur = raw_binary_data[i];
		string exp = cur.substr(1,EXP);
		string mh = cur.substr(EXP+1,MH);
		string ml = cur.substr(1+EXP+MH);
		if(exp_track.find(exp)==exp_track.end())
						exp_track.insert(make_pair(exp,1));
		else
						exp_track[exp]++;
		if(mh_track.find(mh)==mh_track.end())
			mh_track.insert(make_pair(mh,1));
		else
			mh_track[mh]++;
			
		if(ml_track.find(ml)==ml_track.end())
			ml_track.insert(make_pair(ml,1));
		else
			ml_track[ml]++;
	}
}
// added by abpd
template<class K>
void hpcl_comp_fph_pl_proc<K>::run(int state,long long& rep_count,long long& t_nibles,unordered_map<string,long long>& exp_track,unordered_map<string,long long>& mh_track,unordered_map<string,long long>& ml_track,unordered_map<string,string>& exp_tracker,unordered_map<string,string>& mh_tracker,unordered_map<string,string>& ml_tracker)
{

	mem_fetch* mf = m_input->get_mem_fetch();
  	m_output->set_mem_fetch(mf);

  	if(!mf) return;

  	//If mf is not type for compression,
  	if(mf->get_real_data_size() == 0)	return;

  	std::vector<unsigned char>& cache_data = mf->get_real_data_ptr();
	std::string packet;
	decompose_data(cache_data,packet);

	transform(packet.begin(),packet.end(),packet.begin(),::toupper);
	mf->compare_data.assign(packet);

	// CODE HERE

	float orig_bytes=0.0;
	float encode_bytes = packet.size()/2.0;
	vector<string> raw_binary_data = convert_binary(packet);
	if(state == 0)
	{
			// sampling
			split_raw_value(exp_track,mh_track,ml_track,raw_binary_data);
	}
	else
	{
			encode_bytes=0;
			//encoding
			string output = do_encode(raw_binary_data,orig_bytes,encode_bytes,exp_tracker,mh_tracker,ml_tracker,rep_count);
	}
	double bytes_count = packet.size()/2;
	//unsigned comp_byte_size = ceil((double)bytes_count);
	unsigned comp_byte_size = ceil((double)encode_bytes);

	mf->set_comp_data_size(comp_byte_size);
	//added by kh(071816)
	mf->set_comp_data_bits(comp_byte_size*8);

	#ifdef COMP_DEBUG
	cout<<"----FINAL COMP BYTES: "<<comp_byte_size<<"---"<<endl;
	#endif

	//mf->set_comp_data_size(mf->get_real_data_size());
	mf->init_size();

}

#endif /* HPCL_COMP_PL_PROC_H_ */
