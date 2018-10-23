# Intro

### What is machine learning

No agreed upon definition.

One definition: the field that gives computers the ability to learn without being explicitly programmed.

Another definition: task T, performance measure P, experience E.
Machine performs task T better by performance measure P with as experience E grows.

### Supervised learning

##### Example: housing price prediction
Given collected pairs of (size, price), find a suitable price for a new given size.
Now, do you fit a linear, quadratic, or what line?

This is a supervised learning, and regression problem.
* **Supervised** refers to the machine being given a set of "right answers", and knowing what the correct output should look like, having the idea that there is a relationship between the input and output
* **Regression** refers predicting continuous values (e.g. price)

##### Example: breast cancer prediction
Given (tumor size, is cancer (bool)) samples, and a tumor of size X, decide P(is\_cancer = true)

This is a supervised learning, and classification problem.
* **Classification** refers to the algorithm predicting discrete values

There may be more features / dimensions, say, (age, tumor size, is cancer), if we plot on a (age, tumor size) space, on which X denotes is cancer true, and O denotes is cancer false, the algorithm may decide to draw a line to divide (the cluster of) X and Os, and then decide if the likelihood of cancer given another (age, tumor size)

Support vector machine allows dealing with an "inifite" number of features.

### Unsupervised learning

**Unsupervised learning**: given data no longer has labels, instead, "here is a dataset, what insight can you find from it?".
Thus allowing us to derive structure from data where we don't necessarily know the effect of the variables.
With unsupervised learning there is no feedback based on prediction results.

##### Example: clustering in news.google.com
Google News groups news story, automatically cluster them together, so that stories on the same topic appear together.

##### Example: categorizing people by genome
Given individuals gene sequence, cluster them into types of people.

In neither of the above were we given the topic of a piece of news, or a predefined gene category of a person.

##### Example: cocktail party problem
In a cocktail party where everyone is talking, we have N speakers and M microphones at different positions, each microphone records a different overlap of N speaker's voices, due to position differences.
Given the recordings of each microphone, separate the different speakers' voices out.

Software: Octave (or Matlab)

Turns out cocktail party algorithm can be expressed in one line with Octave.

Companies often prototype with Octave, and after finding out it works, build a C++ / Java solution.

