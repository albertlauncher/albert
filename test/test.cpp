// Copyright (c) 2024 Manuel Schneider

#include "matcher.h"
#define DOCTEST_CONFIG_COLORS_ANSI
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "itemindex.h"
#include "levenshtein.h"
#include "rankitem.h"
#include "standarditem.h"
#include "topologicalsort.hpp"
#include <QString>
#include <chrono>
#include <ctime>
#include <iostream>
#include <string>
#include <unistd.h>
using namespace albert;
using namespace std::chrono;
using namespace std;

TEST_CASE("Plugin registry topological sort")
{
    // linear
    auto result = topologicalSort(map<int,set<int>>{
        {1, {2}},
        {2, {3}},
        {3, {}}
    });
    CHECK(result.sorted == vector<int>{3, 2, 1});
    CHECK(result.error_set.empty());

    // diamond
    result = topologicalSort(map<int,set<int>>{
        {1, {}},
        {2, {1}},
        {3, {1}},
        {4, {2, 3}}
    });
    CHECK((result.sorted == vector<int>{1,2,3,4} ||
           result.sorted == vector<int>{1,3,2,4}));
    CHECK(result.error_set.empty());

    // cycle
    result = topologicalSort(map<int,set<int>>{
        {1, {2}},
        {2, {1}}
    });
    CHECK(result.sorted == vector<int>{});
    CHECK(result.error_set == map<int,set<int>>{
              {1, {2}},
              {2, {1}}
          });

    // not existing node
    result = topologicalSort(map<int,set<int>>{
        {1, {2}}
    });
    CHECK(result.sorted == vector<int>{});
    CHECK(result.error_set == map<int,set<int>>{{1, {2}}});
}


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

// void levenshtein_compare_benchmarks_and_check_results(const vector<QString> &strings, uint k){
//     Levenshtein l;
//     vector<bool> results_old;
//     vector<bool> results_new;

//     results_old.reserve(strings.size());
//     auto start = system_clock::now();
//     auto i = strings.cbegin();
//     auto j = strings.crbegin();
//     for (; i != strings.cend(); ++i, ++j)
//         results_old.push_back(l.checkPrefixEditDistance_Legacy(*i, *j, k));
//     long duration_old = duration_cast<microseconds>(system_clock::now()-start).count();

//     results_new.reserve(strings.size());
//     start = system_clock::now();
//     i = strings.cbegin();
//     j = strings.crbegin();
//     for (; i != strings.cend(); ++i, ++j)
//         results_new.push_back(l.computePrefixEditDistanceWithLimit(*i, *j, k) <= k);
//     long duration_new = duration_cast<microseconds>(system_clock::now()-start).count();

//     cout << "Levensthein old: " << setw(12)<<duration_old << " µs. New: "<< setw(12) << duration_new
//          << " µs. Ratio: " << duration_new/(float)duration_old <<endl;
//     CHECK(results_old == results_new);
// }

// TEST_CASE("Benchmark new levenshtein")
// {
//     int test_count = 100000;

//     srand((unsigned)time(NULL) * getpid());

//     vector<QString> strings(test_count);
//     auto lens = {4,8,16,24};
//     auto divisor=4;
//     cout << "Randoms"<<endl;
//     for (int len : lens){
//         int k = floor(len/divisor);
//         cout << "len: "<< setw(2)<<len<<". k: "<<k<<" ";
//         for (auto &string : strings)
//             string = QString::fromStdString(gen_random(len));
//         levenshtein_compare_benchmarks_and_check_results(strings, k);
//     }

//     cout << "Equals"<<endl;
//     for (int len : lens) {
//         int k = floor(len/divisor);
//         cout << "len: "<< setw(2)<<len<<". k: "<<k<<" ";
//         for (auto &string : strings)
//             string = QString(len, 'a');
//         levenshtein_compare_benchmarks_and_check_results(strings, k);
//     }

//     cout << "Halfhalf equal random"<<endl;
//     for (int len : lens) {
//         int k = floor(len/divisor);
//         cout << "len: "<< setw(2)<<len<<". k: "<<k<<" ";
//         for (auto &string : strings)
//             string = QString("%1%2").arg(QString(len/2, 'a'), QString::fromStdString(gen_random(len/2)));
//         levenshtein_compare_benchmarks_and_check_results(strings, k);
//     }
// }


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
    auto match = [&](const QStringList & item_strings,
                     const QString & search_string,
                     const MatchConfig & config = {})
    {
        ItemIndex index(config);

        vector<IndexItem> index_items;
        for (auto &string : item_strings)
            index_items.emplace_back(make_shared<StandardItem>(string), string);

        index.setItems(::move(index_items));

        return index.search(search_string, true);
    };

    MatchConfig c;

    // Empty
    {
        auto m = match({"a","A"}, "", c);
        CHECK(m.size() == 2);
        CHECK(m[0].score == 0.);
        CHECK(m[1].score == 0.);
    }

    // Case sensitivity
    {
        auto m = match({"a","A"}, "a", {});
        CHECK(m.size() == 2);
        CHECK(m[0].score == 1.);
        CHECK(m[1].score == 1.);
        m = match({"a","A"}, "a", {.ignore_case = false});
        CHECK(m.size() == 1);
        CHECK(m[0].score == 1.);
    }

    QStringList abc_perm{
        "a b c",
        "a c b",
        "b a c",
        "c ä b",
        "b c a",
        "c b ã"
    };

    // Multiple
    {
        CHECK(match(abc_perm, "a").size() == 6);
        CHECK(match(abc_perm, "a b").size() == 6);
        CHECK(match(abc_perm, "a b c").size() == 6);
    }

    // Order
    {
        CHECK(match(abc_perm, "a", {.ignore_word_order = false}).size() == 6);
        CHECK(match(abc_perm, "a b", {.ignore_word_order = false}).size() == 3);
        CHECK(match(abc_perm, "a b c", {.ignore_word_order = false}).size() == 1);
    }

    // Diacritics
    {
        CHECK(match(abc_perm, "a", {.ignore_diacritics = false}).size() == 4);
        CHECK(match(abc_perm, "a b", {.ignore_diacritics = false}).size() == 4);
        CHECK(match(abc_perm, "a b c", {.ignore_diacritics = false}).size() == 4);
        CHECK(match(abc_perm, "b", {.ignore_diacritics = false}).size() == 6);
    }

    // Separators
    {
        CHECK(match({"a!b", "a b","a-b"}, "a b").size() == 3);
        CHECK(match({"a!b", "a b","a-b"}, "a b",
                    {.separator_regex = QRegularExpression("[ ]+")}).size() == 1);
    }

    // Fuzzy
    {
        QStringList abc{"abcdefghijklmnopqrstuvwxyz"};

        c = {
            .separator_regex = QRegularExpression("[ ]+"),
            .error_tolerance_divisor = 3
        };

        CHECK(match(abc, "abc", c).size() == 1);
        CHECK(match(abc, "ab_", c).size() == 1);
        CHECK(match(abc, "a__", c).size() == 0);
        CHECK(match(abc, "abcdef", c).size() == 1);
        CHECK(match(abc, "abc_e_", c).size() == 1);
        CHECK(match(abc, "a_c_e_", c).size() == 0);
        CHECK(match(abc, "abcdefghi", c).size() == 1);
        CHECK(match(abc, "abcdefg_i", c).size() == 1);
        CHECK(match(abc, "abcde_g_i", c).size() == 1);
        CHECK(match(abc, "abc_e_g_i", c).size() == 1);
        CHECK(match(abc, "a_c_e_g_i", c).size() == 0);

        c.error_tolerance_divisor = 4;

        CHECK(match(abc, "abcd", c).size() == 1);
        CHECK(match(abc, "abc_", c).size() == 1);
        CHECK(match(abc, "ab__", c).size() == 0);
        CHECK(match(abc, "abcdefgh", c).size() == 1);
        CHECK(match(abc, "abcdefg_", c).size() == 1);
        CHECK(match(abc, "abcde_g_", c).size() == 1);
        CHECK(match(abc, "abc_e_g_", c).size() == 0);
    }

    // Score
    {
        auto m = match({"a","ab","abc"}, "a",  { .error_tolerance_divisor = 4 });

        CHECK(m.size() == 3);
        sort(m.begin(), m.end(), [](auto &a, auto &b){ return a.item->id() < b.item->id(); });
        CHECK(qFuzzyCompare(m[0].score, 1.0));
        CHECK(qFuzzyCompare(m[1].score, 1.0/2.0));
        CHECK(qFuzzyCompare(m[2].score, 1.0/3.0));

        m = match({"abc","abd"}, "abe",  { .error_tolerance_divisor = 3 });

        CHECK(m.size() == 2);
        CHECK(qFuzzyCompare(m[0].score, 2.0/3.0));
        CHECK(qFuzzyCompare(m[1].score, 2.0/3.0));
    }
}

TEST_CASE("Matcher")
{
    // Empty
    {
        Matcher m("");
        CHECK(qFuzzyCompare(m.match("a").score(), .0));
        CHECK(qFuzzyCompare(m.match("a b").score(), .0));
        CHECK(m.match("a") == true);
        CHECK(m.match("a b") == true);
        CHECK(m.match("a").isMatch() == true);
        CHECK(m.match("a b").isMatch() == true);
        CHECK(m.match("a").isEmptyMatch() == true);
        CHECK(m.match("a b").isEmptyMatch() == true);
        CHECK(m.match("a").isExactMatch() == false);
        CHECK(m.match("a b").isExactMatch() == false);
    }

    // Single
    {
        Matcher m("a");
        CHECK(qFuzzyCompare(m.match("a").score(), 1.0));
        CHECK(qFuzzyCompare(m.match("a b").score(), 1.0/2));
        CHECK(m.match("a") == true);
        CHECK(m.match("a b") == true);
        CHECK(m.match("a").isMatch() == true);
        CHECK(m.match("a b").isMatch() == true);
        CHECK(m.match("a").isEmptyMatch() == false);
        CHECK(m.match("a b").isEmptyMatch() == false);
        CHECK(m.match("a").isExactMatch() == true);
        CHECK(m.match("a b").isExactMatch() == false);
    }

    // Multiple
    {
        Matcher m("a b");
        CHECK(m.match("a") == false);
        CHECK(m.match("b") == false);
        CHECK(m.match("a b") == true);
        CHECK(m.match("b a") == true);
        CHECK(m.match("a b c") == true);
        CHECK(m.match("a c b") == true);
        CHECK(m.match("b a c") == true);
        CHECK(m.match("c a b") == true);
        CHECK(m.match("b c a") == true);
        CHECK(m.match("c b a") == true);
        CHECK(qFuzzyCompare(m.match("a b c").score(), 2.0/3));
        CHECK(qFuzzyCompare(m.match("a c b").score(), 2.0/3));
        CHECK(qFuzzyCompare(m.match("b a c").score(), 2.0/3));
        CHECK(qFuzzyCompare(m.match("c a b").score(), 2.0/3));
        CHECK(qFuzzyCompare(m.match("b c a").score(), 2.0/3));
        CHECK(qFuzzyCompare(m.match("c b a").score(), 2.0/3));
        CHECK(qFuzzyCompare(m.match("a b").score(), 1.0));
        CHECK(qFuzzyCompare(m.match("b a").score(), 1.0));
    }

    // Multiple ordered
    {
        Matcher m("a b", { .ignore_word_order = false });
        CHECK(m.match("a") == false);
        CHECK(m.match("b") == false);
        CHECK(m.match("a b") == true);
        CHECK(m.match("b a") == false);
        CHECK(m.match("a b c") == true);
        CHECK(m.match("a c b") == true);
        CHECK(m.match("b a c") == false);
        CHECK(m.match("c a b") == true);
        CHECK(m.match("b c a") == false);
        CHECK(m.match("c b a") == false);
    }

    // Diacritics
    {
        Matcher m("é");
        CHECK(m.match("e") == true);
        CHECK(m.match("é") == true);
        Matcher m2("e");
        CHECK(m2.match("e") == true);
        CHECK(m2.match("é") == true);
    }

    // Separators
    {
        Matcher m("a");
        CHECK(qFuzzyCompare(m.match("a b").score(), 1.0/2));
        CHECK(qFuzzyCompare(m.match("a!b").score(), 1./2));
        CHECK(qFuzzyCompare(m.match("a !b").score(), 1./2));
        CHECK(qFuzzyCompare(m.match("!a b").score(), 1./2));

        m = Matcher("a", { .separator_regex = QRegularExpression("[\\s]+") });
        CHECK(qFuzzyCompare(m.match("a b").score(), 1.0/2));
        CHECK(qFuzzyCompare(m.match("a!b").score(), 1./3));
        CHECK(qFuzzyCompare(m.match("a !b").score(), 1./3));
        CHECK(m.match("!a b") == false);
    }

    // Fuzzy
    {
        QString abc{"abcdefghijklmnopqrstuvwxyz"};

        MatchConfig c = {
            .separator_regex = QRegularExpression("[ ]+"),
            .error_tolerance_divisor = 3
        };

        CHECK(Matcher("abc", c).match(abc));
        CHECK(Matcher("ab_", c).match(abc));
        CHECK(!Matcher("a__", c).match(abc));
        CHECK(Matcher("abcdef", c).match(abc));
        CHECK(Matcher("abc_e_", c).match(abc));
        CHECK(!Matcher("a_c_e_", c).match(abc));
        CHECK(Matcher("abcdefghi", c).match(abc));
        CHECK(Matcher("abcdefg_i", c).match(abc));
        CHECK(Matcher("abcde_g_i", c).match(abc));
        CHECK(Matcher("abc_e_g_i", c).match(abc));
        CHECK(!Matcher("a_c_e_g_i", c).match(abc));

        c.error_tolerance_divisor = 4;

        CHECK(Matcher("abcd", c).match(abc));
        CHECK(Matcher("abc_", c).match(abc));
        CHECK(!Matcher("ab__", c).match(abc));
        CHECK(Matcher("abcdefgh", c).match(abc));
        CHECK(Matcher("abcdefg_", c).match(abc));
        CHECK(Matcher("abcde_g_", c).match(abc));
        CHECK(!Matcher("abc_e_g_", c).match(abc));
    }

    // Case
    {
        Matcher m("A", { .ignore_case = true });
        CHECK(m.match("A"));
        CHECK(m.match("a"));

        m = Matcher("A", { .ignore_case = false });
        CHECK(m.match("A"));
        CHECK(!m.match("a"));

        m = Matcher("a", { .ignore_case = true });
        CHECK(m.match("A"));
        CHECK(m.match("a"));

        m = Matcher("a", { .ignore_case = false });
        CHECK(!m.match("A"));
        CHECK(m.match("a"));
    }
}

