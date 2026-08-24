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
    myVec(myVec &&other);
    ~myVec();
    
    int size() const;
    int capacity() const;
    int *begin();
    int *end();
    const int *begin()const;
    const int *end()const;
    void push_back(int value);

    myVec &operator=(const myVec &other);
    myVec &operator=(myVec &&other);
    bool operator==(const myVec &other) const;
    int operator[](std::size_t n) const; 
    int& operator[](std::size_t n); 
    friend std::ostream &operator<<(std::ostream &os,const myVec &a);
};


#endif