#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <functional>
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

bool cmp_for_comb(const card_type& a, const card_type& b)
{
	//非连牌先按重数从少到多再按大小从小到大
	if (a.join == 1 && b.join == 1)
		return a.repeat < b.repeat || (a.repeat == b.repeat&&a.max < b.max);
	else
	{
		//先排完非连牌
		if (a.join == 1)
			return 1;
		if (b.join == 1)
			return 0;
		//然后排连牌
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

bool cmp_for_out(const card_type& a, const card_type& b)
{
	//非连牌还是先按重数从少到多
	if (a.join == 1 && b.join == 1)
	{
		//把三带提前
		if (a.repeat == 3 && b.repeat == 3)
			return a.max < b.max;
		if (a.repeat == 3)
			return 1;
		if (b.repeat == 3)
			return 0;
		return a.repeat < b.repeat || (a.repeat == b.repeat&&a.max < b.max);
	}
	else
	{
		//这次先排完连牌再排散牌
		if (a.join == 1)
			return 0;
		if (b.join == 1)
			return 1;
		//连牌还是先看重数，从多到少
		if (a.repeat > b.repeat)
			return 1;
		if (a.repeat < b.repeat)
			return 0;
		//现在重数相同的连牌按最小牌从小到大排
		return a.max - a.join < b.max - b.join;
	}
}

card_type::card_type(const out_card& pre)
{
	if (pre.type() == 1)
		repeat = 0;//定义过牌的重数为0，这时ai为先手出牌方
	else
	{
		if (pre.type() == 51)//王炸认为是最大的四先炸弹
		{
			repeat = 4;
			max = 14;
			join = 1;
			carry = 0;
		}
		else
		{
			max = pre.level();//level记录的是最大牌
			if (pre.type() < 6)//不带牌
			{
				repeat = pre.type() - 1;
				join = 1;
				carry = 0;
			}
			else if (pre.type() < 8)//三带一或一对
			{
				repeat = 3;
				join = 1;
				carry = pre.type() - 5;
			}
			else if (pre.type() <= 19)
			{
				repeat = 1;
				join = pre.type() - 3;
				carry = 0;
			}
			else if (pre.type() <= 27)
			{
				repeat = 2;
				join = pre.type() - 17;
				carry = 0;
			}
			else if (pre.type() <= 32)
			{
				repeat = 3;
				join = pre.type() - 26;
				carry = 0;
			}
			else if (pre.type() <= 36)
			{
				repeat = 4;
				join = pre.type() - 31;
				carry = 0;
			}
			else if (pre.type() <= 38)
			{
				repeat = 4;
				join = 1;
				carry = pre.type() - 36;
			}
			else if (pre.type() <= 42)
			{
				repeat = 3;
				join = pre.type() - 37;
				carry = 1;
			}
			else if (pre.type() <= 45)
			{
				repeat = 3;
				join = pre.type() - 41;
				carry = 2;
			}
			else if (pre.type() <= 48)
			{
				repeat = 4;
				join = pre.type() - 44;
				carry = 1;
			}
			else//if (pre.type() <= 50)
			{
				repeat = 4;
				join = pre.type() - 47;
				carry = 2;
			}
		}
	}
}

//给出从每张牌起始，最多连几张
vector<int> join_max(const vector<int> remain)
{
	vector<int> res;
	for (int i = 0; i < 12; i++)
	{
		if (remain[i] == 0)
		{
			res.push_back(0);
			continue;
		}
		int j;
		for (j = i; j < 12; j++)
		{
			if (remain[j] < remain[i])
				break;
		}
		res.push_back(j - i);
	}
	return res;
}

void ai::spare()
{
	oneposs_spare copy_res;
	//把大小王提出
	if (hand_card.card[13] && hand_card.card[14])//有王炸
	{
		copy_res.met.push_back(card_type(4, 14, 1, 0));
		copy_res.out_time++;
	}
	else if (hand_card.card[13])
	{
		copy_res.met.push_back(card_type(1, 13, 1, 0));//只有小王
		copy_res.out_time++;
	}
	else if (hand_card.card[14])
	{
		copy_res.met.push_back(card_type(1, 14, 1, 0));//只有大王
		copy_res.out_time++;
	}
	//上面这些牌名在以后拆牌过程都不会考虑到，所以不必修改余牌量
	int i;
	//提出炸弹
	for (i = 0; i < 12; i++)
	{
		if (hand_card.card[i] == 4)
		{
			copy_res.met.push_back(card_type(4, i, 1, 0));
			copy_res.out_time++;
			hand_card.card[i] = 0;
		}
	}
	//以上是所有拆牌方案都遵循的
	//小2如果没有那当然是最简单的
	if (hand_card.card[12] == 0)
	{
		const vector<int> remain_card(hand_card.card, hand_card.card + 12);
		//不会再有大于A的牌了，所以实际上只要考虑前12种牌
		int mini = 0;
		//找到在上面拆牌方案执行完以后，剩下的最小牌
		while (mini < 12 && remain_card[mini] == 0)
			mini++;
		//没有剩下牌了
		if (mini >= 12)
			result.push_back(copy_res);
		else
			dfs(mini, copy_res, remain_card);
	}
	//有小2的话小2也得拆啊！
	else
	{
		for (int i = 0; i <= hand_card.card[12] / 2; i++)
		{
			oneposs_spare res(copy_res);
			int j = hand_card.card[12] - i;//j一定不为0
			if (i != 0)
			{
				res.met.push_back(card_type(i, 12, 1, 0));
				res.out_time++;
			}
			res.met.push_back(card_type(j, 12, 1, 0));
			res.out_time++;
			//又要重复一遍上面的过程了...
			const vector<int> remain_card(hand_card.card, hand_card.card + 12);
			//不会再有大于A的牌了，所以实际上只要考虑前12种牌
			int mini = 0;
			//找到在上面拆牌方案执行完以后，剩下的最小牌
			while (mini < 12 && remain_card[mini] == 0)
				mini++;
			//没有剩下牌了
			if (mini >= 12)
				result.push_back(res);
			else
				dfs(mini, res, remain_card);
		}
	}
}

void ai::dfs_for_nonjoin(int start, const oneposs_spare& now, const vector<int>& remain)
{
	oneposs_spare res(now);
	if (start >= 12)
	{
		//把手数的定义改为小于小二的非炸弹非飞机牌手数
		res.out_time = 0;
		for (int i = 0; i < res.met.size(); i++)
		{
			if ((res.met[i].max < 12 && res.met[i].repeat < 4)//小于小二且非炸弹
				&& !(res.met[i].join > 1 && res.met[i].repeat == 3))//飞机也不考虑进手数
				res.out_time++;
		}
		result.push_back(res);
	}
	else
	{
		int i;
		for (i = start; i < 12; i++)
		{
			if (remain[i] != 0)
			{
				res.met.push_back(card_type(remain[i], i, 1, 0));
				dfs_for_nonjoin(i + 1, res, remain);
				//把对子拆成两个单张的出法
				if (remain[i] == 2)
				{
					res.met.pop_back();
					res.met.push_back(card_type(1, i, 1, 0));
					res.met.push_back(card_type(1, i, 1, 0));
					dfs_for_nonjoin(i + 1, res, remain);
				}
				else if (remain[i] == 3)
				{
					res.met.pop_back();
					res.met.push_back(card_type(1, i, 1, 0));
					res.met.push_back(card_type(2, i, 1, 0));
					dfs_for_nonjoin(i + 1, res, remain);
				}
				break;
			}
		}
		//没有更大的单牌剩下
		if (i == 12)
			dfs_for_nonjoin(12, res, remain);
	}
}

//可能会产生重复方案，但不会影响结果，现有改动，改动后情况尚未测验
//一定要有把对子拆成单张出的方案，否则无法应对托管模式的单张出法！
//以start起始拆出一手连牌或将start作为单牌，每次递归start严格单调递增
void ai::dfs(int start, const oneposs_spare& now, const vector<int>& remain)
{
	if (start >= 12)//连牌拆完
		dfs_for_nonjoin(0, now, remain);
	else
	{
		oneposs_spare res(now);
		vector<int> rem(remain);
		vector<int> can_join = join_max(remain);
		int i;
		//找到最小的能连的牌
		for (i = start; i < 12; i++)
		{
			if (rem[i] == 1 && can_join[i] >= 5)//单张的顺子至少要五连
			{
				dfs(i + 1, now, remain);//就算能连牌也考虑i为单牌出的情形
				//下面考虑以i为起始拆出一副顺子
				for (int j = 5; j <= can_join[i]; j++)
				{
					int k;
					for (k = i; k < i + j; k++)
						rem[k]--;
					res.met.push_back(card_type(1, k - 1, j, 0));
					dfs(i + 1, res, rem);//递归地考虑i+1
					//回溯
					res.met.pop_back();
					for (k = i; k < i + j; k++)
						rem[k]++;
				}
				break;
			}
			else if (rem[i] == 2 && can_join[i] >= 3)//对子至少3连
			{
				dfs(i + 1, now, remain);//就算能连牌也考虑i为单牌出的情形
				//同上以i为起始拆出一副联对
				for (int j = 3; j <= can_join[i]; j++)//j是连牌数
				{
					int k;
					for (k = i; k < i + j; k++)
						rem[k] -= 2;//循环完的k比最大牌大1
					res.met.push_back(card_type(2, k - 1, j, 0));
					dfs(i + 1, res, rem);//递归地考虑i+1
					//回溯
					res.met.pop_back();
					for (k = i; k < i + j; k++)
						rem[k] += 2;
					//把多于五对的对子拆成两副顺子
					if (j >= 5)
					{
						for (k = i; k < i + j; k++)
							rem[k]--;
						res.met.push_back(card_type(1, k - 1, j, 0));
						dfs(i, res, rem);
						//回溯
						res.met.pop_back();
						for (k = i; k < i + j; k++)
							rem[k]++;
					}
				}
				break;
			}
			else if (rem[i] == 3 && can_join[i] >= 2)//飞机至少2连
			{
				dfs(i + 1, now, remain);//就算能连牌也考虑i为单牌出的情形
				//同上以i为起始拆出一副飞机
				for (int j = 2; j <= can_join[i]; j++)
				{
					int k;
					for (k = i; k < i + j; k++)
						rem[k] -= 3;
					res.met.push_back(card_type(3, k - 1, j, 0));
					dfs(i + 1, res, rem);//递归地考虑i+1
					//回溯
					res.met.pop_back();
					for (k = i; k < i + j; k++)
						rem[k] += 3;
				}
				break;
			}
		}
		//没有连牌的情形
		if (i == 12)
			dfs(12, now, remain);
	}
}

/*======================================
待解决的问题：
1、同手数的牌出哪种？
2、出炸弹条件的进一步细化
3、更大手数方案接牌与过牌方案权衡参数的调整
4、一种出牌方案里先出哪手牌？
5、没有对子时需要拆对子来接三带一吗？（可能可以归为3的一种情况）
应该可以解决但暂未解决的问题：
1、把一手牌拆成两手可能会导致飞机带上同样的牌，造成违规出牌
2、决策错误问题的根源（用不属于自己的牌接三带一，有可能是接口的问题）
3、空炸
可以解决的问题：
1、出牌的时候会把要带的牌出掉
======================================*/
vector<int> ai::output(const out_card& pre)
{
	card_type pr(pre);//将上一次非过牌型转换为card_type类型
	vector<int> res;
	set<int> carry_card;//要带的牌有哪些，防止重复
	spare();
	int i;
	for (i = 0; i < result.size(); i++)
		result[i].combine(pr);//根据要接的牌型合并三带
	sort(result.begin(), result.end());//按改变后的手数的大小排序
	if (pr.repeat == 0)//先手出牌
	{
		vector<card_type>& chosen = result[0].met;//选择手数最少的拆牌方案
		sort(chosen.begin(), chosen.end(), cmp_for_out);
		int stop;//连牌终止的地方，接下去都是单牌
		//如果要出连牌就会在这个循环里返回
		for (i = 0; i < chosen.size(); i++)
		{
			if (chosen[i].join > 1 && chosen[i].max - chosen[i].join + 1 < 4)//最小张小于7的连牌都要出掉！
			{
				int cnt = chosen[i].max;
				for (int j = 1; j <= chosen[i].join; j++)
				{
					for (int k = 1; k <= chosen[i].repeat; k++)
						res.push_back(cnt);
					cnt--;
				}
				if (chosen[i].carry > 0)//有带牌
				{
					int j;
					//找到所带牌型的最小牌
					for (j = 0; j < chosen.size(); j++)
					{
						if (chosen[j].join == 1 && chosen[j].repeat == chosen[i].carry)
							break;
					}
					for (int k = 1; k <= chosen[i].join; k++)
					{
						if (carry_card.count(chosen[j].max) == 0)//如果在带牌集中还没出现过的话
						{
							for (int l = 1; l <= chosen[i].carry; l++)
								res.push_back(chosen[j].max);//将带牌压入vector数组
							carry_card.insert(chosen[j].max);
						}//就可以选择带牌
						else//不能算带牌，重新寻找这一手带牌
							k--;
						j++;
					}
				}
				return res;
			}
			if (chosen[i].join == 1)
			{
				stop = i;
				break;
			}
		}
		if (i >= chosen.size())//只剩连牌了
			stop = i;
		int mini = 20, mini_ord = 100;
		//找出单张和对子
		vector<card_type*> one, two;
		for (int j = 0; j < chosen.size(); j++)
		{
			if (chosen[j].repeat == 1 && chosen[j].join == 1 
				&& chosen[j].max < 11)//只考虑小于A的，这些才会去与三带或飞机匹配
				one.push_back(&chosen[j]);
			else if (chosen[j].repeat == 2 && chosen[j].join == 1
				&& chosen[j].max < 11)//同上
				two.push_back(&chosen[j]);
		}
		vector<card_type*>::iterator one_ite = one.begin(), two_ite = two.begin();
		//找到和三带或飞机匹配好的单张和对子
		for (int j = 0; j < chosen.size(); j++)
		{
			if (chosen[j].repeat == 3)
			{
				if (chosen[j].carry == 1)
				{
					for (int k = 0; k < chosen[j].join; k++)
						one_ite++;
				}
				else if (chosen[j].carry == 2)
				{
					for (int k = 0; k < chosen[j].join; k++)
						two_ite++;
				}
			}
		}
		for (int i = stop; i < chosen.size(); i++)
		{
			if ((one.empty() || one_ite != one.end())
				&& (two.empty() || two_ite != two.end()))//都没有要匹配或者都没匹配完
			{
				if (chosen[i].repeat < 4//先手不出炸弹
					&& chosen[i].max < mini)//这种情况下可以出最小的（非炸弹）单牌
				{
					mini = chosen[i].max;//找到最小的单牌（指非连牌）
					mini_ord = i;
				}
			}
			else if (two.empty() || two_ite != two.end())//对子还没匹配完或者没有要匹配的，那么单张一定已经匹配完了
			{
				if (chosen[i].repeat != 1//不能再找单张的牌了！
					&& chosen[i].repeat != 4 && chosen[i].max < mini)//也不能出炸弹
				{
					mini = chosen[i].max;
					mini_ord = i;
				}
			}
			else if (one.empty() || one_ite != one.end())//对子已经匹配完但单张还没匹配完或没有要匹配的
			{
				if (chosen[i].repeat != 4//不出炸弹
					&& chosen[i].repeat != 2 && chosen[i].max < mini)
				{
					mini = chosen[i].max;
					mini_ord = i;
				}
			}
			//剩下只有单张和对子都匹配完的情形了
			else//if (!one.empty() && one_ite == one.end() && !two.empty() && two_ite == two.end())
			{
				//不可能再有重数小于等于2的单牌未匹配了
				if (chosen[i].repeat == 3 && chosen[i].max < mini)//只可能出三张
				{
					mini = chosen[i].max;
					mini_ord = i;
				}
			}
		}
		if (mini_ord < 100 && chosen[mini_ord].repeat < 4//如果还有单牌并且不是炸弹
			&& mini <= 7)//单牌不大于10就出，前提是单牌不能是炸弹
		{
			for (int i = 1; i <= chosen[mini_ord].repeat; i++)
				res.push_back(mini);
			//这是单副的三先，只带一手牌，不可能带重所以不用carry_card检查
			if (chosen[mini_ord].carry > 0)//有三带
			{
				int j;
				//找到所带牌型的最小牌
				for (j = stop; j < chosen.size(); j++)
				{
					if (chosen[j].repeat == chosen[mini_ord].carry)
						break;
				}
				for (int k = 1; k <= chosen[j].repeat; k++)
					res.push_back(chosen[j].max);
			}
			return res;
		}
		else//单牌大于10或者只剩炸弹单牌
		{
			//如果有连牌（那么根据排序原则第一手牌一定是连牌），就出第一手牌
			if (chosen[0].join > 1)
			{
				int cnt = chosen[0].max;
				for (int j = 1; j <= chosen[0].join; j++)
				{
					for (int k = 1; k <= chosen[0].repeat; k++)
						res.push_back(cnt);
					cnt--;
				}
				if (chosen[0].carry > 0)//有带牌
				{
					int j;
					//找到所带牌型的最小牌
					for (j = 1; j < chosen.size(); j++)//从第二手牌开始找带牌
					{
						if (chosen[j].join == 1 && chosen[j].repeat == chosen[0].carry)
							break;
					}
					for (int k = 1; k <= chosen[0].join; k++)
					{
						if (carry_card.count(chosen[j].max) == 0)
						{
							for (int l = 1; l <= chosen[0].carry; l++)
								res.push_back(chosen[j].max);//将带牌压入vector数组
							carry_card.insert(chosen[j].max);
						}
						else//不能算带牌，重新寻找这一手带牌
							k--;
						j++;
					}
				}
				return res;
			}
			else//没有连牌出了，那么一定有单牌出
			{
				if (mini_ord < 100)
				{
					//这个时候mini的牌一定是合法的，并且是单牌里最小的
					//下同Line 334-348
					for (int i = 1; i <= chosen[mini_ord].repeat; i++)
						res.push_back(mini);
					if (chosen[mini_ord].carry > 0)//有三带，单手不用carry_card检查
					{
						int j;
						//找到所带牌型的最小牌
						for (j = stop; j < chosen.size(); j++)
						{
							if (chosen[j].repeat == chosen[mini_ord].carry)
								break;
						}
						for (int k = 1; k <= chosen[j].repeat; k++)
							res.push_back(chosen[j].max);
					}
				}
				else//只剩炸弹了，那么炸弹在第一手牌
				{
					for (int i = 1; i <= 4; i++)
						res.push_back(chosen[0].max);
				}
				return res;
			}
		}
	}
	else//接牌，那么last一定不小于0
	{
		if (friend_out)//如果自己是农民并且上一手牌由队友出，这一段可能可以省略
		{
			if ((pr.repeat == 4)//队友出炸弹或者四带或者（可能有带牌的）四重连牌
				|| pr.max >= 11)//队友出不小于A的牌（或最大牌为A的连牌）
				return res;//选择过牌
		}
		int mini = 100;
		int method, hand;//第几种拆牌方案，以及该拆牌方案里的哪手牌
		for (int i = 0; i < result.size(); i++)
		{
			oneposs_spare& now = result[i];
			sort(now.met.begin(), now.met.end(), cmp_for_out);//排序，把连牌提前
			int val = 0;
			int j;
			for (j = 0; j < now.met.size(); j++)
			{
				//能接先接，排序后炸弹在最后，所以不能接才炸
				if (now.met[j].repeat == pr.repeat&&now.met[j].join == pr.join
					&&now.met[j].carry == pr.carry&&now.met[j].max > pr.max)
				{
					if (now.met[j].join == 1)
					{
						if (now.met[j].max <= 7)
							val = -2;
						else if (friend_out&&now.met[j].max > 10)
							val = 0;//见下，如果必须要用大于K的非连牌接队友的话就选择过牌
						else
							val = -3;
					}
					else
					{
						if (now.met[j].max <= 7)
							val = -6;
						else
							val = -8;
					}
					break;
				}
				else if (now.met[j].repeat == 4)//能炸，在满足下列条件时才考虑
				{
					if (now.met.size() <= 2)//炸完再出一手牌就能出完
					{
						val = -15;
						break;
					}
					else if (!friend_out)//如果炸完出不完那么只炸非队友的牌
					{
						if (pr.repeat == 4 && pr.join == 1)//接的是炸弹（不出自队友）
						{
							if (now.met[j].max > pr.max)
							{
								val = -4;//出炸弹的权值定为-4
								break;
							}
						}
						else if (pr.join > 1)//非队友的连牌也炸
						{
							val = -2;
							break;
						}
						else if (pr.max >= 12)
							//大于小二的牌要炸（这手牌一定不出自队友否则在一开始已经选择过牌）
						{
							val = -2;
							break;
						}
					}
				}
			}
			if (now.out_time + val < mini)
			{
				mini = now.out_time + val;
				method = i;
				if (val < 0)
					hand = j;
				else
					hand = now.met.size();
			}
		}
		vector<card_type>& chosen = result[method].met;
		//如果要接的手牌比手牌编号都要大说明采用过牌
		if (hand >= chosen.size())
			return res;
		else
		{
			card_type& chosen_card = chosen[hand];
			int cnt = chosen_card.max;
			//把主要牌型牌压入
			for (int i = 1; i <= chosen_card.join; i++)
			{
				for (int j = 1; j <= chosen_card.repeat; j++)
					res.push_back(cnt);
				cnt--;
			}
			//压入带牌
			if (chosen_card.carry > 0)
			{
				int j;
				for (j = 0; j < chosen.size(); j++)
				{
					if (chosen[j].join == 1 && chosen[j].repeat == chosen_card.carry)
						break;
				}
				for (int k = 1; k <= chosen_card.join; k++)
				{
					if (carry_card.count(chosen[j].max) == 0)
					{
						for (int l = 1; l <= chosen_card.carry; l++)
							res.push_back(chosen[j].max);
						carry_card.insert(chosen[j].max);
					}
					else
						k--;
					j++;
				}
			}
			return res;
		}
	}
}

void oneposs_spare::combine(const card_type& pre)
{
	sort(met.begin(), met.end(), cmp_for_comb);//先排序，用组三带时排序的比大小函数
	vector<card_type*> three, plane;
	int one = 0, two = 0;
	int onecnt[11] = {};//单张牌分别哪些
	int diffone = 0;//单张牌有几种
	//找到所有三个以及飞机，同时统计出单张和对子数
	for (int i = 0; i < met.size(); i++)
	{
		switch (met[i].repeat)
		{
		case 1:
			//只带小于A的牌
			if (met[i].join == 1 && met[i].max < 11)
			{
				one++;
				if (onecnt[met[i].max] == 0)
					diffone++;
				onecnt[met[i].max]++;
			}
			break;
		case 2:
			//同上只带小于A的牌
			if (met[i].join == 1 && met[i].max < 11)
				two++;//因为不拆小二以下的炸弹，所以对子牌不可能出现重复
			break;
		case 3:
			if (met[i].join > 1)
				plane.push_back(&met[i]);
			else
				three.push_back(&met[i]);
		}
	}
	const card_type* chosen = NULL;//要用来接对手三张或飞机型牌的牌，选定后不能用来随意带牌
	if (pre.repeat != 3)//要接的牌不是三带或三张，尽可能地多组三带即可
	{
		if (pre.repeat == 1 && pre.join == 1)//出的是单张，先选出一张单张来接牌
		{
			for (int i = 0; i < met.size(); i++)
			{
				if (met[i].repeat == 1 && met[i].join == 1 && met[i].max > pre.max
					&&met[i].max < 11)//用小于A的牌能接，那么将它从单张的候选牌中剔除出去
				{
					onecnt[met[i].max]--;//这种单张牌数减一
					if (onecnt[met[i].max] == 0)
						diffone--;//如果正好选完了那么单张牌种数就少一
					one--;//单张候选牌种数减1
					//因为带牌总是从小开始选，所以只要相应种类牌数量充足就不会和接牌冲突
					break;
				}
			}
		}
		else if (pre.repeat == 2 && pre.join == 1)
		{
			for (int i = 0; i < met.size(); i++)
			{
				if (met[i].repeat == 2 && met[i].join == 1 && met[i].max > pre.max
					&&met[i].max < 11)//用小于A的牌能接，那么将它从对子的候选牌中剔除出去
				{
					two--;//对子候选牌种数减1
					//同样地，因为带牌总是从小开始选，所以只要相应种类牌数量充足就不会和接牌冲突
					break;
				}
			}
		}
		sort(onecnt, onecnt + 11, greater<int>());//匹配时单张的大小就无关紧要了，因此从多到少排序
		//然后先让飞机匹配单张或对子
		for (int i = 0; i < plane.size(); i++)
		{
			if (plane[i]->join > diffone)//能否匹配单张要看和单张种类数而不是单张数的大小（因为单张可能重复）
			{
				//不能匹配单张看能不能匹配对子
				if (plane[i]->join > two)//飞机数比单张和对子数都多，没法带
					continue;//飞机连数从大到小因此还要继续考虑下一组飞机
				//可以带对子
				else
				{
					out_time -= plane[i]->join;//带出这么多手牌，手数要减相应值
					plane[i]->carry = 2;//记下带牌种类，出牌时根据种类从最小开始带
					two -= plane[i]->join;
				}
			}
			//可以带单张时直接带单张
			else
			{
				const int& num = plane[i]->join;
				for (int i = 0; i < num; i++)
				{
					onecnt[i]--;
					if (onecnt[i] == 0)
						diffone--;//更改单张种类数
				}
				out_time -= num;
				plane[i]->carry = 1;
				one -= num;
			}
		}
		//然后考虑三带，三带只带一种牌因此不用考虑重复
		for (int i = 0; i < three.size(); i++)
		{
			//能带单张时就带单张
			if (one > 0)
			{
				out_time--;
				three[i]->carry = 1;
				one--;
			}
			//没有多余单张时考虑带对子
			else if (two > 0)
			{
				out_time--;
				three[i]->carry = 2;
				two--;
			}
		}
	}
	else//要接的牌是三张或三带，暂时考虑按照优先接牌的原则组三带
	{
		sort(onecnt, onecnt + 11, greater<int>());//带牌不用考虑大小所以可以直接从多到少排序
		if (pre.carry == 1 && diffone >= pre.join)//同样地要和单张种数而不是单张数比较
		{
			if (pre.join == 1)
			{
				for (int i = 0; i < three.size(); i++)
				{
					//找到最小的能接的牌，让它带上相应的牌
					if (three[i]->max > pre.max)
					{
						chosen = three[i];//为要用来接牌的牌带上标记
						three[i]->carry = 1;
						one--;
						out_time--;
						break;
					}
				}
			}
			else
			{
				for (int i = 0; i < plane.size(); i++)
				{
					if (plane[i]->join == pre.join&&plane[i]->max > pre.max)
					{
						const int& num = pre.join;
						chosen = plane[i];//带标记
						plane[i]->carry = 1;
						for (int i = 0; i < num; i++)
						{
							onecnt[i]--;
							if (onecnt[i] == 0)
								diffone--;//更改单张种类数
						}
						one -= num;
						out_time -= num;
						break;
					}
				}
			}
		}
		else if (pre.carry == 2 && two >= pre.join)
		{
			if (pre.join == 1)
			{
				for (int i = 0; i < three.size(); i++)
				{
					//同上
					if (three[i]->max > pre.max)
					{
						chosen = three[i];//带标记
						three[i]->carry = 2;
						two--;
						out_time--;
						break;
					}
				}
			}
			else
			{
				for (int i = 0; i < plane.size(); i++)
				{
					if (plane[i]->join == pre.join&&plane[i]->max > pre.max)
					{
						chosen = plane[i];//带标记
						plane[i]->carry = 2;
						two -= pre.join;
						out_time -= pre.join;
						break;
					}
				}
			}
		}
		else if (pre.carry == 0)
		{
			if (pre.join == 1)
			{
				for (int i = 0; i < three.size(); i++)
				{
					//找到最小的能接的牌，它不能带牌
					if (three[i]->max > pre.max)
					{
						chosen = three[i];//带标记
						break;
					}
				}
			}
			else
			{
				for (int i = 0; i < plane.size(); i++)
				{
					if (plane[i]->join == pre.join&&plane[i]->max > pre.max)
					{
						chosen = plane[i];//带上标记
						break;
					}
				}
			}
		}
		//先让剩下的飞机匹配单张或对子，同上
		for (int i = 0; i < plane.size(); i++)
		{
			//如果是要接的牌则跳过
			if (plane[i] == chosen)
				continue;
			//下同Line 309-330
			if (plane[i]->join > diffone)
			{
				if (plane[i]->join > two)//飞机数比单张和对子数都多，没法带
					continue;
				//可以带对子
				else
				{
					out_time -= plane[i]->join;//带出这么多手牌，手数要减相应值
					plane[i]->carry = 2;//记下带牌种类，出牌时根据种类从最小开始带
					two -= plane[i]->join;
				}
			}
			//可以带单张时直接带单张
			else
			{
				const int& num = plane[i]->join;
				for (int i = 0; i < num; i++)
				{
					onecnt[i]--;
					if (onecnt[i] == 0)
						diffone--;//更改单张种类数
				}
				out_time -= num;
				plane[i]->carry = 1;
				one -= num;
			}
		}
		//然后考虑三带，三带只带一种牌所以不用考虑重复
		for (int i = 0; i < three.size(); i++)
		{
			//要用来接牌的牌之前已经考虑过了，跳过
			if (three[i] == chosen)
				continue;
			//下同Line 335-348
			//能带单张时就带单张
			if (one > 0)
			{
				out_time--;
				three[i]->carry = 1;
				one--;
			}
			//没有多余单张时考虑带对子
			else if (two > 0)
			{
				out_time--;
				three[i]->carry = 2;
				two--;
			}
		}
	}
}