### Item 33: be wary of `remove`-like algorithms on containers of pointers

Consider this
```
vector<Widget*> v;
v.push_back(new Widget());
...
v.erase(
    remove_if(v.begin(),
              v.end(),
              not1(mem_fun(&Widget::isCertified))),
    v.end());
```

This will leak, and it's possible by the time after `remove` is called, we'll already lose references to those objects, due to the overwriting implementation explained in it32.

In this case partition is a reasonable alternative, or if you have to use `remove`, you can `delete` the elements that will be removed first, then `remove`.

If you want to erase pointers as you iterate, that has some subtle aspects to it, check item 9.

Erase-remove is also fine if you replace the `vector` of raw pointers with `vector` of smart pointers.
