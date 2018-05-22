#include <iostream>
#include <set>
#include <string>
#include <cassert>
#include <cstring> // 注意memset是cstring里的
#include <algorithm>
#include <vector>
#include "jsoncpp/json.h" // 在平台上，C++编译时默认包含此库

using std::vector;
using std::sort;
using std::unique;
using std::set;
using std::string;

constexpr int PLAYER_COUNT = 3;

enum class CardComboType
{
	PASS, // 过
	SINGLE, // 单张
	PAIR, // 对子
	STRAIGHT, // 顺子
	STRAIGHT2, // 双顺
	TRIPLET, // 三条
	TRIPLET1, // 三带一
	TRIPLET2, // 三带二
	BOMB, // 炸弹
	QUADRUPLE2, // 四带二（只）
	QUADRUPLE4, // 四带二（对）
	PLANE, // 飞机
	PLANE1, // 飞机带小翼
	PLANE2, // 飞机带大翼
	SSHUTTLE, // 航天飞机
	SSHUTTLE2, // 航天飞机带小翼
	SSHUTTLE4, // 航天飞机带大翼
	ROCKET, // 火箭
	INVALID // 非法牌型
};

int cardComboScores[] = {
	0, // 过
	1, // 单张
	2, // 对子
	6, // 顺子
	6, // 双顺
	4, // 三条
	4, // 三带一
	4, // 三带二
	10, // 炸弹
	8, // 四带二（只）
	8, // 四带二（对）
	8, // 飞机
	8, // 飞机带小翼
	8, // 飞机带大翼
	10, // 航天飞机（需要特判：二连为10分，多连为20分）
	10, // 航天飞机带小翼
	10, // 航天飞机带大翼
	16, // 火箭
	0 // 非法牌型
};

#ifndef _BOTZONE_ONLINE
string cardComboStrings[] = {
	"PASS",
	"SINGLE",
	"PAIR",
	"STRAIGHT",
	"STRAIGHT2",
	"TRIPLET",
	"TRIPLET1",
	"TRIPLET2",
	"BOMB",
	"QUADRUPLE2",
	"QUADRUPLE4",
	"PLANE",
	"PLANE1",
	"PLANE2",
	"SSHUTTLE",
	"SSHUTTLE2",
	"SSHUTTLE4",
	"ROCKET",
	"INVALID"
};
#endif

// 用0~53这54个整数表示唯一的一张牌
using Card = short;
constexpr Card card_joker = 52;
constexpr Card card_JOKER = 53;

// 除了用0~53这54个整数表示唯一的牌，
// 这里还用另一种序号表示牌的大小（不管花色），以便比较，称作等级（Level）
// 对应关系如下：
// 3 4 5 6 7 8 9 10	J Q K	A	2	小王	大王
// 0 1 2 3 4 5 6 7	8 9 10	11	12	13	14
using Level = short;
constexpr Level MAX_LEVEL = 15;
constexpr Level MAX_STRAIGHT_LEVEL = 11;
constexpr Level level_joker = 13;
constexpr Level level_JOKER = 14;

/**
* 将Card变成Level
*/
constexpr Level card2level(Card card)
{
	return card / 4 + card / 53;
}

// 牌的组合，用于计算牌型
struct CardCombo
{
	// 表示同等级的牌有多少张
	// 会按个数从大到小、等级从大到小排序
	struct CardPack
	{
		Level level;
		short count;

		bool operator< (const CardPack& b) const
		{
			if (count == b.count)
				return level > b.level;
			return count > b.count;
		}
	};
	vector<Card> cards; // 原始的牌，未排序
	vector<CardPack> packs; // 按数目和大小排序的牌种
	CardComboType comboType; // 算出的牌型
	Level comboLevel = 0; // 算出的大小序

						  /**
						  * 检查个数最多的CardPack递减了几个
						  */
	int findMaxSeq() const
	{
		for (unsigned c = 1; c < packs.size(); c++)
			if (packs[c].count != packs[0].count ||
				packs[c].level != packs[c - 1].level - 1)
				return c;
		return packs.size();
	}

	/**
	* 这个牌型最后算总分的时候的权重
	*/
	int getWeight() const
	{
		if (comboType == CardComboType::SSHUTTLE ||
			comboType == CardComboType::SSHUTTLE2 ||
			comboType == CardComboType::SSHUTTLE4)
			return cardComboScores[(int)comboType] + (findMaxSeq() > 2) * 10;
		return cardComboScores[(int)comboType];
	}

	// 创建一个空牌组
	CardCombo() : comboType(CardComboType::PASS) {}

	/**
	* 通过Card（即short）类型的迭代器创建一个牌型
	* 并计算出牌型和大小序等
	* 假设输入没有重复数字（即重复的Card）
	*/
	template <typename CARD_ITERATOR>
	CardCombo(CARD_ITERATOR begin, CARD_ITERATOR end)
	{
		// 特判：空
		if (begin == end)
		{
			comboType = CardComboType::PASS;
			return;
		}

		// 每种牌有多少个
		short counts[MAX_LEVEL + 1] = {};

		// 同种牌的张数（有多少个单张、对子、三条、四条）
		short countOfCount[5] = {};

		cards = vector<Card>(begin, end);
		for (Card c : cards)
			counts[card2level(c)]++;
		for (Level l = 0; l <= MAX_LEVEL; l++)
			if (counts[l])
			{
				packs.push_back(CardPack{ l, counts[l] });
				countOfCount[counts[l]]++;
			}
		sort(packs.begin(), packs.end());

		// 用最多的那种牌总是可以比较大小的
		comboLevel = packs[0].level;

		// 计算牌型
		// 按照 同种牌的张数 有几种 进行分类
		vector<int> kindOfCountOfCount;
		for (int i = 0; i <= 4; i++)
			if (countOfCount[i])
				kindOfCountOfCount.push_back(i);
		sort(kindOfCountOfCount.begin(), kindOfCountOfCount.end());

		int curr, lesser;

		switch (kindOfCountOfCount.size())
		{
		case 1: // 只有一类牌
			curr = countOfCount[kindOfCountOfCount[0]];
			switch (kindOfCountOfCount[0])
			{
			case 1:
				// 只有若干单张
				if (curr == 1)
				{
					comboType = CardComboType::SINGLE;
					return;
				}
				if (curr == 2 && packs[1].level == level_joker)
				{
					comboType = CardComboType::ROCKET;
					return;
				}
				if (curr >= 5 && findMaxSeq() == curr &&
					packs.begin()->level <= MAX_STRAIGHT_LEVEL)
				{
					comboType = CardComboType::STRAIGHT;
					return;
				}
				break;
			case 2:
				// 只有若干对子
				if (curr == 1)
				{
					comboType = CardComboType::PAIR;
					return;
				}
				if (curr >= 3 && findMaxSeq() == curr &&
					packs.begin()->level <= MAX_STRAIGHT_LEVEL)
				{
					comboType = CardComboType::STRAIGHT2;
					return;
				}
				break;
			case 3:
				// 只有若干三条
				if (curr == 1)
				{
					comboType = CardComboType::TRIPLET;
					return;
				}
				if (findMaxSeq() == curr &&
					packs.begin()->level <= MAX_STRAIGHT_LEVEL)
				{
					comboType = CardComboType::PLANE;
					return;
				}
				break;
			case 4:
				// 只有若干四条
				if (curr == 1)
				{
					comboType = CardComboType::BOMB;
					return;
				}
				if (findMaxSeq() == curr &&
					packs.begin()->level <= MAX_STRAIGHT_LEVEL)
				{
					comboType = CardComboType::SSHUTTLE;
					return;
				}
			}
			break;
		case 2: // 有两类牌
			curr = countOfCount[kindOfCountOfCount[1]];
			lesser = countOfCount[kindOfCountOfCount[0]];
			if (kindOfCountOfCount[1] == 3)
			{
				// 三条带？
				if (kindOfCountOfCount[0] == 1)
				{
					// 三带一
					if (curr == 1 && lesser == 1)
					{
						comboType = CardComboType::TRIPLET1;
						return;
					}
					if (findMaxSeq() == curr && lesser == curr &&
						packs.begin()->level <= MAX_STRAIGHT_LEVEL)
					{
						comboType = CardComboType::PLANE1;
						return;
					}
				}
				if (kindOfCountOfCount[0] == 2)
				{
					// 三带二
					if (curr == 1 && lesser == 1)
					{
						comboType = CardComboType::TRIPLET2;
						return;
					}
					if (findMaxSeq() == curr && lesser == curr &&
						packs.begin()->level <= MAX_STRAIGHT_LEVEL)
					{
						comboType = CardComboType::PLANE2;
						return;
					}
				}
			}
			if (kindOfCountOfCount[1] == 4)
			{
				// 四条带？
				if (kindOfCountOfCount[0] == 1)
				{
					// 四条带两只 * n
					if (curr == 1 && lesser == 2)
					{
						comboType = CardComboType::QUADRUPLE2;
						return;
					}
					if (findMaxSeq() == curr && lesser == curr * 2 &&
						packs.begin()->level <= MAX_STRAIGHT_LEVEL)
					{
						comboType = CardComboType::SSHUTTLE2;
						return;
					}
				}
				if (kindOfCountOfCount[0] == 2)
				{
					// 四条带两对 * n
					if (curr == 1 && lesser == 2)
					{
						comboType = CardComboType::QUADRUPLE4;
						return;
					}
					if (findMaxSeq() == curr && lesser == curr * 2 &&
						packs.begin()->level <= MAX_STRAIGHT_LEVEL)
					{
						comboType = CardComboType::SSHUTTLE4;
						return;
					}
				}
			}
		}

		comboType = CardComboType::INVALID;
	}

	/**
	* 判断指定牌组能否大过当前牌组（这个函数不考虑过牌的情况！）
	*/
	bool canBeBeatenBy(const CardCombo& b) const
	{
		if (comboType == CardComboType::INVALID || b.comboType == CardComboType::INVALID)
			return false;
		if (b.comboType == CardComboType::ROCKET)
			return true;
		if (b.comboType == CardComboType::BOMB)
			switch (comboType)
			{
			case CardComboType::ROCKET:
				return false;
			case CardComboType::BOMB:
				return b.comboLevel > comboLevel;
			default:
				return true;
			}
		return b.comboType == comboType && b.cards.size() == cards.size() && b.comboLevel > comboLevel;
	}

	/**
	* 从指定手牌中寻找第一个能大过当前牌组的牌组
	* 如果随便出的话只出第一张
	* 如果不存在则返回一个PASS的牌组
	*/
	template <typename CARD_ITERATOR>
	CardCombo findFirstValid(CARD_ITERATOR begin, CARD_ITERATOR end) const
	{
		if (comboType == CardComboType::PASS) // 如果不需要大过谁，只需要随便出
		{
			CARD_ITERATOR second = begin;
			second++;
			return CardCombo(begin, second); // 那么就出第一张牌……
		}

		// 然后先看一下是不是火箭，是的话就过
		if (comboType == CardComboType::ROCKET)
			return CardCombo();

		// 现在打算从手牌中凑出同牌型的牌
		auto deck = vector<Card>(begin, end); // 手牌
		short counts[MAX_LEVEL + 1] = {};

		unsigned short kindCount = 0;

		// 先数一下手牌里每种牌有多少个
		for (Card c : deck)
			counts[card2level(c)]++;

		// 手牌如果不够用，直接不用凑了，看看能不能炸吧
		if (deck.size() < cards.size())
			goto failure;

		// 再数一下手牌里有多少种牌
		for (short c : counts)
			if (c)
				kindCount++;

		// 否则不断增大当前牌组的主牌，看看能不能找到匹配的牌组
		{
			// 开始增大主牌
			int mainPackCount = findMaxSeq();
			bool isSequential =
				comboType == CardComboType::STRAIGHT ||
				comboType == CardComboType::STRAIGHT2 ||
				comboType == CardComboType::PLANE ||
				comboType == CardComboType::PLANE1 ||
				comboType == CardComboType::PLANE2 ||
				comboType == CardComboType::SSHUTTLE ||
				comboType == CardComboType::SSHUTTLE2 ||
				comboType == CardComboType::SSHUTTLE4;
			for (Level i = 1; ; i++) // 增大多少
			{
				for (int j = 0; j < mainPackCount; j++)
				{
					int level = packs[j].level + i;

					// 各种连续牌型的主牌不能到2，非连续牌型的主牌不能到小王，单张的主牌不能超过大王
					if ((comboType == CardComboType::SINGLE && level > MAX_LEVEL) ||
						(isSequential && level > MAX_STRAIGHT_LEVEL) ||
						(comboType != CardComboType::SINGLE && !isSequential && level >= level_joker))
						goto failure;

					// 如果手牌中这种牌不够，就不用继续增了
					if (counts[level] < packs[j].count)
						goto next;
				}

				{
					// 找到了合适的主牌，那么从牌呢？
					// 如果手牌的种类数不够，那从牌的种类数就不够，也不行
					if (kindCount < packs.size())
						continue;

					// 好终于可以了
					// 计算每种牌的要求数目吧
					short requiredCounts[MAX_LEVEL + 1] = {};
					for (int j = 0; j < mainPackCount; j++)
						requiredCounts[packs[j].level + i] = packs[j].count;
					for (unsigned j = mainPackCount; j < packs.size(); j++)
					{
						Level k;
						for (k = 0; k <= MAX_LEVEL; k++)
						{
							if (requiredCounts[k] || counts[k] < packs[j].count)
								continue;
							requiredCounts[k] = packs[j].count;
							break;
						}
						if (k == MAX_LEVEL + 1) // 如果是都不符合要求……就不行了
							goto next;
					}


					// 开始产生解
					vector<Card> solve;
					for (Card c : deck)
					{
						Level level = card2level(c);
						if (requiredCounts[level])
						{
							solve.push_back(c);
							requiredCounts[level]--;
						}
					}
					return CardCombo(solve.begin(), solve.end());
				}

			next:
				; // 再增大
			}
		}

	failure:
		// 实在找不到啊
		// 最后看一下能不能炸吧

		for (Level i = 0; i < level_joker; i++)
			if (counts[i] == 4 && (comboType != CardComboType::BOMB || i > packs[0].level)) // 如果对方是炸弹，能炸的过才行
			{
				// 还真可以啊……
				Card bomb[] = { Card(i * 4), Card(i * 4 + 1), Card(i * 4 + 2), Card(i * 4 + 3) };
				return CardCombo(bomb, bomb + 4);
			}

		// 有没有火箭？
		if (counts[level_joker] + counts[level_JOKER] == 2)
		{
			Card rocket[] = { card_joker, card_JOKER };
			return CardCombo(rocket, rocket + 2);
		}

		// ……
		return CardCombo();
	}

	void debugPrint()
	{
#ifndef _BOTZONE_ONLINE
		std::cout << "【" << cardComboStrings[(int)comboType] <<
			"共" << cards.size() << "张，大小序" << comboLevel << "】";
#endif
	}
};

/*===================AI相关类===================*/
struct card_type
{
	int repeat;//重数
	int max;//最大牌
	int join;//连牌数
	int carry;//带牌种类（单张还是两张还是不带）
	card_type() {}
	card_type(int _r, int _m, int _j, int _c) :repeat(_r), max(_m), join(_j), carry(_c) {}
	card_type(const CardCombo&);//接口
	//组三带之前排序用这个
	friend bool cmp_for_comb(const card_type& a, const card_type& b);
	//先手出牌前排序
	friend bool cmp_for_out(const card_type&, const card_type&);
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
	int card[15];//手牌
	vector<oneposs_spare> result;//拆牌结果
public:
	ai(const set<Card>& cards)//把手牌传给AI
	{
		memset(card, 0, sizeof(card));
		for (set<Card>::iterator iter = cards.begin(); iter != cards.end(); iter++)
		{
			if (*iter >= 52)
				card[*iter - 39]++;
			else
			{
				card[*iter / 4]++;
			}
		}
	}
	void spare();//手牌拆牌
	void dfs(int, const oneposs_spare&, const vector<int>&);//递归找连牌
	void dfs_for_nonjoin(int, const oneposs_spare&, const vector<int>&);//拆单牌
	vector<int> output(const card_type&);//决定要出的牌
};
	/*======================================*/
/*===================AI相关类实现===================*/
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
		//重数相等看连牌数从少到多
		if (a.join < b.join)
			return 1;
		if (a.join > b.join)
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
	if (card[13] && card[14])//有王炸
	{
		copy_res.met.push_back(card_type(4, 14, 1, 0));
		copy_res.out_time++;
	}
	else if (card[13])
	{
		copy_res.met.push_back(card_type(1, 13, 1, 0));//只有小王
		copy_res.out_time++;
	}
	else if (card[14])
	{
		copy_res.met.push_back(card_type(1, 14, 1, 0));//只有大王
		copy_res.out_time++;
	}
	//上面这些牌名在以后拆牌过程都不会考虑到，所以不必修改余牌量
	int i;
	//提出炸弹
	for (i = 0; i < 12; i++)
	{
		if (card[i] == 4)
		{
			copy_res.met.push_back(card_type(4, i, 1, 0));
			copy_res.out_time++;
			card[i] = 0;
		}
	}
	//以上是所有拆牌方案都遵循的
	//小2如果没有那当然是最简单的
	if (card[12] == 0)
	{
		const vector<int> remain_card(card, card + 12);
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
		for (int i = 0; i <= card[12] / 2; i++)
		{
			oneposs_spare res(copy_res);
			int j = card[12] - i;//j一定不为0
			if (i != 0)
			{
				res.met.push_back(card_type(i, 12, 1, 0));
				res.out_time++;
			}
			res.met.push_back(card_type(j, 12, 1, 0));
			res.out_time++;
			//又要重复一遍上面的过程了...
			const vector<int> remain_card(card, card + 12);
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
		res.out_time = res.met.size();
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


/*===================决策的主要修改部分--begin===================*/


vector<int> ai::output(const card_type& pr)
{
	vector<int> res;
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
						for (int l = 1; l <= chosen[i].carry; l++)
							res.push_back(chosen[j].max);//将带牌压入vector数组
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
				if (chosen[i].max < mini)//这种情况下可以出最小的单牌
				{
					mini = chosen[i].max;//找到最小的单牌（指非连牌）
					mini_ord = i;
				}
			}
			else if (two.empty() || two_ite != two.end())//对子还没匹配完或者没有要匹配的，那么单张一定已经匹配完了
			{
				if (chosen[i].repeat != 1//不能再找单张的牌了！
					&& chosen[i].max < mini)
				{
					mini = chosen[i].max;
					mini_ord = i;
				}
			}
			else if (one.empty() || one_ite != one.end())//对子已经匹配完但单张还没匹配完或没有要匹配的
			{
				if (chosen[i].repeat != 2 && chosen[i].max < mini)
				{
					mini = chosen[i].max;
					mini_ord = i;
				}
			}
			//剩下只有单张和对子都匹配完的情形了
			else//if (!one.empty() && one_ite == one.end() && !two.empty() && two_ite == two.end())
			{
				//不可能再有重数小于等于2的单牌未匹配了
				if (chosen[i].repeat > 2 && chosen[i].max < mini)
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
						for (int l = 1; l <= chosen[0].carry; l++)
							res.push_back(chosen[j].max);//将带牌压入vector数组
						j++;
					}
				}
				return res;
			}
			else//没有连牌出了，那么一定有单牌出
			{
				//这个时候mini的牌一定是合法的，并且是单牌里最小的
				//下同Line 334-348
				for (int i = 1; i <= chosen[mini_ord].repeat; i++)
					res.push_back(mini);
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
		}
	}
	else//接牌
	{
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
						else
							val = -4;
					}
					else
					{
						if (now.met[j].max <= 7)
							val = -8;
						else
							val = -10;
					}
					break;
				}
				else if (now.met[j].repeat == 4)//能炸，在满足下列条件之一时才考虑
				{
					if (pr.repeat == 4 && now.met[j].max > pr.max)//接炸弹
					{
						val = -4;//出炸弹的权值定为-4
						break;
					}
					else if (pr.join > 1)//连牌也炸
					{
						val = -2;
						break;
					}
					else if (now.out_time <= 2)//炸完再出一手牌就能出完
					{
						val = -15;
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
					for (int l = 1; l <= chosen_card.carry; l++)
						res.push_back(chosen[j].max);
					j++;
				}
			}
			return res;
		}
	}
}


/*===================决策的主要修改部分--end===================*/


void oneposs_spare::combine(const card_type& pre)
{
	sort(met.begin(), met.end(), cmp_for_comb);//先排序，用组三带时排序的比大小函数
	vector<card_type*> three, plane;
	int one = 0, two = 0;
	//找到所有三个以及飞机，同时统计出单张和对子数
	for (int i = 0; i < met.size(); i++)
	{
		switch (met[i].repeat)
		{
		case 1:
			//只带小于A的牌
			if (met[i].max < 11)
				one++;
			break;
		case 2:
			//同上
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
		//然后先让飞机匹配单张或对子
		for (int i = 0; i < plane.size(); i++)
		{
			if (plane[i]->join > one)
			{
				if (plane[i]->join > two)//飞机数比单张和对子数都多，没法带
					break;
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
				out_time -= plane[i]->join;
				plane[i]->carry = 1;
				one -= plane[i]->join;
			}
		}
		//然后考虑三带
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
		if (pre.carry == 1 && one >= pre.join)
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
						chosen = plane[i];//带标记
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
			if (plane[i]->join > one)
			{
				if (plane[i]->join > two)//飞机数比单张和对子数都多，没法带
					break;
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
				out_time -= plane[i]->join;
				plane[i]->carry = 1;
				one -= plane[i]->join;
			}
		}
		//然后考虑三带
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
	/*======================================*/
/*===================输入接口===================*/
card_type::card_type(const CardCombo& c)
{
	switch (c.comboType)
	{
	case CardComboType::PASS:
		repeat = 0;//过牌
		join = 0;
		carry = 0;
		max = -1;
		break;
	case CardComboType::SINGLE:
		repeat = 1;
		join = 1;
		carry = 0;
		max = c.comboLevel;
		break;
	case CardComboType::PAIR:
		repeat = 2;
		join = 1;
		carry = 0;
		max = c.comboLevel;
		break;
	case CardComboType::TRIPLET:
		repeat = 3;
		join = 1;
		carry = 0;
		max = c.comboLevel;
		break;
	case CardComboType::BOMB:
		repeat = 4;
		join = 1;
		carry = 0;
		max = c.comboLevel;
		break;
	case CardComboType::STRAIGHT:
		repeat = 1;
		join = c.findMaxSeq();
		carry = 0;
		max = c.comboLevel;
		break;
	case CardComboType::STRAIGHT2:
		repeat = 2;
		join = c.findMaxSeq();
		carry = 0;
		max = c.comboLevel;
		break;
	case CardComboType::TRIPLET1:
		repeat = 3;
		join = 1;
		carry = 1;
		max = c.comboLevel;
		break;
	case CardComboType::TRIPLET2:
		repeat = 3;
		join = 1;
		carry = 2;
		max = c.comboLevel;
		break;
	case CardComboType::QUADRUPLE2:
		repeat = 4;
		join = 1;
		carry = 1;
		max = c.comboLevel;
		break;
	case CardComboType::QUADRUPLE4:
		repeat = 4;
		join = 1;
		carry = 2;
		max = c.comboLevel;
		break;
	case CardComboType::PLANE:
		repeat = 3;
		join = c.findMaxSeq();
		carry = 0;
		max = c.comboLevel;
		break;
	case CardComboType::PLANE1:
		repeat = 3;
		join = c.findMaxSeq();
		carry = 1;
		max = c.comboLevel;
		break;
	case CardComboType::PLANE2:
		repeat = 3;
		join = c.findMaxSeq();
		carry = 2;
		max = c.comboLevel;
		break;
	case CardComboType::SSHUTTLE:
		repeat = 4;
		join = c.findMaxSeq();
		carry = 0;
		max = c.comboLevel;
		break;
	case CardComboType::SSHUTTLE2:
		repeat = 4;
		join = c.findMaxSeq();
		carry = 1;
		max = c.comboLevel;
		break;
	case CardComboType::SSHUTTLE4:
		repeat = 4;
		join = c.findMaxSeq();
		carry = 2;
		max = c.comboLevel;
		break;
	case CardComboType::ROCKET://王炸
		repeat = 4;
		join = 1;
		carry = 0;
		max = 14;
	}
}
	/*======================================*/
/*===================输出接口===================*/
vector<Card> turn(const vector<int>& out, const set<Card>& cards)
{
	vector<Card> res;
	//出王炸
	if (out.size() == 4 && out[0] == 14)
	{
		res.push_back(52);
		res.push_back(53);
		return res;
	}
	vector<Card> hand_card[15];
	for (set<Card>::iterator iter = cards.begin(); iter != cards.end(); iter++)
	{
		if (*iter > 51)//大小王
			hand_card[*iter - 39].push_back(*iter);//小王13对应52，大王14对应53
		else
			hand_card[*iter / 4].push_back(*iter);
	}
	for (int i = 0; i < out.size(); i++)
	{
		res.push_back(*hand_card[out[i]].rbegin());//从尾端取牌以便于删除
		hand_card[out[i]].pop_back();//删除
	}
	return res;
}
	/*======================================*/



// 我的牌有哪些
set<Card> myCards;

// 地主被明示的牌有哪些
set<Card> landlordPublicCards;

// 大家从最开始到现在都出过什么
vector<vector<Card>> whatTheyPlayed[PLAYER_COUNT];

// 当前要出的牌需要大过谁
CardCombo lastValidCombo;

// 大家还剩多少牌
short cardRemaining[PLAYER_COUNT] = { 20, 17, 17 };

// 我是几号玩家（0-地主，1-农民甲，2-农民乙）
int myPosition;

namespace BotzoneIO
{
	using namespace std;
	void input()
	{
		// 读入输入（平台上的输入是单行）
		string line;
		getline(cin, line);
		Json::Value input;
		Json::Reader reader;
		reader.parse(line, input);

		// 首先处理第一回合，得知自己是谁、有哪些牌
		{
			auto firstRequest = input["requests"][0u]; // 下标需要是 unsigned，可以通过在数字后面加u来做到
			auto own = firstRequest["own"];
			auto llpublic = firstRequest["public"];
			auto history = firstRequest["history"];
			for (unsigned i = 0; i < own.size(); i++)
				myCards.insert(own[i].asInt());
			for (unsigned i = 0; i < llpublic.size(); i++)
				landlordPublicCards.insert(llpublic[i].asInt());
			if (history[0u].size() == 0)
				if (history[1].size() == 0)
					myPosition = 0; // 上上家和上家都没出牌，说明是地主
				else
					myPosition = 1; // 上上家没出牌，但是上家出牌了，说明是农民甲
			else
				myPosition = 2; // 上上家出牌了，说明是农民乙
		}

		// history里第一项（上上家）和第二项（上家）分别是谁的决策
		int whoInHistory[] = { (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT, (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT };

		int turn = input["requests"].size();
		for (int i = 0; i < turn; i++)
		{
			// 逐次恢复局面到当前
			auto history = input["requests"][i]["history"]; // 每个历史中有上家和上上家出的牌
			int howManyPass = 0;
			for (int p = 0; p < 2; p++)
			{
				int player = whoInHistory[p]; // 是谁出的牌
				auto playerAction = history[p]; // 出的哪些牌
				vector<Card> playedCards;
				for (unsigned _ = 0; _ < playerAction.size(); _++) // 循环枚举这个人出的所有牌
				{
					int card = playerAction[_].asInt(); // 这里是出的一张牌
					playedCards.push_back(card);
				}
				whatTheyPlayed[player].push_back(playedCards); // 记录这段历史
				cardRemaining[player] -= playerAction.size();

				if (playerAction.size() == 0)
					howManyPass++;
				else
					lastValidCombo = CardCombo(playedCards.begin(), playedCards.end());
			}

			if (howManyPass == 2)
				lastValidCombo = CardCombo();

			if (i < turn - 1)
			{
				// 还要恢复自己曾经出过的牌
				auto playerAction = input["responses"][i]; // 出的哪些牌
				vector<Card> playedCards;
				for (unsigned _ = 0; _ < playerAction.size(); _++) // 循环枚举自己出的所有牌
				{
					int card = playerAction[_].asInt(); // 这里是自己出的一张牌
					myCards.erase(card); // 从自己手牌中删掉
					playedCards.push_back(card);
				}
				whatTheyPlayed[myPosition].push_back(playedCards); // 记录这段历史
				cardRemaining[myPosition] -= playerAction.size();
			}
		}
	}

	/**
	* 输出决策，begin是迭代器起点，end是迭代器终点
	* CARD_ITERATOR是Card（即short）类型的迭代器
	*/
	template <typename CARD_ITERATOR>
	void output(CARD_ITERATOR begin, CARD_ITERATOR end)
	{
		Json::Value result, response(Json::arrayValue);
		for (; begin != end; begin++)
			response.append(*begin);
		result["response"] = response;

		Json::FastWriter writer;
		cout << writer.write(result) << endl;
	}
}

int main()
{
	BotzoneIO::input();

	// 做出决策（你只需修改以下部分）

	ai AI(myCards);//以手牌创建AI类对象
	card_type pre(lastValidCombo);//要大过的牌
	vector<int> out = AI.output(pre);//作出出牌决策
	vector<Card> output = turn(out, myCards);//转换为要求的形式

	CardCombo myAction(output.begin(), output.end());//构建符合要求的输出的牌决策

	// 是合法牌
	assert(myAction.comboType != CardComboType::INVALID);

	assert(
		// 在上家没过牌的时候过牌
		(lastValidCombo.comboType != CardComboType::PASS && myAction.comboType == CardComboType::PASS) ||
		// 在上家没过牌的时候出打得过的牌
		(lastValidCombo.comboType != CardComboType::PASS && lastValidCombo.canBeBeatenBy(myAction)) ||
		// 在上家过牌的时候出合法牌
		(lastValidCombo.comboType == CardComboType::PASS && myAction.comboType != CardComboType::INVALID)
	);

	// 决策结束，输出结果（你只需修改以上部分）

	BotzoneIO::output(myAction.cards.begin(), myAction.cards.end());
}