
// added by abpd

#ifndef HPCL_COMP_BPC_PL_PROC_H_
#define HPCL_COMP_BPC_PL_PROC_H_

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
#include <utility>
#include <algorithm>
//added by abpd
#include "hpcl_comp_anal.h"
#include "hpcl_comp_analysis.h"
using namespace std;
//Global Table for bpc
extern abpd_comp_anal *abpd_anal_ptr;
extern hpcl_comp_anal* g_hpcl_comp_anal;
string lut_bpc = "0123456789ABCDEF";
#define WORD_SIZE_BPC 8 
template<class K>
class hpcl_comp_bpc_pl_proc {
private:
  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;

  int m_pl_index;

public:
  hpcl_comp_bpc_pl_proc();
  ~hpcl_comp_bpc_pl_proc() {
    if(m_input)	 	delete m_input;
    if(m_output)	delete m_output;
      };
  void set_pl_index(int pl_index);
  void set_output(hpcl_comp_pl_data* output);
  hpcl_comp_pl_data* get_output();
  hpcl_comp_pl_data* get_input();
  void reset_output();
  //void init();
  unsigned int get_dec(string raw);
  string  get_bin_str(unsigned int val);
  vector<string> convert_binary(string raw_data);
  void encode_run_len(vector<string>& dbx);
  string form_output(vector<string> dbx);
  void create_lookup(vector<string>& dbx);
  void encode_others(vector<string>& dbx,vector<string> dbp,unsigned row_len);
  string do_actual_encode(vector<string> dbp,vector<string> dbx);
  void unsym_rotate(vector<string>& matrix,int degree);
  vector<string> get_dbx(vector<string> raw_data);
  string get_xor(string a,string b);
  int get_all_zero_row(vector<string> test_data,int& z_pairs,float& len_zero);
  bool is_all_zero(string a,int& z_pairs,float& len_zero);
  void rotate_mat(vector<string>& matrix,int degree);
  string calc_delta(string a,string b);
  void get_delta_value(vector<string>& raw_data);
  long get_num(string a);
  int  traverse(vector<string> dbx,int start);
  bool one_check(vector<string>& tmp_dbx,string code,int len,string act,int it);
  bool zero_check(string a,unsigned row_len);
  void rotate_mat_90(vector<string>& matrix);
  string conv_hex(string out);
  string compress_base(string base);

  string get_bin_str(unsigned int val,int ct);
private:
  int m_type;
  string int_to_hex( long i );



public:
 // vector<string> split(string s,char del);
  void run();
  mem_fetch* get_mf_input();
  void set_mf_input(mem_fetch *mf);
  void set_pl_type(int type);
  int get_pl_type();

  enum 
  {
	  NONE=0,
	  DUMMY,
	  COMP,
	  GET_OUTPUT,
  };

};

template <class K>
hpcl_comp_bpc_pl_proc<K>::hpcl_comp_bpc_pl_proc() {
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_pl_index = -1;
  m_type=NONE;
  //init();
}

template <class K>
void hpcl_comp_bpc_pl_proc<K>::set_pl_type(int type) {
  m_type = type;
}
template <class K>
int hpcl_comp_bpc_pl_proc<K>::get_pl_type() {
  return m_type;
}

template <class K>
void hpcl_comp_bpc_pl_proc<K>::set_pl_index(int pl_index) {
  m_pl_index = pl_index;
}

template <class K>
void hpcl_comp_bpc_pl_proc<K>::set_output(hpcl_comp_pl_data* output) {
  m_output = output;
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_bpc_pl_proc<K>::get_output() {
  return m_output;
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_bpc_pl_proc<K>::get_input() {
  return m_input;
}
template<class K>
string hpcl_comp_bpc_pl_proc<K>::int_to_hex( long i )
{
	string res;
  for(int j=0;j<64;j+=4){
		int num = i&15;
		res+=lut_bpc[num];
		//res+=((i&15)+'0');
		i=i>>4;
  }
  reverse(res.begin(),res.end());
	/*
  stringstream stream;
  stream << std::setfill ('0') << std::setw(sizeof(long)*2) 
	  << std::hex << i;
  return stream.str();
  */
	return res;
}

// added by abpd
template <class K>
mem_fetch* hpcl_comp_bpc_pl_proc<K>::get_mf_input() {
	mem_fetch* mf = m_input->get_mem_fetch();

  return mf;
}
template <class K>
void hpcl_comp_bpc_pl_proc<K>::set_mf_input(mem_fetch *mf) {

	m_input->set_mem_fetch(mf);
}

template <class K>
void hpcl_comp_bpc_pl_proc<K>::reset_output() {
  m_output->clean();
}
// added by abpd
template<class K>
void hpcl_comp_bpc_pl_proc<K>::run()
{
	
		mem_fetch* mf = m_input->get_mem_fetch();
  		m_output->set_mem_fetch(mf);

  		if(!mf) return;
		string raw_data;
  		//If mf is not type for compression,
  		if(mf->get_real_data_size() == 0)	return;
		float encoded_bytes =0.0;
		std::vector<unsigned char>& cache_data = mf->get_real_data_ptr();
		for(int i=0;i<cache_data.size();i++)
		{
			unsigned char temp_str =cache_data[i];
			raw_data.push_back(lut_bpc[temp_str>>4]);
			raw_data.push_back(lut_bpc[temp_str & 15 ]);
		}
		string base;
		//convert data into binary and outputs a vector with each string containing 4 bytes
		vector<string>raw_binary_data=convert_binary(raw_data);
#ifdef COMP_DEBUG
		cout<<"RAW DATA: "<<raw_data<<endl;
		cout<<"BINARY DATA:-----"<<endl;
		for(int i=0;i< raw_binary_data.size();i++){
			cout<<"BINARY DATA "<< i <<" : "<<raw_binary_data[i]<<endl;
		}
		cout<<"-----BINARY DATA OVER:-----"<<endl;
#endif
		get_delta_value(raw_binary_data);
#ifdef COMP_DEBUG
	cout<<"-----AFTER DELTA WITH PREVIOUS:-----"<<endl;
		for(int i=0;i< raw_binary_data.size();i++){
			cout<<"BINARY DATA "<< i <<" : "<<raw_binary_data[i]<<endl;
		}
	cout<<"-----AFTER DELTA WITH PREVIOUS OVER:-----"<<endl;

#endif
	/*	
		int max_degree=90;
		int all_zero_row=0;
		int zero_pairs =0;
		float avg_len_zero =0;
		for(int j=90;j<360;j+=90)
		{
			vector<string> test_data(raw_binary_data.begin(),raw_binary_data.end());
			rotate_mat(test_data,j);
			
			int z_pairs =0;
			float len_zero=0;
			int count = get_all_zero_row(test_data,z_pairs,len_zero);
			if(count > all_zero_row)
			{
				all_zero_row=count;
				max_degree=j;
			}
			else
			{
				max_degree = z_pairs > zero_pairs || len_zero > avg_len_zero ? j : max_degree;
			}
				
		}*/
		
		//rotate_mat(raw_binary_data,max_degree);
		base.assign(raw_binary_data[0]);
		#ifdef COMP_DEBUG
		cout<<"BASE: "<<base<<endl;
		#endif

		vector<string> new_raw_binary_data(raw_binary_data.size()-1);//exlude base
		copy(raw_binary_data.begin()+1,raw_binary_data.end(),new_raw_binary_data.begin());
		rotate_mat_90(new_raw_binary_data);

		vector<string> dbp(new_raw_binary_data.begin(),new_raw_binary_data.end());
		#ifdef COMP_DEBUG
		cout<<"DBP done"<<endl;
		#endif
		vector<string> dbx = get_dbx(new_raw_binary_data);
#ifdef COMP_DEBUG
		cout<<"------------"<<endl;
		int ct=0;
		cout<<"After DBX"<<endl;
		for(auto it=dbx.begin();it!=dbx.end();it++){
			cout<<ct<<". "<<*it<<endl;
				ct++;
		}
		cout<<"------------"<<endl;
#endif

		// do actual encode now
		string output = do_actual_encode(dbp,dbx);
		string comp_base=compress_base(base);	
		output=comp_base+output;
		string temp_out=conv_hex(output);
#ifdef COMP_DEBUG
		cout<<"PD::OUTPUT Binary DATA: "<<output <<endl;
		//cout<<"PD::OUTPUT Compressed DATA: "<<temp_out <<endl;
		cout<<"Final Compression bits: "<<output.size()<<endl;
#endif
		encoded_bytes+=((float)output.size()/8.0);
		mf->set_comp_data_size(ceil(encoded_bytes));

		//added by kh(071616)
		mf->set_comp_data_bits(encoded_bytes*8);
		///

		mf->init_size();
	
	
}


template<class K>
string hpcl_comp_bpc_pl_proc<K>::compress_base(string base){
	string base_4=base.substr(0,28);
	string base_8=base.substr(0,24);
	string base_16=base.substr(0,16);
	string zero_base_all(32,'0');
	string zero_base_4(28,'0');
	string zero_base_8(24,'0');
	string zero_base_16(16,'0');
	string out;
	if(base.compare(zero_base_all)==0){
		out="000";
#ifdef COMP_DEBUG
	cout<<"Zero Base found. Word "<<out<<endl;
#endif
	} else if(base_4.compare(zero_base_4)==0){
		out="001"+base.substr(28);
		#ifdef COMP_DEBUG
			cout<<"4bit integer found. Word "<<out<<endl;
		#endif

	} else if(base_8.compare(zero_base_8)==0){
		out="010"+base.substr(24);
			#ifdef COMP_DEBUG
			cout<<"8bit integer found. Word "<<out<<endl;
		#endif

	}else if(base_16.compare(zero_base_16)==0){
		out="011"+base.substr(16);
			#ifdef COMP_DEBUG
			cout<<"16bit integer found. Word "<<out<<endl;
		#endif

	}else{
		out="1"+base;
		#ifdef COMP_DEBUG
			cout<<"Uncompressed. Word "<<out<<endl;
		#endif

	}
	return out;


}

template<class K>
string hpcl_comp_bpc_pl_proc<K>::conv_hex(string out){
	int n=out.size();
	string res;
	for(int i=0;i<out.size();i+=4){
		int max_len=min(4,n-i);
		string new_str=out.substr(i,max_len);
		int dig=0;
		for(int j=0;j<new_str.size();j++){
			if(new_str[j]=='1'){
				dig+=pow(2,j);
			}
		}
		if(dig<=9)
			res.push_back(dig+'0');
		else
			res.push_back(dig-10+'A');
	}
	return res;
}

template<class K>
vector<string> hpcl_comp_bpc_pl_proc<K>::convert_binary(string raw_data)
{
	vector<string> bin_data;
	for(int i=0;i<raw_data.size();i+=WORD_SIZE_BPC)
	{
		string cur = raw_data.substr(i,WORD_SIZE_BPC);
		string res;
		for(int j = 0; j<cur.size();j++)
		{
			string tmp;
			tmp.push_back(cur[j]);
			unsigned int val = get_dec(tmp);
			res+=get_bin_str(val,4);
		}
		bin_data.push_back(res);
	}
	return bin_data;
	
}

// function to get binary string from dec
template<class K>
string  hpcl_comp_bpc_pl_proc<K>::get_bin_str(unsigned int val)
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
// to convert hex to decimal
template<class K>
unsigned int hpcl_comp_bpc_pl_proc<K>::get_dec(string raw)
{
	stringstream convert(raw);
	unsigned int ret;
	convert >> std::hex >> ret;
	return ret;
}

template<class K>
void hpcl_comp_bpc_pl_proc<K>::rotate_mat_90(vector<string>& matrix){
	vector<string> res_mat;
	int rows = matrix[0].size();
	int cols =matrix.size();
#ifdef COMP_DEBUG
	cout<<rows<<" "<<cols<<endl;
#endif
	res_mat.resize(rows);
	for(int i=0;i<rows;i++){
		res_mat[i].resize(cols);
	}
	for(int i=0;i<matrix.size();i++){
		for(int j=0;j<matrix[0].size();j++){
			res_mat[rows-j-1][i]=matrix[i][j];
		}
	}
		matrix.erase(matrix.begin(),matrix.end());
#ifdef COMP_DEBUG
		cout<<"------------"<<endl;
		int ct=0;
		cout<<"Orig Binary data RES rotat"<<endl;
		for(auto it=res_mat.begin();it!=res_mat.end();it++){
			cout<<ct<<". "<<*it<<endl;
				ct++;
		}
#endif

	matrix=res_mat;
}

template<class K>
void hpcl_comp_bpc_pl_proc<K>::rotate_mat(vector<string>& matrix,int degree)
{
	int row = matrix.size();
	int col = matrix[0].size();
	
	degree = degree % 360;
	
	// invalid rotation degree if not 90 multiple
	if(degree%90!=0 || degree== 0 || matrix.size()==0) return;
	
	if(row!=col)
	{
		unsym_rotate(matrix,degree);
		return;
	}
	
	
	if(degree == 180)
	{
		for(int i=0;i<row;i++) reverse(matrix[i].begin(),matrix[i].end());
		reverse(matrix.begin(),matrix.end());
		return;
	}
	// rotation for sym matrix, inplace algo
	if( degree == 90)
	{
		reverse(matrix.begin(),matrix.end());
	}
	
	if(degree == 270)
	{
		for(int i=0;i<matrix.size();i++)
		{
			reverse(matrix[i].begin(),matrix[i].end());
		}
	}
	
	if(degree == 90 || degree == 270)
	{
		for(int i=0;i<matrix.size();i++)
		{
			for(int j=i+1;j<matrix[i].size();j++)
			{
				swap(matrix[i][j],matrix[j][i]);
			}
		}
	}
}
template<class K>
bool hpcl_comp_bpc_pl_proc<K>::is_all_zero(string a,int& z_pairs,float& len_zero)
{
	string all_zero(WORD_SIZE_BPC*4,'0');
	//cout<<"PD::debug"<<a<<endl;
	assert(a.size()==WORD_SIZE_BPC*4);
	
	if(a.find(all_zero)!=string::npos) return true;
	
	float len=0;
	int ct=0;
	string name;
	for(int i=0;i<a.size();i++)
	{
		if(a[i]=='0') name.push_back(a[i]);
		else
		{
			ct += name.empty()==false ? 1 : 0;
			len+=name.size();
			name.clear(); 
		}
	}
	z_pairs+=ct;
	len_zero+=((1.0*len)/(1.0*ct));
	return false;
}
// function to get all zero row count
template<class K>
int hpcl_comp_bpc_pl_proc<K>::get_all_zero_row(vector<string> test_data,int& z_pairs,float& len_zero)
{
	int zero_count=0;
	for(int i=0;i<test_data.size();i++)
	{
		if(is_all_zero(test_data[i],z_pairs,len_zero)) zero_count++; 
	}
	return zero_count;
}
template<class K>
string hpcl_comp_bpc_pl_proc<K>::get_xor(string a,string b)
{
	string res;
	assert(a.size()==b.size());
	
	for(int i=0;i<a.size();i++)
	{
		res.push_back(((a[i]-'0')^(b[i]-'0'))+'0');
	}
	return res;
}
// N = N XOR N+1
template<class K>
vector<string> hpcl_comp_bpc_pl_proc<K>::get_dbx(vector<string> raw_data)
{
	vector<string> res;
	for(int i=0;i<raw_data.size()-1;i++)
	{
		res.push_back(get_xor(raw_data[i],raw_data[i+1]));
	}
	res.push_back(raw_data.back());
	return res;
}
template<class K>
void hpcl_comp_bpc_pl_proc<K>::unsym_rotate(vector<string>& matrix,int degree)
{
	int row = matrix.size();
	int col = matrix[0].size();
	
	
	if(degree == 180)
	{
		for(int i=0;i<row;i++) reverse(matrix[i].begin(),matrix[i].end());
		reverse(matrix.begin(),matrix.end());
		return;
	}
	vector<string> rotate_matrix;
	
	for(int i=0;i<col;i++)
	{
		string tmp;
		for(int j=0;j<row;j++)
		{
			if(degree == 90)
			tmp.insert(tmp.begin(),matrix[j][i]);
			else if(degree==270)
			tmp.push_back(matrix[j][i]);
		}
		rotate_matrix.push_back(tmp);
	}
	if(degree == 270)
	reverse(rotate_matrix.begin(),rotate_matrix.end());	
}

// Main function for data encode
template<class K>
string hpcl_comp_bpc_pl_proc<K>::do_actual_encode(vector<string> dbp,vector<string> dbx)
{
	assert(dbp.size()==dbx.size());
	string output;
	// Done all encoding on dbp
	unsigned row_len=dbx[0].size();
	create_lookup(dbx);
#ifdef COMP_DEBUG
	cout<<"LOOK UP DONE"<<endl;
#endif
	encode_run_len(dbx);
	#ifdef COMP_DEBUG
	cout<<"-----AFTER RUN LEN ENCODING:-----"<<endl;
		for(int i=0;i< dbx.size();i++){
			cout<< i <<". " <<dbx[i]<<endl;
		}
	cout<<"-----AFTER RUN LEN ENCODING OVER:-----"<<endl;

	#endif

	encode_others(dbx,dbp,row_len);
	#ifdef COMP_DEBUG
	cout<<"-----AFTER ENCODE OTHERS:-----"<<endl;
		for(int i=0;i< dbx.size();i++){
			cout<< i <<". " <<dbx[i]<<endl;
		}
	cout<<"-----AFTER ENCODE OTHERS OVER:-----"<<endl;

	#endif

	output = form_output(dbx);
	return output;
}

// encode for all 1
template<class K>
void hpcl_comp_bpc_pl_proc<K>::encode_others(vector<string>& dbx,vector<string> dbp,unsigned row_len)
{
	#ifdef COMP_DEBUG
		cout<<"ROW LEN USED "<<row_len<<endl;
	#endif
	//string one(WORD_SIZE_BPC*4,'1');
	//string zero(WORD_SIZE_BPC*4,'0');

	string one(row_len,'1');
	string zero(row_len,'0');
	string cons_one(2,'1');
	string sing_one(1,'1');
	
	vector<string> tmp_dbx;
	for(int i=0;i<dbx.size();i++)
	{
		string cur = dbx[i];
		auto it = cur.find("#");
		if(it!=string::npos)
		{
			// eligible for encode
			int org_index = stoi(cur.substr(0,it));
			// check for all ones
			string act = cur.substr(it+1);
			if(act.find(one)!=string::npos)
			{
				#ifdef COMP_DEBUG
					cout<<"word "<<i <<"All One "<<endl;
				#endif
				tmp_dbx.push_back("00000");
				continue;	
			}
			
			// cons 2 1's
			auto c_o = act.find(cons_one);
			if(c_o!=string::npos)
			{
				
				if(one_check(tmp_dbx,"00010",2,act,(int)c_o)){ 
				#ifdef COMP_DEBUG
					cout<<"word "<<i <<" Only 2 Cons One "<<endl;
				#endif
					continue;
				}
			}
			
			// single 1's
			
			auto s_o = act.find(sing_one);
			if(s_o!=string::npos)
			{
				if(one_check(tmp_dbx,"00011",1,act,(int)s_o)){
				#ifdef COMP_DEBUG
					cout<<"word "<<i <<" Only 1 One "<<endl;
				#endif
					continue;
				}
			}
			// DBX!=0 & DBP=0
			assert(org_index<dbp.size());
			if(zero_check(dbp[org_index],row_len))
			{
				tmp_dbx.push_back("00001");
				#ifdef COMP_DEBUG
				cout<<"word "<<i <<" DBP zero "<<endl;
				#endif
				continue;
			}
			
			// Uncompressed 
			string uncomp="1";
			uncomp+=act;
				#ifdef COMP_DEBUG
			cout<<"word "<<i <<" uncompressed "<<endl;
				#endif
			tmp_dbx.push_back(act);
			
		}
		else
		{
			tmp_dbx.push_back(dbx[i]);
		}
	}
	dbx.clear();
	dbx.insert(dbx.end(),tmp_dbx.begin(),tmp_dbx.end());
}
// create lookup
template<class K>
void hpcl_comp_bpc_pl_proc<K>::create_lookup(vector<string>& dbx)
{
	for(int i=0;i<dbx.size();i++)
	{

		stringstream convert;
		convert << i;
		string num = convert.str();
		dbx[i]=num+"#"+dbx[i];
	}
}

template<class K>
string hpcl_comp_bpc_pl_proc<K>::form_output(vector<string> dbx)
{
	string output;
#ifdef COMP_DEBUG

cout<<"------------"<<endl;
cout<<"COMPRESSED OUTPUT"<<endl;
#endif
	for(int i=0;i<dbx.size();i++)
	{

#ifdef COMP_DEBUG
	cout<<i<<". "<<dbx[i] <<" word size: "<<dbx[i].size()<<endl;
	cout<<"------------"<<endl;
#endif
		output+=dbx[i];
	}
	return output;
}

template<class K>
void hpcl_comp_bpc_pl_proc<K>::encode_run_len(vector<string>& dbx)
{
	vector<string> tmp_dbx;
	int i=0;
	while(i<dbx.size())
	{
		int max_zero = traverse(dbx,i);
		string start_index;
		string code;
		// handle for both 1 and more than 1 cons zero case
		if(max_zero >=1)
		{
			start_index = max_zero >1 ? get_bin_str(max_zero-2,5) : "";
			code = max_zero==1 ? "001" : "01";
			tmp_dbx.push_back(code+start_index);
#ifdef COMP_DEBUG
			cout<<"word "<<i<<"to  word "<<i+max_zero-1<<" Zero pattern found"<<endl;
#endif
			i+=max_zero;
		}
		else
		{
#ifdef COMP_DEBUG
			cout<<" word "<< i <<" No zero pattern "<<endl;
#endif
			tmp_dbx.push_back(dbx[i]);
			i++;
		}		
	}
	dbx.clear();
	dbx.insert(dbx.end(),tmp_dbx.begin(),tmp_dbx.end());
}

template<class K>
void hpcl_comp_bpc_pl_proc<K>::get_delta_value(vector<string>& raw_data)
{
	vector<string> new_data;
	new_data.push_back(raw_data[0]);
	for(int i=1;i<raw_data.size();i++)
	{
		new_data.push_back(calc_delta(raw_data[i-1],raw_data[i]));
	}
	raw_data.clear();
	raw_data.insert(raw_data.end(),new_data.begin(),new_data.end());		
}
// N+1 = N+1-N // delta calc
template<class K>
string hpcl_comp_bpc_pl_proc<K>::calc_delta(string a,string b)
{
	long a_num = get_num(a);
	long b_num = get_num(b);
#ifdef COMP_DEBUG
	cout<<"BIN A: "<<a<<" DECIMAL A " <<a_num<<endl;	
	cout<<"BIN B: "<<b<<" DECIMAL B " <<b_num<<endl;	
#endif
	//int res = (int)(b_num-a_num);
	long res = (b_num-a_num);
#ifdef COMP_DEBUG
	cout<<"RES " <<res<<endl;	
#endif
	string hex_str = int_to_hex(res);
#ifdef COMP_DEBUG
	cout<<"HEX_STR " <<hex_str<<endl;
#endif
	unsigned sz= hex_str.size()-9;
	hex_str.assign(hex_str.substr(sz));
#ifdef COMP_DEBUG
	cout<<"HEX_STR " <<hex_str<<endl;	
#endif
	string bi_val;
	for(int j = 0; j<hex_str.size();j++)
	{
		string tmp;
		tmp.push_back(hex_str[j]);
		unsigned int val = get_dec(tmp);
		bi_val+=get_bin_str(val,4);
	}
	bi_val.erase(bi_val.begin(),bi_val.begin()+3);
	return bi_val;
}

template<class K>
int  hpcl_comp_bpc_pl_proc<K>::traverse(vector<string> dbx,int start)
{
	if(start>=dbx.size()) return 0;
	
	//string sam(WORD_SIZE_BPC*4,'0');
	string cur = dbx[start];
	string sam;
	auto it = cur.find("#");
	if(it!=string::npos)
	{
		cur.assign(cur.substr(it+1));
		unsigned sz=cur.size();
		sam.resize(sz,'0');
	}
	else
	{
		// already encoded
		return 0;
	}
	assert(cur.size()==sam.size());
	if(cur.find(sam)!=string::npos)
	{
		return traverse(dbx,start+1)+1;
	}
	return 0;
}

template<class K>
long hpcl_comp_bpc_pl_proc<K>::get_num(string a)
{
	long res=0;
	unsigned  sz=a.size();
	if(a[0]=='1'){
		for(int i=0;i<sz;i++){
			if(i==0){
				res=pow(2,sz-i-1)*-1;
			}else{
				res+=pow(2,sz-i-1)*(a[i]-'0');
			}
		}
	} else{
		for(int i=0;i<sz;i++)
		{
			int c = a[i]-'0';
			res|=(c&1);
			if(i!=a.size()-1)
			res<<=1;	
		}
	}
	return res;
}

template<class K>
bool hpcl_comp_bpc_pl_proc<K>::one_check(vector<string>& tmp_dbx,string code,int len,string act,int it)
{
	string fw_str = act.substr(0,it);
	string bw_str = act.substr(it+len);
	//assert(fw_str.size()+bw_str.size()+len==WORD_SIZE_BPC*4);
	string az_1(fw_str.size(),'0');
	string az_2(bw_str.size(),'0');
	if(fw_str.find(az_1)!=string::npos && bw_str.find(az_2)!=string::npos)
	{
			string index = get_bin_str(it,5);
			tmp_dbx.push_back(code+index);
			return true;
	}
	return false;
}

template<class K>
bool hpcl_comp_bpc_pl_proc<K>::zero_check(string a,unsigned row_len)
{
	string zero(row_len,'0');
	if(a.find(zero)!=string::npos) return true;
	return false;
}

// function to get binary string from dec

template<class K>
string hpcl_comp_bpc_pl_proc<K>::get_bin_str(unsigned int val,int ct)
{
	string res;
	int count=ct; // each 4 bit only , need to change for generic code
	while(count>0)
	{
		res.insert(res.begin(),(val&1)+'0');
		val>>=1;
		count--;
	}
	return res;
}


#endif /* HPCL_COMP_PL_PROC_H_ */
