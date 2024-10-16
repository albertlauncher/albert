// Copyright (c) 2024 Manuel Schneider

#include <QCoreApplication>
#include <QtTest/QtTest>

class Test : public QObject
{
    Q_OBJECT

private slots:

    void topological_sort_linear();
    void topological_sort_diamond();
    void topological_sort_cycle();
    void topological_sort_not_existing_node();

    void levenshtein_fast_levenshtein_threshold();
    void levenshtein_fuzzy_substitution();
    void levenshtein_fuzzy_deletion();
    void levenshtein_fuzzy_insertion();
    void levenshtein_shorter_prefix();

    void matcher_empty();
    void matcher_single();
    void matcher_multiple();
    void matcher_multiple_ordered();
    void matcher_diacritics();
    void matcher_seprarators();
    void matcher_fuzzy();
    void matcher_case();
    void matcher_score();

    void index_empty();
    void index_multiple();
    void index_multiple_ordered();
    void index_diacritics();
    void index_separators();
    void index_fuzzy();
    void index_case();
    void index_score();

    void benchmark_comparison_vanilla_vs_fast_levenshtein();

    void benchmark_hash_qstring();
    void benchmark_hash_qstring_view();
    void benchmark_hash_string();
    void benchmark_hash_string_view();
    void benchmark_hash_u16string();
    void benchmark_hash_u16string_view();

    void benchmark_hash_pair_qstring();
    void benchmark_hash_pair_string();
    void benchmark_hash_pair_u16string();

    void benchmark_maps();

};
