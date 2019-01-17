#pragma once

#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>

template <class T>
class Shuffle
{
public:
    void UnShuffle();
    Shuffle(std::vector<T> input, int seed);
    std::vector<T> vec;
private:
    int seed;
    int size;
};

// Fisher Yates Shuffle
template <class T>
Shuffle<T>::Shuffle(std::vector<T> input, int seed){
    memcpy(this->vec, input, sizeof(input));
    this->seed = seed;
    this->size = input.size();
    int random; srand(this->seed);

    for(int i = this->size -1; i > 0; i--){
        int index = rand() % (i + 1);
        std::swap(this->vec[index], this->vec[i]);
    }
}

// Fisher Yates De-shuffle
template <class T>
void Shuffle<T>::UnShuffle(){
    srand(this->seed);
    std::vector<T> randoms(this->size);
    int j = 0;
    for (int i = this->size - 1; i > 0; i--) {
        randoms[j++] = rand() % (i + 1);
    }

    //deShuffling
    for (int i = 1; i < this->size; i++) {
        //use the random values backwards
        int index = randoms[this->size - i - 1];
        // simple swap
        std::swap(this->vec[index], this->vec[i]);
    }
}