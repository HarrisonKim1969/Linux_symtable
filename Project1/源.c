#include<stdio.h>
#include<string.h>
#include "time.h"
#include <stdlib.h>


#define kallsyms_num_syms 109535 //109535

unsigned long long kallsyms_addresses[kallsyms_num_syms];	//64λ
char* kallsyms_addresses_char[kallsyms_num_syms];	//64λ


struct diction
{
	char front[17];				//��ǰ��ǰ׺��16λ�����磺ffffffff81
	int start;					//��ǰ׺����ʼ��ַ��
	struct diction* child[16];	//ָ�룬ָ���ض���һλ�ĵ�ַ,����[0]��ʾ��һλΪ0�Ľṹ��ַ������ffffffff81Ҳ����ǰ׺Ϊffffffff810
};





//��char[]Ϊǰ׺��������ʼ��ַ�����Խ���
int find_loc(char* front)
{
	int i;
	i = strlen(front);
	for (i = 0; i < kallsyms_num_syms; i++)
	{
		if (strncmp(kallsyms_addresses_char[i], front, strlen(front)) == 0) return i;
	}
	return -1;
}


//�����ʵ����������������ǰ׺��Ľڵ�a�����������������������ӽڵ��a
int build_node(struct diction* a)
{
	for (int i = 0; i < 16; i++)
	{
		a->child[i] = (struct diction*)malloc(sizeof(struct diction));


		strcpy(a->child[i]->front, a->front);
		char tmp[2] = { 0 };
		if (i <= 9) { tmp[0] = '0' + i; }
		else { tmp[0] = 'a' + i - 10; }

		strcat(a->child[i]->front, tmp);		//ȷ�� ǰ׺��

		a->child[i]->start = find_loc(a->child[i]->front);		//ȷ��start

	}
	return 1;
}

static unsigned long get_symbol_pos(unsigned long long addr)		//ԭʼ�㷨
{
	unsigned long symbol_start = 0, symbol_end = 0;
	unsigned long i, low, high, mid;

	low = 0;
	high = kallsyms_num_syms-1;
	int num = 0;


	while (high - low > 1) {
		num++;
		mid = low + (high - low) / 2;
		if (kallsyms_addresses[mid] <= addr)
			low = mid;
		else
			high = mid;
	}
	//printf("%d\n", num);
	return low;
}

static unsigned long get_symbol_pos_tree(unsigned long long addr, struct diction tree1, struct diction tree2)		//�ֵ����㷨
{
	//a�ж���fffffff81����ffffffff82
	int a= (addr & 0x00000000ff000000);

	//λ����ȷ��ffffffff81��ffffffff82����ĵ�1��2��3λ
	int bit1 = (addr & 0x0000000000f00000) >> 20;		//��һλ
	int bit2 = (addr & 0x00000000000f0000) >> 16;		//�ڶ�λ
	int bit3 = (addr & 0x000000000000f000) >> 12;		//����λ
	

	if (a==0x81000000)						//81��ͷ
	{
		int tmp = tree1.child[bit1]->child[bit2]->child[bit3]->start;		//����ȷ����ʼ��ַ

		for (int i = 0; i < 50; i++)		//���������Ż�
		{
			if (kallsyms_addresses[tmp+i]==addr)
			{
				return (tmp + i);
			}
		}

	}
	else if(a == 0x82000000)				//82��ͷ
	{
		int tmp = tree2.child[bit1]->child[bit2]->child[bit3]->start;

		for (int i = 0; i < 50; i++)
		{
			if (kallsyms_addresses[tmp + i] == addr)
			{
				return (tmp + i);
			}
		}
	}
	else {
		return get_symbol_pos(addr);			//С���������ĵ�ַ�Ŀ�����֮ǰ���㷨
	}
	return 0;
}


static unsigned long get_symbol_pos1(unsigned long long addr)	//����㷨
{
	unsigned long symbol_start = 0, symbol_end = 0;
	unsigned long i, low, high, mid;

	low = 0;
	high = kallsyms_num_syms - 1;
	int num = 0;


	while (low <= high) {
		mid = low + (high - low)*(addr -kallsyms_addresses[low]) / (kallsyms_addresses[high]- kallsyms_addresses[low]);
		if (  addr < kallsyms_addresses[mid])
			high = mid - 1;
		else if (  addr > kallsyms_addresses[mid])
			low = mid + 1 ;
		else
			return mid;
	}

	return -1;
}



int main()
{
	FILE* stream;
	/*openafileforupdate*/
	stream = fopen("test.txt", "r");
	char msg[100];
	
	char add[17];
	
	unsigned long long tmp;
	//printf("%d\n\n", sizeof(tmp));	//long 32 long long 64

	for (int i = 0; i < kallsyms_num_syms; i++)		//�����ļ�������һ��char��int���͵�ԭʼ��������
	{
		fgets(msg, 100, stream);
		strncpy(add, msg, 16);
		add[16] = '\0';
		kallsyms_addresses_char[i]= (char*)malloc(sizeof(char)*16) ;	//���䣬Ȼ�����char

		strncpy(kallsyms_addresses_char[i], msg, 16);		//char����ԭʼ���ݳɹ�
		
		tmp = strtoull(add, 0, 16);
		kallsyms_addresses[i] = tmp;						//int����ԭʼ���ݳɹ�
	}

	//�����ʵ����������������ǰ׺��Ľڵ�a�����������������������ӽڵ��a
	//1.�ҵ�ǰ׺Ϊffffffff81(��һλΪ1-e��)�����һ����1
	struct diction a,b;		//����һ��������81
	a.start = find_loc("ffffffff81");
	strcpy(a.front, "ffffffff81");

	//1.�ҵ�ǰ׺Ϊffffffff82(��һλΪ1-e��)�����һ����1
	b.start = find_loc("ffffffff82");	//�ڶ������������ڵ㣺82
	strcpy(b.front, "ffffffff82");

	//������һ��
	build_node(&a);					//��һ��ffffffff81...��1-16��
	build_node(&b);					//��һ��ffffffff82...��1-16��

	//����֮��Ĳ���
	for (int i = 0; i < 16; i++)	//���ڸ��ڵ��ÿһ����֧������build
	{
		build_node(a.child[i]);		//����ffffffff81k...(1-16)
		for (int j = 0; j < 16; j++)	//����
		{
			build_node(a.child[i]->child[j]);
			/*for (int k = 0; k < 16; k++)	//4��
			{
				build_node(a.child[i]->child[j]->child[k]);
			}*/
		}
	}
	
	for (int i = 0; i < 16; i++)
	{
		build_node(b.child[i]);		//����ffffffff82k...(1-16)
		for (int j = 0; j < 16; j++)	//����
		{
			build_node(b.child[i]->child[j]);
			/*for (int k = 0; k < 16; k++)	//4��
			{
				build_node(b.child[i]->child[j]->child[k]);
			}*/
		}
	}
	
	printf("�������");



	//�����������ģ���������
	unsigned long long input;
	clock_t start1, finish1,start2,finish2;
	double  duration1, duration2;
	int random_a[50000];
	unsigned long long random_b[50000];

	///*
	for (int i = 0; i < 50000; i++) {
		//��[1-100]֮��������
		printf("�����������%d		", (random_a[i]=rand() % (109534 + 1)));
		printf("�������Ӧ�Ĳ��Ե�ַ��%llu\n", random_b[i] = kallsyms_addresses[random_a[i]]);
	}
	//*/

	/* ����һ�ֲ��Է�ʽΪ������ԣ�������ȷ��*/
	/*
	while (1)
	{
		input = 0;
		printf("��������Ҫ��ѯ�ĵ�ַ��0�˳�:\n");
		scanf("%llx", &input);
		if (input == 0) break;
		else {
			start = clock();
			

			
			printf("��ͨ��ѯ��ʼ\n");
			for (int i = 0; i < 10000; i++)
			{
				printf("��ַΪ: %lu \n", get_symbol_pos(input));
			}
			

			
			printf("�Ż���ѯ��ʼ\n");
			for (int i = 0; i < 50000; i++)
			{
				printf("��ַΪ: %lu \n", get_symbol_pos_tree(input, a,b));
			}
			
		
			finish = clock();
			duration = (double)(finish - start);
			printf("time:%f\n", duration);

		}

	}
	*/


	///*
	start1 = clock();
	//��һ�ַ�ʽΪѭ������ʱ�䣬����50000����������ٶ�
	for (int i=0;i<50000;i++)
	{
		get_symbol_pos(random_b[i]);		//������Բ�ͬ�㷨
	}
	finish1 = clock();
	duration1 = (double)(finish1 - start1);
	printf("�Ż�ǰʱ��:%f\n", duration1);

	start2 = clock();
	//��һ�ַ�ʽΪѭ������ʱ�䣬����50000����������ٶ�
	for (int i = 0; i < 50000; i++)
	{
		get_symbol_pos_tree(random_b[i], a, b);		//������Բ�ͬ�㷨
	}
	finish2 = clock();
	duration2 = (double)(finish2 - start2);
	printf("�Ż���ʱ��:%f\n", duration2);


	printf("�Ż��ʣ�%f %%", 100 * (duration1 - duration2) / duration1);

	//*/

	fclose(stream);
	return(0);
	
}