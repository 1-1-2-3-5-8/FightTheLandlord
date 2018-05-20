#include <iostream>
#include <time.h>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include "general_class.h"
using namespace std;

string turn_back(int);//�����������ת��Ϊ��ʾ��С�ı�ţ�0~14��
int turn(const char&);//���Ʊ��ת��������
int valid_out(vector<int>&, out_card&, card_in_hand&, int&);//�жϳ����Ƿ�Ϸ��Լ���������
bool judge_char(const char&);//�ж�������ַ��Ƿ��ǺϷ�������

card_in_hand lord("Landlord"), farmer1("Farmer 1"), farmer2("Farmer 2");//�洢����
int pub[3];//3�ŵ���
string pub_name[3];//������
vector<out_card*> history;//������ʷ
out_card pre;//��һ�εķǹ�������
vector<int> out;//�������ĳ���
int player;//������

void deal()//�������
{
	int i;
	vector<int> all_card(54);
	for (i = 0; i < 54; i++)
		all_card[i] = i;//ģ13ֵ��ʾ�Ʊ�ţ�52��53��ʾС����
	//ͨ����������������������
	int seed;
	scanf("%d", &seed);
	srand(seed);
	random_shuffle(all_card.begin(), all_card.end());//����������������ϴ��
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

void output()//���������������������
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
		cin.getline(s, 50);//������һ������һ���ո�ĸ�ʽ���룬�����̽������޷�Ԥ�ϵĴ���
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
		int level = -1;//��¼�����Ƶĵȼ���Խ�߱�ʾ��������ͬ������Խ�󣬷Ƿ����������Ϊ-1��
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
				out = AI.output(pre);//����AI���Ʋ���
				if (out.size() == 4 && out[0] == 14)//����ը��Ҫ����ת��
				{
					out.clear();
					out.push_back(13);
					out.push_back(14);
				}
				if (out.empty())
					printf("Pass\n");
				else
				{
					//���AI����
					for (int i = 0; i < out.size(); i++)
						cout << turn_back(out[i]) << " ";
					cout << endl;
				}
				int level = -1;//��¼�����Ƶĵȼ���Խ�߱�ʾ��������ͬ������Խ�󣬷Ƿ����������Ϊ-1��
				//��AIҪ�����ƴ�������ɾ����Ȼ�󽫳��ƷŽ�������ʷ�У�����ǹ��������pre
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
	player = rand() % 3;//���ȷ��������
	if (!player)
		printf("You are landlord\n");
	else
		printf("You are farmer %d\n", player);
	cin.ignore(10, '\n');//���˻��з�
	play();
	return 0;
}