#include <iostream> 
#include <math.h>
#include<fstream>
#include <cstdlib> 
#include <ctime>
#include <vector>
using namespace std;
const int nMTCD = 50000;
const int simRAo = 2000; // 1=10ms 20s
const int Backoff = 40; //D2D backoff
const int D2DRBP = 50; //D2D resource block pool 25/50
const int beta_a = 3;
const int beta_b = 4;
const int beta_RAo = 1000; // 1=10ms
int RAtime[6][nMTCD] = {{0}}; //0: initiate RA time   1: first RA time    2: success RA time  3: RA retransmission times 4:group index 5:Access Delay

int beta_nMTCD[beta_RAo] = { 0 }; // number of device initiate RA in beta_RAo;
double beta_proability[beta_RAo] = { 0 }; // proability of beta in RA
double beta_proability_ac[beta_RAo] = {0}; // ac_proability of beta in RA
class DEVICE //記錄設備資訊
{
public:
	int time;
	int num;
	int group;
};

//double RTran = 0;
int D = 0;
int D2Dcounter[19] = { 0 };//每個groupRA設備計數   0不用  
vector<DEVICE> devTabley[19];
int eachgrouppre[19] = { 0 };	// 每個groupRA設備計數   0不用
int eachgrouppre_return[19] = { 0 };	// recover前 沒有preamble可以分配 或是 沒有group需要preamble 就要回復上一次preamble分配
int eachgrouppre_return_2[19] = { 0 };  //  recover後 沒有preamble可以分配 或是 沒有group需要preamble 就要回復上一次preamble分配
int utilizationtpre[simRAo + 1] = { 0 };//每個RAOpreamble 使用狀況
int request_pre[19] = { 0 };//每個RAOpreamble 使用狀況
int unusepre_times[19] = { 0 };
int groupstatus[19] = { 0 };// 1:滿載的群 2:preamble可以重新分配的群  -1:pre被拿走後，突然又發起ra的群
int pbg[55] = { 0 };// preamble belong group資訊
int pbg_return[55] = { 0 };
int pbg_return_2[55] = { 0 };
int r = 0;//可重新配置的preamble數量
int prer = 0;  //暫存recover前的可重新配置的preamble數量
int D2Dtime[simRAo + 1] = { 0 };//每次RAO可以發起D2D的數量
int D2Dtimecul[simRAo + 1] = { 0 };//每次RAO可以發起D2D的數量
int SuccessnMTCDcul[simRAo + 1] = { 0 };//成功設備累積
double droprate;
int totaldelay = 0; //成功設備delay時間加總
int SuccessnMTCD = 0;//成功設備加總
int AccessDelay[simRAo + 1] = { 0 };//每個dealy時間累積完成的設備
int total_prb = 0;
int average_prb = 0;
int main()
{
	for (int g = 1; g <= 18; g++)// 每個group的preamble數 預設三個 0不用
	{
		eachgrouppre[g] = 3;
		for (int p = g * 3; p >= g * 3 - 2; p--)
		{
			pbg[p] = g;
		}
	}


	fstream file;
	file.open("status.txt", fstream::out);
	if (!file.is_open())
	{
		cout << "close file" << endl;
		return 0;
	}
	fstream file7;
	file7.open("preamble status.csv", fstream::out);
	if (!file7.is_open())
	{
		cout << "close file" << endl;
		return 0;
	}
	fstream file8;
	
	file8.open(" D2D queue.txt", fstream::out);
	if (!file8.is_open())
	{
		cout << "close file" << endl;
		return 0;
	}




	//初始化 
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
		RAtime[4][a] = rand() % 21 + 1;
		if (RAtime[4][a] == 19)
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
		}
		for (int b = 0; b < beta_RAo; ++b)
			if (i < beta_proability_ac[b]) {
				RAtime[0][a] = b ;// 紀錄設備發起RA time(會改變) 
				RAtime[1][a] = b ;//記錄設備第一次設備發起RA time  
				++beta_nMTCD[b + 1];//記錄設備第一次設備發起RA time  
				break;
			}
	}


	/////
	for (int a = 0; a < nMTCD; ++a)
		RAtime[2][a] = -1; //2: success RA time 初始化


     //// 模擬開始
	//int ppap = 0;
	for (int a = 1; a <= simRAo; ++a) {

		if ((a % 5 == 0))
		{
			for (int g = 1; g <= 18; ++g)
				D2Dcounter[g] = 100; ////在下載40ms 變下載不能傳
		}
		if ((a % 9 == 0))
		{
			for (int g = 1; g <= 18; ++g) ///下載之後又變上傳   歸零
				D2Dcounter[g] = 0;
		}
		
		for (int g = 1; g <= 18; ++g) {
			for (int b = 0; b < nMTCD; ++b) {
				if (RAtime[0][b] == a && RAtime[4][b] == g && (D2Dcounter[g] < D2DRBP))//0: initiate RA time   1: first RA time    2: success RA time  3: RA retransmission times 4:group index
				{
					D2Dcounter[g]++;//計算D2D request成功的設備  

					DEVICE dev;
					dev.time = a;
					dev.num = b;
					dev.group = g;
					devTabley[g].push_back(dev);
					D++;
				}
				else if (RAtime[0][b] == a && RAtime[4][b] == g && (D2Dcounter[g] >= D2DRBP))
				{
					//file<<"設備號"<<b<<" "<<"發起時間"<<a<<" "<<"group"<<g<<" ";
					RAtime[0][b] = RAtime[0][b] +(Backoff / 10);//12個RAO D2D delay + backoff時間  
					//if (RAtime[0][b] > 1100) { cout << a << endl; }
					RAtime[3][b]++;//�؂�++ 
					//file<<"重傳幾次"<< RAtime[3][b]<<endl;  	
				}
			}
		}

		
		for (int g = 1; g <= 18; ++g) {
			for (int p = 0; p < eachgrouppre[g]; p++) {
				if (devTabley[g].empty())
				{break;}
				RAtime[2][devTabley[g][0].num] = a + 5;
				utilizationtpre[a]++;
				request_pre[g]++;
				SuccessnMTCD++;
				devTabley[g].erase(devTabley[g].begin());
			}
		}
		

		int size = 0;
		for (int a = 1; a <= 18; a++) { //紀錄個RAO處理RA後 剩下在D2D DQ內的總設備數
			size += devTabley[a].size();
		}
		file8 << size << endl;

		//////上面不要動

	   //印出每個RAO狀況
		
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
			file << request_pre[g] << " "; //記錄每次RAO 各群preamble 使用狀況
			ttttt += request_pre[g];
		}
		file << endl;
		file <<"total preuse:"<<ttttt << endl << endl;

		file << "devTableysize";
		for (int g = 1; g <= 18; ++g) {
			file << devTabley[g].size() << " ";
		}
		file << endl << endl;
		
	

		///暫存這次RAO rellocate preamble 資訊
		for (int g = 1; g <= 18; g++)
		{
			eachgrouppre_return[g] = eachgrouppre[g];
		}
		///暫存這次RAO rellocate preamble 資訊
		int prer = r;
		///暫存這次RAO preamble belong group 資訊
		for (int r = 1; r <= 54; r++)
		{
			pbg_return[r] = pbg[r];
		}

		///////////////決定下一次RAOpreable分配

		///判斷每個group preamlbe使用狀況

		int m = 0;//有需求的group
		for (int g = 1; g <= 18; ++g) {  //1:滿載的群 2 : preamble可以重新分配的群emptyload - 1 : pre被拿走後，突然又發起ra的群 0:不變 3:需要回收一半的preambles
			if (eachgrouppre[g] == 1){
					if(request_pre[g] == 1)
						{groupstatus[g] = -1;}
					else 
						{groupstatus[g] = 6;}
			}
	
		}

		int havereturn = 0;
		/// 歸還被借走的preamble
		for (int g = 1; g <= 18; ++g) {
			if (groupstatus[g] == -1){
				for (int rt = g * 3; rt >= g * 3 - 2 ; rt--)
				{
					eachgrouppre[pbg[rt]]--;//減少被歸還的preamble
					pbg[rt] = g;//改變preamble歸屬
					eachgrouppre[g]++;
				}
				havereturn = 1;
			}
		}


		///暫存這次RAO 被歸還group preamble 資訊
		for (int g = 1; g <= 18; g++)
		{
			eachgrouppre_return_2[g] = eachgrouppre[g];
		}

		///暫存這次RAO 被歸還preamble belong group 資訊
		for (int r = 1; r <= 54; r++)
		{
			pbg_return_2[r] = pbg[r];
		}


		for (int g = 1; g <= 18; ++g) {  //1:滿載的群 2 : preamble可以重新分配的群emptyload - 1 : pre被拿走後，突然又發起ra的群 0:不變 3:需要回收一半的preambles
			if(groupstatus[g]!=-1 && groupstatus[g]!=6){//避免recover完的group又被判斷成別的group狀態
					if(request_pre[g] >= eachgrouppre[g])
					{
						groupstatus[g] = 1;
						m++;
					}
					else if (request_pre[g] == 0)
					{
						groupstatus[g] = 2;
						r += eachgrouppre[g] - 1;//計算可重新分配的preamble
						eachgrouppre[g] = 1;//把group可用preamble變成1(至少保留一個))
						for (int r = 1; r <= 54; r++)
						{
							if (pbg[r] == g) { pbg[r] = 0; }//把重新分配preamble歸屬那個group變為0
						}
						pbg[g * 3 - 2] = g;//剩餘的一個preamble
					}
				//}				
				else if (request_pre[g] < (eachgrouppre[g]/2)){//抓出可以被重新配置的group
					groupstatus[g] = 3;
					r += eachgrouppre[g]/2;//計算可重新分配的preamble
					int count = (eachgrouppre[g] / 2);
					
					for (int r = 1; r <= 54; r++)
					{
						//if (pbg[r] == g && r!=g*3 && r != (g * 3)-1 && r != (g * 3)-2 && count>0)//本身自己的三個預設preamble不會被改道
						if (pbg[r] == g && r != (g * 3)-2 && count>0)//本身自己的一個預設preamble不會被修改
						{ 
							pbg[r] = 0;
							count--;

						}//把重新分配preamble歸屬那個group變為0
					}
					eachgrouppre[g] = eachgrouppre[g] - (eachgrouppre[g] / 2);//把group可用preamble變成一半
				}
			}

		}

		///計算可以rellocate preamble
		for (int r = 1; r <= 54; r++)
		{
			if (pbg[r] == 0) { r++; }
		}

		int recording_r = r;

		int recording_m = m;

		
		///重新分配preamble
		if (((m == 0) || (r == 0)) && havereturn==1) {//havereturn=1 恢復有return後preamble配置
			for (int g = 1; g <= 18; g++)
			{
				eachgrouppre[g]=eachgrouppre_return_2[g];
			}
			
			for (int r = 1; r <= 54; r++)
			{
				pbg[r]= pbg_return_2[r];
			}
		}
		 else if ((m == 0 || r == 0) && havereturn == 0) {//havereturn=0 恢復沒return時preamble配置
			for (int g = 1; g <= 18; ++g) {
				eachgrouppre[g] = eachgrouppre_return[g];
			}
			r = prer;
			for (int r = 1; r <= 54; r++)
			{
				pbg[r] = pbg_return[r];
			}
		}
		else {
			total_prb = 0;
			for (int g = 1; g <= 18; ++g) {	
				if (groupstatus[g] == 1){
					total_prb += eachgrouppre[g];
				}
			}

			total_prb += r;
			average_prb = total_prb / m;
			for (int g = 1; g <= 18; ++g) {	
				if (groupstatus[g] == 1){
					if(eachgrouppre[g] < average_prb){
						int times = (average_prb - eachgrouppre[g]);
						for (int p = 0; p <times ;p++){
							eachgrouppre[g]++;
							r--;
							for (int r = 1; r <= 54; r++) {
								if (pbg[r] == 0) {
									pbg[r] = g;
									break;
								}
							}
						}
					}
					else if(eachgrouppre[g] > average_prb){
						int times = (eachgrouppre[g] - average_prb);
						for (int p = 0; p < times; p++){
							eachgrouppre[g]--;
							r++;

							for (int r = 1; r <= 54; r++) {
								if (pbg[r] == g && r!=g*3-2) {
									pbg[r] = 0;
									break;
								}
							}
						}
					}
				}		
			}
			for (int g = 1; g <= 18; ++g) {
				if (groupstatus[g] == 1){
					if (r > 0) {
						eachgrouppre[g] ++;
						r--;
						for (int r = 1; r <= 54; r++) {
							if (pbg[r] == 0) {
								pbg[r] = g;
								break;
							}
						}
					}
				}
			}
		}	
		////查看PREAMBLE 有沒有超過54個


		int tpre = 0;
		for (int g = 1; g <= 18; ++g) {
			tpre += eachgrouppre[g];
		}
		if (tpre > 54 || tpre < 54)
		{
			file7 << "RAO" <<","<<a << endl ;

			file7 << "request" <<","<< recording_m << "," << "rellocate"<<"," << recording_r << ","<<" havereturn" << "," << havereturn<< endl;

			file7 << "total preamble"<<"," << tpre << endl;
			file7 << "groupstatus" << ",";
			for (int g = 1; g <= 18; ++g) {
				file7 << groupstatus[g]<< ",";
			}
			file7 << endl;

			file7 << "request"<<"," ;
			for (int g = 1; g <= 18; ++g) {
				file7 << request_pre[g] << ",";
			}
			file7 << endl;

			file7 << "eachgrouppre_return" << ",";
			for (int g = 1; g <= 18; ++g) {
				file7 << eachgrouppre_return[g] << ",";
			}
			file7 << endl;

			file7 << "eachgrouppre_return_2" << ",";
			for (int g = 1; g <= 18; ++g) {
				file7 << eachgrouppre_return_2[g] << ",";
			}
			file7 << endl;

			file7 << "eachgrouppre" << ",";
			for (int g = 1; g <= 18; ++g) {
				file7 << eachgrouppre[g] << ",";
			}
			file7 << endl;
			file7 << "   ,";
			for (int kkk = 1; kkk <= 54; kkk++)
			{
				file7 << pbg[kkk] << " ";
				if (kkk % 3 == 0)
				{
					file7 << ",";
				}
			}

			file7 << endl;
			file7 << endl;
			file7 << endl;
		}
		
		for (int g = 1; g <= 18; ++g) {
				request_pre[g] = 0;
				//groupstatus[g] = 0;//模擬結束
			}

		m = 0;
		r = 0;
		havereturn = 0;
		if ((SuccessnMTCD) == nMTCD) {
			cout << "Complete Time:" << static_cast<double>(a) / 100 << "s" << endl;
			break;
		}
		
	}//模擬結束



	fstream file5;
	file5.open(" AccessDelayDU.txt", fstream::out);
	if (!file5.is_open())
	{
		cout << "close file" << endl;
		return 0;
	}

	for (int i = 0; i < nMTCD; i++)
	{
		if ((RAtime[2][i] != -1))//沒備drop掉的設備
		{
			RAtime[5][i] = RAtime[2][i] - RAtime[1][i];//Access Delay時間
			totaldelay += RAtime[2][i] - RAtime[1][i];
		}
	}

	cout << "Average Access Delay:" << double(totaldelay) / double(SuccessnMTCD) / 100 << endl;//除以100專換成秒

	for (int a = 0; a <= simRAo; ++a) {
		for (int b = 0; b < nMTCD; ++b) {
			if (RAtime[5][b] == a) { AccessDelay[a]++; }
		}
	}
	for (int a = 0; a <= simRAo; a++) {
		file5 << AccessDelay[a] << endl; //每個RAO preamble使用個數
	}

	fstream file1;
	file1.open(" utilizationtpre.txt", fstream::out);
	if (!file1.is_open())
	{
		cout << "close file" << endl;
		return 0;
	}
	
	for (int i = 0; i <= simRAo; i ++) {
		file1 << utilizationtpre[i] << endl; //每個RAO preamble使用個數
	}


	fstream file2;
	file2.open(" D2Dtimeclu.txt", fstream::out);
	if (!file2.is_open())
	{
		cout << "close file" << endl;
		return 0;
	}
	fstream file4;
	file4.open(" D2Dtime.txt", fstream::out);
	if (!file4.is_open())
	{
		cout << "close file" << endl;
		return 0;
	}
	
	for (int b = 0; b < nMTCD; ++b) {
		for (int a = 1; a <= simRAo; ++a) {
			if (RAtime[0][b] == a) { D2Dtime[a]++; }
		}
		
	}
	for (int a = 1; a <= simRAo; ++a) {
		D2Dtimecul[a] = D2Dtimecul[a-1] + D2Dtime[a];
		file2 << D2Dtimecul[a] << endl;
		file4 << D2Dtime[a] << endl;

	}

	fstream file3;
	
	file3.open(" SuccessnMTCDcul.txt", fstream::out);
	if (!file3.is_open())
	{
		cout << "close file" << endl;
		return 0;
	}

	for (int i = 1; i <= simRAo; i++) { 

		SuccessnMTCDcul[i]= SuccessnMTCDcul[i-1] +utilizationtpre[i]; ////上一次+使用的

			file3 << SuccessnMTCDcul[i] << endl;

	}
	file.close();
	file1.close();
	file2.close();
	file3.close();
	file4.close();
	file5.close();
	file7.close();
	cout << D << endl;
	system("pause");
	return 0;
}
