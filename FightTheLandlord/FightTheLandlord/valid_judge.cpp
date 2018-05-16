#include <iostream>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include "general_class.h"
using namespace std;

extern bool judge_char(const char& c)
{
	return c == 'x' || c == 'd' || c == 'j' || c == 'q' || c == 'k' || c == 'a' || c == 't'
		|| (c >= '2'&&c <= '9');
}

struct same_card
{
	int ord;
	int time;
	same_card(int _ord, int _time) :ord(_ord), time(_time) {}
	friend bool operator <(const same_card& a, const same_card& b)
	{
		return a.time < b.time || (a.time == b.time&&a.ord < b.ord);
	}
	friend bool operator >(const same_card& a, const same_card& b)
	{
		return a.time > b.time || (a.time == b.time&&a.ord < b.ord);
	}
};
/*======================================
0 非法牌型
1 过
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
extern int valid_out(vector<int>& out, out_card& pre, card_in_hand& player, int& level)
//如果是非过的牌型，则更新pre，pre的牌型是按照习惯顺序排的
{
	if (pre.type() == 1 && out.empty())//第一出牌者不能选择过牌
		return 0;
	if (pre.type() != 1 && out.empty())//非第一出牌者选择过牌
		return 1;
	map<int, int> card_time;//牌编号-张数映射表
	vector<same_card> record;//牌编号-张数对的主副关键字排序
	int i;
	//获得每张牌的张数
	for (i = 0; i < out.size(); i++)
	{
		const int& name = out[i];
		if (card_time.count(name))
			card_time[name]++;
		else
			card_time.insert(make_pair(name, 1));
	}
	map<int, int>::iterator ite;
	//检查手牌里是否能凑出要出的牌
	for (ite = card_time.begin(); ite != card_time.end(); ite++)
	{
		if (ite->second > player.get_time(ite->first))
			return 0;
		record.push_back(same_card(ite->first, ite->second));
	}
	sort(record.begin(), record.end());//以张数为第一关键字升序、牌编号为第二关键字升序排序
	//获得各种张数的牌种类数
	int cnt[5] = {};//各种张数的牌种类数
	int count_of_cnt = 0;
	for (ite = card_time.begin(); ite != card_time.end(); ite++)
		cnt[ite->second]++;
	for (i = 0; i < 5; i++)
	{
		if (cnt[i] != 0)
			count_of_cnt++;
	}
	//检测牌型
	int type;
	switch (count_of_cnt)
	{
	case 1:
		if (card_time.size() == 1)
		{
			level = record[0].ord;
			type = record[0].time + 1;
		}
		else if (cnt[1] != 0)//顺子或王炸
		{
			if (cnt[1] < 5)
			{
				if ((record[0].ord == 13 && record[1].ord == 14)
					|| (record[0].ord == 14 && record[1].ord == 13))
				{
					level = 100;
					type = 51;//王炸
				}
				else
					type = 0;
			}
			else//检测是否为顺子
			{
				vector<same_card>::iterator vite = record.begin() + 1;
				while (vite< record.end()
					&& vite->ord < 12 && (vite - 1)->ord == vite->ord - 1)
					vite++;
				if (vite == record.end())
				{
					level = (--vite)->ord;
					type = 3 + cnt[1];
				}
				else type = 0;
			}
		}
		else if (cnt[2] != 0)//联对
		{
			if (cnt[2] < 3)
				type = 0;
			else
			{
				vector<same_card>::iterator vite = record.begin() + 1;
				while (vite < record.end()
					&& vite->ord < 12 && (vite - 1)->ord == vite->ord - 1)
					vite++;
				if (vite == record.end())
				{
					level = (--vite)->ord;
					type = 17 + cnt[2];
				}
				else type = 0;
			}
		}
		else//飞机或连炸
		{
			vector<same_card>::iterator vite = record.begin() + 1;
			while (vite < record.end()
				&& vite->ord < 12 && (vite - 1)->ord == vite->ord - 1)
				vite++;
			if (vite == record.end())
			{
				if (cnt[3] != 0)
				{
					level = (--vite)->ord;
					type = 26 + cnt[3];
				}
				else
				{
					level = (--vite)->ord;
					type = 31 + cnt[4];
				}
			}
			else type = 0;
		}
		break;
	case 2://三带一或二、飞机带一或二、四带一或二、连炸带一或二
		if (cnt[3] == 1 && cnt[4] == 0)//三带一或二
		{
			if (cnt[1] == 1)
			{
				level = (--record.end())->ord;
				type = 6;
			}
			else if (cnt[2] == 1)
			{
				level = (--record.end())->ord;
				type = 7;
			}
			else
				type = 0;
		}
		else if (cnt[4] == 1 && cnt[3] == 0)//四带一或二
		{
			if (cnt[1] == 1)
			{
				level = (--record.end())->ord;
				type = 37;
			}
			else if (cnt[2] == 1)
			{
				level = (--record.end())->ord;
				type = 38;
			}
			else
				type = 0;
		}
		else if (cnt[3] > 1 && cnt[4] == 0)//飞机带一或二
		{
			if (cnt[1] == cnt[3])
			{
				vector<same_card>::iterator vite = record.begin();
				while (vite->time != 3)
					vite++;
				vite++;
				while (vite < record.end()
					&& vite->ord < 12 && (vite - 1)->ord == vite->ord - 1)
					vite++;
				if (vite == record.end())
				{
					type = 37 + cnt[3];
					level = (--vite)->ord;
				}
				else
					type = 0;
			}
			else if (cnt[2] == cnt[3])
			{
				vector<same_card>::iterator vite = record.begin();
				while (vite->time != 3)
					vite++;
				vite++;
				while (vite < record.end()
					&& vite->ord < 12 && (vite - 1)->ord == vite->ord - 1)
					vite++;
				if (vite == record.end())
				{
					type = 41 + cnt[3];
					level = (--vite)->ord;
				}
				else
					type = 0;
			}
			else
				type = 0;
		}
		else if (cnt[4] > 1 && cnt[3] == 0)
		{
			if (cnt[1] == cnt[4])
			{
				vector<same_card>::iterator vite = record.begin();
				while (vite->time != 4)
					vite++;
				vite++;
				while (vite < record.end()
					&& vite->ord < 12 && (vite - 1)->ord == vite->ord - 1)
					vite++;
				if (vite == record.end())
				{
					level = (--vite)->ord;
					type = 44 + cnt[4];
				}
				else
					type = 0;
			}
			else if (cnt[2] == cnt[4])
			{
				vector<same_card>::iterator vite = record.begin();
				while (vite->time != 4)
					vite++;
				vite++;
				while (vite < record.end()
					&& vite->ord < 12 && (vite - 1)->ord == vite->ord - 1)
					vite++;
				if (vite == record.end())
				{
					level = (--vite)->ord;
					type = 47 + cnt[4];
				}
				else
					type = 0;
			}
			else
				type = 0;
		}
		else
			type = 0;
		break;
	default:
		type = 0;
	}
	if (type == 0)
		return 0;
	//按照习惯的顺序将输入的牌名排序，唯一要修改实参out的程序块
	{
		sort(record.begin(), record.end(), greater<same_card>());
		int i = 0;
		for (vector<same_card>::iterator iter = record.begin(); iter < record.end(); iter++)
		{
			int t = iter->time;
			while (t--)
			{
				out[i] = iter->ord;
				i++;
			}
		}
	}

	if (pre.type() == 1)//第一出牌者
	{
		player.off_card(out);
		pre = out_card(out, type, level);
		return type;
	}
	if (pre.type() == 51)//不可能大过王炸，非过牌均为非法牌型
		return 0;
	if (type == 51)//出王炸
	{
		player.off_card(out);
		pre = out_card(out, type, level);
		return 51;
	}
	if (type == 5 && pre.type() != 5)//出炸弹炸其他牌
	{
		player.off_card(out);
		pre = out_card(out, type, level);
		return 5;
	}
	else//其他情形牌型和上一次非过的牌必须相同
	{
		if (type == pre.type() && level > pre.level())//牌型相同情况下比较最后一张是否大过前一次的牌
		{
			player.off_card(out);
			pre = out_card(out, type, level);
			return type;
		}
		else
			return 0;
	}
}