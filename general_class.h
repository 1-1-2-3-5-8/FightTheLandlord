#pragma once
#include <iostream>
#include <vector>
#include <string>
using namespace std;

string turn_back(int);//�����������ת��Ϊ��ʾ��С�ı�ţ�0~14��
int turn(const char&);//���Ʊ��ת��������

class card_in_hand
{
public:
	bool ending()
	{
		for (int i = 0; i < 15; i++)
		{
			if (card[i] > 0)
				return 0;
		}
		return 1;
	}
	card_in_hand(const char* s)
	{
		for (int i = 0; i <= strlen(s); i++)
			name[i] = s[i];
		memset(card, 0, sizeof(card));
	}
	void print_name()
	{
		printf("%s", name);
	}
	void get_card(int ord)//ץ��
	{
		if (ord < 52)
			card[ord % 13]++;//�Ǵ�С��
		else
			card[ord - 52 + 13]++;//��С��
	}
	void off_card(const vector<int>& out)
	{
		for (int i = 0; i < out.size(); i++)
			card[out[i]]--;
	}
	int get_time(int type)
	{
		return card[type];
	}
	friend ostream& operator <<(ostream& os, const card_in_hand& player)//�������
	{
		for (int i = 0; i < 15; i++)
		{
			for (int j = 1; j <= player.card[i]; j++)
				os << turn_back(i) << " ";
		}
		os << endl;
		return os;
	}
private:
	char name[10];
	int card[15];
	friend class ai;
};

class out_card
{
public:
	out_card() :Type(1), Level(-1) {}
	out_card(const vector<int>& _out, const int& _type, const int& _level)
		:out(_out), Type(_type), Level(_level) {}
	int type() const { return Type; }
	int level() const { return Level; }
	friend ostream& operator <<(ostream& os, const out_card& _o)
	{
		if (_o.out.empty())
			printf("Pass\n");
		else
		{
			for (int i = 0; i < _o.out.size(); i++)
				os << turn_back(_o.out[i]) << " ";
			os << endl;
		}
		return os;
	}
private:
	vector<int> out;
	int Type;//����
	int Level;//�Ƶĵȼ������ڱȽ�ͬ�����ƵĴ�С
};


/*===================AI���������===================*/


struct card_type
{
	int repeat;//����
	int max;//�����
	int join;//������
	int carry;//�������ࣨ���Ż������Ż��ǲ�����
	card_type() {}
	card_type(int _r, int _m, int _j, int _c) :repeat(_r), max(_m), join(_j), carry(_c) {}
	card_type(const out_card&);
	//������֮ǰ���������
	friend bool cmp_for_comb(const card_type& a, const card_type& b);
	//���ֳ���ǰ����
	friend bool cmp_for_out(const card_type&, const card_type&);
	//����д��������ʲô���ˣ���������˵��
	friend bool cmp_1(const card_type& a, const card_type& b)
	{
		//�������Ȱ��������ٵ����ٰ���С��С����
		if (a.join == 1 && b.join == 1)
			return a.repeat < b.repeat || (a.repeat == b.repeat&&a.max < b.max);
		else
		{
			//�����ȿ����������ٵ���
			if (a.repeat < b.repeat)
				return 1;
			if (a.repeat > b.repeat)
				return 0;
			//������ȿ��������Ӷൽ��
			if (a.join > b.join)
				return 1;
			if (a.join < b.join)
				return 0;
			//��������Ȱ��Ƶĵȼ�������Ʊ�ţ���С����
			return a.max < b.max;
		}
	}
};

struct oneposs_spare//һ�ֲ��Ʒ���
{
	vector<card_type> met;//������Ʒ���
	int out_time;//����
	oneposs_spare() :out_time(0) {}
	void combine(const card_type&);
	friend bool operator <(const oneposs_spare& a, const oneposs_spare& b)
	{
		return a.out_time < b.out_time;//���������֮��������
	}
};

class ai
{
	card_in_hand hand_card;//����
	vector<oneposs_spare> result;//���ƽ��
	int myself;//�Լ���˭
	int last;//��һ������˭����
public:
	ai(const card_in_hand& _cd, int _m, int p_t) :hand_card(_cd), myself(_m)
	{
		if (p_t >= 2)
			last = -1;
		else
			last = (myself + 2 - p_t) % 3;
	}
	void spare();//���Ʋ���
	void dfs(int, const oneposs_spare&, const vector<int>&);//�ݹ�������
	void dfs_for_nonjoin(int, const oneposs_spare&, const vector<int>&);//����
	vector<int> output(const out_card&);//����Ҫ������
};