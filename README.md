# kangsw::markup

This library provides easy-to-use interface for data binding between markup formats(JSON) and c++ native structures.

Markup object, which is representative data type of this library, exposes set of properties to describe an object, makes easy extend with other markup formats.

You can define native structure with set of macros, which will create runtime native structure description automatically.

You can iterate objects' properties only with `markup::object` pointers, without any detailed type information. 

This works similarly with reflection of managed languages, however, this doesn't support runtime type modification, which is pretty out of focus of this library.

# Usage

```c++:tests/automation/test-common-type.hpp
```

## Note

Unlike common XML files, CPPMARKUP notations does not allow mixing both plain text and tags. 

```xml
<-- THIS IS NOT ALLOWED -->
<som_tag>
    plain text. plain text. plain text. plain text. plain text. <some_other_tag></some_other_tag> plain text. plain text. plain text. 
</som_tag>
```

You'd better to understand markup handling of this library as JSON; simple set of key-value pairs.



# References

1. [`zeux/pugixml`](https://github.com/zeux/pugixml)