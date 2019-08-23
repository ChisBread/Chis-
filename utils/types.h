#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <bitset>
namespace chis
{
    template<typename T>
    using vector_type = std::vector<T>;
    template<typename T>
    using unique_ptr_type = std::unique_ptr<T>;
    template<size_t num>
    using bitset_type = std::bitset<num>;

} // namespace chis
