#include <iostream>
#include <set>
#include <string>
#include <cassert>
#include <cstring> // ע��memset��cstring���
#include <algorithm>
#include <vector>
#include "jsoncpp/json.h" // ��ƽ̨�ϣ�C++����ʱĬ�ϰ����˿�

using std::vector;
using std::sort;
using std::unique;
using std::set;
using std::string;

constexpr int PLAYER_COUNT = 3;

enum class CardComboType
{
	PASS, // ��
	SINGLE, // ����
	PAIR, // ����
	STRAIGHT, // ˳��
	STRAIGHT2, // ˫˳
	TRIPLET, // ����
	TRIPLET1, // ����һ
	TRIPLET2, // ������
	BOMB, // ը��
	QUADRUPLE2, // �Ĵ�����ֻ��
	QUADRUPLE4, // �Ĵ������ԣ�
	PLANE, // �ɻ�
	PLANE1, // �ɻ���С��
	PLANE2, // �ɻ�������
	SSHUTTLE, // ����ɻ�
	SSHUTTLE2, // ����ɻ���С��
	SSHUTTLE4, // ����ɻ�������
	ROCKET, // ���
	INVALID // �Ƿ�����
};

int cardComboScores[] = {
	0, // ��
	1, // ����
	2, // ����
	6, // ˳��
	6, // ˫˳
	4, // ����
	4, // ����һ
	4, // ������
	10, // ը��
	8, // �Ĵ�����ֻ��
	8, // �Ĵ������ԣ�
	8, // �ɻ�
	8, // �ɻ���С��
	8, // �ɻ�������
	10, // ����ɻ�����Ҫ���У�����Ϊ10�֣�����Ϊ20�֣�
	10, // ����ɻ���С��
	10, // ����ɻ�������
	16, // ���
	0 // �Ƿ�����
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

// ��0~53��54��������ʾΨһ��һ����
using Card = short;
constexpr Card card_joker = 52;
constexpr Card card_JOKER = 53;

// ������0~53��54��������ʾΨһ���ƣ�
// ���ﻹ����һ����ű�ʾ�ƵĴ�С�����ܻ�ɫ�����Ա�Ƚϣ������ȼ���Level��
// ��Ӧ��ϵ���£�
// 3 4 5 6 7 8 9 10	J Q K	A	2	С��	����
// 0 1 2 3 4 5 6 7	8 9 10	11	12	13	14
using Level = short;
constexpr Level MAX_LEVEL = 15;
constexpr Level MAX_STRAIGHT_LEVEL = 11;
constexpr Level level_joker = 13;
constexpr Level level_JOKER = 14;

/**
* ��Card���Level
*/
constexpr Level card2level(Card card)
{
	return card / 4 + card / 53;
}

// �Ƶ���ϣ����ڼ�������
struct CardCombo
{
	// ��ʾͬ�ȼ������ж�����
	// �ᰴ�����Ӵ�С���ȼ��Ӵ�С����
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
	vector<Card> cards; // ԭʼ���ƣ�δ����
	vector<CardPack> packs; // ����Ŀ�ʹ�С���������
	CardComboType comboType; // ���������
	Level comboLevel = 0; // ����Ĵ�С��

						  /**
						  * ����������CardPack�ݼ��˼���
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
	* �������������ֵܷ�ʱ���Ȩ��
	*/
	int getWeight() const
	{
		if (comboType == CardComboType::SSHUTTLE ||
			comboType == CardComboType::SSHUTTLE2 ||
			comboType == CardComboType::SSHUTTLE4)
			return cardComboScores[(int)comboType] + (findMaxSeq() > 2) * 10;
		return cardComboScores[(int)comboType];
	}

	// ����һ��������
	CardCombo() : comboType(CardComboType::PASS) {}

	/**
	* ͨ��Card����short�����͵ĵ���������һ������
	* ����������ͺʹ�С���
	* ��������û���ظ����֣����ظ���Card��
	*/
	template <typename CARD_ITERATOR>
	CardCombo(CARD_ITERATOR begin, CARD_ITERATOR end)
	{
		// ���У���
		if (begin == end)
		{
			comboType = CardComboType::PASS;
			return;
		}

		// ÿ�����ж��ٸ�
		short counts[MAX_LEVEL + 1] = {};

		// ͬ���Ƶ��������ж��ٸ����š����ӡ�������������
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

		// ���������������ǿ��ԱȽϴ�С��
		comboLevel = packs[0].level;

		// ��������
		// ���� ͬ���Ƶ����� �м��� ���з���
		vector<int> kindOfCountOfCount;
		for (int i = 0; i <= 4; i++)
			if (countOfCount[i])
				kindOfCountOfCount.push_back(i);
		sort(kindOfCountOfCount.begin(), kindOfCountOfCount.end());

		int curr, lesser;

		switch (kindOfCountOfCount.size())
		{
		case 1: // ֻ��һ����
			curr = countOfCount[kindOfCountOfCount[0]];
			switch (kindOfCountOfCount[0])
			{
			case 1:
				// ֻ�����ɵ���
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
				// ֻ�����ɶ���
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
				// ֻ����������
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
				// ֻ����������
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
		case 2: // ��������
			curr = countOfCount[kindOfCountOfCount[1]];
			lesser = countOfCount[kindOfCountOfCount[0]];
			if (kindOfCountOfCount[1] == 3)
			{
				// ��������
				if (kindOfCountOfCount[0] == 1)
				{
					// ����һ
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
					// ������
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
				// ��������
				if (kindOfCountOfCount[0] == 1)
				{
					// ��������ֻ * n
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
					// ���������� * n
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
	* �ж�ָ�������ܷ�����ǰ���飨������������ǹ��Ƶ��������
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
	* ��ָ��������Ѱ�ҵ�һ���ܴ����ǰ���������
	* ��������Ļ�ֻ����һ��
	* ����������򷵻�һ��PASS������
	*/
	template <typename CARD_ITERATOR>
	CardCombo findFirstValid(CARD_ITERATOR begin, CARD_ITERATOR end) const
	{
		if (comboType == CardComboType::PASS) // �������Ҫ���˭��ֻ��Ҫ����
		{
			CARD_ITERATOR second = begin;
			second++;
			return CardCombo(begin, second); // ��ô�ͳ���һ���ơ���
		}

		// Ȼ���ȿ�һ���ǲ��ǻ�����ǵĻ��͹�
		if (comboType == CardComboType::ROCKET)
			return CardCombo();

		// ���ڴ���������дճ�ͬ���͵���
		auto deck = vector<Card>(begin, end); // ����
		short counts[MAX_LEVEL + 1] = {};

		unsigned short kindCount = 0;

		// ����һ��������ÿ�����ж��ٸ�
		for (Card c : deck)
			counts[card2level(c)]++;

		// ������������ã�ֱ�Ӳ��ô��ˣ������ܲ���ը��
		if (deck.size() < cards.size())
			goto failure;

		// ����һ���������ж�������
		for (short c : counts)
			if (c)
				kindCount++;

		// ���򲻶�����ǰ��������ƣ������ܲ����ҵ�ƥ�������
		{
			// ��ʼ��������
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
			for (Level i = 1; ; i++) // �������
			{
				for (int j = 0; j < mainPackCount; j++)
				{
					int level = packs[j].level + i;

					// �����������͵����Ʋ��ܵ�2�����������͵����Ʋ��ܵ�С�������ŵ����Ʋ��ܳ�������
					if ((comboType == CardComboType::SINGLE && level > MAX_LEVEL) ||
						(isSequential && level > MAX_STRAIGHT_LEVEL) ||
						(comboType != CardComboType::SINGLE && !isSequential && level >= level_joker))
						goto failure;

					// ��������������Ʋ������Ͳ��ü�������
					if (counts[level] < packs[j].count)
						goto next;
				}

				{
					// �ҵ��˺��ʵ����ƣ���ô�����أ�
					// ������Ƶ��������������Ǵ��Ƶ��������Ͳ�����Ҳ����
					if (kindCount < packs.size())
						continue;

					// �����ڿ�����
					// ����ÿ���Ƶ�Ҫ����Ŀ��
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
						if (k == MAX_LEVEL + 1) // ����Ƕ�������Ҫ�󡭡��Ͳ�����
							goto next;
					}


					// ��ʼ������
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
				; // ������
			}
		}

	failure:
		// ʵ���Ҳ�����
		// ���һ���ܲ���ը��

		for (Level i = 0; i < level_joker; i++)
			if (counts[i] == 4 && (comboType != CardComboType::BOMB || i > packs[0].level)) // ����Է���ը������ը�Ĺ�����
			{
				// ������԰�����
				Card bomb[] = { Card(i * 4), Card(i * 4 + 1), Card(i * 4 + 2), Card(i * 4 + 3) };
				return CardCombo(bomb, bomb + 4);
			}

		// ��û�л����
		if (counts[level_joker] + counts[level_JOKER] == 2)
		{
			Card rocket[] = { card_joker, card_JOKER };
			return CardCombo(rocket, rocket + 2);
		}

		// ����
		return CardCombo();
	}

	void debugPrint()
	{
#ifndef _BOTZONE_ONLINE
		std::cout << "��" << cardComboStrings[(int)comboType] <<
			"��" << cards.size() << "�ţ���С��" << comboLevel << "��";
#endif
	}
};

/*===================AI�����===================*/
struct card_type
{
	int repeat;//����
	int max;//�����
	int join;//������
	int carry;//�������ࣨ���Ż������Ż��ǲ�����
	card_type() {}
	card_type(int _r, int _m, int _j, int _c) :repeat(_r), max(_m), join(_j), carry(_c) {}
	card_type(const CardCombo&);//�ӿ�
	//������֮ǰ���������
	friend bool cmp_for_comb(const card_type& a, const card_type& b);
	//���ֳ���ǰ����
	friend bool cmp_for_out(const card_type&, const card_type&);
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
	int card[15];//����
	vector<oneposs_spare> result;//���ƽ��
public:
	ai(const set<Card>& cards)//�����ƴ���AI
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
	void spare();//���Ʋ���
	void dfs(int, const oneposs_spare&, const vector<int>&);//�ݹ�������
	void dfs_for_nonjoin(int, const oneposs_spare&, const vector<int>&);//����
	vector<int> output(const card_type&);//����Ҫ������
};
	/*======================================*/
/*===================AI�����ʵ��===================*/
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
	if (card[13] && card[14])//����ը
	{
		copy_res.met.push_back(card_type(4, 14, 1, 0));
		copy_res.out_time++;
	}
	else if (card[13])
	{
		copy_res.met.push_back(card_type(1, 13, 1, 0));//ֻ��С��
		copy_res.out_time++;
	}
	else if (card[14])
	{
		copy_res.met.push_back(card_type(1, 14, 1, 0));//ֻ�д���
		copy_res.out_time++;
	}
	//������Щ�������Ժ���ƹ��̶����ῼ�ǵ������Բ����޸�������
	int i;
	//���ը��
	for (i = 0; i < 12; i++)
	{
		if (card[i] == 4)
		{
			copy_res.met.push_back(card_type(4, i, 1, 0));
			copy_res.out_time++;
			card[i] = 0;
		}
	}
	//���������в��Ʒ�������ѭ��
	//С2���û���ǵ�Ȼ����򵥵�
	if (card[12] == 0)
	{
		const vector<int> remain_card(card, card + 12);
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
	//��С2�Ļ�С2Ҳ�ò𰡣�
	else
	{
		for (int i = 0; i <= card[12] / 2; i++)
		{
			oneposs_spare res(copy_res);
			int j = card[12] - i;//jһ����Ϊ0
			if (i != 0)
			{
				res.met.push_back(card_type(i, 12, 1, 0));
				res.out_time++;
			}
			res.met.push_back(card_type(j, 12, 1, 0));
			res.out_time++;
			//��Ҫ�ظ�һ������Ĺ�����...
			const vector<int> remain_card(card, card + 12);
			//�������д���A�����ˣ�����ʵ����ֻҪ����ǰ12����
			int mini = 0;
			//�ҵ���������Ʒ���ִ�����Ժ�ʣ�µ���С��
			while (mini < 12 && remain_card[mini] == 0)
				mini++;
			//û��ʣ������
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
				//�Ѷ��Ӳ���������ŵĳ���
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
		//û�и���ĵ���ʣ��
		if (i == 12)
			dfs_for_nonjoin(12, res, remain);
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


/*===================���ߵ���Ҫ�޸Ĳ���--begin===================*/


vector<int> ai::output(const card_type& pr)
{
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
				&& (two.empty() || two_ite != two.end()))//��û��Ҫƥ����߶�ûƥ����
			{
				if (chosen[i].max < mini)//��������¿��Գ���С�ĵ���
				{
					mini = chosen[i].max;//�ҵ���С�ĵ��ƣ�ָ�����ƣ�
					mini_ord = i;
				}
			}
			else if (two.empty() || two_ite != two.end())//���ӻ�ûƥ�������û��Ҫƥ��ģ���ô����һ���Ѿ�ƥ������
			{
				if (chosen[i].repeat != 1//�������ҵ��ŵ����ˣ�
					&& chosen[i].max < mini)
				{
					mini = chosen[i].max;
					mini_ord = i;
				}
			}
			else if (one.empty() || one_ite != one.end())//�����Ѿ�ƥ���굫���Ż�ûƥ�����û��Ҫƥ���
			{
				if (chosen[i].repeat != 2 && chosen[i].max < mini)
				{
					mini = chosen[i].max;
					mini_ord = i;
				}
			}
			//ʣ��ֻ�е��źͶ��Ӷ�ƥ�����������
			else//if (!one.empty() && one_ite == one.end() && !two.empty() && two_ite == two.end())
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
						if (chosen[j].join == 1 && chosen[j].repeat == chosen[0].carry)
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
				else if (now.met[j].repeat == 4)//��ը����������������֮һʱ�ſ���
				{
					if (pr.repeat == 4 && now.met[j].max > pr.max)//��ը��
					{
						val = -4;//��ը����Ȩֵ��Ϊ-4
						break;
					}
					else if (pr.join > 1)//����Ҳը
					{
						val = -2;
						break;
					}
					else if (now.out_time <= 2)//ը���ٳ�һ���ƾ��ܳ���
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


/*===================���ߵ���Ҫ�޸Ĳ���--end===================*/


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
	/*======================================*/
/*===================����ӿ�===================*/
card_type::card_type(const CardCombo& c)
{
	switch (c.comboType)
	{
	case CardComboType::PASS:
		repeat = 0;//����
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
	case CardComboType::ROCKET://��ը
		repeat = 4;
		join = 1;
		carry = 0;
		max = 14;
	}
}
	/*======================================*/
/*===================����ӿ�===================*/
vector<Card> turn(const vector<int>& out, const set<Card>& cards)
{
	vector<Card> res;
	//����ը
	if (out.size() == 4 && out[0] == 14)
	{
		res.push_back(52);
		res.push_back(53);
		return res;
	}
	vector<Card> hand_card[15];
	for (set<Card>::iterator iter = cards.begin(); iter != cards.end(); iter++)
	{
		if (*iter > 51)//��С��
			hand_card[*iter - 39].push_back(*iter);//С��13��Ӧ52������14��Ӧ53
		else
			hand_card[*iter / 4].push_back(*iter);
	}
	for (int i = 0; i < out.size(); i++)
	{
		res.push_back(*hand_card[out[i]].rbegin());//��β��ȡ���Ա���ɾ��
		hand_card[out[i]].pop_back();//ɾ��
	}
	return res;
}
	/*======================================*/



// �ҵ�������Щ
set<Card> myCards;

// ��������ʾ��������Щ
set<Card> landlordPublicCards;

// ��Ҵ��ʼ�����ڶ�����ʲô
vector<vector<Card>> whatTheyPlayed[PLAYER_COUNT];

// ��ǰҪ��������Ҫ���˭
CardCombo lastValidCombo;

// ��һ�ʣ������
short cardRemaining[PLAYER_COUNT] = { 20, 17, 17 };

// ���Ǽ�����ң�0-������1-ũ��ף�2-ũ���ң�
int myPosition;

namespace BotzoneIO
{
	using namespace std;
	void input()
	{
		// �������루ƽ̨�ϵ������ǵ��У�
		string line;
		getline(cin, line);
		Json::Value input;
		Json::Reader reader;
		reader.parse(line, input);

		// ���ȴ����һ�غϣ���֪�Լ���˭������Щ��
		{
			auto firstRequest = input["requests"][0u]; // �±���Ҫ�� unsigned������ͨ�������ֺ����u������
			auto own = firstRequest["own"];
			auto llpublic = firstRequest["public"];
			auto history = firstRequest["history"];
			for (unsigned i = 0; i < own.size(); i++)
				myCards.insert(own[i].asInt());
			for (unsigned i = 0; i < llpublic.size(); i++)
				landlordPublicCards.insert(llpublic[i].asInt());
			if (history[0u].size() == 0)
				if (history[1].size() == 0)
					myPosition = 0; // ���ϼҺ��ϼҶ�û���ƣ�˵���ǵ���
				else
					myPosition = 1; // ���ϼ�û���ƣ������ϼҳ����ˣ�˵����ũ���
			else
				myPosition = 2; // ���ϼҳ����ˣ�˵����ũ����
		}

		// history���һ����ϼң��͵ڶ���ϼң��ֱ���˭�ľ���
		int whoInHistory[] = { (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT, (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT };

		int turn = input["requests"].size();
		for (int i = 0; i < turn; i++)
		{
			// ��λָ����浽��ǰ
			auto history = input["requests"][i]["history"]; // ÿ����ʷ�����ϼҺ����ϼҳ�����
			int howManyPass = 0;
			for (int p = 0; p < 2; p++)
			{
				int player = whoInHistory[p]; // ��˭������
				auto playerAction = history[p]; // ������Щ��
				vector<Card> playedCards;
				for (unsigned _ = 0; _ < playerAction.size(); _++) // ѭ��ö������˳���������
				{
					int card = playerAction[_].asInt(); // �����ǳ���һ����
					playedCards.push_back(card);
				}
				whatTheyPlayed[player].push_back(playedCards); // ��¼�����ʷ
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
				// ��Ҫ�ָ��Լ�������������
				auto playerAction = input["responses"][i]; // ������Щ��
				vector<Card> playedCards;
				for (unsigned _ = 0; _ < playerAction.size(); _++) // ѭ��ö���Լ�����������
				{
					int card = playerAction[_].asInt(); // �������Լ�����һ����
					myCards.erase(card); // ���Լ�������ɾ��
					playedCards.push_back(card);
				}
				whatTheyPlayed[myPosition].push_back(playedCards); // ��¼�����ʷ
				cardRemaining[myPosition] -= playerAction.size();
			}
		}
	}

	/**
	* ������ߣ�begin�ǵ�������㣬end�ǵ������յ�
	* CARD_ITERATOR��Card����short�����͵ĵ�����
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

	// �������ߣ���ֻ���޸����²��֣�

	ai AI(myCards);//�����ƴ���AI�����
	card_type pre(lastValidCombo);//Ҫ�������
	vector<int> out = AI.output(pre);//�������ƾ���
	vector<Card> output = turn(out, myCards);//ת��ΪҪ�����ʽ

	CardCombo myAction(output.begin(), output.end());//��������Ҫ���������ƾ���

	// �ǺϷ���
	assert(myAction.comboType != CardComboType::INVALID);

	assert(
		// ���ϼ�û���Ƶ�ʱ�����
		(lastValidCombo.comboType != CardComboType::PASS && myAction.comboType == CardComboType::PASS) ||
		// ���ϼ�û���Ƶ�ʱ�����ù�����
		(lastValidCombo.comboType != CardComboType::PASS && lastValidCombo.canBeBeatenBy(myAction)) ||
		// ���ϼҹ��Ƶ�ʱ����Ϸ���
		(lastValidCombo.comboType == CardComboType::PASS && myAction.comboType != CardComboType::INVALID)
	);

	// ���߽���������������ֻ���޸����ϲ��֣�

	BotzoneIO::output(myAction.cards.begin(), myAction.cards.end());
}