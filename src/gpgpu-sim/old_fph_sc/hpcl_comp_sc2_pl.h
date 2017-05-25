
// added by abpd

#ifndef HPCL_COMP_SC2_PL_H_
#define HPCL_COMP_SC2_PL_H_

#include "hpcl_comp_sc2_pl_proc.h"
#include "hpcl_comp_pl_data.h"

#include <vector>
#include <unordered_map>
#include <utility>

using namespace std;

#define MAX_INST 1000000000
#define VFT_SIZE 7168 // 7kb
#define WORD_SIZE 4 // in bytes
#define SC_FILE_NAME "SC2_HUFFMAN_DATA_"
// HUFFAMN TREE DS
struct Tree
{	          
	public:
	bool is_symbol;
	string symbol;
	Tree *left;
	Tree *right;
	int val;
	Tree(bool is,string sy)
	{
		is_symbol=is;
		symbol.assign(sy);
		val=0;
		left=NULL;
		right=NULL;
	}
};
template<class K>
class hpcl_comp_sc2_pl {
private:
  unsigned m_id;
  unsigned m_pipeline_no;
  unsigned m_cur_state;
  // used for number of cycles , will try with 1 : TODO for rest
  std::vector<hpcl_comp_sc2_pl_proc<K>*> m_comp_pl_proc;

  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;

  vector<mem_fetch*> m_comp_buffer;
  
public:

  hpcl_comp_sc2_pl(unsigned pipeline_no, unsigned id,unsigned cur_state);
  ~hpcl_comp_sc2_pl() {

   cout << " Committed Instructions : " << ins_ct << " M_ID : "<<m_id<<endl;
   if(m_cur_state==0) // sampling state only
   write_data_file();
	/* TMP Commented -> need to change to smart ptr 
    if(m_input)	 	delete m_input;
    if(m_output)	delete m_output;
	*/

    for(int i = 0; i < m_comp_pl_proc.size(); i++) {
	delete m_comp_pl_proc[i];
    }
  }
  unordered_map<string,int> word_table;
  unordered_map<string,string> tracker; 
  long long rep_count;
  long long t_nibles;
  long long ins_ct;
  void create(unsigned buffer_size);
  void set_input(hpcl_comp_pl_data* input_data);
  hpcl_comp_pl_data* get_input();
  mem_fetch* run(int comp_state);
  void print(unsigned pl_index=0);
  
  mem_fetch* top_compressed_mem_fetch();
  void pop_compressed_mem_fetch();
  void write_data_file();
  void traverse(Tree *root,string stack,unordered_map<string,string>& code);
  void write_file(unordered_map<string,string> code);
 static bool myfunc_inc(pair<int,pair<Tree*,string> > a,pair<int,pair<Tree*,string> > b);
  static bool myfunc_dec(pair<int,pair<Tree*,string> > a,pair<int,pair<Tree*,string> > b);
  Tree* gen_hf_tree(vector<pair<int,pair<Tree*,string> > >& track);
  void pick_up_size(vector<pair<int,pair<Tree*,string> > >& track);
private:
  unsigned m_comp_buffer_size;
public:
  bool has_comp_buffer_space();
  // encoding func
  void form_tracker();
  vector<string> split(string s,char del);

};
template<class K>
hpcl_comp_sc2_pl<K>::hpcl_comp_sc2_pl(unsigned pipeline_no, unsigned id,unsigned cur_state) {
  m_pipeline_no = pipeline_no;
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_cur_state = cur_state;
  m_id = id;
  m_comp_buffer_size = -1;
  rep_count=0;
  t_nibles=0;
  ins_ct=0;
  form_tracker();
}
// generic function
template<class K>
vector<string> hpcl_comp_sc2_pl<K>::split(string s,char del)
{
    stringstream ss(s);
	string item;
	vector<string> res;
	while(getline(ss,item,del))
    {
          res.push_back(item);
    }
	return res;
}
//ENCODING PHASE FUNCTION
template<class K>
void hpcl_comp_sc2_pl<K>::form_tracker()
{
	stringstream convert;
	convert << m_id;
    string file_name = SC_FILE_NAME+convert.str();

	ifstream ifile;
	ifile.open(file_name);
	string input;
	while(getline(ifile,input))
    {
    	vector<string> tmp = split(input,' ');
	    tracker.insert(make_pair(tmp[0],tmp[1]));
    }
	ifile.close();
}

// write data to file for next phase
template<class K>
void hpcl_comp_sc2_pl<K>::write_file(unordered_map<string,string> code)
{
	stringstream convert;
	convert << m_id;
    string file_name = SC_FILE_NAME+convert.str();
	ofstream ofile;
    ofile.open(file_name);
	auto lt = code.begin();
	for(;lt!=code.end();lt++)
	{
	    pair<string,string> p = *lt;
		ofile<<p.first<<" "<<p.second<<endl;
    }
}
// compare functions

template<class K>
bool hpcl_comp_sc2_pl<K>::myfunc_inc(pair<int,pair<Tree*,string> > a,pair<int,pair<Tree*,string> > b)
{
	return a.first<b.first;
}

template<class K>
bool hpcl_comp_sc2_pl<K>::myfunc_dec(pair<int,pair<Tree*,string> > a,pair<int,pair<Tree*,string> > b)
{
	return a.first>b.first;
}
// main function to generate huffman tree

template<class K>
Tree* hpcl_comp_sc2_pl<K>::gen_hf_tree(vector<pair<int,pair<Tree*,string> > >& track)
{
    if(track.size()==0) return NULL;
    if(track.size()==1 && track[0].second.first!=NULL) return track[0].second.first;			
	pair<int,pair<Tree*,string> > a = track[0];
	pair<int,pair<Tree*,string> > b = track[1];
	track.erase(track.begin());
	track.erase(track.begin());
	Tree *node = new Tree(false,"");
	node->val=a.first+b.first;
	Tree *one_node = a.second.first;
	Tree *two_node = b.second.first;
	if(!a.second.first)
    {
        one_node = new Tree(true,a.second.second);
	    one_node->val=a.first;  
	}
	if(!b.second.first)
    {
   		two_node = new Tree(true,b.second.second);
   	    two_node->val=b.first;
   	}

	node->left=one_node;
	node->right=two_node;
	pair<int,pair<Tree*,string> > c;
	c.first=a.first+b.first;
	c.second.first=node;
	track.push_back(c);
	sort(track.begin(),track.end(),myfunc_inc);
	return gen_hf_tree(track);      																		                                        
}
// sampling phase function
template<class K>
void hpcl_comp_sc2_pl<K>::traverse(Tree *root,string stack,unordered_map<string,string>& code)
{
	if(!root) return;
	if(root->is_symbol)
	{
		code.insert(make_pair(root->symbol,stack));
		return;
	}
	traverse(root->left,stack+"0",code);
	traverse(root->right,stack+"1",code);
}

template<class K>
void hpcl_comp_sc2_pl<K>::pick_up_size(vector<pair<int,pair<Tree*,string> > >& track)
{
	int max_vft_enteries  = VFT_SIZE/WORD_SIZE;
	max_vft_enteries = track.size()>max_vft_enteries ? max_vft_enteries : track.size();
	vector<pair<int,pair<Tree*,string> > > tmp;
	tmp.insert(tmp.end(),track.begin(),track.begin()+max_vft_enteries);
	track.clear();
	track.insert(track.end(),tmp.begin(),tmp.end());
}

// sampling phase function
template<class K>
void hpcl_comp_sc2_pl<K>::write_data_file()
{
	vector<pair<int,pair<Tree*,string> > > track;
	auto it = word_table.begin();
	for(;it!=word_table.end();it++)
	{
		pair<string,int> pt = *it;
		if(pt.second>0)
		{
			pair<int,pair<Tree*,string> > p;
			p.first=pt.second;
			p.second.first=NULL;
			p.second.second.assign(pt.first);
			track.push_back(p);
																																																				}
	}
	// Pick up only that fits VFT size 7KB
	sort(track.begin(),track.end(),myfunc_dec);
	pick_up_size(track);
#ifdef COMP_DEBUG
	vector<pair<int,string>> track_data;
	for(auto it=track.begin();it!=track.end();it++){
		track_data.push_back(make_pair(it->first,it->second.second));
	}
#endif
	sort(track.begin(),track.end(),myfunc_inc);
	Tree *root = gen_hf_tree(track);

	unordered_map<string,string> code;
	string stack;
	traverse(root,stack,code);
#ifdef COMP_DEBUG
	ofstream file1;
	string file_name;
	stringstream ss;
	ss << m_id;
	file_name="sc2_huff_tree_data_"+ ss.str()+".txt";
	file1.open(file_name);
	for(auto it=track_data.begin();it!=track_data.end();it++){
		string str= it->second;
		file1<<it->first<<"\t"<<str<<"\t"<<code[str]<<endl;
	}
#endif


	write_file(code);
}
template<class K>
void hpcl_comp_sc2_pl<K>::create(unsigned comp_buffer_size) 
{
	// create pipeline  -> 3 for sc2
	for(unsigned i = 0; i < m_pipeline_no; i++) 
  	{
		m_comp_pl_proc.push_back(new hpcl_comp_sc2_pl_proc<K>());
		m_comp_pl_proc[i]->set_pl_index(i);
  	}
	m_comp_pl_proc[0]->set_pl_type(hpcl_comp_sc2_pl_proc<K>::GET_OUTPUT);
	for(unsigned i = 1; i < m_pipeline_no-1; i++)
	{
		m_comp_pl_proc[i]->set_pl_type(hpcl_comp_sc2_pl_proc<K>::DUMMY);
	}
	m_comp_pl_proc[m_pipeline_no-1]->set_pl_type(hpcl_comp_sc2_pl_proc<K>::COMP);
	for(unsigned i = 0; i < m_pipeline_no-1; i++)
	{
		m_comp_pl_proc[i]->set_output(new hpcl_comp_pl_data);
	}


	m_input = m_comp_pl_proc[m_pipeline_no-1]->get_input();
  	m_comp_pl_proc[m_pipeline_no-1]->set_output(new hpcl_comp_pl_data);
  	m_comp_buffer_size = comp_buffer_size;


}

template<class K>
void hpcl_comp_sc2_pl<K>::set_input(hpcl_comp_pl_data* input_data) {
  m_input->copy(input_data);
}

template<class K>
hpcl_comp_pl_data* hpcl_comp_sc2_pl<K>::get_input() {
  return m_input;
}
template<class K>
mem_fetch* hpcl_comp_sc2_pl<K>::run(int comp_state)
{
  /*
  if(ins_ct>MAX_INST) {
    cout << " Reached Max Instruction Mark, Break Simulation "<<MAX_INST<<endl;
    if(m_cur_state==0) write_data_file();
    exit(0);
  } else {
    ins_ct++;
  }
  */

  mem_fetch* ret = NULL;
  //deleted by kh(083116)
  //for(int i = 0; i < m_pipeline_no; i++)
  //added by kh(083116)
  for(int i = (m_pipeline_no-1); i >= 0; i--)
  {
    if(m_comp_pl_proc[i]->get_pl_type()==hpcl_comp_sc2_pl_proc<K>::COMP)
    {
      m_comp_pl_proc[i]->run(word_table,comp_state,rep_count,t_nibles,tracker);

      hpcl_comp_pl_data *tmp = m_comp_pl_proc[i]->get_output();
      m_comp_pl_proc[i-1]->get_output()->set_mem_fetch(tmp->get_mem_fetch());
      tmp->clean();
    }
    else if(m_comp_pl_proc[i]->get_pl_type()==hpcl_comp_sc2_pl_proc<K>::GET_OUTPUT)
    {
      hpcl_comp_pl_data *tmp = m_comp_pl_proc[i]->get_output();
      assert(tmp);
      mem_fetch* mf = tmp->get_mem_fetch();


      tmp->clean();

      ret = mf;
    }
    else
    {
      hpcl_comp_pl_data *tmp = m_comp_pl_proc[i]->get_output();
      assert(tmp);
      mem_fetch* mf = tmp->get_mem_fetch();
      if(mf)
      {
	m_comp_pl_proc[i-1]->get_output()->set_mem_fetch(mf);
      }
      tmp->clean();
    }

  }

  return ret;
		
}

template<class K>
void hpcl_comp_sc2_pl<K>::print(unsigned pl_index) {

}

template<class K>
mem_fetch* hpcl_comp_sc2_pl<K>::top_compressed_mem_fetch() {
  if(m_comp_buffer.size() == 0)	return NULL;
  else				return m_comp_buffer[0];
}

template<class K>
void hpcl_comp_sc2_pl<K>::pop_compressed_mem_fetch() {
  m_comp_buffer.erase(m_comp_buffer.begin());
}

template<class K>
bool hpcl_comp_sc2_pl<K>::has_comp_buffer_space() {
  if(m_comp_buffer.size() >= m_comp_buffer_size)	return false;
  else							return true;
}


#endif /* HPCL_COMP_PL_H_ */
