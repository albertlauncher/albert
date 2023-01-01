// Copyright (c) 2021-2022 Manuel Schneider

#include "levenshtein.h"
#include <algorithm>
#include <iostream>
#include <limits>
using namespace std;

static constexpr uint8_t max_edit_distance = numeric_limits<uint8_t>().max();

uint Levenshtein::computePrefixEditDistanceWithLimit(const QString &prefix, const QString &string, uint k)
{
    if (k == 0)
        return string.startsWith(prefix) ? 0 : 1;

    if (prefix.size() > string.size()+k)
        return k+1;

    uint rows = prefix.size() + 1;
    uint cols = min(prefix.size() + (qsizetype)k + 1, string.size() + 1);

    expand_matrix_if_necessary(rows, cols);

    //  Example distance d=2
    //        a   b   _   e   f   g   h   i   j
    //  ┌───┬───────────────────────────┬───────┐
    //  │ 0 │ 1   2   3   4   5   6   7 │ 8   9 │
    //  ├───┼───────────────────────────┼───────┤
    //a │ 1 │(0)  1   2                 │       │
    //  │   │                           │       d  r-1<d
    //b │ 2 │ 1  (0)  1   2             │       │
    //  │   ├───────────────────────────┼───d───┤
    //c │ 3 │ 2   1  (1)  2   3         │       │
    //  │   │                           │       │
    //d │ 4 │     2  (2)  2   3   4     │       │
    //  │   │                           │       │
    //e │ 5 │         3  (2)  3   4   5 │       │
    //  │   ├───────────────────────────┼───d───┤
    //f │ 6 │             3  (2)  3   4 │ 5     │
    //  │   │                           │       d
    //g │ 7 │                 3  (2)  3 │ 4   5 │
    //  └───┴───────────────────────────┴───────┘

    uint8_t edit_distance;
    for (uint r=1; r < rows; ++r) {
        edit_distance = max_edit_distance;

        if (r>k)
            edit_distance = min(
                edit_distance,
                cell(r, r - k) = min({cell(r - 1, r - k - 1) + (prefix[r-1] == string[r - k-1] ? 0u : 1u),
                                      cell(r - 1, r - k) + 1u}));

        uint end = min(cols, r+k);
        for (uint c = (uint)max(1,1+(int)r-(int)k); c < end; ++c)
            edit_distance = min(
                edit_distance,
                cell(r,c) = min({cell(r - 1, c - 1) + (prefix[r - 1] == string[c - 1] ? 0u : 1u),
                                 cell(r - 1, c) + 1u, cell(r, c - 1) + 1u})
            );

        if (r<cols-k)
            edit_distance = min(
                edit_distance,
                cell(r, r + k) = min({cell(r - 1, r + k - 1) + (prefix[r-1] == string[r + k-1] ? 0u : 1u),
                                      cell(r, r + k - 1) + 1u})
            );
        if (edit_distance > k)
            return edit_distance;
    }

//    cout << "k" << k << endl;
//    print_matrix(prefix, string);
//    print_matrix_view(prefix, string, rows, cols);

    return edit_distance;
}


const uint8_t &Levenshtein::cell(uint r, uint c) const
{
    return matrix[(r)*matrix_cols + c];
}

uint8_t &Levenshtein::cell(uint r, uint c)
{
    return matrix[(r)*matrix_cols + c];
}


void Levenshtein::expand_matrix_if_necessary(uint rows, uint cols)
{
    // if space needed expand and init matrix
    if (matrix_rows < rows || matrix_cols < cols){
        matrix_rows = max(matrix_rows, rows);
        matrix_cols = max(matrix_cols, cols);
        matrix.resize(matrix_rows*matrix_cols);
        for (uint r = 0; r < matrix_rows; ++r)
            cell(r, 0) = r;
        for (uint c = 0; c < matrix_cols; ++c)
            cell(0, c) = c;
    }
}

void Levenshtein::print_matrix_view(const QString &prefix, const QString &string, uint rows, uint cols) const
{
    cout << qPrintable(prefix) << endl;
    cout << qPrintable(string) << endl;
    cout << "   " ;
    for (int r = 0; r < string.size(); ++r)
        cout << " " << qPrintable(string)[r];
    cout  << endl;
    for (uint r = 0; r < rows; ++r){
        cout << qPrintable(QString(" %1").arg(prefix))[r];
        for (uint c = 0; c < cols; ++c){
            cout << " " << (int)cell(r,c);
        }
        cout << '\n';
    }
}

void Levenshtein::print_matrix(const QString &prefix, const QString &string) const
{
    cout << qPrintable(prefix) << endl;
    cout << qPrintable(string) << endl;
    cout << "   " ;
    for (int r = 0; r < string.size(); ++r)
        cout << " " << qPrintable(string)[r];
    cout  << endl;
    for (uint r = 0; r < matrix_rows; ++r){
        cout << qPrintable(QString(" %1").arg(prefix))[r];
        for (uint c = 0; c < matrix_cols; ++c){
            cout << " " << (int)cell(r,c);
        }
        cout << endl;
    }
}
/// Returns true if delta is not exceeded
bool Levenshtein::checkPrefixEditDistance_Legacy(const QString &prefix, const QString &str, uint delta)
{
    uint row_count = prefix.size() + 1;
    uint col_count = min(prefix.size() + (qsizetype)delta + 1, str.size() + 1);

    uint* table = new uint[row_count * col_count];

    // Initialize left and top row.
    for (uint r = 0; r < row_count; ++r) { table[r * col_count + 0] = r; }
    for (uint c = 0; c < col_count; ++c) { table[c] = c; }

    // Now fill the matrix. TODO column-first algo break if <= delta
    for (uint r = 1; r < row_count; ++r)
        for (uint c = 1; c < col_count; ++c) // TODO c<=r?
            table[r * col_count + c] =
                    min({table[(r - 1) * col_count + c - 1] + (prefix[r - 1] == str[c - 1] ? 0 : 1),  // substitution
                         table[r * col_count + c - 1] + 1,  // deletion
                         table[(r - 1) * col_count + c] + 1});  // insertion

    // Check the last row if there is an entry <= delta.
    bool result = false;
    for (uint j = 0; j < col_count; ++j) {
        if (table[(row_count - 1) * col_count + j] <= delta) {
            result = true;
            break;
        }
    }

    delete[] table;
    return result;
}
