# Pythonic thinking

### Know the difference between `bytes` `str` and unicode

* Python 3 - sequences of characters represented by `bytes` and `str`. `bytes` contain raw 8-bit value, `str` contain unicode characters.

* Python 2 - sequences of characters represented by `str` and `unicode`. `str` contain raw 8-bit value, `unicode` contain unicode characters.

Unicode characters can be encoded into binary data in many ways, with the most common encoding being UTF-8.
`str` in Python3 and unicode in Python 2 do not have associated binary encoding: you must use `encode` method.

The core of your program should use unicode character type (unicode in Python 2 and `str` in Python 3) and should not assume anything about character encodings.
Leave encoding and decoding to the boundary of your interfaces.

If you need to operate on binary data of a character sequence, `to_bytes` / `to_str` helper method to convert `bytes_or_str` into `bytes` or `str` in Python 3 might come in handy (similarly `to_unicode` / `to_str` in Python 2).

Two gotchas:
* In Python 2 `unicode` and `str` instances seem to be the same type when `str` only contains 7bit ascii. (the two types can be combined with +, compared with =, pass around and it'll just work as long as only 7 bit ascii is used in `str` etc).
In Python 3 `str` and `bytes` are never the same, not even empty string.
* In Python 3 operations involving file handles default to UTF-8 encoding, Python 2 defaults to binary encoding. This means you can't `file.write(bytes)` as the default `write` of `w` mode expects a sequence of unicode characters, instead you could open the file with `wb` which would work in Python 2 and 3. Same for `rb`.

**Takeaways**

* Python 3, `bytes` are bytes, `str` is a sequence of unicode characters. `bytes` and `str` can't be compared, concatenated, etc.
* Python 2, `str` is bytes, `unicode` is a sequence of unicode characters. `str` and `unicode` can be used together if `str` only contains 7bit ascii characters
* Use helper function to ensure that the inputs you operate on are the type of character sequence you expect (bytes, unicode characters, utf-8 encoded unicode characters)
* If you want to read / write binary data from / to a file, always do `rb` or `wb`
