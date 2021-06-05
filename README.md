# kangsw::markup

This library provides easy-to-use interface for data binding between markup formats(JSON) and c++ native structures.

Markup object, which is representative data type of this library, exposes set of properties to describe an object, makes easy extend with other markup formats.

You can define native structure with set of macros, which will create runtime native structure description automatically.

You can iterate objects' properties only with `markup::object` pointers, without any detailed type information. 

This works similarly with reflection of managed languages, however, this doesn't support runtime type modification, which is pretty out of focus of this library.

# Usage

```
CPPMARKUP_OBJECT_TEMPLATE(my_markup_type) {
    CPPMARKUP_ELEMENT(is_valid, false);

    CPPMARKUP_OBJECT_TEMPLATE(internal_object_type) {
        CPPMARKUP_ELEMENT_A(single_elem, nullptr,
                            CPPMARKUP_ATTRIBUTE(creation, kangsw::refl::timestamp_t::clock::now());
                            CPPMARKUP_ATTRIBUTE(ref_path, "/doc/args"));
    };

    CPPMARKUP_ELEMENT(list_author, std::vector({"abc", "Def"}));
    CPPMARKUP_ELEMENT_A(some_obj_arr, std::vector({internal_object_type::get_default()}),
                        CPPMARKUP_ATTRIBUTE(encrypt, kangsw::refl::binary_chunk::from("hello, world!", 32, 42));
                        CPPMARKUP_ATTRIBUTE(stamp, kangsw::refl::timestamp_t::clock::now()));
    CPPMARKUP_ELEMENT_A(some_obj_map, CPPMARKUP_MAP(u8"entity", internal_object_type::get_default()));
    CPPMARKUP_ELEMENT(rev_major, 0);
    CPPMARKUP_ELEMENT(rev_minor, 31);
    CPPMARKUP_ELEMENT(rev_minor2, 315);
    CPPMARKUP_ELEMENT(some_dbl, 315.411);
    CPPMARKUP_ELEMENT(version_vector, std::vector({1, 0, 141, 5}));
};

...
        namespace refl    = kangsw::refl;
        namespace marshal = refl::marshal;

        my_markup_type mk;
        mk.reset();

        refl::u8str s;
        marshal::string_output os{s, 4};
        marshal::json_dump{}(mk, os);
...
```

This outputs below JSON string

```json
{
    "is_valid": false,
    "list_author": [
        "abc", 
        "Def"
    ],
    "some_obj_arr~@@ATTR@@": {
        "encrypt": "aGVsbG8sIHdvcmxkIQAgAAAAKgAAAA==",
        "stamp": "2021-05-12T12:49:25.471Z"
    },
    "some_obj_arr": [
        {
            "single_elem~@@ATTR@@": {
                "creation": "2021-05-12T12:49:25.471Z",
                "ref_path": "/doc/args"
            },
            "single_elem": null
        }
    ],
    "some_obj_map": {
        "entity": {
            "single_elem~@@ATTR@@": {
                "creation": "2021-05-12T12:49:25.471Z",
                "ref_path": "/doc/args"
            },
            "single_elem": null
        }
    ],
    "rev_major": 0,
    "rev_minor": 31,
    "rev_minor2": 315,
    "some_dbl": 315.411,
    "version_vector": [
        1, 
        0, 
        141, 
        5
    ]
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
2. [`zserge/jsmn`](https://github.com/zserge/jsmn)
