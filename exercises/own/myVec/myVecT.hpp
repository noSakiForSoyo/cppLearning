#ifndef MY_VEC_T
#define MY_VEC_T
#include <iostream>

template <typename T>
class myVec;

template <typename T>
std::ostream& operator<<(std::ostream &os,const myVec<T> &a);

template <typename T>
class myVec{
private:
    int size_;
    int capacity_;
    T *data_;
public:
    myVec():size_(0),capacity_(1),data_(nullptr){
    }
    myVec(const myVec &other){
        this->size_ = other.size_;
        this->capacity_ = other.capacity_;
        this->data_ = new T[capacity_];
        T *p = this->data_;
        T *op = other.data_;
        for(int i = 0;i!=this->size_;++i){
            *p=*op;
            ++p;
            ++op;
        }
    }
    myVec noexcept(myVec &&other):
    size_(other.size_),capacity_(other.capacity_),data_(other.data_){
        other.size_ = 0;
        other.capacity_=1;
        other.data_=nullptr;
    }
    ~myVec(){
        delete[] data_;
    }

    int size() const{
        return size_;
    }
    int capacity() const{
        return capacity_;
    }
    T *begin(){
        return this->data_;
    }
    T *end(){
        return (this->data_)+size_;
    }
    const T *begin()const{
        return this->data_;
    }
    const T *end()const{
        return (this->data_)+size_;
    }
    void push_back(T value){
        if(data_==nullptr){
            data_ = new T[capacity_];
        }
        if(size_ == capacity_){
            capacity_ = 2*capacity_;
            T *newData = new T[capacity_];
            T *np = newData;
            T *p = data_;
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

    bool operator==(const myVec &other) const{
        if(size_!=other.size_) return false;
        for (int i = 0; i != size_; ++i)
        if (data_[i] != other.data_[i]) return false;
        return true;
    } 
    myVec &operator=(const myVec &other){
        if(this == &other){
            return *this;
        }
        this->size_ = other.size_;
        this->capacity_ = other.capacity_;
        delete[] this->data_;
        this->data_ = new T[this->capacity_];
        T *p = this->data_;
        T *op = other.data_;
        for(int i = 0;i<this->size_;++i){
            *p=*op;
            ++p;
            ++op;
        }
        return *this;
    }    
    myVec &operator=  noexcept(myVec &&other){
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
    T operator[](std::size_t n) const{
        if(n>=this->size_){
            throw std::out_of_range("myVec: 索引越界");
        }
        return this->data_[n];
    }
    T& operator[](std::size_t n){
        if(n>=this->size_){
            throw std::out_of_range("myVec: 索引越界");
        }
        return this->data_[n];
    }
    friend std::ostream& operator<< <>(std::ostream &os,const myVec<T> &a);
};
template <typename T>
std::ostream& operator<<(std::ostream &os,const myVec<T> &a){
    os << "{";
    for(int i = 0;i!=a.size_;i++){
        os << a.data_[i];
        if(i!=a.size_-1) os << ','; 
    }
    os << "}";
    return os;
}

#endif