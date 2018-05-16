#include <iostream>
#include <vector>
#include "general_class.h"
using namespace std;

/*======================================
û��0
1 ����
2 ����
3 ����
4 ����
5 ��ը
6 ����һ
7 ����һ��
8-19 ˳�ӣ�5��-12�ţ�
20-27 ���ԣ�3��-10�ԣ�
28-32 �ɻ���2-6����
33-36 ��ը��2-5����
37 �Ĵ�����
38 �Ĵ�����
39-42 �ɻ������ţ�2-5����
43-45 �ɻ������ӣ�2-4����
46-48 ��ը�����ţ�2-4����
49&50 ��ը�����ӣ�2|3����
51 ��ը
======================================*/
struct card_type
{
	int repeat;//����
	int max;//�����
	int join;//������
	int carry;//�������ࣨ���Ż������Ż��ǲ�����
	card_type(int _r, int _m, int _j, int _c) :repeat(_r), max(_m), join(_j), carry(_c) {}
	card_type(const out_card&);
};

struct oneposs_spare//һ�ֲ��Ʒ���
{
	vector<card_type> met;//������Ʒ���
	int out_time;//����
	oneposs_spare() :out_time(0) {}
};

class ai
{
	card_in_hand hand_card;//����
	vector<oneposs_spare> result;//���ƽ��
public:
	ai(const card_in_hand& _cd) :hand_card(_cd) {}
	void spare();//���Ʋ���
	out_card output(const out_card&);//����Ҫ������
};

card_type::card_type(const out_card& pre)
{
	if (pre.type == 1)
		repeat = 0;//������Ƶ�����Ϊ0����ʱaiΪ���ֳ��Ʒ�
	else
	{
		if (pre.type == 51)//��ը��Ϊ����������ը��
		{
			repeat = 4;
			max = 14;
			join = 1;
			carry = 0;
		}
		else
		{
			max = pre.level;//level��¼���������
			if (pre.type < 6)//������
			{
				repeat = pre.level - 1;
				join = 1;
				carry = 0;
			}
			else if (pre.type < 8)//����һ��һ��
			{
				repeat = 3;
				join = 1;
				carry = pre.type - 5;
			}
			else if (pre.type <= 19)
			{
				repeat = 1;
				join = pre.type - 3;
				carry = 0;
			}
			else if (pre.type <= 27)
			{
				repeat = 2;
				join = pre.type - 17;
				carry = 0;
			}
			else if (pre.type <= 32)
			{
				repeat = 3;
				join = pre.type - 26;
				carry = 0;
			}
			else if (pre.type <= 36)
			{
				repeat = 4;
				join = pre.type - 31;
				carry = 0;
			}
			else if (pre.type <= 38)
			{
				repeat = 4;
				join = 1;
				carry = pre.type - 36;
			}
			else if (pre.type <= 42)
			{
				repeat = 3;
				join = pre.type - 37;
				carry = 1;
			}
			else if (pre.type <= 45)
			{
				repeat = 3;
				join = pre.type - 41;
				carry = 2;
			}
			else if (pre.type <= 48)
			{
				repeat = 4;
				join = pre.type - 44;
				carry = 1;
			}
			else//if (pre.type <= 50)
			{
				repeat = 4;
				join = pre.type - 47;
				carry = 2;
			}
		}
	}
}

void ai::spare()
{
	int used[15] = {};//��������������Ѿ��ù��˶���
	oneposs_spare res;
	//�Ѵ�С�����
	if (hand_card.card[13] && hand_card.card[14])//����ը
	{
		res.met.push_back(card_type(4, 14, 1, 0));
		res.out_time++;
		used[14] = used[13] = 1;
	}
	else if (hand_card.card[13])
	{
		res.met.push_back(card_type(1, 13, 1, 0));//ֻ��С��
		res.out_time++;
		used[13] = 1;
	}
	else if (hand_card.card[14])
	{
		res.met.push_back(card_type(1, 14, 1, 0));//ֻ�д���
		res.out_time++;
		used[14] = 1;
	}
	//��С2
	if (hand_card.card[12])
	{
		res.met.push_back(card_type(hand_card.card[12], 12, 1, 0));
		res.out_time++;
		used[12] = hand_card.card[12];
	}
	int i;
	//���ը��
	for (i = 0; i < 12; i++)
	{
		if (hand_card.card[i] == 4)
		{
			res.met.push_back(card_type(4, i, 1, 0));
			res.out_time++;
			used[i] = 4;
		}
	}
}

out_card ai::output(const out_card& pre)
{
	card_type pr(pre);//����һ�ηǹ�����ת��Ϊcard_type����
	this->spare();
}