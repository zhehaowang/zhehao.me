# Intro

### What is machine learning

No agreed upon definition.

One definition: the field that gives computers the ability to learn without being explicitly programmed.

Another definition: task T, performance measure P, experience E.
Machine performs task T better by performance measure P with as experience E grows.

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

Training set --> Learning algorithm --> h (hypothesis), where
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


