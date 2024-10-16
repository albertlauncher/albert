// Copyright (c) 2024 Manuel Schneider

#include "itemindex.h"
#include "levenshtein.h"
#include "matcher.h"
#include "rankitem.h"
#include "standarditem.h"
#include "test.h"
#include "topologicalsort.hpp"
#include <chrono>
#include <iostream>
#include <set>
#include <unistd.h>
using namespace albert;
using namespace std::chrono;
using namespace std;

QTEST_MAIN(Test)

void Test::topological_sort_linear()
{
    auto result = topologicalSort(map<int, set<int>>{{1, {2}}, {2, {3}}, {3, {}}});
    auto expect = vector<int>{3, 2, 1};
    QCOMPARE(result.sorted, expect);
    QVERIFY(result.error_set.empty());
}

void Test::topological_sort_diamond()
{
    auto result = topologicalSort(map<int, set<int>>{{1, {}}, {2, {1}}, {3, {1}}, {4, {2, 3}}});
    // auto expect = vector<int>{1,2,3,4};  // or …
    auto expect = vector<int>{1, 3, 2, 4};
    QCOMPARE(result.sorted, expect);
    QVERIFY(result.error_set.empty());
}

void Test::topological_sort_cycle()
{
    auto result = topologicalSort(map<int, set<int>>{{1, {2}}, {2, {1}}});
    auto expect = map<int, set<int>>{{1, {2}}, {2, {1}}};
    QVERIFY(result.sorted.empty());
    QCOMPARE(result.error_set, expect);
}

void Test::topological_sort_not_existing_node()
{
    auto result = topologicalSort(map<int, set<int>>{{1, {2}}});
    auto expect = map<int, set<int>>{{1, {2}}};
    QVERIFY(result.sorted.empty());
    QCOMPARE(result.error_set, expect);
}

void Test::levenshtein_fast_levenshtein_threshold()
{
    Levenshtein l;

    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "0123456789", 0) == 0);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "0123456789", 1) == 0);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "0123456789", 2) == 0);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "0123456789", 3) == 0);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "0123456789", 4) == 0);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "0123456789", 5) == 0);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "0123456789", 6) == 0);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "0123456789", 7) == 0);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "0123456789", 8) == 0);

    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "-123456789", 0) == 1);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "-123456789", 1) == 1);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "-123456789", 2) == 1);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "-123456789", 3) == 1);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "-123456789", 4) == 1);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "-123456789", 5) == 1);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "-123456789", 6) == 1);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "-123456789", 7) == 1);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "-123456789", 8) == 1);

    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "--23456789", 0) == 1);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "--23456789", 1) == 2);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "--23456789", 2) == 2);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "--23456789", 3) == 2);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "--23456789", 4) == 2);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "--23456789", 5) == 2);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "--23456789", 6) == 2);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "--23456789", 7) == 2);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "--23456789", 8) == 2);

    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "--234-6789", 0) == 1);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "--234-6789", 1) == 2);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "--234-6789", 2) == 3);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "--234-6789", 3) == 3);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "--234-6789", 4) == 3);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "--234-6789", 5) == 3);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "--234-6789", 6) == 3);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "--234-6789", 7) == 3);
    QVERIFY(l.computePrefixEditDistanceWithLimit("0123456789", "--234-6789", 8) == 3);
}

void Test::levenshtein_fuzzy_substitution()
{
    Levenshtein l;
    QVERIFY(l.computePrefixEditDistanceWithLimit("test", "_est____", 1) == 1);
    QVERIFY(l.computePrefixEditDistanceWithLimit("test", "__st____", 1) == 2);
    QVERIFY(l.computePrefixEditDistanceWithLimit("test", "_est____", 2) == 1);
    QVERIFY(l.computePrefixEditDistanceWithLimit("test", "__st____", 2) == 2);
    QVERIFY(l.computePrefixEditDistanceWithLimit("test", "___t____", 2) == 3);
}

void Test::levenshtein_fuzzy_deletion()
{
    Levenshtein l;
    QVERIFY(l.computePrefixEditDistanceWithLimit("test", "ttest____", 1) == 1);
    QVERIFY(l.computePrefixEditDistanceWithLimit("test", "tttest____", 1) == 2);
}

void Test::levenshtein_fuzzy_insertion()
{
    Levenshtein l;
    QVERIFY(l.computePrefixEditDistanceWithLimit("test", "est____", 1) == 1);
    QVERIFY(l.computePrefixEditDistanceWithLimit("test", "st____", 1) == 2);
}

void Test::levenshtein_shorter_prefix()
{
    Levenshtein l;
    QVERIFY(l.computePrefixEditDistanceWithLimit("abc", "abc", 1) == 0);
    QVERIFY(l.computePrefixEditDistanceWithLimit("abc", "ab", 1) == 1);
    QVERIFY(l.computePrefixEditDistanceWithLimit("abc", "a", 1) == 2);
    QVERIFY(l.computePrefixEditDistanceWithLimit("abc", "", 1) == 2);
}

void Test::matcher_empty()
{
    Matcher m("");
    QVERIFY(qFuzzyCompare(m.match("a").score(), .0));
    QVERIFY(qFuzzyCompare(m.match("a b").score(), .0));
    QVERIFY(m.match("a") == true);
    QVERIFY(m.match("a b") == true);
    QVERIFY(m.match("a").isMatch() == true);
    QVERIFY(m.match("a b").isMatch() == true);
    QVERIFY(m.match("a").isEmptyMatch() == true);
    QVERIFY(m.match("a b").isEmptyMatch() == true);
    QVERIFY(m.match("a").isExactMatch() == false);
    QVERIFY(m.match("a b").isExactMatch() == false);
}

void Test::matcher_single()
{
    Matcher m("a");
    QVERIFY(qFuzzyCompare(m.match("a").score(), 1.0));
    QVERIFY(qFuzzyCompare(m.match("a b").score(), 1.0 / 2));
    QVERIFY(m.match("a") == true);
    QVERIFY(m.match("a b") == true);
    QVERIFY(m.match("a").isMatch() == true);
    QVERIFY(m.match("a b").isMatch() == true);
    QVERIFY(m.match("a").isEmptyMatch() == false);
    QVERIFY(m.match("a b").isEmptyMatch() == false);
    QVERIFY(m.match("a").isExactMatch() == true);
    QVERIFY(m.match("a b").isExactMatch() == false);
}

void Test::matcher_multiple()
{
    Matcher m("a b");
    QVERIFY(m.match("a") == false);
    QVERIFY(m.match("b") == false);
    QVERIFY(m.match("a b") == true);
    QVERIFY(m.match("b a") == true);
    QVERIFY(m.match("a b c") == true);
    QVERIFY(m.match("a c b") == true);
    QVERIFY(m.match("b a c") == true);
    QVERIFY(m.match("c a b") == true);
    QVERIFY(m.match("b c a") == true);
    QVERIFY(m.match("c b a") == true);
    QVERIFY(qFuzzyCompare(m.match("a b c").score(), 2.0 / 3));
    QVERIFY(qFuzzyCompare(m.match("a c b").score(), 2.0 / 3));
    QVERIFY(qFuzzyCompare(m.match("b a c").score(), 2.0 / 3));
    QVERIFY(qFuzzyCompare(m.match("c a b").score(), 2.0 / 3));
    QVERIFY(qFuzzyCompare(m.match("b c a").score(), 2.0 / 3));
    QVERIFY(qFuzzyCompare(m.match("c b a").score(), 2.0 / 3));
    QVERIFY(qFuzzyCompare(m.match("a b").score(), 1.0));
    QVERIFY(qFuzzyCompare(m.match("b a").score(), 1.0));
}

void Test::matcher_multiple_ordered()
{
    Matcher m("a b", {.ignore_word_order = false});
    QVERIFY(m.match("a") == false);
    QVERIFY(m.match("b") == false);
    QVERIFY(m.match("a b") == true);
    QVERIFY(m.match("b a") == false);
    QVERIFY(m.match("a b c") == true);
    QVERIFY(m.match("a c b") == true);
    QVERIFY(m.match("b a c") == false);
    QVERIFY(m.match("c a b") == true);
    QVERIFY(m.match("b c a") == false);
    QVERIFY(m.match("c b a") == false);
}

void Test::matcher_diacritics()
{
    Matcher m("é");
    QVERIFY(m.match("e") == true);
    QVERIFY(m.match("é") == true);
    Matcher m2("e");
    QVERIFY(m2.match("e") == true);
    QVERIFY(m2.match("é") == true);
}

void Test::matcher_seprarators()
{
    Matcher m("a");
    QVERIFY(qFuzzyCompare(m.match("a b").score(), 1.0 / 2));
    QVERIFY(qFuzzyCompare(m.match("a!b").score(), 1. / 2));
    QVERIFY(qFuzzyCompare(m.match("a !b").score(), 1. / 2));
    QVERIFY(qFuzzyCompare(m.match("!a b").score(), 1. / 2));

    m = Matcher("a", {.separator_regex = QRegularExpression("[\\s]+")});
    QVERIFY(qFuzzyCompare(m.match("a b").score(), 1.0 / 2));
    QVERIFY(qFuzzyCompare(m.match("a!b").score(), 1. / 3));
    QVERIFY(qFuzzyCompare(m.match("a !b").score(), 1. / 3));
    QVERIFY(m.match("!a b") == false);
}

void Test::matcher_fuzzy()
{
    QString abc{"abcdefghijklmnopqrstuvwxyz"};

    MatchConfig c = {.separator_regex = QRegularExpression("[ ]+"), .fuzzy = true};

    QVERIFY(Matcher("abcd", c).match(abc));
    QVERIFY(Matcher("abc_", c).match(abc));
    QVERIFY(!Matcher("ab__", c).match(abc));
    QVERIFY(Matcher("abcdefgh", c).match(abc));
    QVERIFY(Matcher("abcdefg_", c).match(abc));
    QVERIFY(Matcher("abcde_g_", c).match(abc));
    QVERIFY(!Matcher("abc_e_g_", c).match(abc));
}

void Test::matcher_case()
{
    auto m = Matcher("A", {.ignore_case = true});
    QVERIFY(m.match("A"));
    QVERIFY(m.match("a"));

    m = Matcher("A", {.ignore_case = false});
    QVERIFY(m.match("A"));
    QVERIFY(!m.match("a"));

    m = Matcher("a", {.ignore_case = true});
    QVERIFY(m.match("A"));
    QVERIFY(m.match("a"));

    m = Matcher("a", {.ignore_case = false});
    QVERIFY(!m.match("A"));
    QVERIFY(m.match("a"));
}

void Test::matcher_score()
{
    auto m = Matcher("a");

    QCOMPARE(m.match("a").score(), 1./1.);
    QCOMPARE(m.match("a ab").score(), 1./3.);
    QCOMPARE(m.match("a ab abc").score(), 1./6.);

    m = Matcher("ab");

    QCOMPARE(m.match("a").score(), -1);
    QCOMPARE(m.match("a ab").score(), 2./3.);
    QCOMPARE(m.match("a ab abc").score(), 2./6.);

    m = Matcher("abcc", {.fuzzy = true});

    QCOMPARE(m.match("ab--").score(), -1);
    QCOMPARE(m.match("abcc").score(), 1.);
    QCOMPARE(m.match("abcd").score(), 3./4.);
}

static auto indexMatch(const QStringList &item_strings,
                       const QString &search_string,
                       const MatchConfig &config = {})
{
    ItemIndex index(config);

    vector<IndexItem> index_items;
    for (auto &string : item_strings)
        index_items.emplace_back(make_shared<StandardItem>(string), string);

    index.setItems(::move(index_items));

    return index.search(search_string, true);
};

void Test::index_empty()
{
    auto m = indexMatch({"a","A"}, "");
    QVERIFY(m.size() == 2);
    QVERIFY(m[0].score == 0.);
    QVERIFY(m[1].score == 0.);
}

static const QStringList abc_perm
{
    "a b c",
    "a c b",
    "b a c",
    "c ä b",
    "b c a",
    "c b ã"
};

void Test::index_multiple()
{
    QVERIFY(indexMatch(abc_perm, "a").size() == 6);
    QVERIFY(indexMatch(abc_perm, "a b").size() == 6);
    QVERIFY(indexMatch(abc_perm, "a b c").size() == 6);
}

void Test::index_multiple_ordered()
{
    QVERIFY(indexMatch(abc_perm, "a", {.ignore_word_order = false}).size() == 6);
    QVERIFY(indexMatch(abc_perm, "a b", {.ignore_word_order = false}).size() == 3);
    QVERIFY(indexMatch(abc_perm, "a b c", {.ignore_word_order = false}).size() == 1);
}

void Test::index_diacritics()
{
    QVERIFY(indexMatch(abc_perm, "a", {.ignore_diacritics = false}).size() == 4);
    QVERIFY(indexMatch(abc_perm, "a b", {.ignore_diacritics = false}).size() == 4);
    QVERIFY(indexMatch(abc_perm, "a b c", {.ignore_diacritics = false}).size() == 4);
    QVERIFY(indexMatch(abc_perm, "b", {.ignore_diacritics = false}).size() == 6);
}

void Test::index_separators()
{
    QVERIFY(indexMatch({"a!b", "a b","a-b"}, "a b").size() == 3);
    QVERIFY(indexMatch({"a!b", "a b","a-b"}, "a b",
                       {.separator_regex = QRegularExpression("[ ]+")}).size() == 1);
}

void Test::index_fuzzy()
{
    QStringList abc{"abcdefghijklmnopqrstuvwxyz"};

    MatchConfig c = {
        .separator_regex = QRegularExpression("[ ]+"),
        .fuzzy = true
    };

    QVERIFY(indexMatch(abc, "abcd", c).size() == 1);
    QVERIFY(indexMatch(abc, "abc_", c).size() == 1);
    QVERIFY(indexMatch(abc, "ab__", c).size() == 0);
    QVERIFY(indexMatch(abc, "abcdefgh", c).size() == 1);
    QVERIFY(indexMatch(abc, "abcdefg_", c).size() == 1);
    QVERIFY(indexMatch(abc, "abcde_g_", c).size() == 1);
    QVERIFY(indexMatch(abc, "abc_e_g_", c).size() == 0);
}

void Test::index_case()
{
    auto m = indexMatch({"a","A"}, "a", {});
    QVERIFY(m.size() == 2);
    QVERIFY(m[0].score == 1.);
    QVERIFY(m[1].score == 1.);
    m = indexMatch({"a","A"}, "a", {.ignore_case = false});
    QVERIFY(m.size() == 1);
    QVERIFY(m[0].score == 1.);
}

void Test::index_score()
{
    auto m = indexMatch({"a","ab","abc"}, "a",  {.fuzzy = true});

    QVERIFY(m.size() == 3);
    sort(m.begin(), m.end(), [](auto &a, auto &b){ return a.item->id() < b.item->id(); });
    QVERIFY(qFuzzyCompare(m[0].score, 1.0));
    QVERIFY(qFuzzyCompare(m[1].score, 1.0/2.0));
    QVERIFY(qFuzzyCompare(m[2].score, 1.0/3.0));

    m = indexMatch({"abcd","abcb"}, "abcc", {.fuzzy = true});

    QVERIFY(m.size() == 2);
    QVERIFY(qFuzzyCompare(m[0].score, 3./4.));
    QVERIFY(qFuzzyCompare(m[1].score, 3./4.));
}

static string gen_random(const int len) {
    static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
    string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}

static void levenshtein_compare_benchmarks_and_check_results(const vector<QString> &strings, uint k)
{
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

    cout << "Levensthein old: "
         << setw(12)
         << duration_old
         << " µs. New: "
         << setw(12)
         << duration_new
         << " µs. Improvement: "
         << (double)duration_old/duration_new
         << endl;

    QVERIFY(results_old == results_new);
}

void Test::benchmark_comparison_vanilla_vs_fast_levenshtein()
{
    int test_count = 100000;

    srand((unsigned)time(NULL) * getpid());

    vector<QString> strings(test_count);
    auto lens = {4,8,16,24};
    auto divisor=4;
    cout << "Randoms"<<endl;
    for (int len : lens){
        int k = floor(len/divisor);
        cout << "len: "<< setw(2)<<len<<". k: "<<k<<" ";
        for (auto &string : strings)
            string = QString::fromStdString(gen_random(len));
        levenshtein_compare_benchmarks_and_check_results(strings, k);
    }

    cout << "Equals"<<endl;
    for (int len : lens) {
        int k = floor(len/divisor);
        cout << "len: "<< setw(2)<<len<<". k: "<<k<<" ";
        for (auto &string : strings)
            string = QString(len, 'a');
        levenshtein_compare_benchmarks_and_check_results(strings, k);
    }

    cout << "Halfhalf equal random"<<endl;
    for (int len : lens) {
        int k = floor(len/divisor);
        cout << "len: "<< setw(2)<<len<<". k: "<<k<<" ";
        for (auto &string : strings)
            string = QString("%1%2").arg(QString(len/2, 'a'), QString::fromStdString(gen_random(len/2)));
        levenshtein_compare_benchmarks_and_check_results(strings, k);
    }
}


// -------------------------------------------------------------------------------------------------

template<typename S>
static void benchmark_hash(const S &s)
{
    size_t h = 0;
    auto hf = std::hash<S>();
    QBENCHMARK {
        for (int i = 0; i < 1'000'000; ++i)
            h |= hf(s);
    }
    qDebug() << h;
}

QStringList strings = {
    "0123456789",
    "abcdefghijklmnopqrstuvwxyz",
    "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZÜÖÄ,.-;:_+*#@<>)"
};

void Test::benchmark_hash_string()
{
    for (auto &s : strings)
        benchmark_hash(s.toStdString());
}

void Test::benchmark_hash_string_view()
{
    for (auto &s : strings)
        benchmark_hash(string_view(s.toStdString()));
}

void Test::benchmark_hash_u16string()
{
    for (auto &s : strings)
        benchmark_hash(s.toStdU16String());
}

void Test::benchmark_hash_u16string_view()
{
    for (auto &s : strings)
        benchmark_hash(u16string_view(s.toStdU16String()));
}

void Test::benchmark_hash_qstring()
{
    for (auto &s : strings)
        benchmark_hash(s);
}

void Test::benchmark_hash_qstring_view()
{
    for (auto &s : strings)
        benchmark_hash(QStringView(s));
}


// -------------------------------------------------------------------------------------------------

static std::hash<std::string> s_hash;
static std::hash<std::u8string> u8_hash;
static std::hash<std::u16string> u16_hash;
static std::hash<QString> q_hash;

template <>
struct hash<std::pair<QString, QString>>
{
    // https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key#comment39936543_17017281
    inline size_t operator()(const std::pair<QString, QString>& k) const noexcept
    { return (qHash(k.first) ^ (qHash(k.second) << 1)); }
};

template <>
struct hash<std::pair<string, string>>
{
    // https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key#comment39936543_17017281
    inline size_t operator()(const std::pair<string, string>& k) const noexcept
    { return (s_hash(k.first) ^ (s_hash(k.second)<< 1)); }
};

template <>
struct hash<std::pair<u8string, u8string>>
{
    // https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key#comment39936543_17017281
    inline size_t operator()(const std::pair<u8string, u8string>& k) const noexcept
    { return (u8_hash(k.first) ^ (u8_hash(k.second)<< 1)); }
};

template <>
struct hash<std::pair<u16string, u16string>>
{
    // https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key#comment39936543_17017281
    inline size_t operator()(const std::pair<u16string, u16string>& k) const noexcept
    { return (u16_hash(k.first) ^ (u16_hash(k.second)<< 1)); }
};


template<typename S>
static void benchmark_hash_pair(const S &s)
{
    auto p = make_pair(s, s);
    auto h = std::hash<std::pair<S, S>>();
    QBENCHMARK {
        for (int i = 0; i < 1'000'000; ++i)
            h(p);
    }
}


void Test::benchmark_hash_pair_qstring()
{ benchmark_hash_pair(QString("abcdefghijklmnopqrstuvwxyz")); }

void Test::benchmark_hash_pair_string()
{ benchmark_hash_pair(u8"abcdefghijklmnopqrstuvwxyz"s); }

void Test::benchmark_hash_pair_u16string()
{ benchmark_hash_pair(u"abcdefghijklmnopqrstuvwxyz"s); }


// -------------------------------------------------------------------------------------------------

#include <iostream>
#include <string>
#include <vector>
#include <random>
#include "timeit.h"


std::string generateRandomWord(size_t length) {
    const std::string charset = "abcdefghijklmnopqrstuvwxyz";
    std::string result;
    std::default_random_engine generator(std::random_device{}());
    std::uniform_int_distribution<size_t> distribution(0, charset.size() - 1);

    for (size_t i = 0; i < length; ++i) {
        result += charset[distribution(generator)];
    }
    return result;
}


double generateRandomDouble(double min, double max) {
    std::default_random_engine generator(std::random_device{}());
    std::uniform_real_distribution<double> distribution(min, max);
    return distribution(generator);
}

template<
    template<
        typename ...
        > typename C,
    typename KD
    >
struct Benchmark
{
    const char * n;
    C<pair<KD, KD>, double> c;

    Benchmark(const char *name, vector<tuple<KD, KD, double>> &data)
        : n(name)
    {
        for (const auto &[k1, k2, d] : data)
            c.emplace(pair<KD,KD>{k1, k2}, d);
    }

    template<typename K>
    auto & run(const char *name, vector<tuple<K, K>> &lookup_strings)
    {
        for (int i = 0; i < 5; ++i) {
            TimeIt t(QString("%1 %2").arg(n, name));

            if constexpr (std::is_same<KD, K>::value)
                for (const auto &[k1, k2] : lookup_strings)
                    c.contains({k1, k2});

            else if constexpr (is_same<KD, QString>::value && is_same<K, string>::value)
                for (const auto &[k1, k2] : lookup_strings)
                    c.contains({QString::fromStdString(k1), QString::fromStdString(k2)});


            else if constexpr (is_same<KD, string>::value && is_same<K, QString>::value)
                for (const auto &[k1, k2] : lookup_strings)
                    c.contains({k1.toStdString(), k2.toStdString()});

            else if constexpr (is_same<KD, QString>::value && is_same<K, string>::value)
                for (const auto &[k1, k2] : lookup_strings)
                    c.contains({QString::fromStdString(k1), QString::fromStdString(k2)});


            else if constexpr (is_same<KD, u16string>::value && is_same<K, QString>::value)
            {
                u16string u1, u2;
                for (const auto &[k1, k2] : lookup_strings)
                    c.contains({u16string(reinterpret_cast<const char16_t*>(k1.utf16()), k1.size()),
                                u16string(reinterpret_cast<const char16_t*>(k2.utf16()), k2.size())});
            }

            else if constexpr (is_same<KD, QString>::value && is_same<K, u16string>::value)
                for (const auto &[k1, k2] : lookup_strings)
                    c.contains({QString::fromStdU16String(k1), QString::fromStdU16String(k2)});

            else
                std::cout << "run_ is not available for this type.\n";
        }
        return *this;
    }
};


void Test::benchmark_maps()
{
    vector<tuple<string, string, double>> cdata;
    for (size_t i = 0; i < 10'000; ++i)
        cdata.emplace_back(generateRandomWord(10), generateRandomWord(10), generateRandomDouble(0, 1));

    vector<tuple<QString, QString, double>> qdata;
    for (auto & [k1, k2, d] : cdata)
        qdata.emplace_back(QString::fromStdString(k1), QString::fromStdString(k2), d);

    vector<tuple<u16string, u16string, double>> u16data;
    for (auto & [k1, k2, d] : qdata)
        u16data.emplace_back(k1.toStdU16String(), k2.toStdU16String(), d);


    vector<tuple<string, string>> clookup_strings;
    for (size_t i = 0; i < 1'000'000; ++i)
        clookup_strings.emplace_back(generateRandomWord(10), generateRandomWord(10));

    vector<tuple<QString, QString>> qlookup_strings;
    for (auto & [k1, k2] : clookup_strings)
        qlookup_strings.emplace_back(QString::fromStdString(k1), QString::fromStdString(k2));



    vector<tuple<u16string, u16string>> u16lookup_strings;
    for (auto & [k1, k2] : qlookup_strings)
        u16lookup_strings.emplace_back(k1.toStdU16String(), k2.toStdU16String());



    Benchmark<QHash, QString>("QHash QString", qdata)
        .run("QString", qlookup_strings)
        .run("string", clookup_strings);

    Benchmark<QHash, string>("QHash string", cdata)
        .run("QString", qlookup_strings)
        .run("string", clookup_strings);

    Benchmark<unordered_map, QString>("unordered_map QString", qdata)
        .run("QString", qlookup_strings)
        .run("string", clookup_strings);

    Benchmark<unordered_map, string>("unordered_map string", cdata)
        .run("QString", qlookup_strings)
        .run("string", clookup_strings);

    Benchmark<QHash, u16string>("QHash u16string", u16data)
        .run("QString", qlookup_strings)
        .run("u16string", u16lookup_strings);

    Benchmark<unordered_map, u16string>("unordered_map u16string", u16data)
        .run("QString", qlookup_strings)
        .run("u16string", u16lookup_strings);
}

