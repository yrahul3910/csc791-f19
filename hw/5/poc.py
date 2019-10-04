from sklearn.naive_bayes import GaussianNB
from sklearn.metrics import classification_report, accuracy_score
import pandas as pd
import numpy as np

df = pd.read_csv('diabetes.csv')
X = np.array(df[df.columns[:-1]])
y = np.array(df[df.columns[-1]])


nb = GaussianNB()
nb.partial_fit(X[:30], y[:30], classes=['tested_positive', 'tested_negative'])

preds = []
targs = []

for i in range(30, len(df)):
    preds.append(nb.predict(X[i].reshape(1, -1)))
    targs.append(y[i])
    nb.partial_fit(X[i].reshape(1, -1), [y[i]])

print(classification_report(preds, targs))
print('Accuracy =', accuracy_score(preds, targs))

means = [[3.298, 109.98, 68.184, 19.664, 68.792, 30.3042, 0.429734, 31.19],
        [4.86567, 141.257, 70.8246, 22.1642, 100.336, 35.1425, 0.5505, 37.0672]]
sd = [[9.1034, 683.362, 326.275, 221.711, 9774.35, 59.1339, 0.089452, 136.134],
      [13.9969, 1020.14, 461.898, 312.572, 19234.7, 52.7507, 0.138648, 120.303]]
print('MAE for means =', np.mean(np.abs(nb.theta_ - means)))
print('MAE for var =', np.mean(np.abs(nb.sigma_ - sd)))

print('MAPE for means =', np.mean(np.abs((nb.theta_ - means) / nb.theta_)) * 100)
print('MAPE for var =', np.mean(np.abs((nb.sigma_ - sd) / nb.sigma_)) * 100)