---
jupyter:
  jupytext:
    formats: ipynb,md
    text_representation:
      extension: .md
      format_name: markdown
      format_version: '1.2'
      jupytext_version: 1.7.1
  kernelspec:
    display_name: Python 3
    language: python
    name: python3
---

### Statistics

A **statistic** is something you calculate on a sequence of r.v. $X_1, \dots X_n$.

**Degree of freedom** is the number of variables in the calculation of a statistic that are free to vary.

A **statistical experiment** is a pair (Event Space, Probability distribution).

```python
import numpy as np
from scipy import stats
```

### Confidence interval

```python
# Example: given data (normal distribution) and desired level of confidence,
# calculate the confidence interval of mean
def mean_confidence_interval(data, confidence=0.95):
    n = len(data)
    # mean and std error of mean
    m, se = np.mean(data), stats.sem(data)
    # PPF, percent-point-function inverse of the Gaussian CDF
    h = se * stats.t.ppf((1 + confidence) / 2., n - 1)
    return m, m - h, m + h

mu = 100
sigma = 10
samples = 10
data = np.random.normal(mu, sigma, samples)
mean_confidence_interval(data, 0.95)
# With 95% confidence, population mean fall into this range.
```

### Parametric hypothesis testing

Type I error is where you erroneously rejected null hypothesis.

Type II error is where you failed to reject null hypothesis whereas you should have.

A **test** is a statistic that maps sample data to 0 or 1, 0 - reject null hypothesis, 1 - don't reject null hypothesis.

**Level of the test** (defined on $\Theta$) is when we shouldn't reject null hypothesis ($\theta \in \Theta_0$), the maximum likelihood that we do (erroneously) reject it.

**Power of the test** (defined on $\Theta$) is when we should reject null hypothesis ($\theta \in \Theta_1$), the minimum likelihood of us (correctly) rejecting null hypothesis.

**p-value of the test** is the smallest level at which the test rejects the null hypothesis.


```python
# T-test

from scipy import stats

## Define 2 random distributions
# Sample Size
N = 10
# Gaussian distributed data with mean = 2 and var = 1
a = np.random.randn(N) + 2
# Gaussian distributed data with with mean = 0 and var = 1
b = np.random.randn(N)

## Calculate the Standard Deviation
# Calculate the variance to get the standard deviation

# For unbiased max likelihood estimate we have to divide the var by N-1, and therefore the parameter ddof = 1
var_a = a.var(ddof=1)
var_b = b.var(ddof=1)

# std deviation
s = np.sqrt((var_a + var_b)/2)
s
```

```python
## Calculate the t-statistics
t = (a.mean() - b.mean()) / (s * np.sqrt(2 / N))

## Compare with the critical t-value
# Degrees of freedom
df = 2 * N - 2

# p-value after comparison with the t 
p = 1 - stats.t.cdf(t, df = df)


print("t = " + str(t))
print("p = " + str(2*p))
### You can see that after comparing the t statistic with the critical t value (computed internally) we get a good p value of 0.0005 and thus we reject the null hypothesis and thus it proves that the mean of the two distributions are different and statistically significant.

## Cross Checking with the internal scipy function
t2, p2 = stats.ttest_ind(a, b)
print("t = " + str(t2))
print("p = " + str(p2))
```

```python
a1 = np.random.randn(N)
b1 = np.random.randn(N)

stats.ttest_ind(a1, b1)
```

```python
# Permutation testing
```

### Linear regression


Suppose model $y = \beta_1 x_1 + \beta_2 x_2 + \epsilon$ where the true $\beta$s are $(1, 1)$.


If $x_1$ and $x_2$ are uncorrelated.

```python
mu = 0
sigma = 1
X = np.random.normal(mu, sigma, (1000, 10000, 2))
e = np.random.normal(mu, sigma, (1000, 10000))
y = X[:,:,0] + X[:,:,1] + e

y.shape
```

```python
from sklearn import datasets, linear_model
from sklearn.metrics import mean_squared_error, r2_score

coefs = [None] * 1000

for i in range(0, 1000):
    regr = linear_model.LinearRegression()
    regr.fit(X[i], y[i])
    coefs[i] = regr.coef_

coefs_arr = np.array(coefs)
```

```python
import plotly.express as px
fig = px.scatter(x=coefs_arr[:,0], y=coefs_arr[:,1])
fig.show()
```

The scatter plot $(beta_1, beta_2)$ is a circle around true $beta$s (1, 1)


If $x_1$ and $x_2$ are correlate (drawn from a bivariate normal with 0.9 correlation

```python
means = (0, 0)
cov = [
    [1, 0.9],
    [0.9, 1],
]

X = np.random.multivariate_normal(means, cov, (1000, 10000))
e = np.random.normal(mu, sigma, (1000, 10000))
y = X[:,:,0] + X[:,:,1] + e

y.shape
```

```python
for i in range(0, 1000):
    regr = linear_model.LinearRegression()
    regr.fit(X[i], y[i])
    coefs[i] = regr.coef_

coefs_arr = np.array(coefs)
```

```python
fig = px.scatter(x=coefs_arr[:,0], y=coefs_arr[:,1])
fig.show()
```

The scatter plot $(beta_1, beta_2)$ is a line $y = -x$ around true $beta$s (1, 1): we see offsetting coefficients.


Now if we think of this setup as a prediction problem, and do train/test split

```python
from sklearn.model_selection import train_test_split
from sklearn.metrics import r2_score, mean_squared_error

means = (0, 0)
cov = [
    [1, 0.9],
    [0.9, 1],
]

X = np.random.multivariate_normal(means, cov, (10000, 1000))
e = np.random.normal(mu, sigma, (10000, 1000))
y = X[:,:,0] + X[:,:,1] + e

X_train, X_test, y_train, y_test, e_train, e_test = train_test_split(X, y, e, random_state=42, train_size=0.75)
X_train.shape
```

```python
coefs_arr = [None] * 1000
ins_rsquared = [None] * 1000
oos_rsquared = [None] * 1000
ins_mse = [None] * 1000
oos_mse = [None] * 1000

for i in range(0, 1000):
    regr = linear_model.LinearRegression()
    regr.fit(X_train[:,i], y_train[:,i])
    
    y_pred_test = X_test[:,i,0] * regr.coef_[0] + X_test[:,i,1] * regr.coef_[1] + e_test[:,i]
    y_pred_train = X_train[:,i,0] * regr.coef_[0] + X_train[:,i,1] * regr.coef_[1] + e_train[:,i]
    
    oos_rsquared[i] = r2_score(y_test[:,i], y_pred_test)
    ins_rsquared[i] = r2_score(y_train[:,i], y_pred_train)
    oos_mse[i]= mean_squared_error(y_test[:,i], y_pred_test)
    ins_mse[i]= mean_squared_error(y_train[:,i], y_pred_train)
    coefs[i] = regr.coef_

coefs_arr = np.array(coefs)
oos_rsquared_arr = np.array(oos_rsquared)
ins_rsquared_arr = np.array(ins_rsquared)
oos_mse_arr = np.array(oos_mse)
ins_mse_arr = np.array(ins_mse)
```

```python
oos_mse_arr.mean()
```

```python
# are we expecting oos r2 to be this high?
oos_rsquared_arr.mean()
```

```python
coefs_arr = [None] * 1000
ins_rsquared = [None] * 1000
oos_rsquared = [None] * 1000
ins_mse = [None] * 1000
oos_mse = [None] * 1000

for i in range(0, 1000):
    # why does lasso look really bad?
    regr = linear_model.Ridge()
    regr.fit(X_train[:,i], y_train[:,i])
    
    y_pred_test = X_test[:,i,0] * regr.coef_[0] + X_test[:,i,1] * regr.coef_[1] + e_test[:,i]
    y_pred_train = X_train[:,i,0] * regr.coef_[0] + X_train[:,i,1] * regr.coef_[1] + e_train[:,i]
    
    oos_rsquared[i] = r2_score(y_test[:,i], y_pred_test)
    ins_rsquared[i] = r2_score(y_train[:,i], y_pred_train)
    oos_mse[i]= mean_squared_error(y_test[:,i], y_pred_test)
    ins_mse[i]= mean_squared_error(y_train[:,i], y_pred_train)
    coefs[i] = regr.coef_

coefs_arr = np.array(coefs)
oos_rsquared_arr = np.array(oos_rsquared)
ins_rsquared_arr = np.array(ins_rsquared)
oos_mse_arr = np.array(oos_mse)
ins_mse_arr = np.array(ins_mse)
```

```python
oos_mse_arr.mean()
```

```python
oos_rsquared_arr.mean()
```

```python

```

```python
fig = px.scatter(x=coefs_arr[:,0], y=coefs_arr[:,1])
fig.show()
```

```python

```

```python

```
