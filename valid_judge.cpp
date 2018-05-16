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
0 �Ƿ�����
1 ��
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
extern int valid_out(vector<int>& out, out_card& pre, card_in_hand& player, int& level)
//����Ƿǹ������ͣ������pre��pre�������ǰ���ϰ��˳���ŵ�
{
	if (pre.type() == 1 && out.empty())//��һ�����߲���ѡ�����
		return 0;
	if (pre.type() != 1 && out.empty())//�ǵ�һ������ѡ�����
		return 1;
	map<int, int> card_time;//�Ʊ��-����ӳ���
	vector<same_card> record;//�Ʊ��-�����Ե������ؼ�������
	int i;
	//���ÿ���Ƶ�����
	for (i = 0; i < out.size(); i++)
	{
		const int& name = out[i];
		if (card_time.count(name))
			card_time[name]++;
		else
			card_time.insert(make_pair(name, 1));
	}
	map<int, int>::iterator ite;
	//����������Ƿ��ܴճ�Ҫ������
	for (ite = card_time.begin(); ite != card_time.end(); ite++)
	{
		if (ite->second > player.get_time(ite->first))
			return 0;
		record.push_back(same_card(ite->first, ite->second));
	}
	sort(record.begin(), record.end());//������Ϊ��һ�ؼ��������Ʊ��Ϊ�ڶ��ؼ�����������
	//��ø�����������������
	int cnt[5] = {};//������������������
	int count_of_cnt = 0;
	for (ite = card_time.begin(); ite != card_time.end(); ite++)
		cnt[ite->second]++;
	for (i = 0; i < 5; i++)
	{
		if (cnt[i] != 0)
			count_of_cnt++;
	}
	//�������
	int type;
	switch (count_of_cnt)
	{
	case 1:
		if (card_time.size() == 1)
		{
			level = record[0].ord;
			type = record[0].time + 1;
		}
		else if (cnt[1] != 0)//˳�ӻ���ը
		{
			if (cnt[1] < 5)
			{
				if ((record[0].ord == 13 && record[1].ord == 14)
					|| (record[0].ord == 14 && record[1].ord == 13))
				{
					level = 100;
					type = 51;//��ը
				}
				else
					type = 0;
			}
			else//����Ƿ�Ϊ˳��
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
		else if (cnt[2] != 0)//����
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
		else//�ɻ�����ը
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
	case 2://����һ������ɻ���һ������Ĵ�һ�������ը��һ���
		if (cnt[3] == 1 && cnt[4] == 0)//����һ���
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
		else if (cnt[4] == 1 && cnt[3] == 0)//�Ĵ�һ���
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
		else if (cnt[3] > 1 && cnt[4] == 0)//�ɻ���һ���
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
	//����ϰ�ߵ�˳���������������ΨһҪ�޸�ʵ��out�ĳ����
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

	if (pre.type() == 1)//��һ������
	{
		player.off_card(out);
		pre = out_card(out, type, level);
		return type;
	}
	if (pre.type() == 51)//�����ܴ����ը���ǹ��ƾ�Ϊ�Ƿ�����
		return 0;
	if (type == 51)//����ը
	{
		player.off_card(out);
		pre = out_card(out, type, level);
		return 51;
	}
	if (type == 5 && pre.type() != 5)//��ը��ը������
	{
		player.off_card(out);
		pre = out_card(out, type, level);
		return 5;
	}
	else//�����������ͺ���һ�ηǹ����Ʊ�����ͬ
	{
		if (type == pre.type() && level > pre.level())//������ͬ����±Ƚ����һ���Ƿ���ǰһ�ε���
		{
			player.off_card(out);
			pre = out_card(out, type, level);
			return type;
		}
		else
			return 0;
	}
}