/*
 * Copyright (c) 2019 Doku Enterprise
 * Author: Friedrich Doku
 * -----
 * Last Modified: Friday March 22nd 2019 7:02:46 pm
 * -----
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *   This program is distributed in the hope that it will be useful
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


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
    std::copy(input.begin(), input.end(), std::back_inserter(this->vec));
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