#include <iostream>
#include <vector>
#include <cmath>

#define debugout(x) std::cout << #x << ": " << x << std::endl;

template <typename T>
class MinLoserTree
{
private:
    T *_players = nullptr; //选手数组
    int _size = 0;         //选手数量
    int *_tree = nullptr;  // tree中是内部节点，存放输者下标
    int *_temp = nullptr;  // 赢者树

    //最底层有几个外部节点。
    //最底层一定只有外部节点；倒二层左边是内部节点，右边是外部节点；更上层都是内部节点。
    //最底层外部节点的数量是倒二层内部节点的数量的两倍。
    //倒二层是几层？是[log2(内部节点数量=n-1)向下取整]层(从0层开始算)。这个层数记作h。
    //倒二层第一个节点(一定是内部节点)是几号？2^h号。
    //最底层的外部节点i假设看作内部节点，其编号是多少？2^(h+1)+i-1
    //所以这个内到外的偏移量是offset=2^(h+1)-1
    //这个量用来计算最底层外部节点的父节点是谁。父节点=(i+offset)/2
    int _offset = 0;

    //如果不是最底层的外部节点i看作内部节点，其编号是多少？
    //最底层的外部节点一共有倒二层内部节点的两倍，倒二层内部节点有[内部节点数量-(2^h-1)=n-1-(2^h-1)=n-2^h]，
    //因此最底层外部节点有lowExt=2n-2^(h+1)个。它们占据了外部节点编号[1,2n-2^(h+1)]。
    int _lowExt = 0;

    inline int _win(int a, int b) const
    {
        return _players[a] <= _players[b] ? a : b;
    }
    inline int _lose(int a, int b) const
    {
        return _players[a] <= _players[b] ? b : a;
    }

    //比赛。由两个外部节点left和right开始比
    void _play(int p, int left, int right)
    {
        //最下的内部节点记录较小的元素的下标
        _tree[p] = _lose(left, right);
        _temp[p] = _win(left, right);

        //如果p是一个右孩子，则其兄弟的值已经产生，可以接着比赛
        while (p % 2 == 1 && p > 1)
        {
            _tree[p / 2] = _lose(_temp[p], _temp[p - 1]);
            _temp[p / 2] = _win(_temp[p], _temp[p - 1]);
            p /= 2;
        }
    }

public:
    //返回赢者在数组中的下标
    inline int winner() const
    {
        return _tree[0];
    }
    inline int loser() const
    {
        return _tree[1];
    }
    //外部节点个数
    inline int size() const
    {
        return _size;
    }

public:
    MinLoserTree(){};
    ~MinLoserTree()
    {
        delete[] _tree;
        delete[] _temp;
    }

public:
    //根据外部节点，构建一个竞赛树。外部节点数据从1开始排列。
    void initialize(T *players, int numOfPlayers)
    {
        _players = players;
        _size = numOfPlayers;
        _tree = new int[_size]; // n-1个内部节点，分布在[1-n-1]，tree[0]留空
        _temp = new int[_size];

        //开始构建内部节点树
        //先算算1号、2号选手的父节点(倒二层的第一个节点)。为此需要知道倒二层第一个节点的下标。
        //先算算内部节点占到第几层(从0层开始算)
        int h = log2(_size);
        //倒二层第一个节点的下标是s=2^h。倒二层有n-s个内部节点
        int s = pow(2, h);
        //所以最底层就有lowExt=2*(n-s)个外部节点
        _lowExt = 2 * (_size - s);
        //对于[1:lowExt]的外部节点，下标偏移了2^(h+1)-1
        _offset = pow(2, h + 1) - 1;
        //对于lowExt以后的外部节点，下标偏移了n-1-lowExt

        //开赛！先处理最底层的外部节点。倒二层的外部节点先放着。
        int i = 2;
        for (; i <= _lowExt; i += 2)
            //play把赢者放到赢者树节点，输者放到输者树节点
            _play((i + _offset) / 2, i - 1, i);
        //现在倒二层可能会有一个没有兄弟的外部节点。这个节点先处理
        if (_size % 2 == 1)
        {
            _play((_size - 1) / 2, _temp[_size - 1], _lowExt + 1);
            i = _lowExt + 3;
        }
        else
            i = _lowExt + 2;
        //开始处理倒二层的外部节点
        for (; i <= _size; i++)
            _play((i + _size - 1 - _lowExt) / 2, i - 1, i);

        _tree[0]=_temp[1];
        //至此竞赛树构建完毕
    }

    //换下一位选手，重赛
    void replay(int player)
    {
        //记录一下需要重赛的节点，以及其左右孩子(参赛者)
        int match;
        //同样分最底层外部节点和非最底层两种情况
        if (player <= _lowExt)
            match = (player + _offset) / 2;
        else
            match = (player + _size - 1 - _lowExt) / 2;
        //知道了左右子节点就可以老样子开赛了
        //但是因为可以知道输者是谁，每场比赛直接取找上次的输者比
        for (; match >= 1; match /= 2)
        {
            //_tree[match]上次输者，player这次改变的元素
            int tempLoser = _lose(_tree[match], player);
            _temp[match] = _win(_tree[match], player);
            _tree[match] = tempLoser;
            // player指向赢的一方，继续和上个赛点时输的人比
            player = _temp[match];
        }
        _tree[0]=_temp[1];
    }

    void output() const
    {
        int curLevel = 1;
        std::cout << "------------WinnerTree------------" << std::endl;
        for (int i = 1; i < _size; i++)
        {
            if (i == pow(2, curLevel))
            {
                std::cout << std::endl;
                curLevel++;
            }
            std::cout << _players[_temp[i]] << " ";
        }
        std::cout<<std::endl;
        curLevel=1;
        std::cout << "------------LoserTree------------" << std::endl;
        for (int i = 1; i < _size; i++)
        {
            if (i == pow(2, curLevel))
            {
                std::cout << std::endl;
                curLevel++;
            }
            std::cout << _players[_tree[i]] << " ";
        }
        std::cout << std::endl
                  << "-----------------------------" << std::endl;
    }
};