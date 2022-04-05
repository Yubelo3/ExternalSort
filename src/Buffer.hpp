#include <iostream>
#include <string>
#include <fstream>
#include <assert.h>

template <typename T>
class IBuffer
{
private:
    //缓冲区
    T *_buffer = nullptr;
    //缓冲区指针
    unsigned int _pointer = 0;
    //缓冲区尾
    unsigned int _size = 0;
    //最大缓冲区
    unsigned int _maxSize = 0;
    //文件
    std::ifstream _file;
    //是否已经读完
    bool _eof = false;
    //磁盘IO次数
    unsigned int _IOcount = 0;
    //文件头
    int _header = 0;

private:
    //读取一个新的磁盘块，记入IO计数
    void _readNewBlock()
    {
        //重置指针
        _pointer = 0;
        _size = 0;
        while (_size < _maxSize && _file >> _buffer[_size])
            _size++;
        if (_file.eof())
        {
            _eof = true;
            _file.close();
        }

        _IOcount++;
    }

public:
    IBuffer(unsigned int size, const std::string &filePath, bool header = false) : _maxSize(size)
    {
        _buffer = new T[size];
        _file.open(filePath);
        assert(_file.is_open());
        if (header)
            _file >> _header;

        _readNewBlock();
    }
    ~IBuffer()
    {
        delete[] _buffer;
        if (!_eof)
            _file.close();
    }
    IBuffer(const IBuffer<T> &) = delete;
    IBuffer(IBuffer<T> &&ib)
    {
        _file = std::move(ib._file);
        std::swap(_buffer, ib._buffer);
        _size = ib._size;
        _maxSize = ib._maxSize;
        _eof = ib._eof;
        ib._eof = true;
        _IOcount = ib._IOcount;
        _header = ib._header;
        _pointer=ib._pointer;
    }

public:
    inline unsigned int IOcount() const
    {
        return _IOcount;
    }
    inline bool eof() const
    {
        return _eof;
    }
    inline bool maxSize() const
    {
        return _maxSize;
    }
    inline int header() const
    {
        return _header;
    }
    inline std::ifstream &ifs()
    {
        return _file;
    }
    bool read(T &dst)
    {
        if (_pointer >= _size)
        {
            assert(_pointer == _size);
            if (_eof)
                return false;
            _readNewBlock();
        }
        dst = _buffer[_pointer++];
        return true;
    }
    bool operator>>(T &dst)
    {
        return read(dst);
    }
};

template <typename T>
class OBuffer
{
private:
    //缓冲区
    T *_buffer = nullptr;
    //缓冲区指针
    unsigned int _pointer = 0;
    //最大缓冲区
    unsigned int _maxSize = 0;
    //文件
    std::ofstream _file;
    //磁盘IO次数
    unsigned int _IOcount = 0;

private:
    //写入一个磁盘块，记入IO计数
    void _writeNewBlock()
    {
        for (int i = 0; i < _pointer; i++)
            _file << _buffer[i] << " ";
        if (_pointer)
            _IOcount++;
        //重置指针
        _pointer = 0;
    }

public:
    OBuffer(unsigned int size, const std::string &filePath) : _maxSize(size)
    {
        _buffer = new T[size];
        _file.open(filePath);
        assert(_file.is_open());
    }
    ~OBuffer()
    {
        _writeNewBlock();
        _file.close();
    }
    OBuffer(const OBuffer &) = delete;
    OBuffer(OBuffer &&ob)
    {
        ob.flush();
        std::swap(_buffer, ob._buffer);
        _maxSize = ob._maxSize;
        _file = std::move(ob._file);
        _IOcount = ob._IOcount;
    }

public:
    inline unsigned int IOcount() const
    {
        return _IOcount;
    }
    inline bool maxSize() const
    {
        return _maxSize;
    }
    inline bool is_open() const
    {
        return _file.is_open();
    }

    //刷新缓冲区
    void flush()
    {
        _writeNewBlock();
    }
    bool write(const T &dst)
    {
        assert(_pointer<=_maxSize);
        if (_pointer == _maxSize)
            _writeNewBlock();
        _buffer[_pointer++] = dst;
        return true;
    }
    OBuffer<int> &operator<<(const T &dst)
    {
        write(dst);
        return *this;
    }
};