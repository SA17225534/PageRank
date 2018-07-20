#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<afxwin.h>  
#include<time.h>

/*
#define FILENAME "Slashdot0811.txt"
#define VEXNUM 77360
#define ARCNUM 905468
*/

/*
#define FILENAME "roadNet-CA.txt"
#define VEXNUM 1971281
#define ARCNUM 5533214
*/

/**/
#define FILENAME "soc-LiveJournal1.txt"
#define VEXNUM 4847571
#define ARCNUM 68993773

int *outDegree;
double *oldPageRank;
double *newPageRank;

typedef struct ListNode
{
	int index;
	struct ListNode *next;
}ListNode, *LNode;
ListNode inEdegs[VEXNUM];
LNode temp;

typedef struct topTen
{
	int index;
	double value;
	struct topTen* next;
}topTen, *tTen;

int containsKey(int v, int index)
{
	LNode p = inEdegs[index].next;
	while (p != NULL)
	{
		if (p->index == v)
			return 1;
		p = p->next;
	}
	return 0;
}
void CreateDN()
{
	outDegree = (int*)malloc(VEXNUM * sizeof(int));
	oldPageRank = (double *)malloc(VEXNUM * sizeof(double));
	newPageRank = (double *)malloc(VEXNUM * sizeof(double));
	temp = (LNode)malloc(sizeof(ListNode)* ARCNUM);
	if (temp == NULL)
		perror("提示：");
	for (int i = 0; i < VEXNUM; i++)
	{
		outDegree[i] = 0;
		oldPageRank[i] = 1.0;
		newPageRank[i] = 0.0;
		inEdegs[i].index = i;
		inEdegs[i].next = NULL;
	}

	int sourceNode, destNode;

	HANDLE hFile = CreateFile(FILENAME, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	//返回值size_high,size_low分别表示文件大小的高32位/低32位  
	DWORD size_low, size_high;
	size_low = GetFileSize(hFile, &size_high);

	//创建文件的内存映射文件。     
	HANDLE hMapFile = CreateFileMapping(hFile, NULL, PAGE_READONLY,	0, 0, NULL);//
	if (hMapFile == INVALID_HANDLE_VALUE)
	{
		printf("Can't create file mapping.Error%d:\n", GetLastError());
		CloseHandle(hFile);
		exit(0);
	}
	SYSTEM_INFO sinf;
	GetSystemInfo(&sinf);
	DWORD dwAllocationGranularity = sinf.dwAllocationGranularity;

	size_low |= (((__int64)size_high) << 32);
	__int64 qwFileOffset = 0;
	// 块大小  
	DWORD dwBlockBytes = 3000 * dwAllocationGranularity;

	DWORD qwFileSize = size_low;
	if (size_low < 3000 * dwAllocationGranularity)
		dwBlockBytes = (DWORD)size_low;
	int k = 0, temp_index = 0;
	char temp_buff[20];
	while (qwFileOffset < qwFileSize)
	{
		if (size_low < 3000 * dwAllocationGranularity)
			dwBlockBytes = (DWORD)size_low;
		//把文件数据映射到进程的地址空间  
		void* pvFile = MapViewOfFile(hMapFile, FILE_MAP_READ, (DWORD)(qwFileOffset >> 32), (DWORD)(qwFileOffset & 0xFFFFFFFF), dwBlockBytes);// 0, 0, 0);
		//printf("%u\n", GetLastError());
		char *p = (char*)pvFile;

		int len = 0, index = temp_index, node[2], n;



		while (1)
		{
			char buf[20];
			if (temp_buff[0] != '\0')
			{
				for (int b = 0; b < 20; b++)
					buf[b] = temp_buff[b];
				temp_buff[0] = '\0';
			}
			sourceNode = -1;
			destNode = -1;
			while (len <= dwBlockBytes - 1 && *(p + len) != '\n')
			{
				buf[index++] = *(p + len);
				len++;
			}
			if (len == dwBlockBytes)
			{
				for (int t = 0; t <= index; t++)
				{
					temp_buff[t] = buf[t];
				}
				temp_index = index;
				break;
			}
			buf[index++] = '\0';
			index = 0;
			char *substr = strtok(buf, "	");
			n = 0;
			while (substr != NULL)
			{
				node[n++] = atoi(substr);
				substr = strtok(NULL, "	");
			}
			len++;


			sourceNode = node[0];
			destNode = node[1];

			outDegree[sourceNode]++;

			temp[k].index = sourceNode;
			temp[k].next = inEdegs[destNode].next;
			inEdegs[destNode].next = &(temp[k]);
			k++;
		}
		qwFileOffset += dwBlockBytes;
		size_low -= dwBlockBytes;
		UnmapViewOfFile(pvFile); //撤销映射  
	}
	CloseHandle(hMapFile);
	CloseHandle(hFile); //关闭文件  
}

int main()
{
	clock_t start = clock();
	CreateDN();
	double temp0, sumdiff;
	//int iteration = 0;
	tTen Top, tp, tq, tr;
	Top = (tTen)malloc(sizeof(topTen));
	Top->next = NULL;
	while (1)
	{

		sumdiff = 0.0;
		LNode p;
		for (int i = 0; i < VEXNUM; i++)
		{
			temp0 = 0.0;
			p = inEdegs[i].next;
			if (p == NULL && outDegree[i] == 0)	//入出度均为0，在第二个数据集中不存在该顶点
				continue;
			while (p != NULL)
			{
				temp0 += oldPageRank[p->index] / outDegree[p->index];
				p = p->next;
			}
			newPageRank[i] = 0.15 + 0.85 * temp0;

			sumdiff += fabs(newPageRank[i] - oldPageRank[i]);

		}
		for (int i = 0; i < VEXNUM; i++)
		{
			oldPageRank[i] = newPageRank[i];
		}

		//printf("第%d轮\n", iteration++);
		if (fabs(sumdiff - 0) < 0.00001)
			break;
	}
	for (int i = 0; i < 10; i++)
	{
		tp = (tTen)malloc(sizeof(topTen));
		tp->index = i;
		tp->value = newPageRank[i];
		//tp->next = NULL;
		tr = Top;
		tq = Top->next;
		while (tq != NULL)
		{
			if (tp->value > tq->value)
				break;
			tr = tq;
			tq = tq->next;
		}
		tp->next = tq;
		tr->next = tp;

	}

	for (int i = 10; i < VEXNUM; i++)
	{
		tr = Top;
		tq = Top->next;
		while (tq != NULL)
		{
			if (newPageRank[i] > tq->value)
				break;
			tr = tq;
			tq = tq->next;
		}
		if (tq != NULL)
		{
			tp = (tTen)malloc(sizeof(topTen));
			tp->index = i;
			tp->value = newPageRank[i];
			tp->next = Top->next;

			tp->next = tq;
			tr->next = tp;
			//删除最后一个结点
			tr = tp;
			while (tq->next != NULL)
			{
				tr = tq;
				tq = tq->next;
			}
			tr->next = NULL;
		}


	}
	tq = Top->next;
	while (tq != NULL)
	{
		printf("%d\t%.3lf\n", tq->index, tq->value);
		tq = tq->next;
	}
	printf("%lf\n", (double)(clock() - start) / CLOCKS_PER_SEC);
	system("pause");
	return 0;
}