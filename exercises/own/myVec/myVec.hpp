#ifndef MY_VEC
#define MY_VEC
#include <iostream>
class myVec{
private:
    int size_;
    int capacity_;
    int *data_;
public:

    myVec();
    myVec(const myVec &other);
    ~myVec();
    int size() const;
    int capacity() const;
    void push_back(int value);
    
};


#endif