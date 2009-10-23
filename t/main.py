#!/usr/bin/python
# -*- coding: utf-8 -*-

import Test;

class A:
  pass

A.longField = 314
A.floatField = 3.14
A.stringField = "Some string field"

Test.my_method(A)
