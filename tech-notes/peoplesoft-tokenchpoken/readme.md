# "Turtle" - PeopleSoft tokenchpoken attack

An attempt to perform privilege escalation by leveraging vulnerability in Oracle PeopleSoft.

* [Credit: erpscan](https://erpscan.com/press-center/blog/peoplesoft-security-part-3-peoplesoft-sso-tokenchpoken-attack/)

### Folder structure

* _token_, code from [erpscan](https://erpscan.com/) to serialize, deserialize tokens and perform brute force to derive shared password
* _pwdgen_, dictionary generation following given patterns, or download from a password dictionary online
* _replay_, given IE's exported traffic xml, override get / post parameters and send requests masquerading as other users

### Result

Successfully logged in as other users in a production system.
