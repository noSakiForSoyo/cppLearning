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
    myVec::myVec(myVec &&other):
    size_(other.size_),capacity_(other.capacity_),data_(other.data_){
        other.size_ = 0;
        other.capacity_=1;
        other.data_=nullptr;
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
    int *myVec::begin(){
        return this->data_;
    }
    int *myVec::end(){
        return (this->data_)+size_;
    }
    const int *myVec::begin()const{
        return this->data_;
    }
    const int *myVec::end()const{
        return (this->data_)+size_;
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
                *np = *p;
                ++p;
                ++np;
            }
            delete[] data_;
            data_=newData;
        }
        data_[size_]=value;
        size_++;
    }

    bool myVec::operator==(const myVec &other) const{
        if(size_!=other.size_) return false;
        for (int i = 0; i != size_; ++i)
        if (data_[i] != other.data_[i]) return false;
        return true;
    } 
    myVec &myVec::operator=(const myVec &other){
        if(this == &other){
            return *this;
        }
        this->size_ = other.size_;
        this->capacity_ = other.capacity_;
        delete[] this->data_;
        this->data_ = new int[this->capacity_];
        int *p = this->data_;
        int *op = other.data_;
        for(int i = 0;i<this->size_;++i){
            *p=*op;
            ++p;
            ++op;
        }
        return *this;
    }    
    myVec &myVec::operator=(myVec &&other){
        if(this==&other){
            return *this;
        }
        this->size_ = other.size_;
        this->capacity_ = other.capacity_;
        delete[] this->data_;
        this->data_ = other.data_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 1;
        return *this;
    }
    int myVec::operator[](std::size_t n) const{
        if(n>=this->size_){
            throw std::out_of_range("myVec: 索引越界");
        }
        return this->data_[n];
    }
    int& myVec::operator[](std::size_t n){
        if(n>=this->size_){
            throw std::out_of_range("myVec: 索引越界");
        }
        return this->data_[n];
    }
    std::ostream& operator<<(std::ostream &os,const myVec &a){
        os << "{";
        for(int i = 0;i!=a.size_;i++){
            os << a.data_[i];
            if(i!=a.size_-1) os << ','; 
        }
        os << "}";
        return os;
    }
