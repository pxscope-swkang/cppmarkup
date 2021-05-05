# kangsw::markup

This library provides easy-to-use interface for data binding between markup formats(JSON) and c++ native structures.

Markup object, which is representative data type of this library, exposes set of properties to describe an object, makes easy extend with other markup formats.

You can define native structure with set of macros, which will create runtime native structure description automatically.

You can iterate objects' properties only with `markup::object` pointers, without any detailed type information. 

This works similarly with reflection of managed languages, however, this doesn't support runtime type modification, which is pretty out of focus of this library.

# Usage

```
CPPMARKUP_OBJECT_TEMPLATE(sample_type)
{
    CPPMARKUP_ELEMENT(SomeBoolValue, true);
    CPPMARKUP_ELEMENT(SomeNullValueWithAttr, nullptr,
                      CPPMARKUP_ATTRIBUTE(SomeAttr1, "Abcd");
                      CPPMARKUP_ATTRIBUTE(SomeAttr2, false);
                      CPPMARKUP_ATTRIBUTE(SomeAttr3, 315.241););

    CPPMARKUP_EMBED_OBJECT_begin(SomeEmbeddedObject,
                                 CPPMARKUP_ATTRIBUTE(SomeDateValue, 1534))
    {
        CPPMARKUP_ELEMENT(EmbeddedArray, CPPMARKUP_ARRAY(1, 2, 3, 4, 5));
    }
    CPPMARKUP_EMBED_OBJECT_end(SomeEmbeddedObject)
};

...
    using namespace kangsw::markup;
    auto s = sample_type::get_default(); // warning: sample_type{} returns 0-initialized structure
    u8string buff;
    dump(json_dump{buff, 4, 0}, s);
...
```

This outputs below JSON string

```json
{
    "SomeBoolValue": true,
    "SomeNullValueWithAttr~@@ATTR@@": {
        "SomeAttr1": "Abcd",
        "SomeAttr2": false,
        "SomeAttr3": 315.241000
    },
    "SomeNullValueWithAttr": null,
    "SomeEmbeddedObject~@@ATTR@@": {
        "SomeDateValue": 1534
    },
    "SomeEmbeddedObject": {
        "EmbeddedArray": [
            1,
            2,
            3,
            4,
            5
        ]
    }
}
```

> TODO: Add cases for XML, BSON dumps ...

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