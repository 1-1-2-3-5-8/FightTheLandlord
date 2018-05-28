#pragma once
#include <iostream>
#include <vector>
#include <string>
using namespace std;

string turn_back(int);//把输入的牌名转化为表示大小的编号（0~14）
int turn(const char&);//把牌编号转换回牌名

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
	void get_card(int ord)//抓牌
	{
		if (ord < 52)
			card[ord % 13]++;//非大小王
		else
			card[ord - 52 + 13]++;//大小王
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
	friend ostream& operator <<(ostream& os, const card_in_hand& player)//输出手牌
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
	int Type;//牌型
	int Level;//牌的等级，用于比较同牌型牌的大小
};


/*===================AI相关类声明===================*/


struct card_type
{
	int repeat;//重数
	int max;//最大牌
	int join;//连牌数
	int carry;//带牌种类（单张还是两张还是不带）
	card_type() {}
	card_type(int _r, int _m, int _j, int _c) :repeat(_r), max(_m), join(_j), carry(_c) {}
	card_type(const out_card&);
	//组三带之前排序用这个
	friend bool cmp_for_comb(const card_type& a, const card_type& b);
	//先手出牌前排序
	friend bool cmp_for_out(const card_type&, const card_type&);
	//忘了写这个是想干什么用了，先留着再说吧
	friend bool cmp_1(const card_type& a, const card_type& b)
	{
		//非连牌先按重数从少到多再按大小从小到大
		if (a.join == 1 && b.join == 1)
			return a.repeat < b.repeat || (a.repeat == b.repeat&&a.max < b.max);
		else
		{
			//连牌先看重数，从少到多
			if (a.repeat < b.repeat)
				return 1;
			if (a.repeat > b.repeat)
				return 0;
			//重数相等看连牌数从多到少
			if (a.join > b.join)
				return 1;
			if (a.join < b.join)
				return 0;
			//连牌数相等按牌的等级（最大牌编号）从小到大
			return a.max < b.max;
		}
	}
};

struct oneposs_spare//一种拆牌方法
{
	vector<card_type> met;//具体拆牌方法
	int out_time;//手数
	oneposs_spare() :out_time(0) {}
	void combine(const card_type&);
	friend bool operator <(const oneposs_spare& a, const oneposs_spare& b)
	{
		return a.out_time < b.out_time;//三带的组合之后再排序
	}
};

class ai
{
	card_in_hand hand_card;//手牌
	vector<oneposs_spare> result;//拆牌结果
	int myself;//自己是谁
	int last;//上一手牌是谁出的
public:
	ai(const card_in_hand& _cd, int _m, int p_t) :hand_card(_cd), myself(_m)
	{
		if (p_t >= 2)
			last = -1;
		else
			last = (myself + 2 - p_t) % 3;
	}
	void spare();//手牌拆牌
	void dfs(int, const oneposs_spare&, const vector<int>&);//递归找连牌
	void dfs_for_nonjoin(int, const oneposs_spare&, const vector<int>&);//拆单牌
	vector<int> output(const out_card&);//决定要出的牌
};