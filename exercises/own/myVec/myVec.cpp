#include "myVec.hpp"

    myVec::myVec():size_(0),capacity_(1),data_(nullptr){
    }
    myVec::myVec(const myVec &other){
        this->size_ = other.size_;
        this->capacity_ = other.capacity_;
        this->data_ = new int[capacity_];
        int *p = this->data_;
        int *op = other.data_;
        for(int i = 0;i!=this->size_;++i){
            *p=*op;
            ++p;
            ++op;
        }
    }
    myVec::~myVec(){
        delete[] data_;
    }
    int myVec::size() const{
        return size_;
    }
    int myVec::capacity() const{
        return capacity_;
    }
    void myVec::push_back(int value){
        if(data_==nullptr){
            data_ = new int[capacity_];
        }
        if(size_ == capacity_){
            capacity_ = 2*capacity_;
            int *newData = new int[capacity_];
            int *np = newData;
            int *p = data_;
            for(int i = 0;i!=size_;i++){
                *np = *data_;
                ++p;
                ++np;
            }
            delete[] data_;
            data_=newData;
        }
        data_[size_]=value;
        size_++;
    }
int main(){
    myVec a;
    a.push_back(1);
    
    int i;
    while(std::cin >> i){
        a.push_back(i);
        std::cout << a.size()<<'\n';
    }
    
}