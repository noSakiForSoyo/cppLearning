#include "myVec_unique.hpp"

    myVec_unique::myVec_unique():size_(0),capacity_(1),data_(nullptr){
    }
    myVec_unique::myVec_unique(const myVec_unique &other){
        this->size_ = other.size_;
        this->capacity_ = other.capacity_;
        this->data_ = std::make_unique<int[]>(capacity_);
        int *p = this->data_.get();
        int *op = other.data_.get();
        for(int i = 0;i!=this->size_;++i){
            *p=*op;
            ++p;
            ++op;
        }
    }
    myVec_unique::myVec_unique(myVec_unique &&other) =default;
    myVec_unique::~myVec_unique() = default;
    // myVec_unique::~myVec_unique(){
    //     delete[] data_;
    // }

    int myVec_unique::size() const{
        return size_;
    }
    int myVec_unique::capacity() const{
        return capacity_;
    }
    int *myVec_unique::begin(){
        return (this->data_).get();
    }
    int *myVec_unique::end(){
        return ((this->data_).get())+size_;
    }
    const int *myVec_unique::begin()const{
        return (this->data_).get();
    }
    const int *myVec_unique::end()const{
        return ((this->data_).get())+size_;
    }
    void myVec_unique::push_back(int value){
        if(data_==nullptr){
            data_ = std::make_unique<int[]>(capacity_);
        }
        if(size_ == capacity_){
            capacity_ = 2*capacity_;
            int *newData = new int[capacity_];
            int *np = newData;
            int *p = data_.get();
            for(int i = 0;i!=size_;i++){
                *np = *p;
                ++p;
                ++np;
            }
            data_=std::unique_ptr<int[]>(newData);
        }
        data_[size_]=value;
        size_++;
    }

    bool myVec_unique::operator==(const myVec_unique &other) const{
        if(size_!=other.size_) return false;
        for (int i = 0; i != size_; ++i)
        if (data_[i] != other.data_[i]) return false;
        return true;
    } 
    myVec_unique &myVec_unique::operator=(const myVec_unique &other){
        if(this == &other){
            return *this;
        }
        this->size_ = other.size_;
        this->capacity_ = other.capacity_;
        this->data_ = std::make_unique<int []>(capacity_);
        int *p = (this->data_).get();
        int *op = (other.data_).get();
        for(int i = 0;i<this->size_;++i){
            *p=*op;
            ++p;
            ++op;
        }
        return *this;
    }    
    myVec_unique &myVec_unique::operator=(myVec_unique &&other)= default;
    int myVec_unique::operator[](std::size_t n) const{
        if(n>=this->size_){
            throw std::out_of_range("myVec_unique: 索引越界");
        }
        return this->data_[n];
    }
    int& myVec_unique::operator[](std::size_t n){
        if(n>=this->size_){
            throw std::out_of_range("myVec_unique: 索引越界");
        }
        return this->data_[n];
    }
    std::ostream& operator<<(std::ostream &os,const myVec_unique &a){
        os << "{";
        for(int i = 0;i!=a.size_;i++){
            os << a.data_[i];
            if(i!=a.size_-1) os << ','; 
        }
        os << "}";
        return os;
    }
