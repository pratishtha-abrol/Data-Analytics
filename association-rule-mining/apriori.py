from array import array
import numpy as np
import pandas as pd
from itertools import combinations


def generate_new_combinations(old_combinations):
    items_types_in_previous_step = np.unique(old_combinations.flatten())
    for old_combination in old_combinations:
        max_combination = old_combination[-1]
        mask = items_types_in_previous_step > max_combination
        valid_items = items_types_in_previous_step[mask]
        old_tuple = tuple(old_combination)
        for item in valid_items:
            yield from old_tuple
            yield item

def apriori(df, min_support, min_confidence):
    def _support(x, n_row):
        out = np.sum(x, axis=0)/n_row
        return np.array(out).reshape(-1)
    
    if (min_support<=0.0):
        print("min_support must be positive")

    if (min_confidence<=0.0):
        print("min_confidence must be positive")
    
    X = df.values
    support = _support(X, X.shape[0])
    ary_col_idx = np.arange(X.shape[1])
    support_dict = {1: support[support >= min_support]}
    itemset_dict = {1: ary_col_idx[support >= min_support].reshape(-1, 1)}
    max_itemset = 1
    rows_count = float(X.shape[0])

    while max_itemset and max_itemset<float("inf") :
        next_max_itemset = max_itemset+1
        combin = generate_new_combinations(itemset_dict[max_itemset])
        combin = np.fromiter(combin, dtype=int)
        combin = combin.reshape(-1, next_max_itemset)
        if combin.size==0:
            break
        _bools = np.all(X[:,combin], axis=2)

        support = _support(np.array(_bools), rows_count)
        _mask = (support >= min_support).reshape(-1)
        if any(_mask):
            itemset_dict[next_max_itemset] = np.array(combin[_mask])
            support_dict[next_max_itemset] = np.array(support[_mask])
            max_itemset = next_max_itemset
        else:
            break

    all_res = []
    for k in sorted(itemset_dict):
        support = pd.Series(support_dict[k])
        itemsets = pd.Series([frozenset(i) for i in itemset_dict[k]], dtype='object')

        res = pd.concat((support, itemsets), axis=1)
        all_res.append(res)
        
    res_df = pd.concat(all_res)
    res_df.columns = ["support", "itemsets"]
    mapping = {idx: item for idx, item in enumerate(df.columns)}
    res_df["itemsets"] = res_df["itemsets"].apply(
        lambda x: frozenset([mapping[i] for i in x])
    )
    res_df = res_df.reset_index(drop=True)
    return res_df


def association_rules(df, min_confidence):
    def conviction_helper(sAC, sA, sC):
        confidence = sAC / sA
        conviction = np.empty(confidence.shape, dtype=float)
        if not len(conviction.shape):
            conviction = conviction[np.newaxis]
            confidence = confidence[np.newaxis]
            sAC = sAC[np.newaxis]
            sA = sA[np.newaxis]
            sC = sC[np.newaxis]
        conviction[:] = np.inf
        conviction[confidence < 1.0] = (1.0 - sC[confidence < 1.0]) / (
            1.0 - confidence[confidence < 1.0]
        )
        return conviction

    metric_dict = {
        "antecedent support": lambda _, sA, __: sA,
        "consequent support": lambda _, __, sC: sC,
        "support": lambda sAC, _, __: sAC,
        "confidence": lambda sAC, sA, _: sAC / sA,
        "lift": lambda sAC, sA, sC: metric_dict["confidence"](sAC, sA, sC) / sC,
        "leverage": lambda sAC, sA, sC: metric_dict["support"](sAC, sA, sC) - sA * sC,
        "conviction": lambda sAC, sA, sC: conviction_helper(sAC, sA, sC),
    }

    columns_ordered = [
        "antecedent support",
        "consequent support",
        "support",
        "confidence",
        "lift",
        "leverage",
        "conviction",
    ]

    keys = df["itemsets"].values
    values = df["support"].values
    frozenset_vect = np.vectorize(lambda x: frozenset(x))
    frequent_items_dict = dict(zip(frozenset_vect(keys), values))

    rule_antecedents = []
    rule_consequents = []
    rule_supports = []

    for k in frequent_items_dict.keys():
        sAC = frequent_items_dict[k]
        for idx in range(len(k)-1, 0, -1):
            for c in combinations(k, r=idx):
                antecedant = frozenset(c)
                consequent = k.difference(antecedant)

                sA = frequent_items_dict[antecedant]
                sC = frequent_items_dict[consequent]

                score = metric_dict["confidence"](sAC, sA, sC)
                if score>min_confidence:
                    rule_antecedents.append(antecedant)
                    rule_consequents.append(consequent)
                    rule_supports.append([sAC, sA, sC])
    
    if not rule_supports:
        return pd.DataFrame(columns=['antecedants', 'consequents'] + columns_ordered)
    else:
        rule_supports = np.array(rule_supports).T.astype(float)
        df_res = pd.DataFrame(
            data=list(zip(rule_antecedents, rule_consequents)),
            columns=['antecedants', 'consequents'],
        )

        sAC = rule_supports[0]
        sA = rule_supports[1]
        sC = rule_supports[2]
        for m in columns_ordered:
            df_res[m] = metric_dict[m](sAC,sA,sC)
        
        return df_res