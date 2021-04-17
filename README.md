# EZDATA

c++ <-> json, xml 간 데이터 바인딩 라이브러리

## Contraints

XML은 <OuterTag>PlainText<InnerTag/></OuterTag>등과 같이, plain text와 tag를 혼용할 수 없음. 순서에 둔감함 (JSON과 호환)

# 사용 예제

```c++
// 서술적 문법 ...
// 등장 순서대로 빌드하며, 문맥은 컴파일 단위로 저장(단순 static 변수)
CPPMARKUP_OBJECT_TEMPLATE(template_type_name) 
{
    // <VoidName Value="3"/>
    // 뒤쪽의 어트리뷰트는 attribute 구조체를 만들어 이를 상속하는 식으로 구현
    CPPMARKUP_ADD(
        value_id_void, "VoidName", void,
        CPPMARKUP_ATTR(AttrName1, "DefaultValue1"),
        CPPMARKUP_ATTR(AttrName2, "DefaultValue2"));
        
    // 바로 아래 엘리먼트의 설명 작성
    CPPMARKUP_DESCRIPTION(
        u8"lorem ipsum fa er .. qw.e.fdasdvccc ..."
        u8"나랏말싸미 듕귁에 달아 ....");
    
    // <IntName>3</IntName>
    CPPMARKUP_ADD(value_id_int, "IntName", 3);
    
    // 내부에 중첩도 가능.
    CPPMARKUP_OBJECT_TEMPLATE(nested0)
    {
        
    };
    CPPMARKUP_ADD(annonymous0, "Annonymous", nested0{});
    
    // 배열 표현, initializer_list로 표현할 수 있어야 함(타입 항상 동일)
    CPPMARKUP_ADD_ARRAY(array_id, "Array", {3, 4, 5, 6}, CPPMARKUP_ATTR(a, "C"));
    CPPMARKUP_ADD_ARRAY(array_id2, "Array2", {nested0{}, nested0{}});
};

// 오브젝트 중첩 ...
CPPMARKUP_OBJECT_TEMPLATE(nested_object_type_name)
{
    CPPMARKUP_ADD(
        nested_id,
        "NestedObj", 
        template_type_name{
            .value_id_void{.attributes{.AttrName1="Abc", .AttrName2="Def"}},
            .value_id_int{.value=3}},
        CPPMARKUP_ATTR(AttrName1, "DefaultValue1"));
};

```

# 구현 노트

오브젝트 템플릿은 단순히 새로운 구조체 타입을 정의 ... 파싱 및 덤프 될 수 있음

기본적으로 파싱/덤핑 템플릿 함수로 동작, 그러나 CPPMARKUP_base_object를 상속하는 함수에 대한 특수화로 재귀적 파싱 구현

`CPPMARKUP_OBJECT_TEMPLATE`는 파싱 규칙 벡터를 멤버로 갖는 클래스를 상속, 현재 스코프에서 정적 인라인 변수 `node_list` 이름을 자신의 것으로 설정. (즉, 중첩이 가능)

`CPPMARKUP_ADD` 매크로는 등장 순서대로 `node_list`에 태그, 자기 구조체의 오프셋을 넣음.

`CPPMARKUP_ATTR` 매크로는 `CPPMARKUP_ADD`가 푸시한 `node_list`의 가장 탑 엘리먼트의 `attrs` 배열에 자기 자신을 푸시하는 구조체를 생성. (매크로 각각 자신의 상대 오프셋을 집어넣음)

`CPPMARKUP_DESCRIPTION`은 다음 `node_list`에 대한 push에서 release되는 문자열을 설정. 다음 인스턴스 한 개에 설명을 덧붙임.

`CPPMARKUP_ADD_ARRAY`는 내부적으로 Tag 이름 밑에 중첩되는 다수의 `<elem>` 태그로 표현. 항상 `std::vector<std::decltype(std::initializer_list{ARR})::element_type>`

Array 파싱은 두 가지 전략이 있음.

하나는 모든 array parsing 시나리오에 대해 overload .. 다른 하나는 템플릿으로 공통 로직 뽑아내기.

# References

1. [`nlohmann/json`](https://github.com/nlohmann/json)
2. [`zeux/pugixml`](https://github.com/zeux/pugixml)