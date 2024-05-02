#pragma once

#include <initializer_list>
#include <algorithm>

const int kBLOCK = 512 / sizeof(int);
const int kBIG = 2e6;

class Deque {
public:
    Deque() {
        cnt_blocks_ = kBIG / kBLOCK + 1;
        data_ = new int*[cnt_blocks_];
        for (ssize_t i = 0; i < cnt_blocks_; ++i) {
            data_[i] = new int[kBLOCK];
        }
        head_ = {0, 0};
        tail_ = {0, 0};
        sz_ = 0;
    }
    Deque(const Deque& rhs) {
        cnt_blocks_ = rhs.cnt_blocks_;
        head_ = rhs.head_;
        tail_ = rhs.tail_;
        data_ = new int*[cnt_blocks_];
        sz_ = rhs.sz_;
        for (ssize_t i = 0; i < cnt_blocks_; ++i) {
            data_[i] = new int[kBLOCK];
            for (ssize_t j = 0; j < kBLOCK; ++j) {
                data_[i][j] = rhs.data_[i][j];
            }
        }
    }

    Deque(Deque&& rhs) {
        cnt_blocks_ = rhs.cnt_blocks_;
        head_ = rhs.head_;
        tail_ = rhs.tail_;
        data_ = rhs.data_;
        sz_ = rhs.sz_;
        rhs.data_ = nullptr;
        rhs.cnt_blocks_ = 0;
    }

    explicit Deque(size_t size) {
        cnt_blocks_ = (size + kBLOCK - 1) / kBLOCK;
        head_ = {0, 0};
        tail_ = {(size + kBLOCK - 1) / kBLOCK - 1, (size + kBLOCK - 1) % kBLOCK + 1};
        data_ = new int*[cnt_blocks_];
        sz_ = size;
        for (ssize_t i = 0; i < cnt_blocks_; ++i) {
            data_[i] = new int[kBLOCK];
            for (ssize_t j = 0; j < kBLOCK; ++j) {
                data_[i][j] = 0;
            }
        }
    }

    Deque(std::initializer_list<int> list) {
        cnt_blocks_ = kBIG / kBLOCK + 1;
        data_ = new int*[cnt_blocks_];
        for (ssize_t i = 0; i < cnt_blocks_; ++i) {
            data_[i] = new int[kBLOCK];
        }
        head_ = {0, 0};
        tail_ = {0, 0};
        for (const auto& elem : list) {
            this->PushBack(elem);
        }
        sz_ = list.size();
    }

    ~Deque() {
        for (ssize_t i = 0; i < cnt_blocks_; ++i) {
            delete[] data_[i];
        }
        delete[] data_;
    }

    Deque& operator=(Deque rhs) {
        Swap(rhs);
        return *this;
    }

    void Swap(Deque& rhs) {
        std::swap(cnt_blocks_, rhs.cnt_blocks_);
        std::swap(data_, rhs.data_);
        std::swap(head_, rhs.head_);
        std::swap(tail_, rhs.tail_);
        std::swap(sz_, rhs.sz_);
    }

    void PushBack(int value) {
        ++sz_;
        data_[tail_.first][tail_.second] = value;
        ++tail_.second;
        if (tail_.second == kBLOCK) {
            ++tail_.first;
            tail_.first %= cnt_blocks_;
            tail_.second = 0;
        }
        if (tail_ == head_) {
            int** big_data;
            big_data = new int*[2 * cnt_blocks_];
            for (ssize_t i = 0; i < 2 * cnt_blocks_; ++i) {
                big_data[i] = new int[kBLOCK];
            }
            std::pair<ssize_t, ssize_t> tail{0, 0};
            std::pair<ssize_t, ssize_t> head{0, 0};
            for (ssize_t i = 0; i < Size(); ++i) {
                big_data[tail_.first][tail_.second] = this->operator[](i);
                ++tail_.second;
                if (tail_.second == kBLOCK) {
                    ++tail_.first;
                    tail_.first %= cnt_blocks_;
                    tail_.second = 0;
                }
            }
            for (ssize_t i = 0; i < cnt_blocks_; ++i) {
                delete[] data_[i];
            }
            delete[] data_;
            data_ = std::move(big_data);
            head_ = head;
            tail_ = tail;
            cnt_blocks_ = 2 * cnt_blocks_;
        }
    }

    void PopBack() {
        --sz_;
        if (tail_.second == 0) {
            tail_.first = (tail_.first + cnt_blocks_ - 1) % cnt_blocks_;
            tail_.second = kBLOCK - 1;
        } else {
            --tail_.second;
        }
        if (tail_ == head_) {
            Clear();
        }
    }

    void PushFront(int value) {
        ++sz_;
        --head_.second;
        if (head_.second < 0) {
            head_.second += kBLOCK;
            head_.first = (head_.first + cnt_blocks_ - 1) % cnt_blocks_;
        }
        data_[head_.first][head_.second] = value;
        if (tail_ == head_) {
            int** big_data;
            big_data = new int*[2 * cnt_blocks_];
            for (ssize_t i = 0; i < 2 * cnt_blocks_; ++i) {
                big_data[i] = new int[kBLOCK];
            }
            std::pair<ssize_t, ssize_t> tail{0, 0};
            std::pair<ssize_t, ssize_t> head{0, 0};
            for (ssize_t i = 0; i < Size(); ++i) {
                big_data[tail_.first][tail_.second] = this->operator[](i);
                ++tail_.second;
                if (tail_.second == kBLOCK) {
                    ++tail_.first;
                    tail_.first %= cnt_blocks_;
                }
            }
            for (ssize_t i = 0; i < cnt_blocks_; ++i) {
                delete[] data_[i];
            }
            delete[] data_;
            data_ = std::move(big_data);
            head_ = head;
            tail_ = tail;
            cnt_blocks_ = 2 * cnt_blocks_;
        }
    }

    void PopFront() {
        --sz_;
        ++head_.second;
        if (head_.second != kBLOCK) {
            return;
        }
        if (tail_.first == head_.first) {
            Clear();
        } else {
            head_ = {(head_.first + 1) % cnt_blocks_, 0};
        }
    }

    int& operator[](size_t ind) {
        size_t block = (head_.first + (ind + head_.second) / kBLOCK) % cnt_blocks_;
        size_t ind_in_block = (ind + head_.second) % kBLOCK;
        return data_[block][ind_in_block];
    }

    int operator[](size_t ind) const {
        size_t block = (head_.first + (ind + head_.second) / kBLOCK) % cnt_blocks_;
        size_t ind_in_block = (ind + head_.second) % kBLOCK;
        return data_[block][ind_in_block];
    }

    size_t Size() const {
        return sz_;
    }

    void Clear() {
        head_ = {0, 0};
        tail_ = {0, 0};
        sz_ = 0;
    }

private:
    int** data_;
    // std::vector<std::vector<int>> data_;
    std::pair<ssize_t, ssize_t> head_;
    std::pair<ssize_t, ssize_t> tail_;
    ssize_t cnt_blocks_;
    ssize_t sz_;
};
