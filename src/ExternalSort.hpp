#include <sstream>
#include <iostream>
#include <fstream>
#include "RunPayload.hpp"
#include <assert.h>
#include "Buffer.hpp"
#include <vector>
#include "MinLoserTree.hpp"
#include <limits>
#include <string>
#include <stdlib.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif

template <typename T>
class ExternalSort
{
private:
    //使用k路归并
    int _k = 0;
    //产生初始顺串所用树选手个数
    int _size = 0;
    //缓冲区写次数
    int _IOcount = 0;
    //排序元素个数
    int _numElem = 0;
    //顺串个数
    int _numSegment = 0;
    //数据集
    int _dataset = 0;

public:
    ExternalSort(int dataset) : _dataset(dataset){};
    ExternalSort(const ExternalSort &) = delete;
    ExternalSort(ExternalSort &&) = delete;

public:
    inline int k() const
    {
        return _k;
    }
    inline int size() const
    {
        return _size;
    }
    inline int IOcount() const
    {
        return _IOcount;
    }
    inline int num_elements() const
    {
        return _numElem;
    }

private:
    void _loadProperty(const std::string &propertyPath)
    {
        std::cout << "Load property from " << propertyPath << std::endl;
        std::ifstream ifs(propertyPath);
        if (!ifs.is_open())
        {
            std::cerr << "Failed to load property." << std::endl;
            return;
        }

        std::string rawSize, rawK;
        ifs >> rawSize >> rawK;
        ifs.close();

        for (int i = 0; i < rawSize.size(); i++)
            if (isdigit(rawSize[i]))
            {
                rawSize = rawSize.substr(i);
                break;
            }
        _size = std::stoi(rawSize);

        int subStart = rawK.find('=') + 1;
        int offset = 0;
        while (isdigit(rawK[subStart + offset]))
            offset++;
        _k = std::stoi(rawK.substr(subStart, offset + 1));
    }

    OBuffer<T> _openNewFile(int turn, int seg) const
    {
        std::stringstream ss;
        ss << "../myoutput/ans" << _dataset << "/Seg" << turn << "-" << seg << ".txt";
        std::cout << "Open new file " << ss.str() << std::endl;
        OBuffer<int> ob(BUFFER_SIZE, ss.str());
        assert(ob.is_open());
        return ob;
    }
    IBuffer<T> _openExistFile(int turn, int seg) const
    {
        std::stringstream ss;
        ss << "../myoutput/ans" << _dataset << "/Seg" << turn << "-" << seg << ".txt";
        std::cout << "Read from file " << ss.str() << std::endl;
        IBuffer<int> ib(BUFFER_SIZE, ss.str());
        return ib;
    }

    void _initializeRun()
    {
        //选手
        runPayload *players = new runPayload[_size + 1];
        std::stringstream ss;
        ss << "../input/data" << _dataset << "/data" << _dataset << ".in";
        IBuffer<int> buffer(BUFFER_SIZE, ss.str(), true);
        _numElem = buffer.header();

        //刚开始最大顺串号是1
        int maxRun = 1;
        std::vector<OBuffer<int>> outfiles;

        //文件放入outfile中
        outfiles.push_back(_openNewFile(0, 1));

        //生成初始竞赛树
        for (int i = 1; i <= _size; i++)
        {
            buffer >> players[i].elem;
            players[i].runId = 1;
        }
        MinLoserTree<runPayload> mlt;
        mlt.initialize(players, _size);

        //开始生成初始归并串
        //当已经没有元素可以加入选手时，该选手位顺串号和元素都被置为infinity，避免对其他元素造成影响
        int curWinner = mlt.winner();
        while (players[curWinner].elem != std::numeric_limits<int>::max())
        {
            //输出赢者到对应顺串
            outfiles[players[curWinner].runId - 1] << players[curWinner].elem;
            //填充该赢者位置
            int newPlayer;
            //如果还能读入，正常填充
            if (buffer >> newPlayer)
            {
                players[curWinner].runId += (newPlayer < players[curWinner].elem);
                players[curWinner].elem = newPlayer;
                //需要开新段的情况下，打开新文件
                if (players[curWinner].runId > maxRun)
                {
                    maxRun = players[curWinner].runId;
                    outfiles.push_back(_openNewFile(0, maxRun));
                }
            }
            //否则用inf填充
            else
                players[curWinner] = {std::numeric_limits<int>::max(), std::numeric_limits<int>::max()};
            mlt.replay(curWinner);
            curWinner = mlt.winner();
        }
        delete[] players;
        //统计产生的顺串数目
        _numSegment = outfiles.size();

        //统计IO次数
        for (auto &ob : outfiles)
        {
            ob.flush();
            _IOcount += ob.IOcount();
        }
        _IOcount += buffer.IOcount();

        std::cout << "Segments initialized" << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
    }

    void _extSort(int turn)
    {
        //外部节点
        T *players = new T[_k + 1];

        int numGroups = ceil((float)_numSegment / _k);
        std::cout << "Turn " << turn << ": Merge " << _numSegment << " segments to " << numGroups << " segments" << std::endl;

        //对每组进行归并
        int curGroup = 0;
        while (curGroup < numGroups)
        {
            int numPlayers = _k;
            if (curGroup == numGroups - 1)
                numPlayers = _numSegment - (numGroups - 1) * _k;
            //输入缓冲区，每个输入文件一个，数量是输出文件的k倍
            std::vector<IBuffer<int>> infiles;
            for (int i = 1; i <= numPlayers; i++)
            {
                infiles.push_back(_openExistFile(turn - 1, i + curGroup * _k));
                infiles[i - 1] >> players[i];
            }
            //输出缓冲区，每组一个输出文件
            OBuffer<int> outfile = _openNewFile(turn, curGroup + 1);

            //只有一个文件，特殊处理
            if (numPlayers == 1)
            {
                int temp;
                //不要忘了把已经输出的加上
                outfile << players[1];
                while (infiles[0] >> temp)
                    outfile << temp;
                outfile.flush();
                _IOcount += infiles[0].IOcount();
                _IOcount += outfile.IOcount();
                break;
            }

            MinLoserTree<int> mlt;
            mlt.initialize(players, numPlayers);

            //开始生成初始归并串
            //当已经没有元素可以加入选手时，该选手位顺串号和元素都被置为infinity，避免对其他元素造成影响
            int curWinner = mlt.winner();
            while (players[curWinner] != std::numeric_limits<int>::max())
            {
                //输出赢者到对应顺串
                outfile << players[curWinner];
                //填充该赢者位置
                int newPlayer;
                //如果还能读入，正常填充
                if (infiles[curWinner - 1] >> newPlayer)
                    players[curWinner] = newPlayer;
                //否则用inf填充
                else
                    players[curWinner] = std::numeric_limits<int>::max();
                mlt.replay(curWinner);
                curWinner = mlt.winner();
            }
            //统计IO次数
            outfile.flush();
            _IOcount += outfile.IOcount();
            for (auto &ib : infiles)
                _IOcount += ib.IOcount();
            curGroup++;
        }
        delete[] players;
        //统计产生的顺串数目
        _numSegment = numGroups;
        std::cout << "Turn " << turn << " finished" << std::endl;
        std::cout << "----------------------------------------------------------" << std::endl;
    }

public:
    bool excute()
    {
        //如果文件夹不存在，创建文件夹
        std::stringstream ss;
        ss<<"../myoutput/ans"<<_dataset;
        system(("mkdir -p "+ss.str()).c_str());

        std::string dataPropertyPath = "../input/data" + std::to_string(_dataset) + "/properties.txt";
        _loadProperty(dataPropertyPath);
        if (!_k)
            return false;

        //生成初始顺串
        _initializeRun();
        //对顺串排序
        int curTurn = 1;
        while (_numSegment > 1)
            _extSort(curTurn++);

        return true;
    }
};
