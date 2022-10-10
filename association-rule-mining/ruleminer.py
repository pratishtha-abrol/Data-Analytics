import numpy as np 
import pandas as pd 

raw_ratings = pd.read_csv("./ml-latest-small/ratings.csv")
raw_movies = pd.read_csv("./ml-latest-small/movies.csv")
raw_links = pd.read_csv("./ml-latest-small/links.csv")
raw_tags = pd.read_csv("./ml-latest-small/tags.csv")