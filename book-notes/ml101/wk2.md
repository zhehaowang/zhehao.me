# Introduction to statistical learning

### Formalizing a data science problem

**Decision theory**: we generally need to make a decision, take an action, produce some output (e.g. a 0/1 classification), and have some evaluation criteria.

Formalizing a problem
* Define the action space: **A**
* Decide the evaluation criteria
* Input (ML) / covariates (statistics): **X** (input space), often paired with outcomes / labels / outputs: **Y** (outcome space)

E.g. a prediction function produced by linear regression, input space is R^d, outcome and action space are R.

The workflow could look like
* Observe an input x
* Take an action a
* Observe an output y
* Evaluate how well we did: l(a, y)

l is often independent of a, but not always the case.

```
l: A X Y --> R
```
l maps action space and outcome space to a real number.

Statistical learning theory evaluates the decision function as a whole.
Assume there is a data generating distribution P\_{x * y}, all input / output pairs are generated independent, and identically distributed from P\_{x * y}, we want a decision function that "does well on average"

**Risk of a decision function**: the expected loss of a decision function f on a new example (x, y) drawn randomly from P\_{x * y}
```
R(f) = E l(f(x), y), where E stands for expectation
```

A **Bayesian decision function**
```
f^* : X --> A
```
is the function that achieves the minimal risk among all possible functions
```
f^* = argmin_{f}{R(f)}, where the minimum is taken over all functions from X --> A
```
Bayes decision function is often called the **target function**, since it's the best possible one.

##### Example: least square regression

Spaces A = Y = R

Square loss l(a, y) = (a - y)^2

Mean square risk
```
R(f) = E [(f(x) - y)^2] = E [(f(x) - E[y|x])^2] + E [(y - E[y|x])^2]
```

Target function
```
f^{*}(x) = E[y|x]
```

##### Example: multiclass classification

Spaces A = Y = {0, ..., K - 1}

0-1 loss:
```
l(a,y) = 1(a \neq y), meaning if a \neq y, l = 1; otherwise l = 0
```

Thus
```
R(f) = P(f(x) \ neq y)
```

Target function
```
f^{*}(x) = argmax_{1 \leq k \leq K}P(y = k | x)
```
Note that target function is not 0 as we assume just x is not enough to determine y: in P\_{x * y}, y|x follows a probability distribution.


But we can't compute the risk because we don't know P\_{x * y}.
Let's draw inspiration from the Strong Law of Large Numbers: if z\_1, ..., z\_n are generated independent, and identically distributed (iid) with expected value Ez, then
```
lim_{n --> \infty} 1/n \sum_{i=1}^{n}{z_i} = Ez with probability 1
```

With this we introduce the **empirical risk** of
```
f : X --> A with respect to D_n, where D_n = ((x_1, y_1), ..., (x_n, y_n)) be drawn iid from P_{X * Y}

R^{hat}_{n}(f) = 1/n \sum_{i = 1}^{n}{ l(f(x_i), y_i) }
```

Basically, a proximation of the risk, or an average loss on a given set of data samples.

Strong law of large numbers would suggest the empirical risk converges to risk of the function, when n approaches infinity.

This is our justification of the test set.

Let's then define an **empirical risk minimizer**,
```
f^{hat} = argmin_{f}(R^{hat}_{n}(f)), a minimum taken over all functions
```

Empirical risk minimization can lead to a function that just memorizes the data, "overfit".

One approach to avoid this (generalize empirical risk minimization), is **constrained empirical risk minimization**, where instead of minimizing empirical risk over all decision functions, we constrain to a particular subset of them.

We denote a **hypothesis space** as a set of functions mapping X --> A, meaning the collection of decision functions we are considering (e.g. those that have desired regularity (smoothness, simplicity, etc, like continuous functions, polynomial of order less than n, etc))

With this, we define an **empirical risk minimizer defined in hypothesis**.

**Terms to recap**
* Loss function, Risk, Bayesian decision function, Empirical Risk, Empirical Risk Minimizer, Empirical Risk Minimization over Hypothesis space
* Strong law of large numbers

### Stochastic gradient descent

Gradient descent, fixed step size (factor to time gradient with) (small enough it will converge; if not, may diverge)

Contour plot

Backtracking line search

Convergence theorem for fixed step size, Lipschitz continuous (Given a convex function, intuitively if 2nd derivative is bound by L, then gradient descent with `fixed step size <= 1/L` converges.
More intuition, the slower the gradient changes, the larger the fixed step size you can take and not risk divergence.
This gives the lower bound of the step size you should take, don't bother going slower than `1/L`)

When to stop? epsilon bound; or in a learning context, until performance on validation test does not improve

Noisy gradient descent: by default we'll need to run through all data points (pass them to loss function) to take a step (batch gradient descent). we can use an unbiased estimator (a subsample). Minibatch gradient works. Intuition: minor offs from gradient direction is Ok, the iterative steps will correct the offs.

Stochastic gradient descent is when minibatch size is 1 (use single randomly chosen point to determine step direction; faster to compute, worse estimate of gradient)

SGD step size, robbins-monro conditions

