#pragma once
#include <random>
// Utilities for getting random numbers

void setGlobalSeed(unsigned int seed);

void setGlobalRandomSeed();

unsigned int getGlobalRandomSeed();

std::mt19937 getGlobalRNG();

float randomFloat(float min, float max);

int getRandInt(int min, int max);

bool randomSuccess(float chance);
