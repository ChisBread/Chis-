#pragma once
#include <algorithm>
#include <array>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <random>
#include <set>
#include <unordered_map>
#include <vector>
#define ABS(x) ((x)>=0?(x):-(x))
namespace chis {
using std::cout;
using std::endl;
template <typename T>
using vector_type = std::vector<T>;
template <typename T, size_t size>
using array_type = std::array<T, size>;
template <typename T>
using set_type = std::set<T>;
template <typename T>
using unique_ptr_type = std::unique_ptr<T>;
template <size_t num>
using bitset_type = std::bitset<num>;
// rand64
static thread_local std::mt19937_64 random_number_generator{
    std::random_device{}()};
static thread_local std::uniform_int_distribution<uint64_t> number_distribution;
inline uint64_t rand_uint64() {
    return number_distribution(random_number_generator);
}
}  // namespace chis
