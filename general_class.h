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
	int type() { return Type; }
	int level() { return Level; }
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