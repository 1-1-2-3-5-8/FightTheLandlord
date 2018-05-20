#include <iostream>
#include <vector>
#include <algorithm>
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

bool cmp_for_comb(const card_type& a, const card_type& b)
{
	//�������Ȱ��������ٵ����ٰ���С��С����
	if (a.join == 1 && b.join == 1)
		return a.repeat < b.repeat || (a.repeat == b.repeat&&a.max < b.max);
	else
	{
		//�����������
		if (a.join == 1)
			return 1;
		if (b.join == 1)
			return 0;
		//Ȼ��������
		//�����ȿ����������ٵ���
		if (a.repeat < b.repeat)
			return 1;
		if (a.repeat > b.repeat)
			return 0;
		//������ȿ����������ٵ���
		if (a.join < b.join)
			return 1;
		if (a.join > b.join)
			return 0;
		//��������Ȱ��Ƶĵȼ�������Ʊ�ţ���С����
		return a.max < b.max;
	}
}

bool cmp_for_out(const card_type& a, const card_type& b)
{
	//�����ƻ����Ȱ��������ٵ���
	if (a.join == 1 && b.join == 1)
	{
		//��������ǰ
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
		//�����������������ɢ��
		if (a.join == 1)
			return 0;
		if (b.join == 1)
			return 1;
		//���ƻ����ȿ��������Ӷൽ��
		if (a.repeat > b.repeat)
			return 1;
		if (a.repeat < b.repeat)
			return 0;
		//����������ͬ�����ư���С�ƴ�С������
		return a.max - a.join < b.max - b.join;
	}
}

card_type::card_type(const out_card& pre)
{
	if (pre.type() == 1)
		repeat = 0;//������Ƶ�����Ϊ0����ʱaiΪ���ֳ��Ʒ�
	else
	{
		if (pre.type() == 51)//��ը��Ϊ����������ը��
		{
			repeat = 4;
			max = 14;
			join = 1;
			carry = 0;
		}
		else
		{
			max = pre.level();//level��¼���������
			if (pre.type() < 6)//������
			{
				repeat = pre.type() - 1;
				join = 1;
				carry = 0;
			}
			else if (pre.type() < 8)//����һ��һ��
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

//������ÿ������ʼ�����������
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
	//�Ѵ�С�����
	if (hand_card.card[13] && hand_card.card[14])//����ը
	{
		copy_res.met.push_back(card_type(4, 14, 1, 0));
		copy_res.out_time++;
	}
	else if (hand_card.card[13])
	{
		copy_res.met.push_back(card_type(1, 13, 1, 0));//ֻ��С��
		copy_res.out_time++;
	}
	else if (hand_card.card[14])
	{
		copy_res.met.push_back(card_type(1, 14, 1, 0));//ֻ�д���
		copy_res.out_time++;
	}
	//��С2
	if (hand_card.card[12])
	{
		copy_res.met.push_back(card_type(hand_card.card[12], 12, 1, 0));
		copy_res.out_time++;
	}
	//������Щ�������Ժ���ƹ��̶����ῼ�ǵ������Բ����޸�������
	int i;
	//���ը��
	for (i = 0; i < 12; i++)
	{
		if (hand_card.card[i] == 4)
		{
			copy_res.met.push_back(card_type(4, i, 1, 0));
			copy_res.out_time++;
			hand_card.card[i] = 0;
		}
	}
	//���������в��Ʒ�������ѭ��
	const vector<int> remain_card(hand_card.card, hand_card.card + 12);
	//�������д���A�����ˣ�����ʵ����ֻҪ����ǰ12����
	int mini = 0;
	//�ҵ���������Ʒ���ִ�����Ժ�ʣ�µ���С��
	while (mini < 12 && remain_card[mini] == 0)
		mini++;
	//û��ʣ������
	if (mini >= 12)
		result.push_back(copy_res);
	else
		dfs(mini, copy_res, remain_card);
}

void ai::dfs_for_nonjoin(int start, const oneposs_spare& now, const vector<int>& remain)
{
	oneposs_spare res(now);
	if (start >= 12)
	{
		res.out_time = res.met.size();
		result.push_back(res);
	}
	else
	{
		if (remain[start] != 0)
		{
			res.met.push_back(card_type(remain[start], start, 1, 0));
			//�Ѷ��Ӳ���������ŵĳ���
			if (remain[start] == 2)
			{
				res.met.push_back(card_type(1, start, 1, 0));
				res.met.push_back(card_type(1, start, 1, 0));
			}
			else if (remain[start] == 3)
			{
				res.met.push_back(card_type(1, start, 1, 0));
				res.met.push_back(card_type(2, start, 1, 0));
			}
		}
		dfs_for_nonjoin(start + 1, res, remain);
	}
}

//���ܻ�����ظ�������������Ӱ���������иĶ����Ķ��������δ����
//һ��Ҫ�аѶ��Ӳ�ɵ��ų��ķ����������޷�Ӧ���й�ģʽ�ĵ��ų�����
//��start��ʼ���һ�����ƻ�start��Ϊ���ƣ�ÿ�εݹ�start�ϸ񵥵�����
void ai::dfs(int start, const oneposs_spare& now, const vector<int>& remain)
{
	if (start >= 12)//���Ʋ���
		dfs_for_nonjoin(0, now, remain);
	else
	{
		oneposs_spare res(now);
		vector<int> rem(remain);
		vector<int> can_join = join_max(remain);
		int i;
		//�ҵ���С����������
		for (i = start; i < 12; i++)
		{
			if (rem[i] == 1 && can_join[i] >= 5)//���ŵ�˳������Ҫ����
			{
				dfs(i + 1, now, remain);//����������Ҳ����iΪ���Ƴ�������
				//���濼����iΪ��ʼ���һ��˳��
				for (int j = 5; j <= can_join[i]; j++)
				{
					int k;
					for (k = i; k < i + j; k++)
						rem[k]--;
					res.met.push_back(card_type(1, k - 1, j, 0));
					dfs(i + 1, res, rem);//�ݹ�ؿ���i+1
					//����
					res.met.pop_back();
					for (k = i; k < i + j; k++)
						rem[k]++;
				}
				break;
			}
			else if (rem[i] == 2 && can_join[i] >= 3)//��������3��
			{
				dfs(i + 1, now, remain);//����������Ҳ����iΪ���Ƴ�������
				//ͬ����iΪ��ʼ���һ������
				for (int j = 3; j <= can_join[i]; j++)//j��������
				{
					int k;
					for (k = i; k < i + j; k++)
						rem[k] -= 2;//ѭ�����k������ƴ�1
					res.met.push_back(card_type(2, k - 1, j, 0));
					dfs(i + 1, res, rem);//�ݹ�ؿ���i+1
					//����
					res.met.pop_back();
					for (k = i; k < i + j; k++)
						rem[k] += 2;
				}
				break;
			}
			else if (rem[i] == 3 && can_join[i] >= 2)//�ɻ�����2��
			{
				dfs(i + 1, now, remain);//����������Ҳ����iΪ���Ƴ�������
				//ͬ����iΪ��ʼ���һ���ɻ�
				for (int j = 2; j <= can_join[i]; j++)
				{
					int k;
					for (k = i; k < i + j; k++)
						rem[k] -= 3;
					res.met.push_back(card_type(3, k - 1, j, 0));
					dfs(i + 1, res, rem);//�ݹ�ؿ���i+1
					//����
					res.met.pop_back();
					for (k = i; k < i + j; k++)
						rem[k] += 3;
				}
				break;
			}
		}
		//û�����Ƶ�����
		if (i == 12)
			dfs(12, now, remain);
	}
}

/*======================================
����������⣺
1��ͬ�������Ƴ����֣�
2����ը�������Ľ�һ��ϸ��
3����������������������Ʒ���Ȩ������ĵ���
4��һ�ֳ��Ʒ������ȳ������ƣ�
5��û�ж���ʱ��Ҫ�������������һ�𣿣����ܿ��Թ�Ϊ3��һ�������
���Խ�������⣺
1�����Ƶ�ʱ����Ҫ�����Ƴ���
======================================*/
vector<int> ai::output(const out_card& pre)
{
	card_type pr(pre);//����һ�ηǹ�����ת��Ϊcard_type����
	vector<int> res;
	spare();
	int i;
	for (i = 0; i < result.size(); i++)
		result[i].combine(pr);//����Ҫ�ӵ����ͺϲ�����
	sort(result.begin(), result.end());//���ı��������Ĵ�С����
	if (pr.repeat == 0)//���ֳ���
	{
		vector<card_type>& chosen = result[0].met;//ѡ���������ٵĲ��Ʒ���
		sort(chosen.begin(), chosen.end(), cmp_for_out);
		int stop;//������ֹ�ĵط�������ȥ���ǵ���
		//���Ҫ�����ƾͻ������ѭ���ﷵ��
		for (i = 0; i < chosen.size(); i++)
		{
			if (chosen[i].join > 1 && chosen[i].max - chosen[i].join + 1 < 4)//��С��С��7�����ƶ�Ҫ������
			{
				int cnt = chosen[i].max;
				for (int j = 1; j <= chosen[i].join; j++)
				{
					for (int k = 1; k <= chosen[i].repeat; k++)
						res.push_back(cnt);
					cnt--;
				}
				if (chosen[i].carry > 0)//�д���
				{
					int j;
					//�ҵ��������͵���С��
					for (j = 0; j < chosen.size(); j++)
					{
						if (chosen[j].join == 1 && chosen[j].repeat == chosen[i].carry)
							break;
					}
					for (int k = 1; k <= chosen[i].join; k++)
					{
						for (int l = 1; l <= chosen[i].carry; l++)
							res.push_back(chosen[j].max);//������ѹ��vector����
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
		if (i >= chosen.size())//ֻʣ������
			stop = i;
		int mini = 20, mini_ord = 100;
		//�ҳ����źͶ���
		vector<card_type*> one, two;
		for (int j = 0; j < chosen.size(); j++)
		{
			if (chosen[j].repeat == 1 && chosen[j].join == 1 
				&& chosen[j].max < 11)//ֻ����С��A�ģ���Щ�Ż�ȥ��������ɻ�ƥ��
				one.push_back(&chosen[j]);
			else if (chosen[j].repeat == 2 && chosen[j].join == 1
				&& chosen[j].max < 11)//ͬ��
				two.push_back(&chosen[j]);
		}
		vector<card_type*>::iterator one_ite = one.begin(), two_ite = two.begin();
		//�ҵ���������ɻ�ƥ��õĵ��źͶ���
		for (int j = 0; j < chosen.size(); j++)
		{
			if (chosen[i].repeat == 3)
			{
				if (chosen[i].carry == 1)
				{
					for (int k = 0; k < chosen[i].join; k++)
						one_ite++;
				}
				else if (chosen[i].carry == 2)
				{
					for (int k = 0; k < chosen[i].join; k++)
						two_ite++;
				}
			}
		}
		for (int i = stop; i < chosen.size(); i++)
		{
			if (one_ite != one.end() && two_ite != two.end())//��ûƥ����
			{
				if (chosen[i].max < mini)//��������¿��Գ���С�ĵ���
				{
					mini = chosen[i].max;//�ҵ���С�ĵ��ƣ�ָ�����ƣ�
					mini_ord = i;
				}
			}
			else if (two_ite != two.end())//���ӻ�ûƥ���꣬��ô����һ���Ѿ�ƥ������
			{
				if (chosen[i].repeat != 1//�������ҵ��ŵ����ˣ�
					&& chosen[i].max < mini)
				{
					mini = chosen[i].max;
					mini_ord = i;
				}
			}
			else if (one_ite != one.end())//�����Ѿ�ƥ���굫���Ż�ûƥ����
			{
				if (chosen[i].repeat != 2 && chosen[i].max < mini)
				{
					mini = chosen[i].max;
					mini_ord = i;
				}
			}
			//ʣ��ֻ�е��źͶ��Ӷ�ƥ�����������
			else//if (one_ite == one.end() && two_ite == two.end())
			{
				//��������������С�ڵ���2�ĵ���δƥ����
				if (chosen[i].repeat > 2 && chosen[i].max < mini)
				{
					mini = chosen[i].max;
					mini_ord = i;
				}
			}
		}
		if (mini_ord < 100 && chosen[mini_ord].repeat < 4//������е��Ʋ��Ҳ���ը��
			&& mini <= 7)//���Ʋ�����10�ͳ���ǰ���ǵ��Ʋ�����ը��
		{
			for (int i = 1; i <= chosen[mini_ord].repeat; i++)
				res.push_back(mini);
			if (chosen[mini_ord].carry > 0)//������
			{
				int j;
				//�ҵ��������͵���С��
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
		else//���ƴ���10����ֻʣը������
		{
			//��������ƣ���ô��������ԭ���һ����һ�������ƣ����ͳ���һ����
			if (chosen[0].join > 1)
			{
				int cnt = chosen[0].max;
				for (int j = 1; j <= chosen[0].join; j++)
				{
					for (int k = 1; k <= chosen[0].repeat; k++)
						res.push_back(cnt);
					cnt--;
				}
				if (chosen[0].carry > 0)//�д���
				{
					int j;
					//�ҵ��������͵���С��
					for (j = 1; j < chosen.size(); j++)//�ӵڶ����ƿ�ʼ�Ҵ���
					{
						if (chosen[j].join == 1 && chosen[j].repeat == chosen[i].carry)
							break;
					}
					for (int k = 1; k <= chosen[0].join; k++)
					{
						for (int l = 1; l <= chosen[0].carry; l++)
							res.push_back(chosen[j].max);//������ѹ��vector����
						j++;
					}
				}
				return res;
			}
			else//û�����Ƴ��ˣ���ôһ���е��Ƴ�
			{
				//���ʱ��mini����һ���ǺϷ��ģ������ǵ�������С��
				//��ͬLine 334-348
				for (int i = 1; i <= chosen[mini_ord].repeat; i++)
					res.push_back(mini);
				if (chosen[mini_ord].carry > 0)//������
				{
					int j;
					//�ҵ��������͵���С��
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
		}
	}
	else//����
	{
		int mini = 100;
		int method, hand;//�ڼ��ֲ��Ʒ������Լ��ò��Ʒ������������
		for (int i = 0; i < result.size(); i++)
		{
			oneposs_spare& now = result[i];
			sort(now.met.begin(), now.met.end(), cmp_for_out);//���򣬰�������ǰ
			int val = 0;
			int j;
			for (j = 0; j < now.met.size(); j++)
			{
				//�ܽ��Ƚӣ������ը����������Բ��ܽӲ�ը
				if (now.met[j].repeat == pr.repeat&&now.met[j].join == pr.join
					&&now.met[j].carry == pr.carry&&now.met[j].max > pr.max)
				{
					if (now.met[j].join == 1)
					{
						if (now.met[j].max <= 7)
							val = -1;
						else
							val = -2;
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
				else if (now.met[j].repeat == 4)//��ը����������������֮һʱ�ſ���
				{
					if (pr.repeat == 4 && now.met[j].max > pr.max)//��ը��
					{
						val = -4;//��ը����Ȩֵ��Ϊ-4
						break;
					}
					else if (pr.join > 1)//����Ҳը
					{
						val = -3;
						break;
					}
					else if (now.out_time <= 2)//ը���ٳ�һ���ƾ��ܳ���
					{
						val = -10;
						break;
					}
				}
			}
			if (now.out_time + val < mini)
			{
				mini = now.out_time + val;
				method = i;
				hand = j;
			}
		}
		vector<card_type>& chosen = result[method].met;
		//���Ҫ�ӵ����Ʊ����Ʊ�Ŷ�Ҫ��˵�����ù���
		if (hand >= chosen.size())
			return res;
		else
		{
			card_type& chosen_card = chosen[hand];
			int cnt = chosen_card.max;
			//����Ҫ������ѹ��
			for (int i = 1; i <= chosen_card.join; i++)
			{
				for (int j = 1; j <= chosen_card.repeat; j++)
					res.push_back(cnt);
				cnt--;
			}
			//ѹ�����
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
					for (int l = 1; l <= chosen_card.carry; l++)
						res.push_back(chosen[j].max);
					j++;
				}
			}
			return res;
		}
	}
}

void oneposs_spare::combine(const card_type& pre)
{
	sort(met.begin(), met.end(), cmp_for_comb);//��������������ʱ����ıȴ�С����
	vector<card_type*> three, plane;
	int one = 0, two = 0;
	//�ҵ����������Լ��ɻ���ͬʱͳ�Ƴ����źͶ�����
	for (int i = 0; i < met.size(); i++)
	{
		switch (met[i].repeat)
		{
		case 1:
			//ֻ��С��A����
			if (met[i].max < 11)
				one++;
			break;
		case 2:
			//ͬ��
			if (met[i].max < 11)
				two++;
			break;
		case 3:
			if (met[i].join > 1)
				plane.push_back(&met[i]);
			else
				three.push_back(&met[i]);
		}
	}
	const card_type* chosen = NULL;//Ҫ�����Ӷ������Ż�ɻ����Ƶ��ƣ�ѡ�����������������
	if (pre.repeat != 3)//Ҫ�ӵ��Ʋ������������ţ������ܵض�����������
	{
		if (pre.repeat == 1 && pre.join == 1)//�����ǵ��ţ���ѡ��һ�ŵ���������
		{
			for (int i = 0; i < met.size(); i++)
			{
				if (met[i].repeat == 1 && met[i].join == 1 && met[i].max > pre.max
					&&met[i].max < 11)//��С��A�����ܽӣ���ô�����ӵ��ŵĺ�ѡ�����޳���ȥ
				{
					one--;//���ź�ѡ��������1
					//��Ϊ�������Ǵ�С��ʼѡ������ֻҪ��Ӧ��������������Ͳ���ͽ��Ƴ�ͻ
					break;
				}
			}
		}
		else if (pre.repeat == 2 && pre.join == 1)
		{
			for (int i = 0; i < met.size(); i++)
			{
				if (met[i].repeat == 2 && met[i].join == 1 && met[i].max > pre.max
					&&met[i].max < 11)//��С��A�����ܽӣ���ô�����Ӷ��ӵĺ�ѡ�����޳���ȥ
				{
					two--;//���Ӻ�ѡ��������1
					//ͬ���أ���Ϊ�������Ǵ�С��ʼѡ������ֻҪ��Ӧ��������������Ͳ���ͽ��Ƴ�ͻ
					break;
				}
			}
		}
		//Ȼ�����÷ɻ�ƥ�䵥�Ż����
		for (int i = 0; i < plane.size(); i++)
		{
			if (plane[i]->join > one)
			{
				if (plane[i]->join > two)//�ɻ����ȵ��źͶ��������࣬û����
					break;
				//���Դ�����
				else
				{
					out_time -= plane[i]->join;//������ô�����ƣ�����Ҫ����Ӧֵ
					plane[i]->carry = 2;//���´������࣬����ʱ�����������С��ʼ��
					two -= plane[i]->join;
				}
			}
			//���Դ�����ʱֱ�Ӵ�����
			else
			{
				out_time -= plane[i]->join;
				plane[i]->carry = 1;
				one -= plane[i]->join;
			}
		}
		//Ȼ��������
		for (int i = 0; i < three.size(); i++)
		{
			//�ܴ�����ʱ�ʹ�����
			if (one > 0)
			{
				out_time--;
				three[i]->carry = 1;
				one--;
			}
			//û�ж��൥��ʱ���Ǵ�����
			else if (two > 0)
			{
				out_time--;
				three[i]->carry = 2;
				two--;
			}
		}
	}
	else//Ҫ�ӵ��������Ż���������ʱ���ǰ������Ƚ��Ƶ�ԭ��������
	{
		if (pre.carry == 1 && one >= pre.join)
		{
			if (pre.join == 1)
			{
				for (int i = 0; i < three.size(); i++)
				{
					//�ҵ���С���ܽӵ��ƣ�����������Ӧ����
					if (three[i]->max > pre.max)
					{
						chosen = three[i];//ΪҪ�������Ƶ��ƴ��ϱ��
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
						chosen = plane[i];//�����
						plane[i]->carry = 1;
						one -= pre.join;
						out_time -= pre.join;
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
					//ͬ��
					if (three[i]->max > pre.max)
					{
						chosen = three[i];//�����
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
						chosen = plane[i];//�����
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
					//�ҵ���С���ܽӵ��ƣ������ܴ���
					if (three[i]->max > pre.max)
					{
						chosen = three[i];//�����
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
						chosen = plane[i];//���ϱ��
						break;
					}
				}
			}
		}
		//����ʣ�µķɻ�ƥ�䵥�Ż���ӣ�ͬ��
		for (int i = 0; i < plane.size(); i++)
		{
			//�����Ҫ�ӵ���������
			if (plane[i] == chosen)
				continue;
			//��ͬLine 309-330
			if (plane[i]->join > one)
			{
				if (plane[i]->join > two)//�ɻ����ȵ��źͶ��������࣬û����
					break;
				//���Դ�����
				else
				{
					out_time -= plane[i]->join;//������ô�����ƣ�����Ҫ����Ӧֵ
					plane[i]->carry = 2;//���´������࣬����ʱ�����������С��ʼ��
					two -= plane[i]->join;
				}
			}
			//���Դ�����ʱֱ�Ӵ�����
			else
			{
				out_time -= plane[i]->join;
				plane[i]->carry = 1;
				one -= plane[i]->join;
			}
		}
		//Ȼ��������
		for (int i = 0; i < three.size(); i++)
		{
			//Ҫ�������Ƶ���֮ǰ�Ѿ����ǹ��ˣ�����
			if (three[i] == chosen)
				continue;
			//��ͬLine 335-348
			//�ܴ�����ʱ�ʹ�����
			if (one > 0)
			{
				out_time--;
				three[i]->carry = 1;
				one--;
			}
			//û�ж��൥��ʱ���Ǵ�����
			else if (two > 0)
			{
				out_time--;
				three[i]->carry = 2;
				two--;
			}
		}
	}
}