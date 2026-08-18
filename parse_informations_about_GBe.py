import pandas as pd
from collections import Counter
if __name__ == "__main__":
    k = 8

    TOL = 1e-7

    df = pd.read_csv(f"./output/results_GBe_{k}.csv")

    n_anc = df.shape[0]

    with open('./ancestors/ancestors_new_8.csv', 'r') as f:
        anc = f.readlines()

    for i in range(n_anc):
        if df.iloc[i]['family_gapII'] >= 4/3 + TOL:
            print(anc[i+1])

   # # print(df.head())
   #
   #  n_anc = df.shape[0]
   #
   #  UB_for_gap = max(df['family_gapII'])
   #
   #  D = Counter(df['family_gapII'])
   #
   #  for x in D.keys():
   #      if x > 4/3:
   #          print(x, D[x])
   #
   #  max_iter = max(df['iterations'])
   #
   #  print(f"{k} & {n_anc} & {UB_for_gap} & {max_iter} ")