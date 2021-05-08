#pragma once
#include "cppmarkup.hpp"

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