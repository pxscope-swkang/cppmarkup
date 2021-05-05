#define CPPMARKUP_BUILD_WITH_DESCRIPTION
#include "catch.hpp"
#include "cppmarkup/impl_macros.hpp"
#include "cppmarkup/marshal_json.hpp"
#include "cppmarkup/object.hpp"

#include <cppmarkup.hpp>
#include <iostream>
#include <list>

namespace test::kangsw::markup {

constexpr auto default_value     = 3.141;
constexpr auto tag_name_str      = "tagname";
constexpr auto attribute_name    = "attrname";
constexpr static auto desciption = "desc";

// CPPMARKUP_OBJECT_TEMPLATE(tname)
// struct tname : ::kangsw::markup::impl::object_base<tname>
// ~CPPMARKUP_OBJECT_TEMPLATE(tname)
//{
//    // CPPMARKUP_OBJECT_TEMPLATE_BODY(tname)
//private:
//    static inline struct INTERNAL_BODY_tname {
//        INTERNAL_BODY_tname()
//        {
//            tname autogen = {};
//        }
//    } INTERNAL_BODY_INIT_tname;
//    // ~CPPMARKUP_OBJECT_TEMPLATE_BODY(tname)
//
//    // CPPMARKUP_DESCRIPTION_BELOW(description)
//private:
//    static inline struct INTERNAL_description_assignment_type___LINE__ {
//        INTERNAL_description_assignment_type___LINE__() { INTERNAL_next_description = desciption; }
//    } INTERNAL_description_assignment;
//
//public:
//    // ~CPPMARKUP_DESCRIPTION_BELOW
//
//    // CPPMARKUP_ADD(varname, tag_name_str, default_value, ...);
//    // INTERNAL_CPPMARKUP_INSTANCE_FORMER(varname, tag_name_str, ...ATTRIBs)
//    class INTERNAL_TYPE_varname : ::kangsw::markup::impl::element_template_base<INTERNAL_TYPE_varname> {
//    private:
//        static constexpr auto _tagstr = tag_name_str;
//
//        static inline struct _internal_description_assignment {
//            _internal_description_assignment()
//            {
//                _description              = INTERNAL_next_description;
//                INTERNAL_next_description = {};
//            }
//        } _description_assignment;
//
//    private:
//        static inline std::vector<::kangsw::markup::property::attribute_representation> _attribs;
//
//    public:
//        // ATTRIBUTES <
//        // __VA_ARGS__ ...
//        // CPPMARKUP_ATTRIBUTE(attr_varname, attr_name, default_value)
//        struct INTERNAL_ATTR_attr_varname : ::kangsw::markup::impl::attribute_base {
//            using attr_value_type = std::remove_const_t<decltype(default_value)>;
//
//            INTERNAL_ATTR_attr_varname(INTERNAL_TYPE_varname* base)
//            {
//                // 각 1회 ... 생성자에서 _attribs에
//                if (!INTERNAL_is_first_entry) { return; }
//
//                INTERNAL_attrbase_init(
//                    base, attribute_name, _attribs,
//                    sizeof *this,
//                    [](void* v) { *(attr_value_type*)v = default_value; });
//            }
//
//        private:
//            attr_value_type _value;
//
//        public:
//            auto& operator()() { return _value; }
//            auto& operator()() const { return _value; }
//        } attr_varname{this /* 어트리뷰트 오프셋 / 사이즈 계산용, 최초 1회 */};
//        // ~CPPMARKUP_ATTRIBUTE()
//        // ATTRIBUTES >
//
//    private:
//        // ~INTERNAL_CPPMARKUP_INSTANCE_FORMER
//        // << 이 위로 UPPER 매크로
//        // >>>> IF TARGET IS NESTED OBJECT
//        // >>>> ELSE (TARGET IS SIMPLE TYPE)
//        // nested object이면 value_type = INTERNAL_nested_object_instance
//        using value_type = std::remove_const_t<decltype(default_value)>;
//        value_type _value;
//        // >>>> ENDIF
//
//        // >> 이 아래로 LOWER 매크로
//        // INTERNAL_CPPMARKUP_INSTANCE_LATER(varname)
//
//    public:
//        INTERNAL_TYPE_varname(::kangsw::markup::object* base)
//        {
//            if (!INTERNAL_is_first_entry) { return; } // 최초 1회만 진입 .. 어차피 정적 생성이므로 1스레드 1회 호출 보장
//
//            printf("FooFOo\n");
//
//            INTERNAL_elembase_init(
//                ::kangsw::markup::get_element_type<value_type>(),
//                base, _tagstr, _description,
//                offsetof(INTERNAL_TYPE_varname, _value),
//                sizeof _value, sizeof *this,
//                [](void* v) { *(value_type*)v = default_value; },
//                _attribs);
//
//        }
//
//        auto& operator()() { return _value; }
//        auto& operator()() const { return _value; }
//    } varname{this};
//    // ~INTERNAL_CPPMARKUP_INSTANCE_LATER
//};

CPPMARKUP_OBJECT_TEMPLATE(obj)
{
    CPPMARKUP_DESCRIPTION_BELOW("설명 설명 설명");
    CPPMARKUP_ELEMENT(ShouldRefreshEveryReceive, false,
                      CPPMARKUP_ATTRIBUTE(IntervalMs, 15);
                      CPPMARKUP_ATTRIBUTE(IntervalMsB, 153);
                      CPPMARKUP_ATTRIBUTE(IntervalMsC, "Alah hu akbarr");
                      CPPMARKUP_ATTRIBUTE(IntervalMsCGF, false););

    CPPMARKUP_ELEMENT(TestArray, CPPMARKUP_ARRAY(1, 2, 45));
    CPPMARKUP_ELEMENT(TestBoolean, CPPMARKUP_ARRAY(false, true));
    CPPMARKUP_ELEMENT(TestStrArray, CPPMARKUP_ARRAY("a", "b", "c", "d..."));

    CPPMARKUP_ELEMENT(ShouldRefreshEveryReceiveA, 0.432,
                      CPPMARKUP_ATTRIBUTE(IntervalMs, 15));
    CPPMARKUP_ELEMENT(ShouldRefreshEveryReceiveB, 0.432,
                      CPPMARKUP_ATTRIBUTE(IntervalMs, 15));
    CPPMARKUP_ELEMENT(ShouldRefreshEveryReceiveE, 0.432);
    CPPMARKUP_ELEMENT(ShouldRefreshEveryReceiveEF, 0.432);
    CPPMARKUP_ELEMENT(ShouldRefreshEveryReceiveEG, 0.432);
    CPPMARKUP_ELEMENT(ShouldRefreshEveryReceiveEH, 0.432);

    CPPMARKUP_ELEMENT(SomeChnk, ::kangsw::markup::binary_chunk{});
};

CPPMARKUP_WRAPPED_OBJECT_TEMPLATE(superobj, obj, Body);

CPPMARKUP_OBJECT_TEMPLATE(objarr)
{
    CPPMARKUP_ELEMENT(ObjArrA, CPPMARKUP_ARRAY(obj{}, obj{}, obj{}));
    CPPMARKUP_ELEMENT(ObjB, obj{});
    CPPMARKUP_ELEMENT(ObjC, obj{});
    CPPMARKUP_ELEMENT(ObjD, obj{});
    CPPMARKUP_ELEMENT(ObjE, obj{});
};

CPPMARKUP_OBJECT_TEMPLATE(elser)
{
    CPPMARKUP_EMBED_OBJECT_begin(hell)
    {
        CPPMARKUP_ELEMENT(TestObjMap, CPPMARKUP_MAP("Hell", obj{}, "Abc", obj{}));
        CPPMARKUP_ELEMENT(TestObjArray, CPPMARKUP_ARRAY(obj::get_default(), obj::get_default()));
        CPPMARKUP_ELEMENT(ShouldRefreshEveryReceiveA, 0.432,
                          CPPMARKUP_ATTRIBUTE(IntervalMs, 15);
                          CPPMARKUP_ATTRIBUTE(IntervalMsBc, 1541););
        CPPMARKUP_ELEMENT(ShouldRefreshEveryReceiveB, 0.432,
                          CPPMARKUP_ATTRIBUTE(IntervalMs, 15));
        CPPMARKUP_ELEMENT(ShouldRefreshEveryReceiveC, 0.432,
                          CPPMARKUP_ATTRIBUTE(IntervalMs, 15));
        CPPMARKUP_ELEMENT(ShouldRefreshEveryReceiveD, 0.432,
                          CPPMARKUP_ATTRIBUTE(IntervalMs, 15));
        CPPMARKUP_ELEMENT(ShouldRefreshEveryReceiveE, 0.432,
                          CPPMARKUP_ATTRIBUTE(IntervalMs, 15));
        CPPMARKUP_ELEMENT(ShouldRefreshEveryReceiveF, 0.432,
                          CPPMARKUP_ATTRIBUTE(IntervalMs, 15));
        CPPMARKUP_ELEMENT(ShouldRefreshEveryReceiveFc, 0.432,
                          CPPMARKUP_ATTRIBUTE(IntervalMs, 15));
        CPPMARKUP_ELEMENT(ShouldRefreshEveryReceiveFg, 0.432,
                          CPPMARKUP_ATTRIBUTE(IntervalMs, 15));
        CPPMARKUP_ELEMENT(ShouldRefreshEveryReceiveFrr, 0.432,
                          CPPMARKUP_ATTRIBUTE(IntervalMs, 15));
        CPPMARKUP_ELEMENT(ShouldRefreshEveryReceiveFqe, 0.432,
                          CPPMARKUP_ATTRIBUTE(IntervalMs, 15));
    }
    CPPMARKUP_EMBED_OBJECT_end(hell);
};

template <typename Ty_>
decltype(auto) ddf(std::map<std::string, Ty_>&& args)
{
    return std::move(args);
}

TEST_CASE("CppMarkup", "Object body Template")
{
    std::map<std::string, bool> ar{{"abs", true}};

    obj r;
    r.ShouldRefreshEveryReceive            = true;
    r.ShouldRefreshEveryReceive.IntervalMs = 7;
    auto g                                 = r.ShouldRefreshEveryReceive.value();

    superobj o;
    obj dddf;
    r = dddf;

    static_assert(std::is_nothrow_move_constructible_v<superobj>);
    static_assert(std::is_nothrow_copy_constructible_v<superobj>);
    static_assert(std::is_nothrow_constructible_v<superobj>);
    static_assert(std::is_nothrow_copy_assignable_v<superobj>);
    static_assert(std::is_nothrow_move_assignable_v<superobj>);

    elser cc;
    superobj cd;

    cd.reset();
    ::kangsw::markup::u8string str;
    ::kangsw::markup::json_dump dmp{str, 4, 0};
    ::kangsw::markup::dump(dmp, cd);

    std::cout << dmp.buff;

    auto& vv = r.TestStrArray.value();

    auto p = r.props();

    elser car;

    auto ras = car.hell->TestObjMap["faer"];

    REQUIRE(r.ShouldRefreshEveryReceive);
}

} // namespace test::kangsw::markup