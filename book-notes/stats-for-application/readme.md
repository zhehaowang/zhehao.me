# Statistics for application, MIT 18.650

### Intro

Goal: formulate a problem in statistical terms, select statiscal methods and understand limitations. Understand maths behind statistical models. Justify quantitative statements. Provide maths toolbox.

Clean use of data.

Understand randomness. Quantifying chance, significance, variability, average, etc.
Probability is the study of randomness.

When you do probability, you are given truth (a distribution of a random variable), and describe outcome / data.
When you do statistics, you are given outcome / data, and infer the truth that generated such.

Usually, statistical modeling does the following
```
Complicated Process = Simple Process + Random Noise (things you don't understand and sweep under the rug hoping it'd get averaged away)
```
Good modeling consists of choosing (plausible) simple process and noise distribution.

IID independent and identically distributed.

T-test

How large should the sample size be?
How many samples with True do you need to see before you are convinced?

An **Estimator** is a random variable (e.g. averaging over the sum of some random variables), an estimate provided by the estimator is when you plug values to those random variables and get a number.
* The accuracy of an estimator. (If you throw away 98 data points out of 100, conceptually you get a worse estimator. More volatile. Variance)
* How close is your estimator. (A constant number is an estimator, it's not close to your problem. Bias.)

Random Variables that take 0 or 1, Bernoulli.

Statistics is about replacing expectations with averages. i.e.
```
E --> \frac{1}{n} \Sigma_{i = 1}^{n}
```

Example question: assume iid `x_i, ..., x_n` where `x_i ~ N(mu, sigma^2)`. Given a list of sample data points, find a good estimator for `mu` and `sigma^2`.
