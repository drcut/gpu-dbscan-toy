#include <iostream>
#include <omp.h>
#include <fstream>
#include <algorithm>
#include <queue>
#include <set>
#define SIZE_N 1000000
using namespace std;
int num;
unsigned int esp_power=200*200;
unsigned int esp = 200;
int m=30;
int class_num = 0;
struct point
{
	int x[2];
	int id;
}pt[SIZE_N];
unsigned int dis(int l,int r)
{
	int dx=pt[l].x[0]-pt[r].x[0];
	int dy=pt[l].x[1]-pt[r].x[1];
	return dx*dx+dy*dy;
}
int d[SIZE_N];
int root(int pos)
{
	if(d[pos]==pos)return pos;
	d[pos]=root(d[pos]);
	return d[pos];
}
bool compare(point a,point b)
{
	return a.x[0]<b.x[0];
}
bool compare2(point a,point b)
{
	return a.id<b.id;
}
int mark[1000000];
void read_data()
{
	ifstream fin;
	fin.open("data.txt",ios::in);
	fin>>num;
	for(int i = 0;i<num;++i)
	{
		d[i]=i;
		mark[i]=-1;//stand for noisy
		fin>>pt[i].x[0]>>pt[i].x[1];
		pt[i].id=i;
	}
}
omp_lock_t lock[SIZE_N];
int main()
{
	omp_set_lock(lock);
	printf("read data...\n");
	read_data();
	printf("sort point...\n");
	sort(pt,pt+num,compare);
	double start=omp_get_wtime();
	#pragma omp parallel num_threads(8)
	{
		queue<int> q1;
		queue<int> q2;
		int in_queue=0;
		//step1:Local comp
		int tid = omp_get_thread_num();
		int chunk_size = (num/omp_get_num_threads())+1;
		int start = tid*chunk_size;
		int end = start+chunk_size;
		int *neighbour=new int[num];
		printf("tid:%d step1\n",tid);
		for(int i = start;i<num&&i<end;i++)
		{
			int have= 0 ;
			int tmp = i-1;
			int tx = pt[i].x[0];
			while(tmp>-1)
			{
				if(tx-pt[tmp].x[0]>esp)break;
				if(dis(i,tmp)<esp_power)//is neighbour
				{
					neighbour[have++]=tmp;
				}
				tmp--;
			}
			tmp=i+1;
			while(tmp<num)
			{
				if(pt[tmp].x[0]-tx>esp)break;
				if(dis(i,tmp)<esp_power)//is neighbour
				{
					neighbour[have++]=tmp;
				}
				tmp++;
			}
			//if i is a core
			if(have>m)
			{
				mark[pt[i].id]=class_num++;
				int ri=root(i);
				for(int j = 0;j<have;j++)
				{
					int npos = neighbour[j];
					//if neighbour' in local
					if(npos>=start&&npos<=start+chunk_size)
					{
						int rj=root(npos);
						d[rj]=ri;
					}
					else//not in local
					{
						if(i<npos)
						{
							q1.push(i);
							q2.push(npos);
						}
					}
				}
			}
		}
		printf("tid:%d step1 finish\n",tid);
		#pragma omp barrier
		//step2:Merging
		printf("tid:%d step2\n",tid);
		while(!q1.empty())
		{
			int r1=root(q1.front());
			int r2=root(q2.front());
			//#pragma omp atomic
			omp_set_lock(&lock[r1]);
				d[r1]=r2;
			omp_unset_lock(&lock[r1]);
			q1.pop();
			q2.pop();
		}
		printf("tid:%d step2 finish\n",tid);
		#pragma omp barrier
		//step3:Mark the class
		printf("tid:%d step3\n",tid);
		for(int i = start;i<num&&i<start+chunk_size;i++)
			mark[pt[i].id]=mark[pt[root(i)].id];
	}
	double end=omp_get_wtime();
	sort(pt,pt+num,compare2);
	set<int> s;
	ofstream fout;
	fout.open("ans.txt",ios::out);
	for(int i = 0;i<num;i++)
	{
		fout<<pt[i].id<<' '<<mark[pt[i].id]<<endl;
		s.insert(mark[pt[i].id]);
	}
	fout.close();
	set<int>::iterator rit;
	cout<<"find class num:"<<s.size()<<endl;
	cout<<"use time :"<<end-start<<endl;
	
    return 0;
}