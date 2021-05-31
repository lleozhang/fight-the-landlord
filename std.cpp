// 斗地主·改（FightTheLandlord2）样例程序
// 无脑策略
// 最后修改：2021-05-23，修复了whoInHistory的更新。
// 2021-05-08，去除了有问题的isArray。
// 作者：zhouhy
// 游戏信息：http://www.botzone.org/games#FightTheLandlord2

#include <iostream>
#include <set>
#include <string>
#include <cassert>
#include <cstring> // 注意memset是cstring里的
#include <algorithm>
#include <cmath>
#include "jsoncpp/json.h" // 在平台上，C++编译时默认包含此库
using namespace std;
using std::set;
using std::sort;
using std::string;
using std::unique;
using std::vector;

constexpr int PLAYER_COUNT = 3;

enum class Stage
{
	BIDDING, // 叫分阶段
	PLAYING	 // 打牌阶段
};

enum class CardComboType
{
	PASS,		// 过
	SINGLE,		// 单张
	PAIR,		// 对子
	STRAIGHT,	// 顺子
	STRAIGHT2,	// 双顺
	TRIPLET,	// 三条
	TRIPLET1,	// 三带一
	TRIPLET2,	// 三带二
	BOMB,		// 炸弹
	QUADRUPLE2, // 四带二（只）
	QUADRUPLE4, // 四带二（对）
	PLANE,		// 飞机
	PLANE1,		// 飞机带小翼
	PLANE2,		// 飞机带大翼
	SSHUTTLE,	// 航天飞机
	SSHUTTLE2,	// 航天飞机带小翼
	SSHUTTLE4,	// 航天飞机带大翼
	ROCKET,		// 火箭
	INVALID		// 非法牌型
};

int cardComboScores[] = {
	0,	// 过
	1,	// 单张
	2,	// 对子
	6,	// 顺子
	6,	// 双顺
	4,	// 三条
	4,	// 三带一
	4,	// 三带二
	10, // 炸弹
	8,	// 四带二（只）
	8,	// 四带二（对）
	8,	// 飞机
	8,	// 飞机带小翼
	8,	// 飞机带大翼
	10, // 航天飞机（需要特判：二连为10分，多连为20分）
	10, // 航天飞机带小翼
	10, // 航天飞机带大翼
	16, // 火箭
	0	// 非法牌型
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
	"INVALID"};
#endif

#include <ctime>
clock_t s,e;
int est_dfs(int *cyt,int typ,int las)//将牌拆解成不同的牌组进行估分，为避免重复按牌型顺序进行拆解，一个牌组的总评分为各牌型的评分之和减去牌型数量
{
	e=clock();
	if(double(e-s)/CLOCKS_PER_SEC>0.85)return 0;
	bool fl=0;
	int cnt=0;
	for(int i=0;i<=14;i++)if(cyt[i]){fl=1;cnt+=cyt[i];}
	if(!fl)return 0;
	int tempret=-0x3f3f3f3f;
	if(typ>7||las>14)return -0x3f3f3f3f;
	tempret=max(tempret,est_dfs(cyt,typ+1,0));//不使用本牌型
	if(typ==1)//王炸
	{
		bool fl=0;
		if(cyt[14]&&cyt[13])
		{
			cyt[14]--,cyt[13]--;
			int temp=est_dfs(cyt,typ+1,0)-1;
			cyt[14]++,cyt[13]++;
			tempret=max(tempret,temp+30);
			fl=1;
		}
		if(!fl)tempret=max(tempret,est_dfs(cyt,typ+1,0));
	}else if(typ==2)//四带
	{
		bool fl=0;
		for(int i=las;i<=12;i++)
		{
			if(cyt[i]!=4)continue;
			cyt[i]=0;
			fl=1;
			tempret=max(tempret,15+i+est_dfs(cyt,typ,i+1)-1);
			tempret=max(tempret,est_dfs(cyt,typ,i+1));
			for(int j=0;j<=14;j++)//四带一/二
			{
				for(int k=j+1;k<=14;k++)
				{
					if(cyt[j]&&cyt[k])
					{
						cyt[j]--,cyt[k]--;
						tempret=max(tempret,7+i+est_dfs(cyt,typ,i+1)-1);
						if(cyt[j]&&cyt[k])
						{
							cyt[j]--,cyt[k]--;
							tempret=max(tempret,7+i+est_dfs(cyt,typ,i+1)-1);
							cyt[j]++,cyt[k]++;
						}
						cyt[j]++,cyt[k]++;
					}
				}
			}
			cyt[i]=4;
			break;
		}
		if(!fl)tempret=max(tempret,est_dfs(cyt,typ+1,0));
	}else if(typ==3)//三带
	{
		bool fl=0;
		for(int i=las;i<=12;i++)
		{
			if(cyt[i]<3)continue;
			fl=1;
			tempret=max(tempret,est_dfs(cyt,typ,i+1));
			cyt[i]-=3;
			for(int j=0;j<=14;j++)
			{
				if(cyt[j])
				{
					cyt[j]--;
					tempret=max(tempret,i+3+est_dfs(cyt,typ,i+1)-1);
					if(cyt[j])
					{
						cyt[j]--;
						tempret=max(tempret,i+3+est_dfs(cyt,typ,i+1)-1);
						cyt[j]++;
					}
					cyt[j]++;
				}
			}
			cyt[i]+=3;
			break;
		}
		for(int i=las;i<12;i++)
		{
			if(cyt[i]>=3&&cyt[i+1]>=3)
			{
				cyt[i]-=3,cyt[i+1]-=3;
				for(int j=0;j<=14;j++)
				{
					for(int k=j+1;k<=14;k++)
					{
						if(cyt[j]&&cyt[k]&&i!=j&&i!=k)
						{
							cyt[j]--,cyt[k]--;
							tempret=max(tempret,i+9+est_dfs(cyt,typ,i+2)-1);
							if(cyt[j]&&cyt[k])
							{
								cyt[j]--,cyt[k]--;
								tempret=max(tempret,i+9+est_dfs(cyt,typ,i+2)-1);
								cyt[j]++,cyt[k]++;
							}
							cyt[j]++,cyt[k]++;
						}
					}
				}
				cyt[i]+=3,cyt[i+1]+=3;
				break;
			}
		}
		if(!fl)tempret=max(tempret,est_dfs(cyt,typ+1,0));
	}else if(typ==4)//单顺
	{
		tempret=max(tempret,est_dfs(cyt,typ,las+1));
		bool fl=0;
		for(int i=5;i<=13;i++)
		{
			bool flag=0;
			for(int j=0;j<i;j++)
			{
				if(!cyt[las+j]||las+j>11){flag=1;break;}
			}
			if(flag)break;
			else 
			{
				fl=1;
				for(int j=0;j<i;j++)cyt[las+j]--;
				tempret=max(tempret,i/5+las+est_dfs(cyt,typ,las+1)-1);
				for(int j=0;j<i;j++)cyt[las+j]++;
			}
		}
		if(!fl)tempret=max(tempret,est_dfs(cyt,typ+1,0));
	}else if(typ==5)//双顺
	{
		tempret=max(tempret,est_dfs(cyt,typ,las+1));
		bool fl=0;
		for(int i=3;i<=13;i++)
		{
			bool flag=0;
			for(int j=0;j<i;j++)
			{
				if(cyt[las+j]<2||las+j>11){flag=1;break;}
			}
			if(flag)break;
			else
			{
				fl=1;
				for(int j=0;j<i;j++)cyt[las+j]-=2;
				tempret=max(tempret,i/3+las+est_dfs(cyt,typ,las+1)-1);
				for(int j=0;j<i;j++)cyt[las+j]+=2;
			}
		}
		if(!fl)tempret=max(tempret,est_dfs(cyt,typ+1,0));
	}else if(typ==6)//对
	{
		bool fl=0;
		for(int i=las;i<=12;i++)
		{
			if(cyt[i]<2)continue;
			tempret=max(tempret,est_dfs(cyt,typ,i+1));
			cyt[i]-=2;
			tempret=max(tempret,max((i-10),0)+est_dfs(cyt,typ,i+1)-1);
			cyt[i]+=2;
			fl=1;		
			break;
		}
		if(!fl)tempret=max(tempret,est_dfs(cyt,typ+1,0));
	}else if(typ==7)//单
	{
		int temps=0;
		for(int i=0;i<=14;i++)temps+=max(0,(i-11)*cyt[i]);
		temps-=cnt;
		tempret=max(tempret,temps);
	}
	return tempret;
}
// 用0~53这54个整数表示唯一的一张牌
using Card = short;
constexpr Card card_joker = 52;
constexpr Card card_JOKER = 53;

/* 状态 */

// 我的牌有哪些
set<Card> myCards;

// 地主明示的牌有哪些
set<Card> landlordPublicCards;

// 大家从最开始到现在都出过什么
vector<vector<Card>> whatTheyPlayed[PLAYER_COUNT];

// 当前要出的牌需要大过谁


// 大家还剩多少牌
short cardRemaining[PLAYER_COUNT] = {17, 17, 17};

// 我是几号玩家（0-地主，1-农民甲，2-农民乙）
int myPosition;

// 地主位置
int landlordPosition = -1;

// 地主叫分
int landlordBid = -1;

// 阶段
Stage stage = Stage::BIDDING;

int lastplayer;//上一个玩家是谁

// 自己的第一回合收到的叫分决策
vector<int> bidInput;

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

		bool operator<(const CardPack &b) const
		{
			if (count == b.count)
				return level > b.level;
			return count > b.count;
		}
	};
	vector<Card> cards;		 // 原始的牌，未排序
	vector<CardPack> packs;	 // 按数目和大小排序的牌种
	CardComboType comboType; // 算出的牌型
	Level comboLevel = 0;	 // 算出的大小序

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
				packs.push_back(CardPack{l, counts[l]});
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


	void debugPrint()
	{
#ifndef _BOTZONE_ONLINE
		std::cout << "【" << cardComboStrings[(int)comboType] << "共" << cards.size() << "张，大小序" << comboLevel << "】";
#endif
	}
};
CardCombo lastValidCombo;
bool canBeBeatenBy(const CardCombo &b) 
{
	if (lastValidCombo.comboType == CardComboType::INVALID || b.comboType == CardComboType::INVALID)
		return false;
	if (b.comboType == CardComboType::ROCKET)
		return true;
	if (b.comboType == CardComboType::BOMB)
		switch (lastValidCombo.comboType)
		{
		case CardComboType::ROCKET:
			return false;
		case CardComboType::BOMB:
			return b.comboLevel > lastValidCombo.comboLevel;
		default:
			return true;
		}
	return b.comboType == lastValidCombo.comboType && b.cards.size() == lastValidCombo.cards.size() && b.comboLevel > lastValidCombo.comboLevel;
}

/**
* 从指定手牌中寻找第一个能大过当前牌组的牌组
* 如果随便出的话只出第一张
* 如果不存在则返回一个PASS的牌组
*/
using namespace std;
bool check(int *cyt)//检验是否能一次出完
{
	int cnt=0;
	for(int i=0;i<15;i++)cnt+=cyt[i];
	if(cnt==1)return 1;
	else if(cnt==2)
	{
		for(int i=0;i<15;i++)if(cyt[i]==2)return 1;
		if(cyt[13]&&cyt[14])return 1;
		return 0;
	}else if(cnt==3)
	{
		for(int i=0;i<15;i++)if(cyt[i]==3)return 1;
		return 0;
	}else if(cnt==4)
	{
		for(int i=0;i<15;i++)if(cyt[i]==4||cyt[i]==3)return 1;
	}
	for(int i=0;i<15;i++)
	{
		if(cyt[i]==3)
		{
			if(cyt[i+1]==3)
			{
				for(int j=0;j<15;j++)
				{
					for(int k=j+1;k<15;k++)
					{
						if(cyt[j]==1&&cyt[k]==1&&cnt==8)return 1;
						else if(cyt[j]==2&&cyt[k]==2&&cnt==10)return 1;
					}
				}
			}
			for(int j=0;j<15;j++)if(cyt[j]==2&&cnt==5)return 1;
		}else if(cyt[i]==4)
		{
			for(int j=0;j<15;j++)
			{
				for(int k=j+1;k<15;k++)
				{
					if(cyt[j]==1&&cyt[k]==1&&cnt==6)return 1;
					else if(cyt[j]==2&&cyt[k]==2&&cnt==8)return 1;
				}
			}
		}
	}
	for(int i=0;i<12;i++)
	{
		bool flag=0;
		for(int j=0;j<cnt;j++)
		{
			if(cyt[i+j]!=1||i+j>11){flag=1;break;}
		}
		if(!flag)return 1;
	}
	if(cnt%2==0)
	{
		for(int i=0;i<12;i++)
		{
			bool flag=0;
			for(int j=0;j<cnt/2;j++)
			{
				if(cyt[i+j]!=2||i+j>11){flag=1;break;}
			}
			if(!flag)return 1;
		}
	}
	return 0;
}
template <typename CARD_ITERATOR>
CardCombo find_single_line(int *cyt,CARD_ITERATOR begin, CARD_ITERATOR end,int l=5,int r=13,int las=-1)
{
	set <Card> S;
	bool fl=0;
	int cnt=0;
	for(int i=r;i>=l;i--)//单顺
	{
		for(int st=las+1;st<12;st++)
		{
			bool flag=0;
			for(int j=0;j<i;j++)
			{
				if(!cyt[j+st]||st+j>11||cyt[st+j]==4){flag=1;break;}
				else if(cyt[j+st]==2)cnt++;
			}
			if(flag||cnt>2)continue;
			else 
			{
				fl=1;
				for(int j=0;j<i;j++)
				{
					for(auto it=begin;it!=end;it++)
					{
						if(card2level(*it)==j+st){S.insert(*it);break;}
					}
				}
				break;
			}
		}
		if(fl)break;
	}
	if(fl)
	{
		return CardCombo(S.begin(),S.end());
	}
	return CardCombo();
}
template <typename CARD_ITERATOR>
CardCombo find_double_line(int *cyt,CARD_ITERATOR begin, CARD_ITERATOR end,int l=5,int r=13,int las=-1)
{
	set<Card>S;
	bool fl=0;
	for(int i=r;i>=l;i--)//双顺
	{
		for(int st=las+1;st<11;st++)
		{
			bool flag=0;
			int cnt=0;
			for(int j=0;j<i;j++)
			{
				if(cyt[st+j]<2||cyt[st+j]==4||st+j>11){flag=1;break;}
				else if(cyt[st+j]==3)cnt++;
			}
			if(flag||cnt>1)continue;
			else
			{
				fl=1;
				for(int j=0;j<i;j++)
				{
					int cct=0;
					for(auto it=begin;it!=end;it++)
					{
						if(card2level(*it)==st+j)
						{
							S.insert(*it),cct++;
							if(cct==2)break;
						}
					}
				}
				break;
			}
		}
		if(fl){break;}
	}
	if(fl)
	{
		return CardCombo(S.begin(),S.end());
	}
	return CardCombo();
}
template <typename CARD_ITERATOR>
CardCombo find_plane_one(int *cyt,CARD_ITERATOR begin, CARD_ITERATOR end,int las=-1)
{
	int *ctt=new int[25];
	memcpy(ctt,cyt,sizeof(ctt));
	set<Card>S;
	bool fl=0;
	for(int i=las+1;i<11;i++)//飞机
	{
		if(cyt[i]==3&&cyt[i+1]==3)
		{
			
			ctt[i]-=3,ctt[i+1]-=3;
			CardCombo t1=find_single_line(ctt,begin,end,5,13);
			CardCombo t2=find_double_line(ctt,begin,end,3,10);
			for(int j=0;j<=11;j++)
			{
				for(int k=j+1;k<=11;k++)
				{
					if(cyt[j]==1&&cyt[k]==1&&i!=j&&i!=k)
					{
						ctt[j]--,ctt[k]--;
						CardCombo t3=find_single_line(ctt,begin,end,5,13);
						CardCombo t4=find_double_line(ctt,begin,end,3,10);
						if(t1.comboType!=CardComboType::PASS&&t3.comboType==CardComboType::PASS)
						{
							ctt[j]++,ctt[k]++;
							continue;
						}
						if(t2.comboType!=CardComboType::PASS&&t4.comboType==CardComboType::PASS)
						{
							ctt[j]++,ctt[k]++;
							continue;
						}
						fl=1;
						int cct=0;
						for(auto it=begin;it!=end;it++)
						{
							if(card2level(*it)==i)
							{
								S.insert(*it),cct++;
								if(cct==3)break;
							}
						}
						cct=0;
						for(auto it=begin;it!=end;it++)
						{
							if(card2level(*it)==i+1)
							{
								S.insert(*it),cct++;
								if(cct==3)break;
							}
						}
						cct=0;
						for(auto it=begin;it!=end;it++)
						{
							if(card2level(*it)==j)
							{
								S.insert(*it),cct++;
								if(cct==1)break;
							}
						}
						cct=0;
						for(auto it=begin;it!=end;it++)
						{
							if(card2level(*it)==k)
							{
								S.insert(*it),cct++;
								if(cct==1)break;
							}
						}
						return CardCombo(S.begin(),S.end());
					}
				}
			}
			if(!fl)
			{
				for(int j=0;j<=14;j++)
				{
					for(int k=j+1;k<=14;k++)
					{
						if(cyt[j]<3&&cyt[k]<3&&cyt[j]&&cyt[k]&&i!=j&&i!=k)
						{
							
							ctt[j]--,ctt[k]--;
							CardCombo t3=find_single_line(ctt,begin,end);
							CardCombo t4=find_double_line(ctt,begin,end);
							if(t1.comboType!=CardComboType::PASS&&t3.comboType==CardComboType::PASS)
							{
								ctt[j]++,ctt[k]++;
								continue;
							}
							if(t2.comboType!=CardComboType::PASS&&t4.comboType==CardComboType::PASS)
							{
								ctt[j]++,ctt[k]++;
								continue;
							}
							fl=1;
							int cct=0;
							for(auto it=begin;it!=end;it++)
							{
								if(card2level(*it)==i)
								{
									S.insert(*it),cct++;
									if(cct==3)break;
								}
							}
							cct=0;
							for(auto it=begin;it!=end;it++)
							{
								if(card2level(*it)==i+1)
								{
									S.insert(*it),cct++;
									if(cct==3)break;
								}
							}
							cct=0;
							for(auto it=begin;it!=end;it++)
							{
								if(card2level(*it)==j)
								{
									S.insert(*it),cct++;
									if(cct==1)break;
								}
							}
							cct=0;
							for(auto it=begin;it!=end;it++)
							{
								if(card2level(*it)==k)
								{
									S.insert(*it),cct++;
									if(cct==1)break;
								}
							}
							return CardCombo(S.begin(),S.end());
						}
					}
				}
			}
			if(fl)break;
		}
		if(fl)return CardCombo(S.begin(),S.end());
	}
	return CardCombo();
}
template <typename CARD_ITERATOR>
CardCombo find_plane_two(int *cyt,CARD_ITERATOR begin, CARD_ITERATOR end,int las=-1)
{
	int *ctt=new int[25];
	memcpy(ctt,cyt,sizeof(ctt));
	set<Card>S;
	bool fl=0;
	for(int i=las+1;i<11;i++)//飞机
	{
		if(cyt[i]==3&&cyt[i+1]==3)
		{
			ctt[i]-=3,ctt[i+1]-=3;
			CardCombo t1=find_single_line(ctt,begin,end,5,13);
			CardCombo t2=find_double_line(ctt,begin,end,3,10);
			for(int j=0;j<=11;j++)
			{
				for(int k=j+1;k<=11;k++)
				{
					if(cyt[j]==2&&cyt[k]==2&&i!=j&&i!=k)
					{
						ctt[j]--,ctt[k]--;
						CardCombo t3=find_single_line(ctt,begin,end,5,13);
						CardCombo t4=find_double_line(ctt,begin,end,3,10);
						if(t1.comboType!=CardComboType::PASS&&t3.comboType==CardComboType::PASS)
						{
							ctt[j]+=2,ctt[k]+=2;
							continue;
						}
						if(t2.comboType!=CardComboType::PASS&&t4.comboType==CardComboType::PASS)
						{
							ctt[j]+=2,ctt[k]+=2;
							continue;
						}
						fl=1;
						int cct=0;
						for(auto it=begin;it!=end;it++)
						{
							if(card2level(*it)==i)
							{
								S.insert(*it),cct++;
								if(cct==3)break;
							}
						}
						cct=0;
						for(auto it=begin;it!=end;it++)
						{
							if(card2level(*it)==i+1)
							{
								S.insert(*it),cct++;
								if(cct==3)break;
							}
						}
						cct=0;
						for(auto it=begin;it!=end;it++)
						{
							if(card2level(*it)==j)
							{
								S.insert(*it),cct++;
								if(cct==2)break;
							}
						}
						cct=0;
						for(auto it=begin;it!=end;it++)
						{
							if(card2level(*it)==k)
							{
								S.insert(*it),cct++;
								if(cct==2)break;
							}
						}
					}
				}
			}
			if(fl)break;
		}
		if(fl)return CardCombo(S.begin(),S.end());
	}
	return CardCombo();
}

template <typename CARD_ITERATOR>
CardCombo find_three_two(int *cyt,CARD_ITERATOR begin, CARD_ITERATOR end,int las=-1)
{
	bool fl=0;
	set<Card>S;
	int *ctt=new int[25];
	memcpy(ctt,cyt,sizeof(ctt));
	for(int i=las+1;i<12;i++)//三带
	{
		if(cyt[i]!=3)continue;
		ctt[i]-=3;
		CardCombo t1=find_single_line(ctt,begin,end,5,13);
		CardCombo t2=find_double_line(ctt,begin,end,3,10);
		for(int j=0;j<=11;j++)
		{
			if(cyt[j]==2&&i!=j)
			{
				ctt[j]-=2;
				CardCombo t3=find_single_line(ctt,begin,end,5,13);
				CardCombo t4=find_double_line(ctt,begin,end,3,10);
				if(t1.comboType!=CardComboType::PASS&&t3.comboType==CardComboType::PASS)
				{
					ctt[j]+=2;
					continue;
				}
				if(t2.comboType!=CardComboType::PASS&&t4.comboType==CardComboType::PASS)
				{
					ctt[j]+=2;
					continue;
				}
				int cct=0;
				fl=1;
				for(auto it=begin;it!=end;it++)
				{
					if(card2level(*it)==i)
					{
						S.insert(*it),cct++;
						if(cct==3)break;
					}
				}
				cct=0;
				for(auto it=begin;it!=end;it++)
				{
					if(card2level(*it)==j)
					{
						S.insert(*it),cct++;
						if(cct==2)break;
					}
				}
				return CardCombo(S.begin(),S.end());
			}
		}
		if(fl)break;
	}
	if(fl)
	{
		return CardCombo(S.begin(),S.end());
	}
	return CardCombo();
}
template <typename CARD_ITERATOR>
CardCombo find_three_one(int *cyt,CARD_ITERATOR begin, CARD_ITERATOR end,int las=-1)
{
	bool fl=0;
	set<Card>S;
	int *ctt=new int[25];
	memcpy(ctt,cyt,sizeof(ctt));
	for(int i=las+1;i<12;i++)//三带
	{
		if(cyt[i]!=3)continue;
		ctt[i]-=3;
		CardCombo t1=find_single_line(ctt,begin,end,5,13);
		CardCombo t2=find_double_line(ctt,begin,end,3,10);
		for(int j=0;j<=11;j++)
		{
			if(cyt[j]==1&&i!=j)
			{
				ctt[j]--;
				CardCombo t3=find_single_line(ctt,begin,end,5,13);
				CardCombo t4=find_double_line(ctt,begin,end,3,10);
				if(t1.comboType!=CardComboType::PASS&&t3.comboType==CardComboType::PASS)
				{
					ctt[j]++;
					continue;
				}
				if(t2.comboType!=CardComboType::PASS&&t4.comboType==CardComboType::PASS)
				{
					ctt[j]++;
					continue;
				}
				fl=1;
				int cct=0;
				for(auto it=begin;it!=end;it++)
				{
					if(card2level(*it)==i)
					{
						S.insert(*it),cct++;
						if(cct==3)break;
					}
				}
				cct=0;
				for(auto it=begin;it!=end;it++)
				{
					if(card2level(*it)==j)
					{
						S.insert(*it),cct++;
						if(cct==1)break;
					}
				}
				return CardCombo(S.begin(),S.end());
			}
			if(fl)break;
		}
		if(!fl)
		{
			for(int j=0;j<=14;j++)
			{
				if(cyt[j]<3&&i!=j&&cyt[j])
				{
					ctt[j]--;
					CardCombo t3=find_single_line(ctt,begin,end,5,13);
					CardCombo t4=find_double_line(ctt,begin,end,3,10);
					if(t1.comboType!=CardComboType::PASS&&t3.comboType==CardComboType::PASS)
					{
						ctt[j]++;
						continue;
					}
					if(t2.comboType!=CardComboType::PASS&&t4.comboType==CardComboType::PASS)
					{
						ctt[j]++;
						continue;
					}
					fl=1;
					int cct=0;
					for(auto it=begin;it!=end;it++)
					{
						if(card2level(*it)==i)
						{
							S.insert(*it),cct++;
							if(cct==3)break;
						}
					}
					cct=0;
					for(auto it=begin;it!=end;it++)
					{
						if(card2level(*it)==j)
						{
							S.insert(*it),cct++;
							if(cct==1)break;
						}
					}
					return CardCombo(S.begin(),S.end());
				}
				if(fl)break;
			}
		}
	}
	if(fl)
	{
		return CardCombo(S.begin(),S.end());
	}
	return CardCombo();
}
template <typename CARD_ITERATOR>
CardCombo find_bomb(int *cyt,CARD_ITERATOR begin, CARD_ITERATOR end,int las=-1)
{
	set<Card>S;
	bool fl=0;
	for(int i=las+1;i<=12;i++)
	{
		if(cyt[i]==4)
		{
			fl=1;
			for(auto it=begin;it!=end;it++)
			{
				if(card2level(*it)==i)S.insert(*it);
			}
			break;
		}
	}
	if(!fl&&cyt[13]&&cyt[14])
	{
		fl=1;
		for(auto it=begin;it!=end;it++)
		{
			if(card2level(*it)==13||card2level(*it)==14)S.insert(*it);
		}
	}
	if(fl)return CardCombo(S.begin(),S.end());
	return CardCombo();
}
template <typename CARD_ITERATOR>
CardCombo find_plane(int *cyt,CARD_ITERATOR begin, CARD_ITERATOR end,int las=-1)
{
	set<Card>S;
	bool fl=0;
	for(int i=las+1;i<11;i++)
	{
		if(cyt[i]!=3||cyt[i+1]!=3)continue;
		int cct=0;
		fl=1;
		for(auto it=begin;it!=end;it++)
		{
			if(card2level(*it)==i)
			{
				S.insert(*it),cct++;
				if(cct==3)break;
			}
		}
		cct=0;
		for(auto it=begin;it!=end;it++)
		{
			if(card2level(*it)==i+1)
			{
				S.insert(*it),cct++;
				if(cct==3)break;
			}
		}
		return CardCombo(S.begin(),S.end());
	}
	return CardCombo();
}
template <typename CARD_ITERATOR>
CardCombo find_four_one(int *cyt,CARD_ITERATOR begin, CARD_ITERATOR end,int las=-1)
{
	int *ctt=new int[25];
	memcpy(ctt,cyt,sizeof(ctt));
	set<Card>S;
	bool fl=0;
	for(int i=las+1;i<=11;i++)//飞机
	{
		if(cyt[i]<4)continue;
		ctt[i]-=4;
		CardCombo t1=find_single_line(ctt,begin,end,5,13);
		CardCombo t2=find_double_line(ctt,begin,end,3,10);
		for(int j=0;j<=11;j++)
		{
			for(int k=j+1;k<=11;k++)
			{
				if(cyt[j]==1&&cyt[k]==1)
				{
					ctt[j]--,ctt[k]--;
					CardCombo t3=find_single_line(ctt,begin,end,5,13);
					CardCombo t4=find_double_line(ctt,begin,end,3,10);
					if(t1.comboType!=CardComboType::PASS&&t3.comboType==CardComboType::PASS&&t1.packs[0].level<4)
					{
						ctt[j]++,ctt[k]++;
						continue;
					}
					if(t2.comboType!=CardComboType::PASS&&t4.comboType==CardComboType::PASS&&t2.packs.size()>=5)
					{
						ctt[j]++,ctt[k]++;
						continue;
					}
					fl=1;
					int cct=0;
					for(auto it=begin;it!=end;it++)
					{
						if(card2level(*it)==i)
						{
							S.insert(*it),cct++;
							if(cct==4)break;
						}
					}
					cct=0;
					for(auto it=begin;it!=end;it++)
					{
						if(card2level(*it)==j)
						{
							S.insert(*it),cct++;
							if(cct==1)break;
						}
					}
					cct=0;
					for(auto it=begin;it!=end;it++)
					{
						if(card2level(*it)==k)
						{
							S.insert(*it),cct++;
							if(cct==1)break;
						}
					}
					return CardCombo(S.begin(),S.end());
				}
			}
		}
		if(!fl)
		{
			for(int j=0;j<=14;j++)
			{
				for(int k=j+1;k<=14;k++)
				{
					if(cyt[j]<3&&cyt[k]<3&&cyt[j]&&cyt[k]&&i!=j&&i!=k)
					{
						ctt[j]--,ctt[k]--;
						CardCombo t3=find_single_line(ctt,begin,end);
						CardCombo t4=find_double_line(ctt,begin,end);
						if(t1.comboType!=CardComboType::PASS&&t3.comboType==CardComboType::PASS&&t1.packs[0].level<4)
						{
							ctt[j]++,ctt[k]++;
							continue;
						}
						if(t2.comboType!=CardComboType::PASS&&t4.comboType==CardComboType::PASS&&t2.packs.size()>=5)
						{
							ctt[j]++,ctt[k]++;
							continue;
						}
						fl=1;
						int cct=0;
						for(auto it=begin;it!=end;it++)
						{
							if(card2level(*it)==i)
							{
								S.insert(*it),cct++;
								if(cct==4)break;
							}
						}
						cct=0;
						for(auto it=begin;it!=end;it++)
						{
							if(card2level(*it)==j)
							{
								S.insert(*it),cct++;
								if(cct==1)break;
							}
						}
						cct=0;
						for(auto it=begin;it!=end;it++)
						{
							if(card2level(*it)==k)
							{
								S.insert(*it),cct++;
								if(cct==1)break;
							}
						}
						return CardCombo(S.begin(),S.end());
					}
				}
			}
		}
		if(fl)return CardCombo(S.begin(),S.end());
	}
	return CardCombo();
}
template <typename CARD_ITERATOR>
CardCombo find_four_two(int *cyt,CARD_ITERATOR begin, CARD_ITERATOR end,int las=-1)
{
	int *ctt=new int[25];
	memcpy(ctt,cyt,sizeof(ctt));
	set<Card>S;
	bool fl=0;
	for(int i=las+1;i<=11;i++)//飞机
	{
		if(cyt[i]<4)continue;
		ctt[i]-=4;
		CardCombo t1=find_single_line(ctt,begin,end,5,13);
		CardCombo t2=find_double_line(ctt,begin,end,3,10);
		for(int j=0;j<=11;j++)
		{
			for(int k=j+1;k<=11;k++)
			{
				if(cyt[j]==2&&cyt[k]==2)
				{
					ctt[j]-=2,ctt[k]-=2;
					CardCombo t3=find_single_line(ctt,begin,end,5,13);
					CardCombo t4=find_double_line(ctt,begin,end,3,10);
					if(t1.comboType!=CardComboType::PASS&&t3.comboType==CardComboType::PASS&&t1.packs[0].level<4)
					{
						ctt[j]+=2,ctt[k]+=2;
						continue;
					}
					if(t2.comboType!=CardComboType::PASS&&t4.comboType==CardComboType::PASS&&t2.packs.size()>=5)
					{
						ctt[j]+=2,ctt[k]+=2;
						continue;
					}
					fl=1;
					int cct=0;
					for(auto it=begin;it!=end;it++)
					{
						if(card2level(*it)==i)
						{
							S.insert(*it),cct++;
							if(cct==4)break;
						}
					}
					cct=0;
					for(auto it=begin;it!=end;it++)
					{
						if(card2level(*it)==j)
						{
							S.insert(*it),cct++;
							if(cct==2)break;
						}
					}
					cct=0;
					for(auto it=begin;it!=end;it++)
					{
						if(card2level(*it)==k)
						{
							S.insert(*it),cct++;
							if(cct==2)break;
						}
					}
				}
			}
		}
		if(fl)return CardCombo(S.begin(),S.end());
	}
	return CardCombo();
}
template <typename CARD_ITERATOR>
CardCombo find_single(int *cyt,CARD_ITERATOR begin, CARD_ITERATOR end,int las=-1)
{
	CardCombo t1=find_single_line(cyt,begin,end,5,13);
	set<Card>S;
	for(int i=las+1;i<=min(las+6,14);i++)
	{
		if(!cyt[i]||cyt[i]>=3)continue;
		if(cyt[i]==1)
		{
			bool fl=0;
			for(auto j:t1.packs)
			{
				if(i==j.level){fl=1;break;}
			}
			if(!fl)
			{
				for(auto j=begin;j!=end;j++)
				{
					if(card2level(*j)==i){S.insert(*j);break;}
				}
				return CardCombo(S.begin(),S.end());
			}
		}
	}
	int mid=(las+1+min(las+6,14))>>1;
	for(int i=mid;i<=min(las+6,14);i++)
	{
		if(!cyt[i]||cyt[i]>=3)continue;
		if(cyt[i])
		{
			for(auto j=begin;j!=end;j++)
			{
				if(card2level(*j)==i){S.insert(*j);break;}
			}
			return CardCombo(S.begin(),S.end());
		}
	}
	return CardCombo();
}
template <typename CARD_ITERATOR>
CardCombo find_double(int *cyt,CARD_ITERATOR begin, CARD_ITERATOR end,int las=-1)
{
	CardCombo t1=find_double_line(cyt,begin,end,3,10);
	CardCombo t2=find_single_line(cyt,begin,end,5,13);
	set<Card>S;
	for(int i=las+1;i<=min(las+6,12);i++)
	{
		if(cyt[i]==2)
		{
			bool fl=0;
			for(auto j:t1.packs)
			{
				if(i==j.level){fl=1;break;}
			}
			for(auto j:t2.packs)
			{
				if(i==j.level){fl=1;break;}
			}
			if(!fl)
			{
				int cct=0;
				for(auto j=begin;j!=end;j++)
				{
					if(card2level(*j)==i)
					{
						S.insert(*j);
						cct++;
						if(cct==2)break;
					}
				}
				return CardCombo(S.begin(),S.end());
			}
		}
	}
	if(rand()%2)return CardCombo();
	int mid=(las+1+min(las+6,14))>>1;
	for(int i=mid;i<=min(las+6,12);i++)
	{
		if(cyt[i]==3)
		{
			int cct=0;
			for(auto j=begin;j!=end;j++)
			{
				if(card2level(*j)==i)
				{
					S.insert(*j);
					cct++;
					if(cct==2)break;
				}
			}
			return CardCombo(S.begin(),S.end());
		}
	}
	return CardCombo();
}
template <typename CARD_ITERATOR>
CardCombo findFirstValid(CARD_ITERATOR begin, CARD_ITERATOR end)
{
	
	int *cyt=new int[25];
	for(int i=0;i<=20;i++)cyt[i]=0;
	for(auto it=begin;it!=end;it++){cyt[card2level(*it)]++;}
	if (lastValidCombo.comboType == CardComboType::PASS) // 如果不需要大过谁，只需要随便出
	{
		if(check(cyt))
		{
			return CardCombo(begin,end);
		}else
		{
			bool fl=0;
			set<Card>S;
			CardCombo ret=find_plane_one(cyt,begin,end,-1);
			if(ret.comboType!=CardComboType::PASS)return ret;
			ret=find_plane_two(cyt,begin,end,-1);
			if(ret.comboType!=CardComboType::PASS)return ret;
			ret=find_single_line(cyt,begin,end,5,13);
			if(ret.comboType!=CardComboType::PASS)return ret;
			ret=find_double_line(cyt,begin,end,3,13);
			if(ret.comboType!=CardComboType::PASS)return ret;
			ret=find_three_one(cyt,begin,end,-1);
			if(ret.comboType!=CardComboType::PASS)return ret;
			ret=find_three_two(cyt,begin,end,-1);
			if(ret.comboType!=CardComboType::PASS)return ret;
			ret=find_four_one(cyt,begin,end,-1);
			if(ret.comboType!=CardComboType::PASS)return ret;
			ret=find_four_two(cyt,begin,end,-1);
			if(ret.comboType!=CardComboType::PASS)return ret;
			for(int i=0;i<=14;i++)
			{
				if(cyt[i]==2)
				{
					int cct=0;
					for(auto it=begin;it!=end;it++)
					{
						if(card2level(*it)==i)S.insert(*it),cct++;
						if(cct==2)break;
					}
					return CardCombo(S.begin(),S.end());
				}else if(cyt[i]==1)
				{
					for(auto it=begin;it!=end;it++)
					{
						if(card2level(*it)==i){S.insert(*it);break;}
					}
					return CardCombo(S.begin(),S.end());
				}
			}
			for(int i=0;i<=12;i++)
			{
				if(cyt[i]==4)
				{
					int cct=0;
					for(auto it=begin;it!=end;it++)
					{
						if(card2level(*it)==i)S.insert(*it),cct++;
						if(cct==4)break;
					}
				}
			}
			return CardCombo(S.begin(),S.end());
		}
	}
	// 然后先看一下是不是火箭，是的话就过
	if (lastValidCombo.comboType == CardComboType::ROCKET)
		return CardCombo();
	
	
	
	CardCombo ret=CardCombo();
	int p1=0,p2=0;
	for(int i=0;i<3;i++)
	{
		if(!p1&&i!=landlordPosition)p1=i;
		else if(!p2&&i!=landlordPosition)p2=i;
	}
	if(myPosition==landlordPosition)
	{
		if(lastValidCombo.comboType==CardComboType::STRAIGHT)
		{
			ret=find_single_line(cyt,begin,end,lastValidCombo.packs.size(),lastValidCombo.packs.size(),lastValidCombo.packs[lastValidCombo.packs.size()-1].level);
			if(ret.comboType==CardComboType::PASS)
			{
				if(cardRemaining[p1]<=5||cardRemaining[p2]<=5)
				{
					return find_bomb(cyt,begin,end,-1);
				}
			}//else return ret;
		}else if(lastValidCombo.comboType==CardComboType::STRAIGHT2)
		{
			ret=find_double_line(cyt,begin,end,lastValidCombo.packs.size(),lastValidCombo.packs.size(),lastValidCombo.packs[lastValidCombo.packs.size()-1].level);
			if(ret.comboType==CardComboType::PASS)
			{
				if(cardRemaining[p1]<=5||cardRemaining[p2]<=5)
				{
					return find_bomb(cyt,begin,end,-1);
				}
			}//else return ret;
		}else if(lastValidCombo.comboType==CardComboType::TRIPLET1)
		{
			ret=find_three_one(cyt,begin,end,lastValidCombo.packs[0].level);
			if(ret.comboType==CardComboType::PASS)
			{
				if(cardRemaining[p1]<=5||cardRemaining[p2]<=5)
				{
					return find_bomb(cyt,begin,end,-1);
				}
			}//else return ret;
		}else if(lastValidCombo.comboType==CardComboType::TRIPLET2)
		{
			ret=find_three_two(cyt,begin,end,lastValidCombo.packs[0].level);
			if(ret.comboType==CardComboType::PASS)
			{
				if(cardRemaining[p1]<=5||cardRemaining[p2]<=5)
				{
					return find_bomb(cyt,begin,end,-1);
				}
			}//else return ret;
		}else if(lastValidCombo.comboType==CardComboType::BOMB)
		{
			CardCombo ret=find_bomb(cyt,begin,end,lastValidCombo.packs[0].level);
			//return ret;
		}else if(lastValidCombo.comboType==CardComboType::PLANE1)
		{
			ret=find_plane_one(cyt,begin,end,lastValidCombo.packs[0].level);
			if(ret.comboType==CardComboType::PASS)
			{
				if(cardRemaining[p1]<=5||cardRemaining[p2]<=5)
				{
					return find_bomb(cyt,begin,end,-1);
				}
			}//else return ret;
		}else if(lastValidCombo.comboType==CardComboType::PLANE2)
		{
			ret=find_plane_two(cyt,begin,end,lastValidCombo.packs[0].level);
			if(ret.comboType==CardComboType::PASS)
			{
				if(cardRemaining[p1]<=5||cardRemaining[p2]<=5)
				{
					return find_bomb(cyt,begin,end,-1);
				}
			}//else return ret;
		}else if(lastValidCombo.comboType==CardComboType::QUADRUPLE2)
		{
			ret=find_four_one(cyt,begin,end,lastValidCombo.packs[0].level);
			if(ret.comboType==CardComboType::PASS)
			{
				if(cardRemaining[p1]<=5||cardRemaining[p2]<=5)
				{
					return find_bomb(cyt,begin,end,-1);
				}
			}//else return ret;
		}else if(lastValidCombo.comboType==CardComboType::QUADRUPLE4)
		{
			ret=find_four_two(cyt,begin,end,lastValidCombo.packs[0].level);
			if(ret.comboType==CardComboType::PASS)
			{
				if(cardRemaining[p1]<=5||cardRemaining[p2]<=5)
				{
					return find_bomb(cyt,begin,end,-1);
				}
			}//else return ret;
		}else if(lastValidCombo.comboType==CardComboType::SINGLE)
		{
			int las=lastValidCombo.packs[0].level;
			ret=find_single(cyt,begin,end,las);
			set<Card>S;
			if(ret.comboType==CardComboType::PASS)
			{
				if(las==13)
				{
					if(cyt[14])
					{
						S.insert(53);
						return CardCombo(S.begin(),S.end());
					}
				}else if(las==12)
				{
					if(cyt[13])
					{
						S.insert(52);
						return CardCombo(S.begin(),S.end());
					}
				}else if(cardRemaining[p1]<=5||cardRemaining[p2]<=5)
				{
					return find_bomb(cyt,begin,end,-1);
				}
			}//else return ret;
		}else if(lastValidCombo.comboType==CardComboType::PAIR)
		{
			ret=find_double(cyt,begin,end,lastValidCombo.packs[0].level);
			if(ret.comboType==CardComboType::PASS)
			{
				if(cardRemaining[p1]<=5||cardRemaining[p2]<=5)
				{
					return find_bomb(cyt,begin,end,-1);
				}
			}//else return ret;
		}
	}else if(lastplayer!=landlordPosition&&(myPosition+1)%3==landlordPosition)
	{
		if(cardRemaining[landlordPosition]<lastValidCombo.cards.size())//地主没有威胁
		{
			s=clock();
			int *ctt=new int[25];
			memcpy(ctt,cyt,sizeof(ctt));
			if(est_dfs(ctt,1,0)<10||lastValidCombo.packs[0].level>9)return CardCombo();
			if(lastValidCombo.comboType==CardComboType::STRAIGHT)
			{
				ret=find_single_line(cyt,begin,end,lastValidCombo.packs.size(),lastValidCombo.packs.size(),lastValidCombo.packs[lastValidCombo.packs.size()-1].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::STRAIGHT2)
			{
				ret=find_double_line(cyt,begin,end,lastValidCombo.packs.size(),lastValidCombo.packs.size(),lastValidCombo.packs[lastValidCombo.packs.size()-1].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::TRIPLET1)
			{
				ret=find_three_one(cyt,begin,end,lastValidCombo.packs[0].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::TRIPLET2)
			{
				ret=find_three_two(cyt,begin,end,lastValidCombo.packs[0].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::BOMB)
			{
				//return CardCombo();
			}else if(lastValidCombo.comboType==CardComboType::PLANE1)
			{
				ret=find_plane_one(cyt,begin,end,lastValidCombo.packs[0].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::PLANE2)
			{
				ret=find_plane_two(cyt,begin,end,lastValidCombo.packs[0].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::QUADRUPLE2)
			{
				ret=find_four_one(cyt,begin,end,lastValidCombo.packs[0].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::QUADRUPLE4)
			{
				ret=find_four_two(cyt,begin,end,lastValidCombo.packs[0].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::SINGLE)
			{
				int las=lastValidCombo.packs[0].level;
				ret=find_single(cyt,begin,end,las);
				set<Card>S;
				if(ret.comboType==CardComboType::PASS&&rand()%2)
				{
					if(las==13)
					{
						if(cyt[14])
						{
							S.insert(53);
							return CardCombo(S.begin(),S.end());
						}
					}else if(las==12)
					{
						if(cyt[13])
						{
							S.insert(52);
							return CardCombo(S.begin(),S.end());
						}
					}else return CardCombo();
				}//else return ret;
			}else if(lastValidCombo.comboType==CardComboType::PAIR)
			{
				ret=find_double(cyt,begin,end,lastValidCombo.packs[0].level);
				//return ret;
			}
		}else//地主有威胁
		{
			s=clock();
			int *ctt=new int[25];
			memcpy(ctt,cyt,sizeof(ctt));
			if(lastValidCombo.packs[0].level>9)return CardCombo();//不需要垫
			if(est_dfs(ctt,1,0)>10)//正常出牌
			{

				if(lastValidCombo.comboType==CardComboType::STRAIGHT)
				{
					ret=find_single_line(cyt,begin,end,lastValidCombo.packs.size(),lastValidCombo.packs.size(),lastValidCombo.packs[lastValidCombo.packs.size()-1].level);
					//return ret;
				}else if(lastValidCombo.comboType==CardComboType::STRAIGHT2)
				{
					ret=find_double_line(cyt,begin,end,lastValidCombo.packs.size(),lastValidCombo.packs.size(),lastValidCombo.packs[lastValidCombo.packs.size()-1].level);
					//return ret;
				}else if(lastValidCombo.comboType==CardComboType::TRIPLET1)
				{
					ret=find_three_one(cyt,begin,end,lastValidCombo.packs[0].level);
					//return ret;
				}else if(lastValidCombo.comboType==CardComboType::TRIPLET2)
				{
					ret=find_three_two(cyt,begin,end,lastValidCombo.packs[0].level);
					//return ret;
				}else if(lastValidCombo.comboType==CardComboType::BOMB)
				{
					return CardCombo();
				}else if(lastValidCombo.comboType==CardComboType::PLANE1)
				{
					ret=find_plane_one(cyt,begin,end,lastValidCombo.packs[0].level);
					//return ret;
				}else if(lastValidCombo.comboType==CardComboType::PLANE2)
				{
					ret=find_plane_two(cyt,begin,end,lastValidCombo.packs[0].level);
					//return ret;
				}else if(lastValidCombo.comboType==CardComboType::QUADRUPLE2)
				{
					ret=find_four_one(cyt,begin,end,lastValidCombo.packs[0].level);
					//return ret;
				}else if(lastValidCombo.comboType==CardComboType::QUADRUPLE4)
				{
					ret=find_four_two(cyt,begin,end,lastValidCombo.packs[0].level);
					//return ret;
				}else if(lastValidCombo.comboType==CardComboType::SINGLE)
				{
					int las=lastValidCombo.packs[0].level;
					ret=find_single(cyt,begin,end,las);
					set<Card>S;
					if(ret.comboType==CardComboType::PASS)
					{
						if(las==13)
						{
							if(cyt[14])
							{
								S.insert(53);
								return CardCombo(S.begin(),S.end());
							}
						}else if(las==12)
						{
							if(cyt[13])
							{
								S.insert(52);
								return CardCombo(S.begin(),S.end());
							}
						}else return CardCombo();
					}//else return ret;
				}else if(lastValidCombo.comboType==CardComboType::PAIR)
				{
					ret=find_double(cyt,begin,end,lastValidCombo.packs[0].level);
					//return ret;
				}
			}else
			{
				if(lastValidCombo.comboType==CardComboType::STRAIGHT)
				{
					CardCombo ret=find_single_line(cyt,begin,end,lastValidCombo.packs.size(),lastValidCombo.packs.size(),lastValidCombo.packs[lastValidCombo.packs.size()-1].level);
					//return ret;
				}else if(lastValidCombo.comboType==CardComboType::STRAIGHT2)
				{
					CardCombo ret=find_double_line(cyt,begin,end,lastValidCombo.packs.size(),lastValidCombo.packs.size(),lastValidCombo.packs[lastValidCombo.packs.size()-1].level);
					//return ret;
				}else if(lastValidCombo.comboType==CardComboType::TRIPLET1)
				{
					CardCombo ret=find_three_one(cyt,begin,end,lastValidCombo.packs[0].level);
					//return ret;
				}else if(lastValidCombo.comboType==CardComboType::TRIPLET2)
				{
					CardCombo ret=find_three_two(cyt,begin,end,lastValidCombo.packs[0].level);
					//return ret;
				}else if(lastValidCombo.comboType==CardComboType::BOMB)
				{
					return CardCombo();
				}else if(lastValidCombo.comboType==CardComboType::PLANE1)
				{
					return CardCombo();
				}else if(lastValidCombo.comboType==CardComboType::PLANE2)
				{
					return CardCombo();
				}else if(lastValidCombo.comboType==CardComboType::QUADRUPLE2)
				{
					return CardCombo();
				}else if(lastValidCombo.comboType==CardComboType::QUADRUPLE4)
				{
					return CardCombo();
				}else if(lastValidCombo.comboType==CardComboType::SINGLE)
				{
					int las=lastValidCombo.packs[0].level+5;//尝试垫牌
					CardCombo ret=find_single(cyt,begin,end,las);
					set<Card>S;
					if(ret.comboType==CardComboType::PASS)
					{
						las=lastValidCombo.packs[0].level;
						ret=find_single(cyt,begin,end,las);
						//return ret;
					}//else return ret;
				}else if(lastValidCombo.comboType==CardComboType::PAIR)
				{
					CardCombo ret=find_double(cyt,begin,end,lastValidCombo.packs[0].level+5);
					if(ret.comboType==CardComboType::PASS)
					{
						ret=find_double(cyt,begin,end,lastValidCombo.packs[0].level);
					}
					//return ret;
				}
			}
		}
	}else if(lastplayer!=landlordPosition&&(myPosition+1)%3!=landlordPosition)
	{
		s=clock();
		int *ctt=new int[25];
		memcpy(ctt,cyt,sizeof(ctt));
		if(lastValidCombo.packs[0].level>9)return CardCombo();//不需要垫
		if(est_dfs(ctt,1,0)>10)//正常出牌
		{
			if(lastValidCombo.comboType==CardComboType::STRAIGHT)
			{
				ret=find_single_line(cyt,begin,end,lastValidCombo.packs.size(),lastValidCombo.packs.size(),lastValidCombo.packs[lastValidCombo.packs.size()-1].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::STRAIGHT2)
			{
				ret=find_double_line(cyt,begin,end,lastValidCombo.packs.size(),lastValidCombo.packs.size(),lastValidCombo.packs[lastValidCombo.packs.size()-1].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::TRIPLET1)
			{
				ret=find_three_one(cyt,begin,end,lastValidCombo.packs[0].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::TRIPLET2)
			{
				ret=find_three_two(cyt,begin,end,lastValidCombo.packs[0].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::BOMB)
			{
				return CardCombo();
			}else if(lastValidCombo.comboType==CardComboType::PLANE1)
			{
				ret=find_plane_one(cyt,begin,end,lastValidCombo.packs[0].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::PLANE2)
			{
				ret=find_plane_two(cyt,begin,end,lastValidCombo.packs[0].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::QUADRUPLE2)
			{
				return CardCombo();
			}else if(lastValidCombo.comboType==CardComboType::QUADRUPLE4)
			{
				return CardCombo();
			}else if(lastValidCombo.comboType==CardComboType::SINGLE)
			{
				int las=lastValidCombo.packs[0].level;
				ret=find_single(cyt,begin,end,las);
				set<Card>S;
				if(ret.comboType==CardComboType::PASS&&rand()%2)
				{
					if(las==13)
					{
						if(cyt[14])
						{
							S.insert(53);
							return CardCombo(S.begin(),S.end());
						}
					}else if(las==12)
					{
						if(cyt[13])
						{
							S.insert(52);
							return CardCombo(S.begin(),S.end());
						}
					}else return CardCombo();
				}//else return ret;
			}else if(lastValidCombo.comboType==CardComboType::PAIR)
			{
				ret=find_double(cyt,begin,end,lastValidCombo.packs[0].level);
				//return ret;
			}
		}else return CardCombo();
	}else if((myPosition+1)%3==landlordPosition)
	{
		s=clock();
		int *ctt=new int[25];
		memcpy(ctt,cyt,sizeof(ctt));
		if(est_dfs(ctt,1,0)>10)//正常出牌
		{
			if(lastValidCombo.comboType==CardComboType::STRAIGHT)
			{
				ret=find_single_line(cyt,begin,end,lastValidCombo.packs.size(),lastValidCombo.packs.size(),lastValidCombo.packs[lastValidCombo.packs.size()-1].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::STRAIGHT2)
			{
				ret=find_double_line(cyt,begin,end,lastValidCombo.packs.size(),lastValidCombo.packs.size(),lastValidCombo.packs[lastValidCombo.packs.size()-1].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::TRIPLET1)
			{
				ret=find_three_one(cyt,begin,end,lastValidCombo.packs[0].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::TRIPLET2)
			{
				ret=find_three_two(cyt,begin,end,lastValidCombo.packs[0].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::BOMB)
			{
				ret=find_bomb(cyt,begin,end,lastValidCombo.packs[0].level);
				return CardCombo();
			}else if(lastValidCombo.comboType==CardComboType::PLANE1)
			{
				ret=find_plane_one(cyt,begin,end,lastValidCombo.packs[0].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::PLANE2)
			{
				ret=find_plane_two(cyt,begin,end,lastValidCombo.packs[0].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::QUADRUPLE2)
			{
				ret=find_four_one(cyt,begin,end,lastValidCombo.packs[0].level);
				////return ret;
			}else if(lastValidCombo.comboType==CardComboType::QUADRUPLE4)
			{
				ret=find_four_two(cyt,begin,end,lastValidCombo.packs[0].level);
				//return ret;
			}else if(lastValidCombo.comboType==CardComboType::SINGLE)
			{
				int las=lastValidCombo.packs[0].level;
				ret=find_single(cyt,begin,end,las);
				set<Card>S;
				if(ret.comboType==CardComboType::PASS)
				{
					if(las==13)
					{
						if(cyt[14])
						{
							S.insert(53);
							return CardCombo(S.begin(),S.end());
						}
					}else if(las==12)
					{
						if(cyt[13])
						{
							S.insert(52);
							return CardCombo(S.begin(),S.end());
						}
					}else return CardCombo();
				}//else
			}else if(lastValidCombo.comboType==CardComboType::PAIR)
			{
				ret=find_double(cyt,begin,end,lastValidCombo.packs[0].level);
				//return ret;
			}
		}else
		{
			if(lastValidCombo.comboType==CardComboType::STRAIGHT)
			{
				ret=find_single_line(cyt,begin,end,lastValidCombo.packs.size(),lastValidCombo.packs.size(),lastValidCombo.packs[lastValidCombo.packs.size()-1].level);
			}else if(lastValidCombo.comboType==CardComboType::STRAIGHT2)
			{
				ret=find_double_line(cyt,begin,end,lastValidCombo.packs.size(),lastValidCombo.packs.size(),lastValidCombo.packs[lastValidCombo.packs.size()-1].level);
			}else if(lastValidCombo.comboType==CardComboType::TRIPLET1)
			{
				ret=find_three_one(cyt,begin,end,lastValidCombo.packs[0].level);
			}else if(lastValidCombo.comboType==CardComboType::TRIPLET2)
			{
				ret=find_three_two(cyt,begin,end,lastValidCombo.packs[0].level);
			}else if(lastValidCombo.comboType==CardComboType::BOMB)
			{
				ret=find_bomb(cyt,begin,end,lastValidCombo.packs[0].level);
				return CardCombo();
			}else if(lastValidCombo.comboType==CardComboType::PLANE1)
			{
				ret=find_plane_one(cyt,begin,end,lastValidCombo.packs[0].level);
			}else if(lastValidCombo.comboType==CardComboType::PLANE2)
			{
				ret=find_plane_two(cyt,begin,end,lastValidCombo.packs[0].level);
			}else if(lastValidCombo.comboType==CardComboType::QUADRUPLE2)
			{
				ret=find_four_one(cyt,begin,end,lastValidCombo.packs[0].level);
			}else if(lastValidCombo.comboType==CardComboType::QUADRUPLE4)
			{
				ret=find_four_two(cyt,begin,end,lastValidCombo.packs[0].level);
			}else if(lastValidCombo.comboType==CardComboType::SINGLE)
			{
				int las=max((int)lastValidCombo.packs[0].level,min(lastValidCombo.packs[0].level+5,9));
				ret=find_single(cyt,begin,end,las);
				set<Card>S;
				if(ret.comboType==CardComboType::PASS)
				{
					las=lastValidCombo.packs[0].level;
					ret=find_single(cyt,begin,end,las);
					if(ret.comboType!=CardComboType::PASS)return ret;
					if(las==13)
					{
						if(cyt[14])
						{
							S.insert(53);
							return CardCombo(S.begin(),S.end());
						}
					}else if(las==12)
					{
						if(cyt[13])
						{
							S.insert(52);
							return CardCombo(S.begin(),S.end());
						}
					}else return CardCombo();
				}
			}else if(lastValidCombo.comboType==CardComboType::PAIR)
			{
				
				int las=max((int)lastValidCombo.packs[0].level,min(lastValidCombo.packs[0].level+5,9));
				ret=find_double(cyt,begin,end,las);
				if(ret.comboType==CardComboType::PASS)
				{
					las=lastValidCombo.packs[0].level;
					ret=find_double(cyt,begin,end,las);
				}
			}
		}
	}else
	{
		s=clock();
		int *ctt=new int[25];
		memcpy(ctt,cyt,sizeof(ctt));
		if(lastValidCombo.comboType==CardComboType::STRAIGHT)
		{
			CardCombo ret=find_single_line(cyt,begin,end,lastValidCombo.packs.size(),lastValidCombo.packs.size(),lastValidCombo.packs[lastValidCombo.packs.size()-1].level);
			//return ret;
		}else if(lastValidCombo.comboType==CardComboType::STRAIGHT2)
		{
			CardCombo ret=find_double_line(cyt,begin,end,lastValidCombo.packs.size(),lastValidCombo.packs.size(),lastValidCombo.packs[lastValidCombo.packs.size()-1].level);
			//return ret;
		}else if(lastValidCombo.comboType==CardComboType::TRIPLET1)
		{
			CardCombo ret=find_three_one(cyt,begin,end,lastValidCombo.packs[0].level);
			//return ret;
		}else if(lastValidCombo.comboType==CardComboType::TRIPLET2)
		{
			CardCombo ret=find_three_two(cyt,begin,end,lastValidCombo.packs[0].level);
			//return ret;
		}else if(lastValidCombo.comboType==CardComboType::BOMB)
		{
			return CardCombo();
		}else if(lastValidCombo.comboType==CardComboType::PLANE1)
		{
			CardCombo ret=find_plane_one(cyt,begin,end,lastValidCombo.packs[0].level);
			//return ret;
		}else if(lastValidCombo.comboType==CardComboType::PLANE2)
		{
			CardCombo ret=find_plane_two(cyt,begin,end,lastValidCombo.packs[0].level);
			//return ret;
		}else if(lastValidCombo.comboType==CardComboType::QUADRUPLE2)
		{
			return CardCombo();
		}else if(lastValidCombo.comboType==CardComboType::QUADRUPLE4)
		{
			return CardCombo();
		}else if(lastValidCombo.comboType==CardComboType::SINGLE)
		{
			int las=lastValidCombo.packs[0].level;
			ret=find_single(cyt,begin,end,las);
			set<Card>S;
			if(ret.comboType==CardComboType::PASS)
			{
				if(las==13)
				{
					if(cyt[14])
					{
						S.insert(53);
						return CardCombo(S.begin(),S.end());
					}
				}else if(las==12)
				{
					if(cyt[13])
					{
						S.insert(52);
						return CardCombo(S.begin(),S.end());
					}
				}else return CardCombo();
			}
		}else if(lastValidCombo.comboType==CardComboType::PAIR)
		{
			CardCombo ret=find_double(cyt,begin,end,lastValidCombo.packs[0].level);
			//return ret;
		}
	}
	return ret;
	/*	auto deck = vector<Card>(begin, end); // 手牌
		short counts[MAX_LEVEL + 1] = {};

		unsigned short kindCount = 0;

		// 先数一下手牌里每种牌有多少个
		for (Card c : deck)
			counts[card2level(c)]++;

		// 手牌如果不够用，直接不用凑了，看看能不能炸吧
		if (deck.size() < lastValidCombo.cards.size())
			return CardCombo();

		// 再数一下手牌里有多少种牌
		for (short c : counts)
			if (c)
				kindCount++;

		// 否则不断增大当前牌组的主牌，看看能不能找到匹配的牌组
		
			// 开始增大主牌
			int mainPackCount = lastValidCombo.findMaxSeq();
			bool isSequential =
				lastValidCombo.comboType == CardComboType::STRAIGHT ||
				lastValidCombo.comboType == CardComboType::STRAIGHT2 ||
				lastValidCombo.comboType == CardComboType::PLANE ||
				lastValidCombo.comboType == CardComboType::PLANE1 ||
				lastValidCombo.comboType == CardComboType::PLANE2 ||
				lastValidCombo.comboType == CardComboType::SSHUTTLE ||
				lastValidCombo.comboType == CardComboType::SSHUTTLE2 ||
				lastValidCombo.comboType == CardComboType::SSHUTTLE4;
			for (Level i = 1;; i++) // 增大多少
			{
				for (int j = 0; j < mainPackCount; j++)
				{
					int level = lastValidCombo.packs[j].level + i;

					// 各种连续牌型的主牌不能到2，非连续牌型的主牌不能到小王，单张的主牌不能超过大王
					if ((lastValidCombo.comboType == CardComboType::SINGLE && level > MAX_LEVEL) ||
						(isSequential && level > MAX_STRAIGHT_LEVEL) ||
						(lastValidCombo.comboType != CardComboType::SINGLE && !isSequential && level >= level_joker))
						return CardCombo();

					// 如果手牌中这种牌不够，就不用继续增了
					if (counts[level] < lastValidCombo.packs[j].count)
						goto next;
				}

				{
					// 找到了合适的主牌，那么从牌呢？
					// 如果手牌的种类数不够，那从牌的种类数就不够，也不行
					if (kindCount < lastValidCombo.packs.size())
						continue;

					// 好终于可以了
					// 计算每种牌的要求数目吧
					short requiredCounts[MAX_LEVEL + 1] = {};
					for (int j = 0; j < mainPackCount; j++)
						requiredCounts[lastValidCombo.packs[j].level + i] = lastValidCombo.packs[j].count;
					for (unsigned j = mainPackCount; j < lastValidCombo.packs.size(); j++)
					{
						Level k;
						for (k = 0; k <= MAX_LEVEL; k++)
						{
							if (requiredCounts[k] || counts[k] < lastValidCombo.packs[j].count)
								continue;
							requiredCounts[k] = lastValidCombo.packs[j].count;
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

			next:; // 再增大
			}*/
	
	


}
/*
PASS,		// 过
	SINGLE,		// 单张
	PAIR,		// 对子
	STRAIGHT,	// 顺子
	STRAIGHT2,	// 双顺
	TRIPLET,	// 三条
	TRIPLET1,	// 三带一
	TRIPLET2,	// 三带二
	BOMB,		// 炸弹
	QUADRUPLE2, // 四带二（只）
	QUADRUPLE4, // 四带二（对）
	PLANE,		// 飞机
	PLANE1,		// 飞机带小翼
	PLANE2,		// 飞机带大翼
	SSHUTTLE,	// 航天飞机
	SSHUTTLE2,	// 航天飞机带小翼
	SSHUTTLE4,	// 航天飞机带大翼
	ROCKET,		// 火箭
	INVALID		// 非法牌型
*/
namespace BotzoneIO
{
	using namespace std;
	void read()
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
			for (unsigned i = 0; i < own.size(); i++)
				myCards.insert(own[i].asInt());
			if (!firstRequest["bid"].isNull())
			{
				// 如果还可以叫分，则记录叫分
				auto bidHistory = firstRequest["bid"];
				myPosition = bidHistory.size();
				for (unsigned i = 0; i < bidHistory.size(); i++)
					bidInput.push_back(bidHistory[i].asInt());
			}
		}

		// history里第一项（上上家）和第二项（上家）分别是谁的决策
		int whoInHistory[] = {(myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT, (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT};

		int turn = input["requests"].size();
		for (int i = 0; i < turn; i++)
		{
			auto request = input["requests"][i];
			auto llpublic = request["publiccard"];
			if (!llpublic.isNull())
			{
				// 第一次得知公共牌、地主叫分和地主是谁
				landlordPosition = request["landlord"].asInt();
				landlordBid = request["finalbid"].asInt();
				myPosition = request["pos"].asInt();
				whoInHistory[0] = (myPosition - 2 + PLAYER_COUNT) % PLAYER_COUNT;
				whoInHistory[1] = (myPosition - 1 + PLAYER_COUNT) % PLAYER_COUNT;
				cardRemaining[landlordPosition] += llpublic.size();
				for (unsigned i = 0; i < llpublic.size(); i++)
				{
					landlordPublicCards.insert(llpublic[i].asInt());
					if (landlordPosition == myPosition)
						myCards.insert(llpublic[i].asInt());
				}
			}

			auto history = request["history"]; // 每个历史中有上家和上上家出的牌
			if (history.isNull())
				continue;
			stage = Stage::PLAYING;

			// 逐次恢复局面到当前
			int howManyPass = 0;
			for (int p = 0; p < 2; p++)
			{
				int player = whoInHistory[p];	// 是谁出的牌
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
					lastValidCombo = CardCombo(playedCards.begin(), playedCards.end()),lastplayer=player;
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
					myCards.erase(card);				// 从自己手牌中删掉
					playedCards.push_back(card);
				}
				whatTheyPlayed[myPosition].push_back(playedCards); // 记录这段历史
				cardRemaining[myPosition] -= playerAction.size();
			}
		}
	}

	/**
	* 输出叫分（0, 1, 2, 3 四种之一）
	*/
	void bid(int value)
	{
		Json::Value result;
		result["response"] = value;

		Json::FastWriter writer;
		cout << writer.write(result) << endl;
	}

	/**
	* 输出打牌决策，begin是迭代器起点，end是迭代器终点
	* CARD_ITERATOR是Card（即short）类型的迭代器
	*/
	template <typename CARD_ITERATOR>
	void play(CARD_ITERATOR begin, CARD_ITERATOR end)
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
	srand(time(nullptr));
	BotzoneIO::read();

	if (stage == Stage::BIDDING)
	{
		// 做出决策（你只需修改以下部分）

		auto maxBidIt = std::max_element(bidInput.begin(), bidInput.end());
		int maxBid = maxBidIt == bidInput.end() ? -1 : *maxBidIt;

		int *cyt=new int[25];
		set <Card>::iterator it;
		for(it=myCards.begin();it!=myCards.end();it++)
		{
			cyt[card2level(*it)]++;
		}
		s=clock();
		int sc=est_dfs(cyt,1,0);
		int bidValue=0;
		if(sc>55)bidValue=3;
		else if(sc>35)bidValue=2;
		else if(sc>20)bidValue=1;
		else bidValue=0;
		if(bidValue<=maxBid)bidValue=0;
		// 决策结束，输出结果（你只需修改以上部分）

		BotzoneIO::bid(bidValue);
	}
	else if (stage == Stage::PLAYING)
	{
		// 做出决策（你只需修改以下部分）

		// findFirstValid 函数可以用作修改的起点
		CardCombo myAction = findFirstValid(myCards.begin(), myCards.end());

		// 是合法牌
		assert(myAction.comboType != CardComboType::INVALID);

		assert(
			// 在上家没过牌的时候过牌
			(lastValidCombo.comboType != CardComboType::PASS && myAction.comboType == CardComboType::PASS) ||
			// 在上家没过牌的时候出打得过的牌
			(lastValidCombo.comboType != CardComboType::PASS && canBeBeatenBy(myAction)) ||
			// 在上家过牌的时候出合法牌
			(lastValidCombo.comboType == CardComboType::PASS && myAction.comboType != CardComboType::INVALID));

		// 决策结束，输出结果（你只需修改以上部分）

		BotzoneIO::play(myAction.cards.begin(), myAction.cards.end());
	}
}
