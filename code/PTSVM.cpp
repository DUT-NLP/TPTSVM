#include "PTSVM.h"

PTSVM::PTSVM()
{
	source_test_data_file="source_test_data_file.test";
	train_test_data_file="train_test_data_file.test";
	test_test_data_file="test_test_data_file.test";
	des_test_data_file="des_test_data_file.test";
	semi_test_data_file="semi_test_data_file.test";

	whole_train_data_file="whole_train_data_file.train";
	des_train_data_file="des_train_data_file.train";

	whole_model_file="whole_model_file.model";
	des_model_file="des_model_file.model";

	source_result_by_whole_file="source_result_by_whole_file.result";
	des_result_by_whole_file="des_result_by_whole_file.result";
	des_result_by_des_file="des_result_by_des_file.result";
	test_result_by_whole_file="test_result_by_whole_file.result";
	test_result_by_des_file="test_result_by_des_file.result";
}

PTSVM::~PTSVM()
{
	delete[] altnegHeap;
	delete[] altposHeap;
}



void PTSVM::input(string source_file_path_input,string train_file_path_input, string test_file_path_input, 
								string svm_pre_tenstep_path, int iter_N/* =50 */,
								int semi_add_M/* =20 */, double weight_test_max/* =1.0 */,
								string tempDir/* ="D:PTSVMtemp" */)
{
	this->source_file_path_input=source_file_path_input;
	this->train_file_path_input=train_file_path_input;
	this->test_file_path_input=test_file_path_input;
	this->iter_N=iter_N;
	this->semi_add_M=semi_add_M;
	this->weight_test_max=weight_test_max;
	this->tempDir=tempDir;
	this->svm_pre_tenstep_path=svm_pre_tenstep_path;

	altnegHeap= new AltNode[semi_add_M/2+1];
	altposHeap= new AltNode[semi_add_M/2+1];

	//临时目录存在 ，返回
	if( 0==_access(tempDir.c_str() , 0 ) )
	{
		return;
	}
	//不存在创建，失败退出
	if( -1 == _mkdir(tempDir.c_str()) )
	{
		cout<<"创建临时文件失败"<<endl;
		exit(1);
	}
	return;

}

void PTSVM::learn()
{
	init();
	for (int curT=1; curT<=iter_N; curT++)
	{
		cout<<endl;
		cout<<endl;
		cout<<curT<<endl;
		iter(curT);
	}
	return;
}

void PTSVM::test()
{
	vector<int> Stand;
	ifstream DtestIn(filename2path(test_test_data_file).c_str());
	int label;
	string line;
	while(DtestIn>>label)
	{
		getline(DtestIn,line);
		Stand.push_back(label);
	}
	DtestIn.close();

	for (int n=0;n<iter_N;n++)
	{
		int recallright=0;
		int unrecall=0;
		int recallerr=0;

		for (int i=0;i<test_data_num;i++)
		{
			int standlabel=Stand[i];
			int reslabel=-1;
			int start=(n+1)/2;
			double left=0;
			double right=0;
			for(int j=start;j<=n;j++)
			{
				left+=fresleft[i][j];
				right+=fresright[i][j];
			}
			if(left>=right)
			{
				reslabel=1;
			}
			if(reslabel==1 && standlabel==1)	recallright++;
			if(reslabel==1 && standlabel==-1) recallerr++;
			if(reslabel==-1&& standlabel==1) unrecall++;
		}
		double p=(double) recallright/(recallright+recallerr);
		double r=(double)recallright/(recallright+unrecall);
		double f=p*r*2/(p+r);
		double a=(double) (test_data_num-unrecall-recallerr)/test_data_num;
		cout<<endl;
		cout<<endl;
		cout<<n<<endl;
		cout<<"A:"<<a<<"	P:"<<p<<"		R:"<<r<<"			F:"<<f<<endl;
	}
	return;
}

void PTSVM::init()
{
	string line;
	int label;

	//拷贝辅助数据
	source_data_num=0;
	ifstream source_data_instream(source_file_path_input.c_str());
	ofstream source_data_outstream(filename2path(source_test_data_file).c_str());
	while (source_data_instream>>label)
	{
		getline(source_data_instream , line);
		source_data_num++;
		source_data_label.push_back(label);
		source_data_outstream<<label2string(label)<<line<<endl;
	}
	source_data_instream.close();
	source_data_outstream.close();

	//拷贝训练数据
	train_data_num=0;
	ifstream train_data_instream(train_file_path_input.c_str());
	ofstream train_data_outstream(filename2path(train_test_data_file).c_str());
	while (train_data_instream>>label)
	{
		getline(train_data_instream,line);
		train_data_num++;
		train_data_label.push_back(label);
		train_data_outstream<<label2string(label)<<line<<endl;
	}
	train_data_instream.close();
	train_data_outstream.close();

	//测试数据
	test_data_num=0;
	ifstream test_data_instream(test_file_path_input.c_str());
	ofstream test_data_outstream(filename2path(test_test_data_file).c_str());
	while (getline(test_data_instream,line))
	{
		if (line=="")
		{
			continue;
		}
		test_data_num++;
		test_data_outstream<<line<<endl;
	}
	test_data_instream.close();
	test_data_outstream.close();

	// 删除之前的结果数据
	if( 0==_access(filename2path(test_result_by_whole_file).c_str() , 0 ) )
	{
		DeleteFileA(filename2path(test_result_by_whole_file).c_str());
	}

	//初始化权重
	for (int i=0; i<source_data_num; i++)
	{
		weight_source.push_back(1.0);
	}
	for(int i=0; i<train_data_num; i++)
	{
		weight_train.push_back(1.0);
	}

	//计算β
	Beita=1.0 / ( 1.0+sqrt( 2 * log((double) source_data_num) / iter_N ) );

	//===================
	vector<double> vtemp;
	for (int i=0;i<iter_N;i++)
	{
		vtemp.push_back(0);
	}
	for (int i=0;i<test_data_num;i++)
	{
		fresleft.push_back(vtemp);
		fresright.push_back(vtemp);
	}

	return;

}

void PTSVM::iter(int curT)
{
	cropus();
	callSVM(curT);
	errCalculate();
	reWeightSource();
	reWeightTrain();
	adjustTestData();
	reWeightTest(curT);
}

void PTSVM::cropus()
{
	testCropus();
	trainCropus();
}

void PTSVM::testCropus()
{
	ifstream test_result_instream(filename2path(test_result_by_whole_file).c_str());
	if(!test_result_instream)
	{
		return;
	}
	ifstream test_test_instream(filename2path(test_test_data_file).c_str());
	ofstream semi_test_outstream(filename2path(semi_test_data_file).c_str());

	int add_err=0;
	int index=0;
	string line;
	int label;

	while (test_test_instream>>label)
	{
		getline(test_test_instream,line);
		map<int,int>::iterator map_ite=test_data_indexs.find(index++);
		if(test_data_indexs.end()==map_ite)
		{
			continue;
		}
		if(map_ite->second != label)
		{
			add_err++;
		}
		semi_test_outstream<<label2string(map_ite->second)<<line<<endl;
	}

	cout<<"EEEEEEEEEE"<<add_err<<"/"<<test_data_indexs.size()<<endl;
	test_result_instream.close();
	test_test_instream.close();
	semi_test_outstream.close();
	return;	

}

void PTSVM::trainCropus()
{
	ofstream whole_train_out_stream(filename2path(whole_train_data_file).c_str());
	ofstream des_train_out_stream(filename2path(des_train_data_file).c_str());
	ofstream des_test_out_stream(filename2path(des_test_data_file).c_str());

	whole_train_out_stream.precision(8);
	whole_train_out_stream.setf(ios::fixed);
	des_train_out_stream.precision(8);
	des_train_out_stream.setf(ios::fixed);

	int index=0;
	int label;
	string line;
	//辅助训练数据
	ifstream source_test_instream(filename2path(source_test_data_file).c_str());
	while(source_test_instream>>label)
	{
		getline(source_test_instream,line);
		whole_train_out_stream<<label2string(label)<<" cost:"<<weight_source[index++]<<line<<endl;
	}
	source_test_instream.close();

	//训练语料数据
	index=0;
	ifstream train_test_instream(filename2path(train_test_data_file).c_str());
	while (train_test_instream>>label)
	{
		getline(train_test_instream,line);
		whole_train_out_stream<<label2string(label)<<" cost:"<<weight_train[index]<<line<<endl;
		des_train_out_stream<<label2string(label)<<" cost:"<<weight_train[index++]<<line<<endl;
		des_test_out_stream<<label2string(label)<<line<<endl;
	}
	train_test_instream.close();

	//测试语料数据
	ifstream semi_test_instream(filename2path(semi_test_data_file).c_str());
	while(semi_test_instream>>label)
	{
		getline(semi_test_instream,line);
		whole_train_out_stream<<label2string(label)<<" cost:"<<weight_test<<line<<endl;
		des_train_out_stream<<label2string(label)<<" cost:"<<weight_test<<line<<endl;
		des_test_out_stream<<label2string(label)<<line<<endl;
	}
	semi_test_instream.close();

	whole_train_out_stream.close();
	des_train_out_stream.close();
	des_test_out_stream.close();
}

string PTSVM::filename2path(string filename)
{
	return tempDir+"\\"+filename;
}

string PTSVM::label2string(int label)
{
	if(label>0)
	{
		return "+1";
	}
	return "-1";
}

void PTSVM::callSVM(int curT)
{
	//训练
	cout<<"/********************************whole_train*************************************/"<<endl;
	string cmdstr="svm_learn "+filename2path(whole_train_data_file)+" "+filename2path(whole_model_file);
	system(cmdstr.c_str());

	cout<<"/*************************************des_train********************************/"<<endl;
	cmdstr="svm_learn "+filename2path(des_train_data_file)+" "+filename2path(des_model_file);
	system(cmdstr.c_str());

	//测试
	cout<<"/***************************source_by_whole******************************************/"<<endl;
	cmdstr="svm_classify "+filename2path(source_test_data_file)+" "
		+filename2path(whole_model_file)+" "
		+filename2path(source_result_by_whole_file);
	system(cmdstr.c_str());

	cout<<"/************************des_by_whole*********************************************/"<<endl;
	cmdstr="svm_classify "+filename2path(des_test_data_file)+" "
		+filename2path(whole_model_file)+" "
		+filename2path(des_result_by_whole_file);
	system(cmdstr.c_str());

	cout<<"/************************des_by_des*********************************************/"<<endl;
	cmdstr="svm_classify "+filename2path(des_test_data_file)+" "
		+filename2path(des_model_file)+" "
		+filename2path(des_result_by_des_file);
	system(cmdstr.c_str());

	cout<<"/************************test_by_whole*********************************************/"<<endl;
	cmdstr="svm_classify "+filename2path(test_test_data_file)+" "
		+filename2path(whole_model_file)+" "
		+filename2path(test_result_by_whole_file);
	system(cmdstr.c_str());

	//if(i%10==0) copy test_result_by_whole_file test_detail
	//下一步修改
	if (curT%10==1 || curT<11)
	{
		stringstream ss;
		ss<<svm_pre_tenstep_path<<"\\PTSVM_pre_iter"<<curT;
		string tempstr=ss.str();
		cmdstr="copy "+filename2path(test_result_by_whole_file)+" "+tempstr;
		system(cmdstr.c_str());
	}

	cout<<"/************************test_by_des*******************************************/"<<endl;
	cmdstr="svm_classify "+filename2path(test_test_data_file)+" "
		+filename2path(des_model_file)+" "
		+filename2path(test_result_by_des_file);
	system(cmdstr.c_str());

}


void PTSVM::errCalculate()
{
	ifstream des_result_by_des_instream(filename2path(des_result_by_des_file).c_str());
	ifstream des_result_by_whole_instream(filename2path(des_result_by_whole_file).c_str());

	sum_weight_des=0;
	sum_weight_train=0;
	train_err_by_des=0;
	train_err_by_whole=0;
	des_err_by_des=0;
	des_err_by_whole=0;

	double result;
	for (int i=0; i<train_data_num; i++)
	{
		des_result_by_des_instream>>result;
		if (train_data_label[i]!=res2label(result))
		{
			train_err_by_des+=weight_train[i];
		}
		sum_weight_train+=weight_train[i];
	}

	sum_weight_des+=sum_weight_train;
	des_err_by_des=train_err_by_des;
	for(map<int,int>::iterator ite=test_data_indexs.begin(); ite!=test_data_indexs.end(); ite++)
	{
		des_result_by_des_instream>>result;
		if(ite->second!=res2label(result))
		{
			des_err_by_des+=weight_test;
		}
		sum_weight_des+=weight_test;
	}


	for (int i=0; i<train_data_num; i++)
	{
		des_result_by_whole_instream>>result;
		if (train_data_label[i]!=res2label(result))
		{
			train_err_by_whole+=weight_train[i];
		}
	}

	des_err_by_whole=train_err_by_whole;
	for(map<int,int>::iterator ite=test_data_indexs.begin(); ite!=test_data_indexs.end(); ite++)
	{
		des_result_by_whole_instream>>result;
		if(ite->second!=res2label(result))
		{
			des_err_by_whole+=weight_test;
		}
	}

	des_result_by_des_instream.close();
	des_result_by_whole_instream.close();
}

void PTSVM::reWeightSource()
{

	double dist=exp((des_err_by_des-des_err_by_whole)/sum_weight_des);

	double result;
	ifstream source_result_instream(filename2path(source_result_by_whole_file).c_str());
	for(int i=0;i<source_data_num; i++)
	{
		source_result_instream>>result;
		if (source_data_label[i] != res2label(result))
		{
			weight_source[i] *= Beita;
		}
		weight_source[i] *= dist;
	}
	source_result_instream.close();
}

void PTSVM::reWeightTrain()
{
	double e=(train_err_by_whole+1.0)/(sum_weight_train+1.0);

	cout<<"et: "<<e<<endl;
	double beit=e/(1.0-e);

	double result;
	ifstream train_result_instream(filename2path(des_result_by_whole_file).c_str());
	for (int i=0;i<train_data_num; i++)
	{
		train_result_instream>>result;
		if(train_data_label[i] != res2label(result))
		{
			/*
			double bt=e/(1.0-e);
			if(bt<1) bt=1.0;
			weight_train[i] *=bt;
			*/
			//double temp=exp(0.0-(double)train_data_num/iter_N);
			//weight_train[i]*=(1.0+temp)/(1.0-temp);
			//weight_train[i] /=1.0-e;
			//weight_train[i] *=exp(1-e);

			weight_train[i] /=beit;

		}
	}
	train_result_instream.close();
}

void PTSVM::reWeightTest(int curT)
{
	//weight_test=1;
	weight_test=weight_test_max*curT/iter_N*(1-train_err_by_whole/sum_weight_train);

	//测试语料结果记录
	double e=(train_err_by_whole+1.0)/(sum_weight_train+1.0);
	double beit=e/(1.0-e);
	double restemp;

	ifstream RestestIn(filename2path(test_result_by_whole_file).c_str());

	int resindex=0;
	while(RestestIn>>restemp)
	{
		double rtemp=log(1/beit);
		fresright[resindex][curT-1]=0.5*rtemp;
		if(restemp>0)
		{
			fresleft[resindex][curT-1]=rtemp;
		}
		resindex++;
	}
	RestestIn.close();

}

void PTSVM::adjustTestData()
{
	int maxPos=semi_add_M/2;
	int maxNeg=maxPos;

	AltNode altini=AltNode(-1,0);
	for (int i=0; i<maxPos;i++)
	{
		altposHeap[i]=altini;
		altnegHeap[i]=altini;
	}

	int index=0;
	double result;
	ifstream test_result_instream(filename2path(test_result_by_whole_file).c_str());
	while(test_result_instream>>result)
	{
		map<int,int>::iterator map_ite=test_data_indexs.find(index);
		if(map_ite!=test_data_indexs.end())
		{
			if(res2label(result)!=map_ite->second || abs(result)<0.5 )
			{
				test_data_indexs.erase(map_ite);
			}
			else
			{
				index++;
				continue;
			}
		}
		
		if( abs(result)<1.0   &&  abs(result) > 0.5  )
		{
			AltNode inode=AltNode(index,result);
			if(1==res2label(result))
			{
				insertToAlt(altposHeap,maxPos,1,inode);
			}
			else
			{
				insertToAlt(altnegHeap,maxNeg,-1,inode);
			}
		}
		index++;
	}

	test_result_instream.close();

	int posNum=0;
	int negNum=0;
	for (int i=0; i<maxPos;i++)
	{
		if(-1!=altposHeap[i].index)
		{
			posNum++;
		}
		if(-1!=altnegHeap[i].index)
		{
			negNum++;
		}
	}
	if(negNum>posNum) negNum=posNum;
	if(posNum>negNum) posNum=negNum;


	for (int i=maxPos-1;  i>=0; i--)
	{
		if( posNum>0 && -1!=altposHeap[i].index)
		{
			posNum--;
			test_data_indexs[altposHeap[i].index]=1;
		}
		if(negNum>0 && -1!=altnegHeap[i].index)
		{
			negNum--;
			test_data_indexs[altnegHeap[i].index]=-1;
		}
	}

}

int PTSVM::res2label(double res)
{
	if (res > 0)
	{
		return 1;
	}
	return -1;
}


void PTSVM::insertToAlt(AltNode* heap, int heap_size,int label, AltNode node)
{
	if(heap[0].less(node,label))
	{
		heap[0]=node;
	}
	Heapify(heap,heap_size,0,label);
}

void PTSVM::Heapify(AltNode* heap, int heap_size, int cur, int label)
{
	int lchild=2*cur+1;
	int rchild=2*cur+2;
	int min=cur;
	if(lchild<heap_size && heap[lchild].less(heap[min], label) )
	{
		min=lchild;
	}
	if (rchild<heap_size && heap[rchild].less(heap[min],label) )
	{
		min=rchild;
	}
	if(min!=cur)
	{
		AltNode temp=heap[cur];
		heap[cur]=heap[min];
		heap[min]=temp;
		Heapify(heap,heap_size,min,label);
	}
}

/************************************************************************/
AltNode::AltNode()
{

}
AltNode::AltNode(int index, double value)
{
	this->index=index;
	this->value=value;
}

bool AltNode::less(AltNode anode,int label)
{
	if(1==label)
	{
		if(this->value < anode.value)
		{
			return true;
		}
		return false;
	}

	if(this->value > anode.value)
	{
		return true;
	}
	return false;
}

