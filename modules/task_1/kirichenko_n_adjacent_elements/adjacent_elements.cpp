// Copyright 2020 kirichenko Nikita
#include <mpi.h>
#include <vector>
#include <random>
#include <ctime>
#include <algorithm>
#include <iostream>
#include "../../../modules/task_1/kirichenko_n_adjacent_elements/adjacent_elements .h"

std::vector<int> getRandomVector(int size) {
    std::mt19937 gen(time(0));
    gen.seed(static_cast<unsigned int>(time(0)));
    std::vector<int> vec(size);
    for (int i = 0; i < size; i++) { vec[i] = gen() % 100; }
    return vec;
}

int getSequentialOperations(std::vector<int> vec, int SizeVector) {
    if (vec.size() < 2) {
        throw std::runtime_error("Incorrect vector size");
    }
    int diff = abs(vec[1] - vec[0]);
    int index = 0;
    for (int i = 1; i < SizeVector - 1; i++)
        if (abs(vec[i + 1] - vec[i]) < diff) {
            diff = abs(vec[i + 1] - vec[i]);
            index = i;
        }
    return index;
}

int getParallelOperations(std::vector<int> globalVec, int SizeVector) {
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int delta = SizeVector / size;
    int remainder = SizeVector % size;
    int* LocVec;
    if (rank == 0) {
        LocVec = new int[delta+remainder];
    } else {
        LocVec = new int[delta+1];
    }
    if (rank == 0) {
        for (int i = 1; i < size; i++) {
            MPI_Send(globalVec.data() + i * delta + remainder - 1, delta + 1,
                MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }
    if (rank == 0) {
        for (int i = 0; i < delta + remainder; i++) {
            LocVec[i] = globalVec[i];
        }
    } else {
        MPI_Status status;
        MPI_Recv(LocVec, delta + 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }
    struct {
        int value;
        int index;
    }
    LocMin, GlobMin;
    LocMin.value = abs(LocVec[1] - LocVec[0]);
    LocMin.index = 0;
    if (rank == 0) {
        for (int i = 0; i < delta+remainder - 1; i ++)
            if (abs(LocVec[i + 1] - LocVec[i]) < LocMin.value) {
                LocMin.value = abs(LocVec[i + 1] - LocVec[i]);
                LocMin.index = i;
            }
    } else {
        LocMin.index = rank * delta + remainder -1;
        for (int i = 0; i < delta; i++)
            if (abs(LocVec[i + 1] - LocVec[i]) < LocMin.value) {
                LocMin.value = abs(LocVec[i + 1] - LocVec[i]);
                LocMin.index = rank * delta + remainder - 1 + i;
            }
    }
    MPI_Reduce(&LocMin, &GlobMin, 1, MPI_2INT, MPI_MINLOC, 0, MPI_COMM_WORLD);
    return GlobMin.index;
}
