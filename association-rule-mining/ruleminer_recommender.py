import numpy as np 
import pandas as pd 
from sklearn.model_selection import train_test_split

raw_ratings = pd.read_csv("./ml-latest-small/ratings.csv")
raw_movies = pd.read_csv("./ml-latest-small/movies.csv")
raw_links = pd.read_csv("./ml-latest-small/links.csv")
raw_tags = pd.read_csv("./ml-latest-small/tags.csv")

users = raw_movies.merge(raw_tags, on='movieId', how='inner')
users.drop(columns=['tag','timestamp','genres'],inplace=True)

raw_ratings.drop(columns='timestamp',inplace=True)

# considering entries rated above 2 only
ratings=raw_ratings[raw_ratings['rating']>2]
ratings.head()

user_counts=ratings['userId'].value_counts()

# considering only users who have rated more than 10 movies
for i in range(1, 611):
    if(user_counts[i]<11):
        print(i, user_counts[i])
        ratings=ratings[ratings['userId']!=i]

print('\nRatings(preprocessed): \n', ratings.head())

# dividing into training and test datasets
ids = ratings['userId'].unique()

import warnings
warnings.filterwarnings('ignore')
univ_train = pd.DataFrame(columns = ['userId', 'movieId', 'rating'])
univ_test = pd.DataFrame(columns = ['userId', 'movieId', 'rating'])
for i in ids:
    train, test = train_test_split(ratings[ratings['userId']==i], test_size=0.2)
    univ_train = univ_train.append(train)
    univ_test = univ_test.append(test)

print('\nTrain: \n', univ_train.head())
print('\nTest: \n', univ_test.head())


movies = univ_train.copy(deep=True)
movies = movies.groupby(by = ['userId'])['movieId'].apply(list).reset_index()
movie_list = movies['movieId'].tolist()
test = univ_test.groupby(by = ['userId'])['movieId'].apply(list).reset_index()
# print(len(movie_list))
# print(movies['userId'].nunique())

unique_items = set()
for transaction in movie_list:
    for item in transaction:
        unique_items.add(item)
cols = sorted(unique_items)
cols_mapping = {}
for col_idx, item in enumerate(cols):
    cols_mapping[item] = col_idx

arr = np.zeros((len(movie_list), len(cols)), dtype=bool)
for row_idx, transaction in enumerate(movie_list):
    for item in transaction:
        col_idx = cols_mapping[item]
        arr[row_idx, col_idx] = True

df = pd.DataFrame(arr, columns=cols)

print('\nFinal dataframe: \n', df.head(), df.shape)

### ----- APRIORI ALGORITHM -----

from apriori import apriori, association_rules

apriori_frequent_itemsets = apriori(df, min_support=0.1, min_confidence=0.1)

print("\nApriori result counts: \n", apriori_frequent_itemsets['itemsets'].apply(lambda x: len(x)).value_counts())
print("\nApriori result DataFrame: \n", apriori_frequent_itemsets.head())

rules = association_rules(apriori_frequent_itemsets, min_confidence=0.1)
print("\nAssociation Rules DataFrame: \n", rules.head())

## ATQ List1 sorted according to support and List2 sorted according to confidence
List1 = rules.sort_values(by='support', ascending=False).head(100)
List2 = rules.sort_values(by='confidence', ascending=False).head(100)

print("\nList 1: \n", List1)
print("\nList 2: \n", List2)

## merged_list based on the intersection of the two lists, sorted on confidence, exported to rules.csv
merge = pd.merge(List1, List2, how='inner')
merge = merge.sort_values(by='confidence', ascending=False)
print("\nMerged List of Rules: \n", merge)

List1.to_csv('RulesMaxSupport.csv',index=False)
List2.to_csv('RulesMaxConf.csv',index=False)
merge.to_csv('AssocRules.csv',index=False)

## ----- RECOMMENDER SYSTEM -----

rules = rules.sort_values(by='confidence', ascending=False).reset_index(drop=True)

final_dict = []
acc_prec=0
acc_rec=0
for idx in movies.index:
    pred_df = dict.fromkeys(['userId', 'pred_set', 'test_set', 'hit_set', 'precision', 'recall'])
    user_pred_set = []
    for movie in movies['movieId'][idx]:
        cons = [list(x) for x in rules[rules['antecedants'].apply(lambda x: len(x)==1 and next(iter(x))==movie)]['consequents'].tolist()]
        # print(cons)
        if(len(cons)):
            for i in cons:
                for j in i:
                    user_pred_set.append(j)
    user_pred_set=list(set(user_pred_set)-set(movies['movieId'][idx]))
    test_set = test[test['userId']==movies['userId'][idx]]['movieId'].tolist()[0]
    # print(user_pred_set)
    # print(test_set)
    hit_set = list(set(user_pred_set) & set(test_set))
    pred_df['userId']=movies['userId'][idx]
    pred_df['pred_set']=user_pred_set
    pred_df['test_set']=test_set
    pred_df['hit_set']=hit_set
    prec = len(hit_set) / len(list(set(test_set)))
    acc_prec+=prec
    pred_df['precision']=prec
    rec = len(hit_set) / len(user_pred_set) if len(user_pred_set)>0 else 0
    acc_rec+=rec
    pred_df['recall']= rec
    # print(pred_df)
    final_dict.append(pred_df)

## Graphs an visualization

import matplotlib.pyplot as plt
import random

subset_len = int(len(final_dict)*0.1)
subset = random.sample(final_dict, subset_len)

x_rating=[]
y_rating=[]
precision=[]
recall=[]
for dic in subset:
    temp_test = univ_test[univ_test['userId']==dic['userId']]
    for i in dic['hit_set']:
        x_rating.append(i)
        y_rating.append(float(temp_test[temp_test['movieId']==i]['rating']))
        recall.append(dic['recall'])
        precision.append(dic['precision'])
# plt.bar(x_rating, y_rating, color ='maroon')
# plt.xlabel("MovieIds")
# plt.ylabel("Ratings")
# plt.title("MovieId vs Rating")
# plt.show()
# plt.savefig('movie_rating.png')

plt.scatter(y_rating, precision, c='maroon')
plt.ylabel("precision")
plt.xlabel("Ratings")
plt.title('Checking precision wrt ratings')
plt.show()
plt.savefig('precision.png')

plt.scatter(y_rating, recall, c='maroon')
plt.ylabel("recall")
plt.xlabel("Ratings")
plt.title('Checking recall wrt ratings')
plt.show()
plt.savefig('recall.png')

print("AVERAGE PRECISION: ", acc_prec/len(final_dict))
print("AVERAGE RECALL: ", acc_rec/len(final_dict))