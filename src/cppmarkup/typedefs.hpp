#pragma once
#include <string>
#include <string_view>
#include <vector>


#if __cplusplus > 202000
#include <span>
#else
#endif

/** 포워딩 */
namespace kangsw::markup {
namespace impl {
    class basic_marshaller;
    template <typename> class element_template_base;
    class element_base;
} // namespace impl

class object;
} // namespace kangsw::markup

namespace kangsw::markup
{
#if __cplusplus > 202000
using u8string      = std::u8string;
using u8string_view = std::u8string_view;
#else
using u8string      = std::string;
using u8string_view = std::string_view;
#endif

/** 데이터 고속 송수신을 위한 바이너리 표현입니다. */
enum class compact_byte : uint8_t;
using compact_binary = std::vector<compact_byte>;

#if __cplusplus > 202000
using compact_binary_view = std::span<compact_byte>;
#else
using compact_binary_view = std::vector<compact_byte> const&;
#endif


}