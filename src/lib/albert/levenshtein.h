// Copyright (c) 2021-2022 Manuel Schneider

#pragma once
#include <QString>
#include <vector>

// Fast allocation-avoiding Levenshtein distance
// See https://doi.org/10.1137/S0097539794264810
class Levenshtein
{
public:
    /// Fast computation of Levenshtein distance from prefix to string up to a max of max_delta
    /// @note Requires prefix.size < str.size. No bounds are checked!
    /// @return The error count up to max_delta. If there are more errors, always returns max_delta+1.
    uint computePrefixEditDistanceWithLimit(const QString &prefix, const QString &string, uint k);
    static bool checkPrefixEditDistance_Legacy(const QString &prefix, const QString &str, uint delta);

private:
    inline uint8_t &cell(uint r, uint c) ;
    inline const uint8_t &cell(uint r, uint c) const;
    void expand_matrix_if_necessary(uint rows, uint cols);
    void print_matrix(const QString &prefix, const QString &string) const;
    void print_matrix_view(const QString &prefix, const QString &string, uint rows, uint cols) const;

    std::vector<uint8_t> matrix;
    uint matrix_rows = 0;
    uint matrix_cols = 0;

};
