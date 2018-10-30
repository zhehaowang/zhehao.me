# Black box machine learning

https://www.youtube.com/watch?v=MsD28INtSv8&feature=youtu.be

### What is machine learning

One definition: the field that gives computers the ability to learn without being explicitly programmed.

Another definition: task T, performance measure P, experience E.
Machine performs task T better by performance measure P with as experience E grows.

What is not ML: rule-based expert systems popular in the 80s (rules, knowledge bases, logic, inference, etc: labor intensive to build, rules work well for the areas they cover, don't naturally handle uncertainty, seen as brittle)

Now instead of reverse engineering the expert's decision making, we provide a large enough training set to have the machines learn itself.

### Supervised learning

##### Example 1.1: housing price prediction
Given collected pairs of (size, price), find a suitable price for a new given size.
Now, do you fit a linear, quadratic, or what line?

This is a supervised learning, and regression problem.
* **Supervised** refers to the machine being given a set of "right answers", and knowing what the correct output should look like, having the idea that there is a relationship between the input and output
* **Regression** refers predicting continuous values (e.g. price)

##### Example 1.2: breast cancer prediction
Given (tumor size, is cancer (bool)) samples, and a tumor of size X, decide P(is\_cancer = true)

This is a supervised learning, and classification problem.
* **Classification** refers to the algorithm predicting discrete values

There may be more features / dimensions, say, (age, tumor size, is cancer), if we plot on a (age, tumor size) space, on which X denotes is cancer true, and O denotes is cancer false, the algorithm may decide to draw a line to divide (the cluster of) X and Os, and then decide if the likelihood of cancer given another (age, tumor size)

Support vector machine allows dealing with an "inifite" number of features.

### Unsupervised learning

**Unsupervised learning**: given data no longer has labels, instead, "here is a dataset, what insight can you find from it?".
Thus allowing us to derive structure from data where we don't necessarily know the effect of the variables.
With unsupervised learning there is no feedback based on prediction results.

##### Example 2.1: clustering in news.google.com
Google News groups news story, automatically cluster them together, so that stories on the same topic appear together.

##### Example 2.2: categorizing people by genome
Given individuals gene sequence, cluster them into types of people.

In neither of the above were we given the topic of a piece of news, or a predefined gene category of a person.

##### Example 2.3: cocktail party problem
In a cocktail party where everyone is talking, we have N speakers and M microphones at different positions, each microphone records a different overlap of N speaker's voices, due to position differences.
Given the recordings of each microphone, separate the different speakers' voices out.

Software: Octave (or Matlab)

Turns out cocktail party algorithm can be expressed in one line with Octave.

Companies often prototype with Octave, and after finding out it works, build a C++ / Java solution.

### Model representation

Using Example 1.1 (supervised learning, regression), we learn from a **training set** of given (size, price)s

We use _m_ to denote the number of training samples, _x_ to denote the input variables / features, _y_ to denote output / "target" variable.

(x, y) denotes one training example, and (x^(i), y^(i)) denotes the i-th training example.

Raw input -- Feature extraction --> Training set -- Learning algorithm --> h (hypothesis / prediction function), where
```
h: X -> Y
```
h is a function mapping from input space to output space.

Suppose h is a linear function (univariate (meaning one-variable) linear regression), to represent h, we could use
```
h_theta(x) = theta_0 + theta_1 * x
```
to represent h.

### Cost function

Using example 1.1 and the linear regression from previous section, theta\_0 and theta\_1 are parameters of the hypothesis function.

We want to come up with theta\_0 and theta\_1 such that h(x) is close to y for training examples (x, y)

To formalize this, we want to choose theta\_0 and theta\_1 s.t. 
```
Let J(theta_0, theta_1) = (1/(2m) * Sigma_{i = m}^{m}{(h(x) - y)^2})

Find min_{theta_0, theta1}{J(theta_0, theta_1)}
```
J is called a cost function, in particular this is a square error function / mean square error. (1/2 such that it's easier to calculate gradient descent)

### Cost function: intuition

Suppose theta\_0 = 0.

Plot the cost function (theta\_1, J(theta\_1)), unsurprisingly J is a quadratic function.

Now keep both theta\_0 and theta\_1. The cost function is now a 3-D surface that still follows the "bow" shape.

Using contour plots to illustrate the 3-D surface.
(x: theta\_0, y: theta\_1, and z (J(theta\_0, theta\_1) is flattened but we draw a curve to connect those (theta\_0, theta\_1)s whose J are equal. So we end up with a series of concentric ellipses (contour lines))

### Gradient descent

Given a function `J(theta_0, theta_1, ..., theta_n)` find a `min{theta_0, ..., theta_n}{J(theta_0, ..., theta_n)}` by keep changing `theta_0`, `theta_1` in small steps such that we descend the fastest, until a local minimum is achieved

Algorithm
```
theta_j := theta_j - alpha * {d/d{theta_j}} * J(theta_0, theta_1)
```
where `alpha` is the learning rate, `:=` stands for assignment, `{d/d{theta_j}}` is a partial derivative.

`theta_0` and `theta_1` should be updated simultaneously.

Gradient descent can converge to a local minimum, even with the learning rate `alpha` fixed.

Cost function for linear regression is a convex function, which doesn't have any local maxima except the global one.

This algorithm is termed "batch" gradient descent, where each step of gradient descent uses all the training examples.

There exists other flavors of gradient descent.

Normal equations method can solve linear regressions, but scales worse than gradient descent.


### Terms

Feature extraction, feature extraction, one-hot encoding (essentially a single-select combo box), categorical variable (a variable that takes one of several discrete possible values)

Labeled datum, unlabeled datum

Loss function (evaluating a single prediction), 0/1 loss (for classification), square loss (for regression)

### Evaluating a prediciton function

Test set (independent of training data). Train/test, train/deploy

A big part of real-world machine learning is ensuring your performance evaluation with train/test is a good estimate of deployment performance.

Principle of train/test splitting:
* random split of labeled data. Training should represent test set, test should represent real world.
* what about time series data? If you still do random split, you train/test no longer resemble the train/deploy scenario (use all the time in the past to predict the future)! So with time series, we pick a time t to split the set into train and test.

##### k-fold cross validation

Suppose test set too small for good performance estimate.
Partition labeled data into equal-sized folds, each time use one (different) fold to test, and all other folds to train.
Report mean and std error of performance measurements.
This can evaluate the model building procedure.
We then apply the model building procedure to all the labeled data to build a model for deployment.

##### Forward chaining

Cross validation for time series.
```
|-train-|-test-|--------------------|
|----train-----|-test-|-------------|
|---------train-------|-test-|------|
```

`model([train])` evaluates `[test]`, then when we deploy, do we deploy `model([train + test])`?
Seems the latter. By inspecting the underlying method, we'll believe the model will be more accurate, when presented with more training data.

When time is not an issue, is k-fold cross validation or forward chaining always better?
Seems so, not definitively answered.

Finite class lemma.
Basically, if you are not examining what went awry given your test set, and using that information to influence the model building process, then you are safe.

##### Leakage

Information about the label leaks into the features, which wouldn't reflect deployment in real world. (e.g. problem is to derive user sentiment from yelp review text, but star rating leaks in as a feature.)

##### Sample bias

Test inputs and deployment inputs have different distributions. (e.g. to predict stock price, use a random selection of companies that exist today and use their historical data - survivorship bias)

##### Nonstationarity
When the thing you are modeling changes over time:
* Covariate shift: input distribution changed between training and deployment (say, popular search terms in football season)
* Concept drift: correct output for given input changes

### Overfitting

Loosely speaking, a model overfits when training performance is good but test/validation performance is poor.

Example: polynomial fitting on a curve, given training data with noise.
```
f(x) = w_0 + w_1 * x + w_2 * x^2 + ... + w_M * x^M
```
Here `M` is a hyperparameter, a parameter of the ML algorithm itself, usually adjusted by the data scientist.

Larger model complexity (higher `M`) means better fit to training data, but not necessarily better fit to deployment scenario.

To overcome overfitting, increase data points, or reduce model complexity.

Almost every learning algorithm has at least one hyperparameter or tuning parameter.
The data scientist must tune these values.

### General workflow

* Split labeled data into training, validation and test. (don't bother with k-fold cross validation)
* Repeat the following until happy with performance on validation set:
  * Revise feature extraction
  * Choose ML algorithm
  * Train ML model with various hyperparameter settings
  * Evaluate prediction function on validation set
* Retrain model on (train + validation)
* Evaluate performance on test set (good enough --> proceed; not good enough --> reiterate)
* Retrain on all labeled data (training + validation + test)
* Deploy resulting prediction function

Feature extraction often times are not automated, but in cases like neural network, feature extraction can be thought of as part of the ML algorithm: the ML algorithm recognizes features.

##### Example: cell phone churn problem

https://www.youtube.com/watch?v=kE_t3Mm8Z50&feature=youtu.be

Given a customer profile, predict how likely they are to churn / leave?
(So that you can target him with some promotion in order to retain the customer, which costs less to the company than the customer leaving)

You have two years of historical data (of what your customer was doing before) (of one particular company) until the date of the churn.

Assume no contracts and everyone pays as they go

Map this to a precisely defined machine learning problem.

* Raw Input: two years of historical data until churning
* Label: churner or not in a time window (classification); for every user, at time `T`, whether they churn or not in `(T + 2 weeks, T + 3 months)`
* Output of our prediction function, it could be
  * the possibility of a user churning on `day_i` (probability distribution), or
  * time period till their churning (regression), or
  * whether a person churns or not in 3 months (classification)
* Train/test split (think about how your train/test split could match with train/deploy)
  * Based on user? Choosing a time point `T` then randomly pick 80% of the users for training, the rest for testing?
  * Choosing a time point `T`? At Christmas time the result may differ a lot from summertime

To illustrate the complexity of a real world ML problem

