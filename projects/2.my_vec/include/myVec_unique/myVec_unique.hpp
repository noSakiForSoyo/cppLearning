#ifndef MY_VEC_UNIQUE
#define MY_VEC_UNIQUE
#include <iostream>
#include <memory>
class myVec_unique{
private:
    int size_;
    int capacity_;
    std::unique_ptr<int[]> data_;
public:

    myVec_unique();
    myVec_unique(const myVec_unique &other);
    myVec_unique(myVec_unique &&other);
    ~myVec_unique();
    
    int size() const;
    int capacity() const;
    int *begin();
    int *end();
    const int *begin()const;
    const int *end()const;
    void push_back(int value);

    myVec_unique &operator=(const myVec_unique &other);
    myVec_unique &operator=(myVec_unique &&other);
    bool operator==(const myVec_unique &other) const;
    int operator[](std::size_t n) const; 
    int& operator[](std::size_t n); 
    friend std::ostream &operator<<(std::ostream &os,const myVec_unique &a);
};


#endif