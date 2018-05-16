#include <iostream>
#include <vector>
#include "general_class.h"
using namespace std;

/*======================================
没有0
1 过牌
2 单张
3 对子
4 三个
5 纯炸
6 三带一
7 三带一对
8-19 顺子（5张-12张）
20-27 联对（3对-10对）
28-32 飞机（2-6连）
33-36 连炸（2-5连）
37 四带单张
38 四带对子
39-42 飞机带单张（2-5连）
43-45 飞机带对子（2-4连）
46-48 连炸带单张（2-4连）
49&50 连炸带对子（2|3连）
51 王炸
======================================*/
struct card_type
{
	int repeat;//重数
	int max;//最大牌
	int join;//连牌数
	int carry;//带牌种类（单张还是两张还是不带）
	card_type(int _r, int _m, int _j, int _c) :repeat(_r), max(_m), join(_j), carry(_c) {}
	card_type(const out_card&);
};

struct oneposs_spare//一种拆牌方法
{
	vector<card_type> met;//具体拆牌方法
	int out_time;//手数
	oneposs_spare() :out_time(0) {}
};

class ai
{
	card_in_hand hand_card;//手牌
	vector<oneposs_spare> result;//拆牌结果
public:
	ai(const card_in_hand& _cd) :hand_card(_cd) {}
	void spare();//手牌拆牌
	out_card output(const out_card&);//决定要出的牌
};

card_type::card_type(const out_card& pre)
{
	if (pre.type == 1)
		repeat = 0;//定义过牌的重数为0，这时ai为先手出牌方
	else
	{
		if (pre.type == 51)//王炸认为是最大的四先炸弹
		{
			repeat = 4;
			max = 14;
			join = 1;
			carry = 0;
		}
		else
		{
			max = pre.level;//level记录的是最大牌
			if (pre.type < 6)//不带牌
			{
				repeat = pre.level - 1;
				join = 1;
				carry = 0;
			}
			else if (pre.type < 8)//三带一或一对
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
	int used[15] = {};//各种牌在组合中已经用过了多少
	oneposs_spare res;
	//把大小王提出
	if (hand_card.card[13] && hand_card.card[14])//有王炸
	{
		res.met.push_back(card_type(4, 14, 1, 0));
		res.out_time++;
		used[14] = used[13] = 1;
	}
	else if (hand_card.card[13])
	{
		res.met.push_back(card_type(1, 13, 1, 0));//只有小王
		res.out_time++;
		used[13] = 1;
	}
	else if (hand_card.card[14])
	{
		res.met.push_back(card_type(1, 14, 1, 0));//只有大王
		res.out_time++;
		used[14] = 1;
	}
	//提小2
	if (hand_card.card[12])
	{
		res.met.push_back(card_type(hand_card.card[12], 12, 1, 0));
		res.out_time++;
		used[12] = hand_card.card[12];
	}
	int i;
	//提出炸弹
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
	card_type pr(pre);//将上一次非过牌型转换为card_type类型
	this->spare();
}