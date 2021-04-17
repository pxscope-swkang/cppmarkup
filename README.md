# EZDATA

c++ <-> json, xml 간 데이터 바인딩 라이브러리

# 사용 예제

```XML
<Foo Date="2021-04-23"> <!-- 오브젝트 타입 with ATTR -->
    <Vlah>33</Vlah> <!-- 값 타입 -->
    <Trs D="as">[true, false, true, true, false]</Trs> <!-- 값 타입 with ATTR -->
    <FooFoo Value="dae"/> <!-- ATTR 타입 -->
    <Str>"hell, world!"</Str> <!-- 값 타입 -->
    <Algh.Frl> <!-- 오브젝트 배열 타입 with or without ATTR -->
        <_ELEM R="32344"> <!-- 배열 엔트리 -->
            <A>3142</A>
            <B>3222</B>
        </_ELEM>
        <_ELEM R="65433">
            <A>3411</A>
            <B>3324</B>
        </_ELEM>
    </Algh.Frl>
    <Ddd> <!-- 인라인 오브젝트 타입 (템플릿 없음) -->
        <Peua R="ds"> <!-- 위의 배열 엔트리 템플릿 재활용 -->
            <A>0</A>
            <B>0</B>
        <Peua>
    </Ddd>
</Foo>
```

```json
{ // note: bson으로도 가능
    "Foo":{
        "~@@ATTRS@@":{ // 어트리뷰트 없으면 이 엘리먼트만 단순 사라짐
            "Date":"2021-04-23" // Attribute is always string
        },
        // 단순 값 타입
        "Vlah": 33,
        // 값 타입이 attribute를 가진 경우
        "Trs": [true, false, true, true, false],
        "Trs~@@ATTRS@@": { "D": "as" },
        // Value 없이 Attr만 있는 경우 ...
        "FooFoo~@@ATTRS@@": { "Value": "dae" }, 
        // 오브젝트 배열 타입.
        // 템플릿을 열거
        "Algh.Frl": [
            { "~@@ATTRS@@":{ "R": 32344 }, "A": 3142, "B": 3222 },
            { "~@@ATTRS@@":{ "R": 32344 }, "A": 3142, "B": 3222 },
        ],
        "Ddd":{
            "Peua":{
                "~@@ATTRS@@":{ "R": "ds"},
                "A":0,
                "B":0
            }
        }
    } 
}
```

```c++
// 출력 XML에 dscription 문자열 주석으로 포함 여부. Json 해당 없음.
#define EZDATA_INCLUDE_DESCRIPTION_ON_OUTPUT_XML true

EZDATA_OBJECT_TEMPLATE( // nested object type. generate agrregate constructor
    some_vector,
    "SomeVector", 
    {{"R", ""}});
{
    EZDATA_VALUE(a, "A", 0);
    EZDATA_VALUE(b, "B", 0);
}
// Generates aggregate initializer ... 
// olla(map<u8string_view, u8string> attr, int _a, int _b) 

EZDATA_OBJECT_TYPE(  // 가장 바깥쪽에 있는 게 바람직. 안쪽에 들어가도 상관은 없음
    /*typename*/foo, // foo
    /*default display name of most outer element*/"Foo", 
    /*attributes:map<u8string_view, u8string>*/{{"Date", "2021-04-23"}},
    /*description*/"@brief Object description written in Doxygen syntax")
{
    EZDATA_VALUE(
        /*cpp member name*/vlah, // ezdata_value<int> ... sizeof vlah == sizeof(int)
        /*display name*/"Vlah", 
        /*default value, type deduction에도 사용*/33, 
        /*description. attribute 없는 버전 overloaded*/
        u8"@brief non-ascii 문자열은 u8 접두 필수");
    
    EZDATA_VALUE(
        trs, // ezdata_value<std::vector<bool>>
        "Trs", 
        {true, false, true, true, false} // initializer_list 형태로 어레이 인식
    ); // desciption까지 생략 가능 ... overloaded!
    
    EZDATA_ATTR_OBJECT(
        foofoo, // foo::ezdata_object<foofoo_type>
        "FooFoo", 
        {{"Value", "dae"}}, 
        "@brief ...");
    
    EZDATA_OBJECT_ARRAY(
        algh_frl, 
        "Algh.Frl", // 배열 이름
        some_vector, // 배열 오브젝트 템플릿
        {});
        
    EZDATA_INLINE_OBJECT_BEGIN(/* display name */"Ddd", /*[attr], [description]*/) 
    { // 익명의 인라인 오브젝트
        // 오브젝트 템플릿 인스턴스화
        EZDATA_OBJECT(
            /*template object type*/ some_vector,
            /*member variable name*/ peua,
            /*display name*/"Peua",
            /*aggregate initializer*/ {},
            /*optional: attributes, description*/);
    } EZDATA_INLINE_OBJECT_END(ddd); // 인스턴스 네임 
    
};
```

# References

1. [`nlohmann/json`](https://github.com/nlohmann/json)
2. [`zeux/pugixml`](https://github.com/zeux/pugixml)
3. [`catchorg/Catch2`](https://github.com/catchorg/Catch2)
