#include <iostream>
#include <time.h>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include "general_class.h"
using namespace std;

string turn_back(int);//把输入的牌名转化为表示大小的编号（0~14）
int turn(const char&);//把牌编号转换回牌名
int valid_out(vector<int>&, out_card&, card_in_hand&, int&);//判断出牌是否合法以及所出牌型
bool judge_char(const char&);//判断输入的字符是否是合法的牌名

card_in_hand lord("Landlord"), farmer1("Farmer 1"), farmer2("Farmer 2");//存储手牌
int pub[3];//3张底牌
string pub_name[3];//底牌名
vector<out_card*> history;//出牌历史
out_card pre;//上一次的非过的牌型
vector<int> out;//玩家输入的出牌
int player;//玩家身份

void deal()//随机发牌
{
	int i;
	vector<int> all_card(54);
	for (i = 0; i < 54; i++)
		all_card[i] = i;//模13值表示牌编号，52和53表示小大王
	//通过输入的数产生随机数种子
	int seed;
	scanf("%d", &seed);
	srand(seed);
	random_shuffle(all_card.begin(), all_card.end());//依据随机数种子随机洗牌
	for (i = 0; i < 51; i++)
	{
		switch (i % 3)
		{
		case 0:
			lord.get_card(all_card[i]);
			break;
		case 1:
			farmer1.get_card(all_card[i]);
			break;
		case 2:
			farmer2.get_card(all_card[i]);
		}
	}
	for (i = 51; i < 54; i++)
	{
		pub[i - 51] = all_card[i] < 52 ? all_card[i] % 13 : all_card[i] - 52 + 13;
		pub_name[i - 51] = turn_back(pub[i - 51]);
		lord.get_card(all_card[i]);
	}
}

void output()//输出底牌名和所有人手牌
{
	cout << "public cards:" <<
		pub_name[0] << " " << pub_name[1] << " " << pub_name[2] << endl;
	cout << "lord:" << lord;
	cout << "farmer1:" << farmer1;
	cout << "farmer2:" << farmer2;
}

void player_turn(card_in_hand& player)
{
	bool valid = 0;
	out.clear();
	char s[100];
	while (!valid)
	{
		cin.getline(s, 50);//必须以一张牌名一个空格的格式输入，滚键盘将导致无法预料的错误！
		int len = strlen(s);
		len = (len + 1) / 2;
		char _n;
		int i;
		for (i = 0; i < len; i++)
		{
			_n = *(s + 2 * i);
			if (!judge_char(_n))
				break;
			out.push_back(turn(_n));
		}
		int level = -1;//记录出的牌的等级（越高表示出的牌在同牌型里越大，非法牌型与过牌为-1）
		int type = valid_out(out, pre, player, level);
		if (i == len && type)
		{
			valid = 1;
			history.push_back(new out_card(out, type, level));
			player.print_name();
			printf("'s output:");
			cout << **history.rbegin();
		}
		else
		{
			printf("Invalid input! Please re-input\n");
			player.print_name();
			printf("'s turn:");
			out.clear();
		}
	}
}

void play()
{
	int cycle = 0;
	int pass_time = 0;
	while (!lord.ending() && !farmer1.ending() && !farmer2.ending())
	{
		if (pass_time >= 2)
			pre = out_card();
		switch (cycle % 3)
		{
		case 0:
			printf("Landlord's turn:");
			player_turn(lord);
			break;
		case 1:
			printf("Farmer 1's turn:\n");
			
			{
				printf("Farmer 1's output:");
				ai AI(farmer1);
				out = AI.output(pre);//传出AI出牌策略
				if (out.size() == 4 && out[0] == 14)//出王炸需要额外转换
				{
					out.clear();
					out.push_back(13);
					out.push_back(14);
				}
				if (out.empty())
					printf("Pass\n");
				else
				{
					//输出AI出牌
					for (int i = 0; i < out.size(); i++)
						cout << turn_back(out[i]) << " ";
					cout << endl;
				}
				int level = -1;//记录出的牌的等级（越高表示出的牌在同牌型里越大，非法牌型与过牌为-1）
				//将AI要出的牌从手牌中删除，然后将出牌放进出牌历史中，如果非过牌则更新pre
				int type = valid_out(out, pre, farmer1, level);
					history.push_back(new out_card(out, type, level));
			}

			break;
		case 2:
			printf("Farmer 2's turn:");
			player_turn(farmer2);
		}
		if (out.empty())
			pass_time++;
		else
			pass_time = 0;
		output();
		cycle++;
	}
	if (lord.ending())
		printf("Lord Win!\n");
	else
		printf("Farmers Win!\n");
}

int main()
{
	deal();
	output();
	srand(time(0));
	player = rand() % 3;//随机确定玩家身份
	if (!player)
		printf("You are landlord\n");
	else
		printf("You are farmer %d\n", player);
	cin.ignore(10, '\n');//过滤换行符
	play();
	return 0;
}