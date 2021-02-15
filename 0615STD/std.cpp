#include <iostream> 
#include <math.h>
#include<fstream>

using namespace std;

const int nMTCD =10000;
const int simRAo = 2000; // 1=10ms
//const int interRAo = 5; // 10: index 3    5: index 6
//const int cell_r = 1000;
//const int d2d_r = 100;
const int Backoff = 20;
const int beta_a = 3;
const int beta_b = 4;
const int beta_RAo = 1000; // 1=10ms
const int maxreTimes = 20;


//double sd = 0; // standard deviation
int numMTCDfail = 0;
int RAtime[4][nMTCD]={{0}}; //0: initiate RA time   1: first RA time    2: success RA time  3: RA retransmission times
int PreStatus[4][simRAo]={{0}}; //0: number of device initiate RA    1: emtpyPre    2: colliPre    3: successPre
int beta_nMTCD[beta_RAo]={0}; // number of device initiate RA in beta_RAo;
double beta_proability[beta_RAo]={0}; // proability of beta in RA
double beta_proability_ac[beta_RAo]={0}; // ac_proability of beta in RA
int SuccessnMTCD = 0;

double reRAtime=0;
int SuccessnMTCDslot[simRAo] = { 0 };
double droprate;
int totaldelay = 0;
int main()
{
//��l�� 
	//srand((unsigned)time(NULL));
    ////////// generate beta distribution
    for( int a=0; a<beta_RAo-1; ++a ){
        beta_proability[a+1] = 60*pow(a+1,beta_a-1)*pow( beta_RAo-a-1, beta_b-1)/pow(beta_RAo,beta_a+beta_b-1);
        beta_proability_ac[a+1] = beta_proability_ac[a] + beta_proability[a+1];
    }
    beta_proability_ac[beta_RAo-1] = 1;
    ////////// generate RAtime in each MTCD
    for( int a=0; a <nMTCD; ++a ){
        double i = (double)rand()/(RAND_MAX+1);
        for( int b=0; b <beta_RAo; ++b ){
            if( i < beta_proability_ac[b] ){
                RAtime[0][a] = b;
                RAtime[1][a] = b;
                ++beta_nMTCD[b+1];
                break;
            }   
        }
    }
    /////
    for( int a=0; a <nMTCD; ++a )
        RAtime[2][a] = -1;
        
        
        
   //// STD�����}�l
    int ACBP = 1;
    for( int a=0; a <simRAo; ++a ){	

        int eachRAoPre[55]={0};
        int SelectPre[nMTCD]={0};
        for( int b=0; b <nMTCD; ++b ){
            if( RAtime[0][b] == a ){
				//if ((rand() % ACBP) == 0) {
				double i = (double)rand() / (RAND_MAX + 1);//�]�Ʋ��ͪ����v
				if (i <= ACBP){
					++PreStatus[0][a];
					SelectPre[b] = rand() % 54 + 1;
					++eachRAoPre[SelectPre[b]];
				}
				else
					RAtime[0][b] = RAtime[0][b] + (Backoff/10);
					//RAtime[0][b] = RAtime[0][b] + 400;
            }
        }

       

        for (int c = 1; c < 55; ++c){
            if (eachRAoPre[c] == 0){
                ++PreStatus[1][a];
            }
            else if (eachRAoPre[c] == 1){
                ++PreStatus[3][a];
                ++SuccessnMTCD;
                SuccessnMTCDslot[a]++;
                for (int d = 0; d < nMTCD; ++d){
                    if (SelectPre[d] == c)
                        RAtime[2][d] = a + 5; //5�� RAR windows size + Contention resolution timer
                }
            }
            else if (eachRAoPre[c] > 1){
                ++PreStatus[2][a];
                for (int d = 0; d < nMTCD; ++d){
                    if (c == SelectPre[d]){
                        if (RAtime[3][d] < maxreTimes){
                            RAtime[0][d] = RAtime[0][d] + (Backoff / 10);
                            ++RAtime[3][d];
                        }
                        else
                            ++numMTCDfail;
                    }
                }
            }
        }
		if ((SuccessnMTCD + numMTCDfail) == nMTCD) {
			cout << "Complete Time:" << static_cast<double>(a) /100 <<"s"<< endl;
			break;
			}
     
    
    }////模擬結束
    
    for(int i=0;i<nMTCD;i++)
    {
    	if ((RAtime[2][i] != -1))//���Ѳv
    	{
    		totaldelay+=RAtime[2][i]-RAtime[1][i];
		}	
	}

	cout <<"Average Access Delay:"<<double(totaldelay)/double(SuccessnMTCD)/100 << endl;//���H100�M������

	fstream file1;
	file1.open(" stdsuccessdevice.txt", fstream::out);
	if (!file1.is_open())
	{
		cout << "�ɮ�����" << endl;
		return 0;
	}

	int SuccessnMTCDslotclu[simRAo] = { 0 };
	for (int a = 1; a < simRAo; ++a) {
		SuccessnMTCDslotclu[a] = SuccessnMTCDslotclu[a-1] + SuccessnMTCDslot[a];
		file1 << SuccessnMTCDslotclu[a] << endl;
	}
	cout << numMTCDfail+ SuccessnMTCD << endl;
	
	cout << "Drop rate:" << (double(numMTCDfail)/double(nMTCD))*100<< "%" << endl;

	file1.close();
	system("pause");
	return 0;   
}
    
