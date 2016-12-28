#ifndef  _MY_PTSVM_H_
#define _MY_PTSVM_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <direct.h>
#include <io.h>
#include <Windows.h>
#include <vector>
#include <map>
#include <math.h>
using namespace std;

class AltNode
{
public:
	int index;
	double value;
	AltNode();
	AltNode(int index, double value);
	bool less(AltNode anode,int label);
};

class PTSVM
{
private:
	//输入
	string source_file_path_input;  //辅助训练数据的路径
	string train_file_path_input;	//训练数据的路径
	string test_file_path_input;	//测试数据的路径
	int iter_N;	//	迭代次数
	int semi_add_M;	//semi-supervised 过程中每次添加的实例数
	string tempDir;	//训练过程中临时文件存放位置

	//
	int source_data_num; //辅助训练数据个数
	int train_data_num;  //训练数据个数
	int test_data_num;
//最终结果存放
	vector<vector<double>> fresleft;
	vector<vector<double>> fresright;

	map<int,int> test_data_indexs;
	double weight_test;
	double weight_test_max;
	
	double train_err_by_des;
	double train_err_by_whole;
	double des_err_by_des;
	double des_err_by_whole;
	double sum_weight_des;
	double sum_weight_train;

	vector<int> source_data_label;
	vector<int> train_data_label;

	vector<double> weight_source;
	vector<double> weight_train;

	double Beita;


	//tempfilename
	string source_test_data_file;
	string train_test_data_file;
	string test_test_data_file;
	string des_test_data_file;
	string semi_test_data_file;

	string whole_train_data_file;
	string des_train_data_file;

	string whole_model_file;
	string des_model_file;

	string source_result_by_whole_file;
	string des_result_by_whole_file;
	string des_result_by_des_file;
	string test_result_by_whole_file;
	string test_result_by_des_file;

	string svm_pre_tenstep_path;

	AltNode *altposHeap;
	AltNode *altnegHeap;

public:

	PTSVM();
	~PTSVM();

	void input(string source_file_path_input,string train_file_path_input, string test_file_path_input,
						string svm_pre_tenstep_path,int iter_N=50, int semi_add_M=20,
						double weight_test_max=1.0, string tempDir="D:\\PTSVMtemp");
	void learn();
	void test();
	void init();
	void iter(int curT);
	string filename2path(string filename);
	void cropus();
	void testCropus();
	void trainCropus();
	string label2string(int label);
	void callSVM(int curT);
	void errCalculate();
	int PTSVM::res2label(double res);
	void reWeightSource();
	void reWeightTrain();
	void adjustTestData();
	void 	reWeightTest(int curT);

	void insertToAlt(AltNode* heap, int heap_size,int label, AltNode node);
	void Heapify(AltNode* heap, int heap_size, int cur, int label);

};



#endif