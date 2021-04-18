#include "catch.hpp"
#include "cppmarkup/cppmarkup.hpp"

namespace test::cppmarkup::objects {
CPPMARKUP_OBJECT_TEMPLATE(some_type)
{
    CPPMARKUP_DESCRIPTION_BELOW(u8"");
    CPPMARKUP_ADD(fooa, u8"Fooa", 3.141, CPPMARKUP_ATTR2(Attr1, u8"Hell, world!"));
    CPPMARKUP_ADD(grr, u8"EOFO", 2);

    CPPMARKUP_ADD2(AdminPw, 0x864321, CPPMARKUP_ATTR3(Attr1); CPPMARKUP_ATTR3(Attr2));

    CPPMARKUP_NESTED_OBJECT(
        lod, u8"Lod",
        CPPMARKUP_ADD(laa, u8"Laa", 34);
        CPPMARKUP_ADD2(lapa, 34);
        CPPMARKUP_ADD2(lapua, 34);
        CPPMARKUP_ADD2(lapuwa, 34);
        CPPMARKUP_ADD2(lapuwtra, 34);
        CPPMARKUP_ADD2(lapuwtrtta, 34);
        CPPMARKUP_ADD2(lapuwtrttwea, 34);
        CPPMARKUP_ADD2(lapuw2trttwea, 34);
        CPPMARKUP_NESTED_OBJECT(
            vlad, u8"Vlad",
            CPPMARKUP_ADD2(abar, 34ll);
            CPPMARKUP_ADD2(cpa, 34);
            CPPMARKUP_ADD2(rooe, 34);
            CPPMARKUP_ADD2(qut, 34);
            CPPMARKUP_ADD2(quti32o, 34);
            CPPMARKUP_ADD2(quti321o, 34);
            CPPMARKUP_ADD2(quti32115o, 34);
            CPPMARKUP_ADD2(qutio, 34);
            CPPMARKUP_ADD2(iottt, 34);
            CPPMARKUP_ADD2(iotttp, 34);));

    struct INTERNAL_EZ_varname_INSTANCE {
    public:
        struct _internal_type
        /*MACRO_BEGIN*/
        {
        };
        /*MACRO_END*/
    private:
        _internal_type _value;

    public:
        _internal_type& operator()() { return _value; }
        _internal_type const& operator()() const { return _value; }
    };
};

TEST_CASE("instanciation")
{
    some_type r;
    std::atomic<void*> k;
    r.AdminPw.Attr2 = u8"df";
    r.AdminPw()     = 3;
}

TEST_CASE("type")
{
    static_assert(::cppmarkup::get_node_type<int>() == ::cppmarkup::node_type::integral_number);
    static_assert(::cppmarkup::get_node_type<float>() == ::cppmarkup::node_type::real_number);
    static_assert(::cppmarkup::get_node_type<some_type>() == ::cppmarkup::node_type::object);
    static_assert(::cppmarkup::get_node_type<std::vector<int>>() == ::cppmarkup::node_type::array);
    static_assert(::cppmarkup::get_node_type<std::vector<float>>() == ::cppmarkup::node_type::array);
    static_assert(::cppmarkup::get_node_type<std::vector<double>>() == ::cppmarkup::node_type::array);
    static_assert(::cppmarkup::get_node_type<std::vector<some_type>>() == ::cppmarkup::node_type::array);
    static_assert(::cppmarkup::get_node_type<nullptr_t>() == ::cppmarkup::node_type::null);
    static_assert(::cppmarkup::get_node_type<std::u8string>() == ::cppmarkup::node_type::string);
    static_assert(::cppmarkup::get_node_type<std::wstring>() == ::cppmarkup::node_type::string);
    static_assert(::cppmarkup::get_node_type<std::string>() == ::cppmarkup::node_type::string);
}
} // namespace test::cppmarkup::objects