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
    private:
        static auto constexpr INTERNAL_EZ_tagstr = u8"tagstr";

    public:
        struct _attr_type {
            /*__VA_ARGS__*/
            CPPMARKUP_ATTR3(Allbi);
        };

    public:
        CPPMARKUP_OBJECT_TEMPLATE(_internal_type)
        /*MACRO_BEGIN*/
        {};
        /*MACRO_END*/
    private:
        static inline std::atomic_size_t _node_ref = -1;
        static inline ::cppmarkup::marshaller<_internal_type> _marshal;
        static inline ::cppmarkup::impl::object_inst_init<
            INTERNAL_EZ_varname_INSTANCE, _internal_type, CPPMARKUP_ALIGNMENT>
            _init{
                _node_ref,
                INTERNAL_EZ_node_list,
                INTERNAL_EZ_tagstr,
                sizeof(_internal_type),
                &INTERNAL_EZ_description_str,
                &_marshal,
                ::cppmarkup::get_node_type<_internal_type>(),
                [](void* v, size_t s) { *(_internal_type*)v = {}; }};

    public:
        INTERNAL_EZ_varname_INSTANCE(void* owner_base)
        {
            if (auto idx = _node_ref.exchange(-1); idx != -1)
            {
                auto ptr         = INTERNAL_EZ_node_list.data() + idx;
                ptr->offset      = (intptr_t)this - (intptr_t)owner_base;
                ptr->next_offset = ptr->offset + sizeof *this;

                assert(ptr->total_size == sizeof *this);

                for (int i = 0; i < INTERNAL_EZ_depth; ++i) { printf("    "); }
                printf("[%s] ptr: %p ", (char*)ptr->tag.data(), ptr);
                printf("offset: %llu ~ size: %llu\n", ptr->offset, ptr->total_size);
            }
        }

    private:
        alignas(CPPMARKUP_ALIGNMENT) _internal_type _value;

    public:
        alignas(CPPMARKUP_ALIGNMENT) _attr_type attribute;

    public:
        _internal_type& operator()() { return _value; }
        _internal_type const& operator()() const { return _value; }
    } varname{this};
};

TEST_CASE("instanciation")
{
    some_type r;
    std::atomic<void*> k;
    r.AdminPw.Attr2 = u8"df";
    r.AdminPw()     = 3;

    r.varname.attribute.Allbi = u8"ere";
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