#include<stdio.h>
#include<string.h>
#include "time.h"
#include <stdlib.h>


#define kallsyms_num_syms 109535 //109535

unsigned long long kallsyms_addresses[kallsyms_num_syms];	//64位
char* kallsyms_addresses_char[kallsyms_num_syms];	//64位


struct diction
{
	char front[17];				//当前的前缀，16位。例如：ffffffff81
	int start;					//该前缀的起始地址。
	struct diction* child[16];	//指针，指向特定下一位的地址,例如[0]表示下一位为0的结构地址，对于ffffffff81也即是前缀为ffffffff810
};





//以char[]为前缀的数组起始地址，用以建树
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


//构建词典树：传入参数带有前缀码的节点a，传出参数，构造完所有子节点的a
int build_node(struct diction* a)
{
	for (int i = 0; i < 16; i++)
	{
		a->child[i] = (struct diction*)malloc(sizeof(struct diction));


		strcpy(a->child[i]->front, a->front);
		char tmp[2] = { 0 };
		if (i <= 9) { tmp[0] = '0' + i; }
		else { tmp[0] = 'a' + i - 10; }

		strcat(a->child[i]->front, tmp);		//确定 前缀码

		a->child[i]->start = find_loc(a->child[i]->front);		//确定start

	}
	return 1;
}

static unsigned long get_symbol_pos(unsigned long long addr)		//原始算法
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

static unsigned long get_symbol_pos_tree(unsigned long long addr, struct diction tree1, struct diction tree2)		//字典树算法
{
	//a判断是fffffff81还是ffffffff82
	int a= (addr & 0x00000000ff000000);

	//位运算确定ffffffff81或ffffffff82后面的第1、2、3位
	int bit1 = (addr & 0x0000000000f00000) >> 20;		//第一位
	int bit2 = (addr & 0x00000000000f0000) >> 16;		//第二位
	int bit3 = (addr & 0x000000000000f000) >> 12;		//第三位
	

	if (a==0x81000000)						//81开头
	{
		int tmp = tree1.child[bit1]->child[bit2]->child[bit3]->start;		//链表确定起始地址

		for (int i = 0; i < 50; i++)		//暴力，可优化
		{
			if (kallsyms_addresses[tmp+i]==addr)
			{
				return (tmp + i);
			}
		}

	}
	else if(a == 0x82000000)				//82开头
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
		return get_symbol_pos(addr);			//小部分其他的地址的可以用之前的算法
	}
	return 0;
}


static unsigned long get_symbol_pos1(unsigned long long addr)	//差分算法
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

	for (int i = 0; i < kallsyms_num_syms; i++)		//读入文件，创建一颗char和int类型的原始数据数组
	{
		fgets(msg, 100, stream);
		strncpy(add, msg, 16);
		add[16] = '\0';
		kallsyms_addresses_char[i]= (char*)malloc(sizeof(char)*16) ;	//分配，然后读入char

		strncpy(kallsyms_addresses_char[i], msg, 16);		//char类型原始数据成功
		
		tmp = strtoull(add, 0, 16);
		kallsyms_addresses[i] = tmp;						//int类型原始数据成功
	}

	//构建词典树：传入参数带有前缀码的节点a，传出参数，构造完所有子节点的a
	//1.找到前缀为ffffffff81(下一位为1-e的)放入第一层界点1
	struct diction a,b;		//其中一颗子树：81
	a.start = find_loc("ffffffff81");
	strcpy(a.front, "ffffffff81");

	//1.找到前缀为ffffffff82(下一位为1-e的)放入第一层界点1
	b.start = find_loc("ffffffff82");	//第二颗子树，根节点：82
	strcpy(b.front, "ffffffff82");

	//构建第一层
	build_node(&a);					//建一层ffffffff81...（1-16）
	build_node(&b);					//建一层ffffffff82...（1-16）

	//构建之后的层数
	for (int i = 0; i < 16; i++)	//对于根节点的每一个分支，调用build
	{
		build_node(a.child[i]);		//二层ffffffff81k...(1-16)
		for (int j = 0; j < 16; j++)	//三层
		{
			build_node(a.child[i]->child[j]);
			/*for (int k = 0; k < 16; k++)	//4层
			{
				build_node(a.child[i]->child[j]->child[k]);
			}*/
		}
	}
	
	for (int i = 0; i < 16; i++)
	{
		build_node(b.child[i]);		//二层ffffffff82k...(1-16)
		for (int j = 0; j < 16; j++)	//三层
		{
			build_node(b.child[i]->child[j]);
			/*for (int k = 0; k < 16; k++)	//4层
			{
				build_node(b.child[i]->child[j]->child[k]);
			}*/
		}
	}
	
	printf("建树完成");



	//创建随机数，模拟随机访问
	unsigned long long input;
	clock_t start1, finish1,start2,finish2;
	double  duration1, duration2;
	int random_a[50000];
	unsigned long long random_b[50000];

	///*
	for (int i = 0; i < 50000; i++) {
		//求[1-100]之间的随机数
		printf("生成随机数：%d		", (random_a[i]=rand() % (109534 + 1)));
		printf("随机数对应的测试地址：%llu\n", random_b[i] = kallsyms_addresses[random_a[i]]);
	}
	//*/

	/* 其中一种测试方式为单点测试，测试正确性*/
	/*
	while (1)
	{
		input = 0;
		printf("输入你想要查询的地址，0退出:\n");
		scanf("%llx", &input);
		if (input == 0) break;
		else {
			start = clock();
			

			
			printf("普通查询开始\n");
			for (int i = 0; i < 10000; i++)
			{
				printf("地址为: %lu \n", get_symbol_pos(input));
			}
			

			
			printf("优化查询开始\n");
			for (int i = 0; i < 50000; i++)
			{
				printf("地址为: %lu \n", get_symbol_pos_tree(input, a,b));
			}
			
		
			finish = clock();
			duration = (double)(finish - start);
			printf("time:%f\n", duration);

		}

	}
	*/


	///*
	start1 = clock();
	//另一种方式为循环测试时间，测试50000个随机访问速度
	for (int i=0;i<50000;i++)
	{
		get_symbol_pos(random_b[i]);		//这里测试不同算法
	}
	finish1 = clock();
	duration1 = (double)(finish1 - start1);
	printf("优化前时间:%f\n", duration1);

	start2 = clock();
	//另一种方式为循环测试时间，测试50000个随机访问速度
	for (int i = 0; i < 50000; i++)
	{
		get_symbol_pos_tree(random_b[i], a, b);		//这里测试不同算法
	}
	finish2 = clock();
	duration2 = (double)(finish2 - start2);
	printf("优化后时间:%f\n", duration2);


	printf("优化率：%f %%", 100 * (duration1 - duration2) / duration1);

	//*/

	fclose(stream);
	return(0);
	
}