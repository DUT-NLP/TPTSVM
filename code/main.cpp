#include "PTSVM.h"
int main(int argc, char* argv[])
{
	PTSVM trasvm=PTSVM();
	trasvm.input(argv[1],argv[2],argv[3],argv[4],101,20);	
	trasvm.learn();
	trasvm.test();
	//system("PAUSE");
}
