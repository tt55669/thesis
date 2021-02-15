#include <iostream> 
#include <math.h>
#include<fstream>
#include <cstdlib> 
#include <ctime>
#include <vector>
using namespace std;
const int nMTCD = 10000;
const int simRAo = 2000; // 1=10ms 20s
const int Backoff = 40; //D2D backoff
const int D2DRBP = 50; //D2D resource block pool
const int beta_a = 3;
const int beta_b = 4;
const int beta_RAo = 1000; // 1=10ms
//const int maxreTimes = 10;
//const int CPrb = 25; Contention resource block pool

//int numMTCDfail = 0;
int RAtime[5][nMTCD] = { {0} }; //0: initiate RA time   1: first RA time    2: success RA time  3: RA retransmission times 4:group index

//int PreStatus[4][simRAo]={0}; //0: number of device initiate RA    1: emtpyPre    2: colliPre    3: successPre

int beta_nMTCD[beta_RAo] = { 0 }; // number of device initiate RA in beta_RAo;

double beta_proability[beta_RAo] = { 0 }; // proability of beta in RA
double beta_proability_ac[beta_RAo] = { 0 }; // ac_proability of beta in RA
//int SuccessnMTCD=0;
class DEVICE
{
public:
	int time;
	int num;
	int group;
};
double RTran = 0;
int D = 0;
int D2Dcounter[19] = { 0 };//�C��groupRA�]�ƭp��   0����  
vector<DEVICE> devTabley[19];
int eachgrouppre[19] = { 0 };	// �C��group��preamble��
int eachgrouppre_return[19] = { 0 };	// �S��preamble�i�H���t �άO �S��group�ݭnpreamble �N�n�^�_�W�@��preamble���t
int eachgrouppre_return_2[19] = { 0 };
int utilizationtpre[simRAo + 1] = { 0 };//�C��RAOpreamble �ϥΪ��p
int request_pre[19] = { 0 };//�C��RAO�C�spramble �ϥΪ��p
int unusepre_times[19] = { 0 };
int groupstatus[19] = { 0 };// 1:�������s 2:preamble�i�H���s���t���s  -1:pre�Q������A��M�S�o�_ra���s
int pbg[55] = { 0 };// preamble belong group��T
int pbg_return[55] = { 0 };
int pbg_return_2[55] = { 0 };
int r = 0;//�i���s�t�m��preamble
int prer = 0;
int dsa = 0;
int z = 0;
int SuccessnMTCDcul[simRAo + 1] = { 0 };
double droprate;
int totaldelay = 0;
int SuccessnMTCD = 0;
int main()
{
	for (int g = 1; g <= 18; g++)// �C��group��preamble�� �w�]�T�� 0����
	{
		eachgrouppre[g] = 3;
		for (int p = g * 3; p >= g * 3 - 2; p--)
		{
			pbg[p] = g;
		}
	}


	fstream file;
	file.open("456.txt", fstream::out);
	if (!file.is_open())
	{
		cout << "�ɮ�����" << endl;
		return 0;
	}
	//��l�� 
		//srand((unsigned)time(NULL));
		///////generate beta distribution
	for (int a = 0; a < beta_RAo - 1; ++a) {
		beta_proability[a + 1] = 60 * pow(a + 1, beta_a - 1)*pow(beta_RAo - a - 1, beta_b - 1) / pow(beta_RAo, beta_a + beta_b - 1);
		beta_proability_ac[a + 1] = beta_proability_ac[a] + beta_proability[a + 1];
	}
	beta_proability_ac[beta_RAo-1] = 1;
	///////generate RAtime in each MTCD
	for (int a = 0; a < nMTCD; ++a) {
		double i = (double)rand() / (RAND_MAX + 1);
		RAtime[4][a] = rand() % 18 + 1;
		/*if (RAtime[4][a] == 19)
		{
			RAtime[4][a] = 1;
		}
		else if (RAtime[4][a] == 20)
		{
			RAtime[4][a] = 2;
		}
		else if (RAtime[4][a] == 21)
		{
			RAtime[4][a] = 3;
		}*/
		for (int b = 0; b < beta_RAo; ++b)
			if (i < beta_proability_ac[b]) {
				RAtime[0][a] = b ;// �����]�Ƶo�_RA time(�|����) +12��D2D Delay
				RAtime[1][a] = b ;//�O���]�ƲĤ@���]�Ƶo�_RA time 
				++beta_nMTCD[b + 1];//�O���C��RAO�o�_���]�Ƽ� 
				break;
			}
	}
	


	/////
	for (int a = 0; a < nMTCD; ++a)
		RAtime[2][a] = -1; //2: success RA time ��l��

   //// �����}�l

	for (int a = 1; a <= simRAo; ++a) {

		if ((a % 5 == 0))
		{
			for (int g = 1; g <= 18; ++g)
				D2Dcounter[g] = 100;  ////�W�L40ms �ܤU�������
		}
		if ((a % 9 == 0))
		{
			for (int g = 1; g <= 18; ++g)
				D2Dcounter[g] = 0;
		}	

		for (int g = 1; g <= 18; ++g) {
			for (int b = 0; b < nMTCD; ++b) {
				if (RAtime[0][b] == a && RAtime[4][b] == g && (D2Dcounter[g] < D2DRBP ))//0: initiate RA time   1: first RA time    2: success RA time  3: RA retransmission times 4:group index
				{
					D2Dcounter[g]++;//�p��D2D request���\���]�� 
					DEVICE dev;
					dev.time = a;
					dev.num = b;
					dev.group = g;
					devTabley[g].push_back(dev);
					D++;
				}
				else if (RAtime[0][b] == a && RAtime[4][b] == g && (D2Dcounter[g] >= D2DRBP ))
				{
					//cout << D2Dcounter[g] << endl;
					//file<<"�]�Ƹ�"<<b<<" "<<"�o�_�ɶ�"<<a<<" "<<"group"<<g<<" ";
					RAtime[0][b] = RAtime[0][b] + (Backoff / 10);//12��RAO D2D delay + backoff�ɶ� 
					//if (RAtime[0][b] >= 1100) { cout << a << endl; }
					RAtime[3][b]++;//����++ 
					//file<<"���ǴX��"<< RAtime[3][b]<<endl; 	
				}
			}
		}


//		for (int a = 1; a <= 18; a++) {
//
//			for (int ix = 0; ix < devTabley[a].size(); ++ix) {
//				cout << devTabley[0][ix].group << endl;
//			}
//		}


		for (int g = 1; g <= 18; ++g) {
			for (int p = 0; p < eachgrouppre[g]; p++) {
				if (devTabley[g].empty())
				{
					break;
				}
				RAtime[2][devTabley[g][0].num] = a + 5;
				utilizationtpre[a]++;
				request_pre[g]++;
				SuccessnMTCD++;
				devTabley[g].erase(devTabley[g].begin());
			}
		}
		/*
		if (D == 50000) {
			cout << "RAO" << a << endl;
		}*/
		//////�W�����n��

	
/*
		file << "RAO" << a << endl;
		file << "groupstatus ";
		for (int g = 1; g <= 18; ++g) {
			//request_pre[g] = 0;
			file << groupstatus[g] << " ";
			groupstatus[g] = 0;
		}

		file << "eachgrouppre";
		for (int g = 1; g <= 18; ++g) {
			file << eachgrouppre[g] << " ";
		}

		file <<"request_pre";
		int ttttt = 0;
		for (int g = 1; g <= 18; ++g) {
			file << request_pre[g] << " "; //�O���C��RAO �U�spreamble �ϥΪ��p
			ttttt += request_pre[g];
		}
		file << endl;
		file <<"total preuse:"<<ttttt << endl;

	

		///�Ȧs�o��RAO �C��group preamble ��T
		for (int g = 1; g <= 18; g++)
		{
			eachgrouppre_return[g] = eachgrouppre[g];
		}
		///�Ȧs�o��RAO rellocate preamble ��T
		int prer = r;
		///�Ȧs�o��RAO preamble belong group ��T
		for (int r = 1; r <= 54; r++)
		{
			pbg_return[r] = pbg[r];
		}

		///////////////�M�w�U�@��RAOpreable���t

		///�P�_�C��group preamlbe�ϥΪ��p
		int m = 0;//���ݨD��group
		
		for (int g = 1; g <= 18; ++g) {  //1:�������s 2 : preamble�i�H���s���t���s - 1 : pre�Q������A��M�S�o�_ra���s //0����
			if ((request_pre[g] == eachgrouppre[g]) && (eachgrouppre[g] > 1)) {
				groupstatus[g] = 1;
				m++;
			}
			else if (request_pre[g] == 0 && eachgrouppre[g] > 1 && unusepre_times[g] < 2) {
				unusepre_times[g]++;
				
			}
			else if (request_pre[g] == 0 && eachgrouppre[g] > 1 && unusepre_times[g] == 2) {
				groupstatus[g] = 2;
				unusepre_times[g] = 0;
			}
			else if (request_pre[g] == 1 && eachgrouppre[g] == 1)
			{
				groupstatus[g] = -1;
			}
		}

		int havereturn = 0;
		/// �k�ٳQ�ɨ���preamble
		for (int g = 1; g <= 18; ++g) {
			if (groupstatus[g] == -1){
				for (int rt = g * 3; rt >= g * 3 - 2 ; rt--)
				{
					eachgrouppre[pbg[rt]]--;
					pbg[rt] = g;
				}
				eachgrouppre[g] = 3;
				havereturn = 1;
			}
		}

		///�Ȧs�o��RAO �Q�k��group preamble ��T
		for (int g = 1; g <= 18; g++)
		{
			eachgrouppre_return_2[g] = eachgrouppre[g];
		}

		///�Ȧs�o��RAO �Q�k��preamble belong group ��T
		for (int r = 1; r <= 54; r++)
		{
			pbg_return_2[r] = pbg[r];
		}

		///��X�i�H�Q���s�t�m��preamble
		for (int g = 1; g <= 18; ++g) {
			if (groupstatus[g] == 2){
				r += eachgrouppre[g] - 1;
				eachgrouppre[g] = 1;
				for (int r = 1; r <= 54; r++)
				{
					if (pbg[r] == g) { pbg[r] = 0; }
				}
				pbg[g * 3 - 2] = g;
			}
		}

		

		///�p��i�Hrellocate preamble
		for (int r = 1; r <= 54; r++)
		{
			if (pbg[r] == 0) { r++; }
		}
		///���s���tpreamble
		if ((m == 0) && (r > 0) && havereturn==1) {
			cout << "RAO" << a << endl;
			for (int g = 1; g <= 18; g++)
			{
				eachgrouppre[g]=eachgrouppre_return_2[g];
			}

			
			for (int r = 1; r <= 54; r++)
			{
				pbg[r]= pbg_return_2[r];
			}
		}
		 else if ((m == 0 || r == 0)) {
			for (int g = 1; g <= 18; ++g) {
				eachgrouppre[g] = eachgrouppre_return[g];
			}
			r = prer;
			for (int r = 1; r <= 54; r++)
			{
				pbg[r] = pbg_return[r];
			}
		}
		else if (r < m ) {
			for (int g = 1; g <= 18; ++g) {
				if (groupstatus[g]==1) {
					if (r > 0) {
						eachgrouppre[g] ++;
						r--;
						for (int r = 1; r <= 54; r++){
							if (pbg[r] == 0) { 
								pbg[r] = g;
								break;
							}
						}
					}
				}
			}
		}
		else if (r >= m ){
			for (int g = 1; g <= 18; ++g) {
				if (groupstatus[g] == 1) {
					for (int t = 1; t <= (r / m); t++) {
						for (int r = 1; r <= 54; r++){
							if (pbg[r] == 0) { 
								pbg[r] = g;
								break;
							}
						}
					}
					eachgrouppre[g] += (r / m);
					r -= (r / m);
				}
			}
			for (int g = 1; g <= 18; ++g) {
				if (r > 0) {
					eachgrouppre[g] ++;
					r--;
					for (int r = 1; r <= 54; r++){
						if (pbg[r] == 0) { 
							pbg[r] = g;
							break;
						}
					}
				}
			}
		}
		
		
	
		
		
		////�d��PREAMBLE ���S���W�L54��
		
		int tpre = 0;
		for (int g = 1; g <= 18; ++g) {
			tpre += eachgrouppre[g];
		}
		if (tpre > 54 || tpre < 54)
		{
			file << "RAO" << a << endl ;

			file << "request" <<  m  << "    " << "rellocate" << r <<"havereturn:"<<havereturn<<endl;

			file << "total preamble: " << tpre << endl;
			file << "groupstatus          ";
			for (int g = 1; g <= 18; ++g) {
				file << groupstatus[g]<<" ";
			}
			file << endl;
			
			
			file << "eachgrouppre_return: ";
			for (int g = 1; g <= 18; ++g) {
				file << eachgrouppre_return[g] << " ";
			}
			file << endl;
			file << "request             :";
			for (int g = 1; g <= 18; ++g) {
				file << request_pre[g] << " ";
			}
			file << endl;
			file << "eachgrouppre       : ";
			for (int g = 1; g <= 18; ++g) {
				file << eachgrouppre[g] << " ";
			}
			file << endl;
		}
		
	
		
		for (int g = 1; g <= 18; ++g) {
				request_pre[g] = 0;
				//groupstatus[g] = 0;
			}

		m = 0;
		r = 0;
		havereturn = 0;
		*/
		if ((SuccessnMTCD) == nMTCD) {
			cout << "Complete Time:" << static_cast<double>(a) / 100 << "s" << endl;
			break;
		}
	}//��������

	for (int i = 0; i < nMTCD; i++)
	{
		if ((RAtime[2][i] != -1))//���Ѳv
		{
			totaldelay += RAtime[2][i] - RAtime[1][i];
		}
	}

	cout << "Average Access Delay:" << double(totaldelay) / double(SuccessnMTCD) / 100 << endl;//���H100�M������
	
	for (int i = 0; i <= simRAo; i ++) {
		file << utilizationtpre[i] << endl; //�C��RAO preamble�ϥέӼ�
	}
	
	fstream file3;
	file3.open(" SuccessnMTCDcul.txt", fstream::out);
	if (!file3.is_open())
	{
		cout << "�ɮ�����" << endl;
		return 0;
	}
	for (int i = 1; i <= simRAo; i++) {

		SuccessnMTCDcul[i] = SuccessnMTCDcul[i - 1] + utilizationtpre[i]; //�W�@��+�ϥΪ�
		file3 << SuccessnMTCDcul[i] << endl;
	}
	

	/*for (int a = 1; a <= 18; a++) {

		dsa += devTabley[a].size();

	}
	cout << dsa << endl;*/

	/*for (int ix = 0; ix < devTabley[18].size(); ++ix){
		if (devTabley[18][ix].group == 18) {
			z++;
		}
			cout << devTabley[1][ix].group << endl;
	}
	cout << z << endl;*/
	fstream file4;
	file4.open(" BETAclu.txt", fstream::out);
	if (!file4.is_open())
	{
		cout << "�ɮ�����" << endl;
		return 0;
	}
	

	for (int i = 1; i < beta_RAo; i++) {

		file4 << beta_nMTCD[i] << endl;
	}




	file.close();
	file3.close();
	file4.close();
	cout << D << endl;
	system("pause");
	return 0;
}
