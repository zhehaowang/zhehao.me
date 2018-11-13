#!/usr/bin/env python3

import pandas as pd
import logging
import numpy as np
import sys
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split
import time

### Assignment Owner: Tian Wang

#######################################
#### Normalization

def feature_normalization(train, test):
    """Rescale the data so that each feature in the training set is in
    the interval [0,1], and apply the same transformations to the test
    set, using the statistics computed on the training set.

    Args:
        train - training set, a 2D numpy array of size (num_instances, num_features)
        test  - test set, a 2D numpy array of size (num_instances, num_features)
    Returns:
        train_normalized - training set after normalization
        test_normalized  - test set after normalization

    """
    # what about test having values that exceed max(train)? what about ptp(0) = 0?
    # TODO: discard feature where train.ptp(0) == 0
    return (train - train.min(0)) / train.ptp(0), (test - train.min(0)) / train.ptp(0)

########################################
#### The square loss function

def compute_square_loss(X, y, theta):
    """
    Given a set of X, y, theta, compute the square loss for predicting y with X*theta

    Args:
        X - the feature vector, 2D numpy array of size (num_instances, num_features)
        y - the label vector, 1D numpy array of size (num_instances)
        theta - the parameter vector, 1D array of size (num_features)

    Returns:
        loss - the square loss, scalar
    """
    res = X.dot(theta)
    
    return ((res - y) ** 2).mean()

########################################
### compute the gradient of square loss function
def compute_square_loss_gradient(X, y, theta):
    """
    Compute gradient of the square loss (as defined in compute_square_loss), at the point theta.

    Args:
        X - the feature vector, 2D numpy array of size (num_instances, num_features)
        y - the label vector, 1D numpy array of size (num_instances)
        theta - the parameter vector, 1D numpy array of size (num_features)

    Returns:
        grad - gradient vector, 1D numpy array of size (num_features)
    """
    # formulaic solution, refer to notations.exercise
    Xt = np.transpose(X)
    num_instances = y.shape[0]

    return (2 * Xt.dot(X).dot(theta) - 2 * Xt.dot(y)) / num_instances


###########################################
### Gradient Checker
#Getting the gradient calculation correct is often the trickiest part
#of any gradient-based optimization algorithm.  Fortunately, it's very
#easy to check that the gradient calculation is correct using the
#definition of gradient.
#See http://ufldl.stanford.edu/wiki/index.php/Gradient_checking_and_advanced_optimization
def grad_checker(X, y, theta, epsilon=0.01, tolerance=1e-4):
    """Implement Gradient Checker
    Check that the function compute_square_loss_gradient returns the
    correct gradient for the given X, y, and theta.

    Let d be the number of features. Here we numerically estimate the
    gradient by approximating the directional derivative in each of
    the d coordinate directions:
    (e_1 = (1,0,0,...,0), e_2 = (0,1,0,...,0), ..., e_d = (0,...,0,1)

    The approximation for the directional derivative of J at the point
    theta in the direction e_i is given by:
    ( J(theta + epsilon * e_i) - J(theta - epsilon * e_i) ) / (2*epsilon).

    We then look at the Euclidean distance between the gradient
    computed using this approximation and the gradient computed by
    compute_square_loss_gradient(X, y, theta).  If the Euclidean
    distance exceeds tolerance, we say the gradient is incorrect.

    Args:
        X - the feature vector, 2D numpy array of size (num_instances, num_features)
        y - the label vector, 1D numpy array of size (num_instances)
        theta - the parameter vector, 1D numpy array of size (num_features)
        epsilon - the epsilon used in approximation
        tolerance - the tolerance error

    Return:
        A boolean value indicate whether the gradient is correct or not

    """
    true_gradient = compute_square_loss_gradient(X, y, theta) #the true gradient
    num_features = theta.shape[0]
    approx_grad = np.zeros(num_features) #Initialize the gradient we approximate

    # more elegant / efficient way to express this?
    for i in range(num_features):
        theta[i] -= epsilon
        val1 = compute_square_loss(X, y, theta)
        theta[i] += epsilon * 2
        val2 = compute_square_loss(X, y, theta)
        theta[i] -= epsilon
        approx_grad[i] = (val2 - val1) / epsilon / 2

    return np.linalg.norm(approx_grad - true_gradient) <= tolerance

#################################################
### Generic Gradient Checker
def generic_gradient_checker(X, y, theta, objective_func, gradient_func, epsilon=0.01, tolerance=1e-4):
    """
    The functions takes objective_func and gradient_func as parameters. And check whether gradient_func(X, y, theta) returned
    the true gradient for objective_func(X, y, theta).
    Eg: In LSR, the objective_func = compute_square_loss, and gradient_func = compute_square_loss_gradient
    """
    true_gradient = gradient_func(X, y, theta)

    num_features = theta.shape[0]
    approx_grad = np.zeros(num_features) #Initialize the gradient we approximate

    # more elegant / efficient way to express this?
    for i in range(num_features):
        theta[i] -= epsilon
        val1 = objective_func(X, y, theta)
        theta[i] += epsilon * 2
        val2 = objective_func(X, y, theta)
        theta[i] -= epsilon
        approx_grad[i] = (val2 - val1) / epsilon / 2

    return np.linalg.norm(approx_grad - true_gradient) <= tolerance

####################################
#### Batch Gradient Descent
def batch_grad_descent(X, y, alpha=0.1, num_iter=1000, check_gradient=False):
    """
    In this question you will implement batch gradient descent to
    minimize the square loss objective

    Args:
        X - the feature vector, 2D numpy array of size (num_instances, num_features)
        y - the label vector, 1D numpy array of size (num_instances)
        alpha - step size in gradient descent
        num_iter - number of iterations to run
        check_gradient - a boolean value indicating whether checking the gradient when updating

    Returns:
        theta_hist - store the the history of parameter vector in iteration, 2D numpy array of size (num_iter+1, num_features)
                    for instance, theta in iteration 0 should be theta_hist[0], theta in ieration (num_iter) is theta_hist[-1]
        loss_hist - the history of objective function vector, 1D numpy array of size (num_iter+1)
    """
    num_instances, num_features = X.shape[0], X.shape[1]
    theta_hist = np.zeros((num_iter+1, num_features))  #Initialize theta_hist
    loss_hist = np.zeros(num_iter+1) #initialize loss_hist
    theta = np.zeros(num_features) #initialize theta
    
    loss_hist[0] = compute_square_loss(X, y, theta)

    if check_gradient:
        if not grad_checker(X, y, theta):
            raise RuntimeError('check grad failed, round 0')

    for i in range(1, num_iter + 1):
        if check_gradient:
            if not grad_checker(X, y, theta):
                raise RuntimeError('check grad failed, round ' + str(i))

        gradient = compute_square_loss_gradient(X, y, theta)

        loss = compute_square_loss(X, y, theta)
        theta -= alpha * gradient
        theta_hist[i] = theta
        loss_hist[i] = loss

    return theta_hist, loss_hist


####################################
###Q2.4b: Implement backtracking line search in batch_gradient_descent
###Check http://en.wikipedia.org/wiki/Backtracking_line_search for details

def backtracking_line_search(X, y, theta, direction, alpha):
    # Q: something is wrong here, this as-is doesn't do anything useful
    c = 0.5
    tor = 0.5

    m = np.linalg.norm(direction) ** 2
    t = - c * m

    j = 0
    temp = alpha
    while compute_square_loss(X, y, theta + temp * direction) - compute_square_loss(X, y, theta) < t * temp:
        j += 1
        temp = tor * temp

    if temp != alpha:
        print('backtracking line search adjusted step size from ' + str(alpha) + ' to ' + str(temp))
    return temp

###################################################
### Compute the gradient of Regularized Batch Gradient Descent
def compute_regularized_square_loss_gradient(X, y, theta, lambda_reg):
    """
    Compute the gradient of L2-regularized square loss function given X, y and theta

    Args:
        X - the feature vector, 2D numpy array of size (num_instances, num_features)
        y - the label vector, 1D numpy array of size (num_instances)
        theta - the parameter vector, 1D numpy array of size (num_features)
        lambda_reg - the regularization coefficient

    Returns:
        grad - gradient vector, 1D numpy array of size (num_features)
    """
    Xt = np.transpose(X)
    num_instances = y.shape[0]

    return (2 * Xt.dot(X).dot(theta) - 2 * Xt.dot(y) + 2 * lambda_reg * theta) / num_instances


###################################################
### Batch Gradient Descent with regularization term
def regularized_grad_descent(X, y, alpha=0.1, lambda_reg=1, num_iter=1000, use_backtracking_line_search = True):
    """
    Args:
        X - the feature vector, 2D numpy array of size (num_instances, num_features)
        y - the label vector, 1D numpy array of size (num_instances)
        alpha - step size in gradient descent
        lambda_reg - the regularization coefficient
        numIter - number of iterations to run

    Returns:
        theta_hist - the history of parameter vector, 2D numpy array of size (num_iter+1, num_features)
        loss_hist - the history of loss function without the regularization term, 1D numpy array.
    """
    (num_instances, num_features) = X.shape
    theta = np.zeros(num_features) #Initialize theta
    theta_hist = np.zeros((num_iter + 1, num_features))  #Initialize theta_hist
    loss_hist = np.zeros(num_iter + 1) #Initialize loss_hist

    loss_hist[0] = compute_square_loss(X, y, theta)

    for i in range(1, num_iter + 1):
        gradient = compute_regularized_square_loss_gradient(X, y, theta, lambda_reg)
        loss = compute_square_loss(X, y, theta)
        theta_hist[i] = theta
        loss_hist[i] = loss

        step = alpha
        if use_backtracking_line_search:
            step = backtracking_line_search(X, y, theta, gradient, alpha)

        theta -= step * gradient

    return theta_hist, loss_hist


#############################################
## Visualization of Regularized Batch Gradient Descent
##X-axis: log(lambda_reg)
##Y-axis: square_loss

#############################################
### Stochastic Gradient Descent
def stochastic_grad_descent(X, y, alpha=0.1, lambda_reg=1, num_iter=1000):
    """
    In this question you will implement stochastic gradient descent with a regularization term

    Args:
        X - the feature vector, 2D numpy array of size (num_instances, num_features)
        y - the label vector, 1D numpy array of size (num_instances)
        alpha - string or float. step size in gradient descent
                NOTE: In SGD, it's not always a good idea to use a fixed step size. Usually it's set to 1/sqrt(t) or 1/t
                if alpha is a float, then the step size in every iteration is alpha.
                if alpha == "1/sqrt(t)", alpha = 1/sqrt(t)
                if alpha == "1/t", alpha = 1/t
        lambda_reg - the regularization coefficient
        num_iter - number of epochs (i.e number of times) to go through the whole training set

    Returns:
        theta_hist - the history of parameter vector, 3D numpy array of size (num_iter, num_instances, num_features)
        loss hist - the history of regularized loss function vector, 2D numpy array of size(num_iter, num_instances)
    """
    num_instances, num_features = X.shape[0], X.shape[1]
    theta = np.ones(num_features) #Initialize theta

    # theta_hist = np.zeros((num_iter + 1, num_instances, num_features))  #Initialize theta_hist
    # Q: why is theta_hist a 3D matrix in the given?
    theta_hist = np.zeros((num_iter + 1, num_features))
    loss_hist = np.zeros((num_iter + 1, num_instances)) #Initialize loss_hist

    loss_hist[0] = compute_square_loss(X, y, theta)

    yt = np.transpose(y[np.newaxis]) # transpose of 1d array results in a 1d array

    data = np.append(X, yt, 1)
    np.random.shuffle(data)

    X_shuffled = data[:, :-1]
    y_shuffled = data[:, -1]

    # Q: this impl seems slow
    for i in range(1, num_iter + 1):
        idx = (i - 1) % num_instances
        loss = compute_square_loss(X_shuffled[idx], y_shuffled[idx], theta)
        gradient = compute_regularized_square_loss_gradient(X_shuffled[idx][np.newaxis], y_shuffled[idx][np.newaxis], theta, lambda_reg)

        theta -= alpha * gradient
        loss_hist[i] = loss
        theta_hist[i] = theta
    return theta_hist, loss_hist


################################################
### Visualization that compares the convergence speed of batch
###and stochastic gradient descent for various approaches to step_size
##X-axis: Step number (for gradient descent) or Epoch (for SGD)
##Y-axis: log(objective_function_value) and/or objective_function_value

def main():
    #Loading the dataset
    print('loading the dataset')

    df = pd.read_csv('data.csv', delimiter=',')
    X = df.values[:,:-1]
    y = df.values[:,-1]

    print('Split into Train and Test')
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size =100, random_state=10)

    print("Scaling all to [0, 1]")
    X_train, X_test = feature_normalization(X_train, X_test)
    X_train = np.hstack((X_train, np.ones((X_train.shape[0], 1))))  # Add bias term
    X_test = np.hstack((X_test, np.ones((X_test.shape[0], 1)))) # Add bias term

    # alpha = 0.1: does not converge
    now = time.time()
    # Q: why do I see a higher loss, when I increase the num_iter from 10000 to 100000 in batch gradient descent and batch gradient descent with regularization?
    theta_hist, loss_hist = batch_grad_descent(X_train, y_train, alpha = 0.005, num_iter = 10000, check_gradient = False)
    print("batch gradient descent, loss: " + str(compute_square_loss(X_test, y_test, theta_hist[-1])) + ", time: " + str(time.time() - now))
    
    now = time.time()
    theta_hist, loss_hist = regularized_grad_descent(X_train, y_train, alpha = 0.005, num_iter = 10000, lambda_reg = 1, use_backtracking_line_search = True)
    print("batch gradient descent (l2 regularization), loss: " + str(compute_square_loss(X_test, y_test, theta_hist[-1])) + ", time: " + str(time.time() - now))

    now = time.time()
    theta_hist, loss_hist = stochastic_grad_descent(X_train, y_train, alpha = 0.005, num_iter = 10000, lambda_reg = 0.01)
    print("stochastic gradient descent (l2 regularization), loss: " + str(compute_square_loss(X_test, y_test, theta_hist[-1])) + ", time: " + str(time.time() - now))

if __name__ == "__main__":
    main()
