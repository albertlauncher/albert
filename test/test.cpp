// Copyright (c) 2022 Manuel Schneider

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "albert/util/standarditem.h"
#include "doctest/doctest.h"
#include "itemindex.h"
#include "levenshtein.h"
#include "albert/extensions/indexqueryhandler.h"
#include <QString>
#include <chrono>
#include <iostream>
using namespace albert;
using namespace std;
using namespace std::chrono;

#include <ctime>
#include <unistd.h>

std::string gen_random(const int len) {
    static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}

void levenshtein_compare_benchmarks_and_check_results(const vector<QString> &strings, uint k){
    Levenshtein l;
    vector<bool> results_old;
    vector<bool> results_new;

    results_old.reserve(strings.size());
    auto start = system_clock::now();
    auto i = strings.cbegin();
    auto j = strings.crbegin();
    for (; i != strings.cend(); ++i, ++j)
        results_old.push_back(l.checkPrefixEditDistance_Legacy(*i, *j, k));
    long duration_old = duration_cast<microseconds>(system_clock::now()-start).count();

    results_new.reserve(strings.size());
    start = system_clock::now();
    i = strings.cbegin();
    j = strings.crbegin();
    for (; i != strings.cend(); ++i, ++j)
        results_new.push_back(l.computePrefixEditDistanceWithLimit(*i, *j, k) <= k);
    long duration_new = duration_cast<microseconds>(system_clock::now()-start).count();

    cout << "Levensthein old: " << setw(12)<<duration_old << " µs. New: "<< setw(12) << duration_new
         << " µs. Ratio: " << duration_new/(float)duration_old <<endl;
    CHECK(results_old == results_new);
}

TEST_CASE("Benchmark new levenshtein")
{
//    int test_count = 100000;
//
//    srand((unsigned)time(NULL) * getpid());
//
//    vector<QString> strings(test_count);
//    auto lens = {4,8,16,24};
//    auto divisor=4;
//    cout << "Randoms"<<endl;
//    for (int len : lens){
//        int k = floor(len/divisor);
//        cout << "len: "<< setw(2)<<len<<". k: "<<k<<" ";
//        for (auto &string : strings)
//            string = QString::fromStdString(gen_random(len));
//        levenshtein_compare_benchmarks_and_check_results(strings, k);
//    }
//
//    cout << "Equals"<<endl;
//    for (int len : lens) {
//        int k = floor(len/divisor);
//        cout << "len: "<< setw(2)<<len<<". k: "<<k<<" ";
//        for (auto &string : strings)
//            string = QString(len, 'a');
//        levenshtein_compare_benchmarks_and_check_results(strings, k);
//    }
//
//    cout << "Halfhalf equal random"<<endl;
//    for (int len : lens) {
//        int k = floor(len/divisor);
//        cout << "len: "<< setw(2)<<len<<". k: "<<k<<" ";
//        for (auto &string : strings)
//            string = QString("%1%2").arg(QString(len/2, 'a'), QString::fromStdString(gen_random(len/2)));
//        levenshtein_compare_benchmarks_and_check_results(strings, k);
//    }
}


TEST_CASE("Levenshtein")
{
    Levenshtein l;

    CHECK(l.computePrefixEditDistanceWithLimit("abcdefg", "ab_efghij", 0) == 1);
    CHECK(l.computePrefixEditDistanceWithLimit("abcdefg", "ab_efghij", 2) == 2);
    CHECK(l.computePrefixEditDistanceWithLimit("abcdefg", "ab_efgh", 2) == 2);
    CHECK(l.computePrefixEditDistanceWithLimit("abcde__h", "abcdefghij", 1) == 2);

    // plain
    CHECK(l.computePrefixEditDistanceWithLimit("test", "test", 0) == 0);
    CHECK(l.computePrefixEditDistanceWithLimit("test", "test", 1) == 0);
    CHECK(l.computePrefixEditDistanceWithLimit("test", "test", 2) == 0);
    CHECK(l.computePrefixEditDistanceWithLimit("test", "test_", 0) == 0);

    //fuzzy substitution
    CHECK(l.computePrefixEditDistanceWithLimit("test", "_est____", 1) == 1);
    CHECK(l.computePrefixEditDistanceWithLimit("test", "__st____", 1) == 2);
    CHECK(l.computePrefixEditDistanceWithLimit("test", "_est____", 2) == 1);
    CHECK(l.computePrefixEditDistanceWithLimit("test", "__st____", 2) == 2);
    CHECK(l.computePrefixEditDistanceWithLimit("test", "___t____", 2) == 3);

    //fuzzy deletion
    CHECK(l.computePrefixEditDistanceWithLimit("test", "ttest____", 1) == 1);
    CHECK(l.computePrefixEditDistanceWithLimit("test", "tttest____", 1) == 2);

    //fuzzy insertion
    CHECK(l.computePrefixEditDistanceWithLimit("test", "est____", 1) == 1);
    CHECK(l.computePrefixEditDistanceWithLimit("test", "st____", 1) > 1);

    CHECK(l.computePrefixEditDistanceWithLimit("abcdefghij", "abcdefghij", 1) == 0);
    CHECK(l.computePrefixEditDistanceWithLimit("abcdefghij", "abcdefghij", 2) == 0);
    CHECK(l.computePrefixEditDistanceWithLimit("abcdefghij", "abcdefghij", 3) == 0);
    CHECK(l.computePrefixEditDistanceWithLimit("abcdefghij", "abcdefghij", 4) == 0);
    CHECK(l.computePrefixEditDistanceWithLimit("abcdefghij", "abcdefghij", 5) == 0);
    CHECK(l.computePrefixEditDistanceWithLimit("abcdefghij", "abcdefghij", 6) == 0);
    CHECK(l.computePrefixEditDistanceWithLimit("abcdefghij", "abcdefghij", 7) == 0);
    CHECK(l.computePrefixEditDistanceWithLimit("abcdefghij", "abcdefghij", 8) == 0);

    // Bug 2022-11-20 string is smaller than prefix
    CHECK(l.computePrefixEditDistanceWithLimit("abc", "abc", 1) == 0);
    CHECK(l.computePrefixEditDistanceWithLimit("abc", "ab", 1) == 1);
    CHECK(l.computePrefixEditDistanceWithLimit("abc", "a", 1) == 2);
    CHECK(l.computePrefixEditDistanceWithLimit("abc", "", 1) == 2);
}

TEST_CASE("Index")
{
    auto match = [&](const QStringList& item_strings, const QString& search_string, bool case_sesitivity, int q, int fuzzy){

        auto index = ItemIndex("[ ]+", case_sesitivity, q, fuzzy);
        vector<IndexItem> index_items;
        for (auto &string : item_strings)
            index_items.emplace_back(make_shared<StandardItem>(string), string);
        index.setItems(::move(index_items));
        return index.search(search_string, true);
    };

    // case sensitivity
    CHECK(match({"a","A"}, "a", true, 0, 0).size() == 1);
    CHECK(match({"a","A"}, "A", true, 0, 0).size() == 1);

    // intersection
    CHECK(match({"a b","b c","c d"}, "a", false, 0, 0).size() == 1);
    CHECK(match({"a b","b c","c d"}, "b", false, 0, 0).size() == 2);
    CHECK(match({"a b","b c","c d"}, "c", false, 0, 0).size() == 2);
    CHECK(match({"a b","b c","c d"}, "d", false, 0, 0).size() == 1);
    CHECK(match({"a b","b c","c d"}, "b c", false, 0, 0).size() == 1);

    // sequence
    CHECK(match({"a b","b a","a b"}, "a b", false, 0, 0).size() == 2);
    CHECK(match({"a b","b a","a b"}, "b a", false, 0, 0).size() == 1);

    // fuzzy
    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abc", false, 2, 3).size() == 1);
    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "ab_", false, 2, 3).size() == 1);
    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "a__", false, 2, 3).size() == 0);
    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abcdef", false, 2, 3).size() == 1);
    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abc_e_", false, 2, 3).size() == 1);
    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "a_c_e_", false, 2, 3).size() == 0);
    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abcdefghi", false, 2, 3).size() == 1);
    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abcdefg_i", false, 2, 3).size() == 1);
    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abcde_g_i", false, 2, 3).size() == 1);
    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abc_e_g_i", false, 2, 3).size() == 1);
    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "a_c_e_g_i", false, 2, 3).size() == 0);
    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abcd", false, 2, 4).size() == 1);
    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abc_", false, 2, 4).size() == 1);
    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "ab__", false, 2, 4).size() == 0);
    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abcdefgh", false, 2, 4).size() == 1);
    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abcdefg_", false, 2, 4).size() == 1);
    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abcde_g_", false, 2, 4).size() == 1);
    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abc_e_g_", false, 2, 4).size() == 0);

    // score
    CHECK(match({"a","ab","abc"}, "a", false, 2, 3).size() == 3);
    CHECK(match({"a","ab","abc"}, "a", false, 2, 3)[0].score == (Score)(1.0/3.0*MAX_SCORE));
    CHECK(match({"a","ab","abc"}, "a", false, 2, 3)[1].score == (Score)(1.0/2.0*MAX_SCORE));
    CHECK(match({"a","ab","abc"}, "a", false, 2, 3)[2].score == (Score)(1.0*MAX_SCORE));

    CHECK(match({"abc","abd"}, "abe", false, 2, 3)[0].score == (Score)(2.0/3.0*MAX_SCORE));
    CHECK(match({"abc","abd"}, "abe", false, 2, 3)[1].score == (Score)(2.0/3.0*MAX_SCORE));

    std::vector<albert::RankItem> M = match({"abc","abd","abcdef"}, "abc", false, 2, 3);
    sort(M.begin(), M.end(), [](auto &a, auto &b){ return a.item->id() < b.item->id(); });
    CHECK(M[0].score == (Score)(3.0/3.0*MAX_SCORE));
    CHECK(M[1].score == (Score)(3.0/6.0*MAX_SCORE));
    CHECK(M[2].score == (Score)(2.0/3.0*MAX_SCORE));
}