#!/usr/bin/python

import Test;

class A:
  pass

A.longField = 314
A.floatField = 3.14
A.stringField = "Some string field"

Test.my_method(A)
