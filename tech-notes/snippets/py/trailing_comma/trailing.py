#!/usr/bin/env python3
import os

class MyClass():
    # trailing comma, style, think list / tuple trailing comma
    def __init__(self, i, ):
        print(i)
        return

    # PEP 3107: function annotation, useful only for user code or 3rd party lib,
    #   no effect on native interpreter
    def test_annotation(self, a : int, b : "some note") -> int:
        return a + b

if __name__ == "__main__":
    m = MyClass(4)
    print(m.test_annotation(3, 4))
    print(m.test_annotation.__annotations__)