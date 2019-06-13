# Pythonic thinking

### Follow PEP8 style guide

Python Enhancement Proposal 8.

Be sure to follow:
* Whitespace
  * 4 whitespace (for syntactically significant)
  * No tabs
  * 80-character
  * Long expressions continuation should be 4 extra spaces from their normal indentation level
  * In a file functions and classes separated by two blank lines
  * In a class method separated by one blank line
  * Don't put spaces around list indexes, function calls or keyword argument assignments
  * Put one space after variable assignment
* Naming
  * Function, variable, attributes: `lowercase_underscore`
  * Protected instance attributes: `_leading_underscore`
  * Private instance methods: `__double_leading_underscore`
  * Classes, Exceptions: `CapitalWord`
  * Module level constants: `ALL_CAPS`
  * Instance methods first parameter should be called `self`
  * Class methods should use `cls` as the name of the first parameter
* Expression and statements (there should be one and preferably only one obvious way to do it)
  * Use inline negation `(if a is not b)` instead of negation of positive expressions `(if not a is b)`
  * Don't check for empty values by checking length (`if len(l) == 0`), use `if not l` instead and assume empty values implicitly evaluate to False. Same goes for checking non-empty.
  * Avoid single line `if` `for` `while` `except` statements. Spread them over multiple lines.
  * Always put `import` at top of file
  * Always use absolute names for modules when importing them, not names relative to the current module's own path. Do `from bar import foo`, not just `import foo`.
  * If you must do relative imports, do `from . import foo`.
  * `import` in this order: std library modules, third party modules, your own modules, each section in alphabetical order.

**Takeaways**
* Always follow PEP 8 style. Easier to collaborate, makes your code consistent and easier to modify later.
