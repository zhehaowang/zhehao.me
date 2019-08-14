# Consider `istreambuf_iterator` for character-by-character input

Suppose you want to copy a text file into a string object,
```
ifstream inputFile("data.file");
string fileData((istream_iterator<char>(inputFile)), istream_iterator<char>());
// see it6 about the extra parentheses and most vexing parse
```
This approach fails to copy whitespace in the file into the string, because `istream_iterator` use `operator<<` to do the actual reading, and by default `operator<<` functions skip whitespace.

You can override the default
```
ifstream inputFile("data.file");
inputFile.unset(ios::skipws)
string fileData((istream_iterator<char>(inputFile)), istream_iterator<char>());
```

This copies all but they aren't copied as quickly as you'd like.
The `operator<<` on which `istream_iterator` depends perform formatted input, meaning they do extra work in creating and destroying sentry objects (setup and cleanup), they have to check stream flags like `skipws`, and perform comprehensive checking for read errors, and if encountering a problem, they have to check the stream's exception mask to determine whether an exception should be thrown.

These are overkill if all you want to do is grab the next character from the input stream.

An alternative is `istreambuf_iterator`, while `istream_iterator<char>` uses `operator<<` to read individual characters from an input stream, `istreambuf_iterator<char>` go straight to the stream's buffer and read the next character (by calling `s.rdbuf()->sgetc()`).
Just do
```
ifstream inputFile("data.file");
string fileData(
    (istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
```
We don't need to unset `skipws` as `istreambuf_iterator` does not skip.

**Takeaways**
* For unformatted character-by-character input, you should always consider `istreambuf_iterator`. Similarly `ostreambuf_iterator` for unformatted character-by-character output.
