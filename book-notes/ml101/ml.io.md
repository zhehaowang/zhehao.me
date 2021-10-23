
Convolution, pooling, layers

### [ML crash course](https://developers.google.com/machine-learning/crash-course)

Empirical risk minimization.
Loss function. (E.g. L2 loss, RMSE)
Optimizer.

SGD, mini-batch gradient descent.

Learning rate. (rate of gradient descent)

One epoch spans sufficient iterations to process every example in the dataset. For example, if the batch size is 12, then each epoch lasts one iteration. However, if the batch size is 6, then each epoch consumes two iterations.

Training set, validation set, test set
If you are iterating on features / hyperparameters based on performance of some dataset, then that dataset should not be your eventual test set.

Feature engineering, what real life machine learning engineers might spend 75% time doing.
One hot encoding for categorical (discrete set of possible values). (Out-of-vocabulary: everything else)
Suppose you have 1,000,000 different street names, use a sparse representation to only store non-zero values instead. (in this case an independent model value is still learnt for each of the 1,000,000 feature value)

Good features
* A feature should be non-zero a couple of times in our dataset, not just once. (Avoid rarely used discrete feature values, like an employee_id)
* Should be easy to sanity check (age years old vs number of seconds since epoch)
* Shouldn't mix magic values with real data, A=-1 for A is unset should be turned into is_A_set (0, 1) and A's value
* Shouldn't change over time
* Should not have extreme outliers

Good habits working with data:
* know your data (hist, plots, dashboard): keep in mind what they should look like, verify that they do look like that, double check they agree with other sources
* Debug (duplicate, missing, outliers)
* Monitor your data over time

Even a few "bad apples" can spoil a large data set.

Some tips working with data
* Scaling weights (in a model with multiple features) may help the model converge faster, and let model pay less attention to features having a wide range.
  * Popular scaling method
    * linear map [min, max] --> [-1, 1]
    * z-score: scaled_value = (value - mean) / stdev (most values will be between -3, +3)
* Dealing with extremely long tail outlier value: consider clipping
* The binning trick: one way to handle non-linearity. Lat, long of California housing price data e.g. where lat or long shouldn't have linear relationship with housing price. Instead we could bin lat long into buckets and use one-hot encoding for the buckets.

Features cross: one way to introduce some non-linearity into the model is to create a synthetic feature by multiplying existing ones.
(neural nets is an alternative.)

Linear models can be trained efficiently on massive datasets, and to introduce some non-linearity into the process without sacrificing training efficiency, we can do features cross.
In practice, machine learning models seldom cross continuous features. However, machine learning models do frequently cross one-hot feature vectors. Think of feature crosses of one-hot feature vectors as logical conjunctions. (like binned longitude, altitude)

### Regularization

Strategies
* early stop
* penalize model complexity (structural risk minimization of `minimize(Loss(Data|Model) + complexity(Model))` )
  * L2 - ridge, penalize by `lambda (sum of weights^2)` where `lambda` is a coefficient you can use to balance getting the model right on training data vs keeping it simple (overfitting vs underfitting). The prior here is that weights should be zero-centered and normally distributed.
  * L1 - lasso

(Low learning rates and high L2 regularization can produce similar effects of driving weights to values near 0.)

### Logistic regression

Produce a probability estimate as output.

Sigmoid function `y' = 1 / (1 + e^(-z))` is used by logistic regression guarantees a value range of `(0, 1)`.
Where `y'` is the output of the logistic regression model for a particular example, and `z = b + w_1 x_1 + ...`, the `x` values of that particular example.

The loss function for linear regression is squared loss.
The loss function for logistic regression is LogLoss.
```latex
$$LogLoss = \Sigma_{(x, y) \in D} -y log (y') - (1 - y) log(1 - y')$$
```
Where `(x, y)` is the dataset containing many labeled examples, `y` is the label `(0, 1)` and `y'` is the predicted label (between 0 and 1).

So given a vector `X`, we would pass it through a linear layer first using coefficients learned via logistic regression (i.e. minimizing LogLoss), then pass the resulting value through sigmoid function (no further parameterization).

Regularization is extremely important in logistic regression modeling.
Without regularization, the asymptotic nature of logistic regression would keep driving loss towards 0 in high dimensions.
Consequently, most logistic regression models use regularization to dampen model complexity.

### Classification

Metric: accuracy (percentage of cases you classified right)

Possible problem: class-imbalanced data set (predicting ad click throughrate, like one over ten thousand. A biased model with zero features and just return false would have a really high accuracy.)
* To help with this, we split accuracy into true/false positive/negative (true -> predicted right, positive -> predict happened)
* accuracy: total_true / total
* precision: true_positive / all_positive_predictions (did the model cry wolf too often)
* recall: true_positive / all_actual_positives (did it miss any wolves)
* precision and recall are usually at odds with each other

Say Logistic regression mapped our value to a `(0, 1)` probability, we then need to choose a classification threshold to map that to 0 or 1.

ROC curve (receiving operator characteristic curve) plots true-positive rate (recall) to false-positive rate `FP / (FP + TN)`. (Up and to the right)

Lower classification threshold: high true-positive and high false-positive rates, a point to the right end of ROC curve.

AUC: area under the ROC curve (performance across all classification thresholds).

One way of interpreting AUC is as the probability that the model ranks a random positive example more highly than a random negative example.
AUC has the properties scale-invariant (just scale model value and AUC does not change) and classification-threshold-invariant.
(However depending on the situation, being scale and classification-threshold-invariant may not be good properties.)

Prediction bias = average of predictions - average of labels in dataset

Adding a calibration layer (e.g. scale model value to have the means of predictions match the mean of dataset) to fix prediction bias is not a good idea: fix your model instead.


