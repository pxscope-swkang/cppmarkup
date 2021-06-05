#pragma once
#include "strutils.hxx"
#include "generics.hxx"
#include "kangsw/markup/reflection/property_proxy.hxx"
#include "kangsw/markup/marshal/details/jsmn.h"

namespace kangsw::refl::marshal {

/**
 * 
 */
class json_parse {
public:
    enum class result {
        ok,
        waiting,
        error_invalid_token = -1,
        error_invalid_type  = -2
    };

public:
    json_parse(size_t num_initial_tokens = 0) noexcept {
        _tokens.reserve(num_initial_tokens);
        jsmn::jsmn_init(&_parse);
    }

    result operator()(u8str_view str, object& out) {
        auto num_tokens = jsmn::jsmn_parse(&_parse, str.data(), str.size(), nullptr, 0);
        if (num_tokens == jsmn::JSMN_ERROR_INVAL) {
            return result::error_invalid_token;
        } else if (num_tokens == jsmn::JSMN_ERROR_PART) {
            // if only part of json string was delievered, buffer it then use later.
            _buffered.append(str.begin(), str.end());
            return result::waiting;
        }

        // if given string was under buffering state previously, then push rest of the string into
        //buffer and process it instead.
        if (!_buffered.empty()) {
            _buffered.append(str.begin(), str.end());
        }

        auto ptr  = _buffered.empty() ? str.data() : _buffered.data();
        auto size = _buffered.empty() ? str.size() : _buffered.size();

        // perform actual parsing
        _tokens.resize(num_tokens);
        jsmn::jsmn_init(&_parse);
        jsmn::jsmn_parse(&_parse, ptr, size, _tokens.data(), (unsigned)_tokens.size());

        // marshal tokenized result into object.
        if (_tokens.size() > 1) { // given result can be empty object; e.g. "{}"
            int idx = 0;          // index 0 is root object
            _str    = str;
            if (!_marshal(out, idx)) {
                return result::error_invalid_type;
            }
        }

        // on finished ...
        _buffered.clear();
        jsmn::jsmn_init(&_parse);
        _tokens.clear();
    }

private:
    struct _visitor {
        template <typename Ty_>
        bool operator()(property_proxy<Ty_, false> dest) {
            return false;
        }
    };

    bool _marshal(object& out, int& token_idx, int parent_idx = -1) const {
        auto baseaddr        = out.base();
        auto& traits         = out.traits();
        int self_idx         = token_idx;
        property const* prop = nullptr;

        while (++token_idx < _tokens.size() &&
               _tokens[token_idx].parent > parent_idx // while it's our children...
        ) {
            auto& token            = _tokens[token_idx];
            u8str_view token_value = _str.substr(token.start, token.size);
            switch (token.type) {
                case jsmn::JSMN_STRING:

                    if (
                      auto const is_tag = _tokens[token.parent].type != jsmn::JSMN_STRING;
                      is_tag) {
                        prop = traits.find_property(token_value);
                        if (prop == nullptr) {
                            // if given tag does not exist, ignore all children tokens.
                            // if any token's parent index is LE with current, it is sibling or
                            //unrelated node of this.
                            auto super = token.parent;
                            while (++token_idx < _tokens.size() &&
                                   _tokens[token_idx].parent > super) {}

                            continue;
                        }
                    } else if (prop->type() == etype::string) {
                        assert(prop);
                        auto proxy = make_proxy<u8str>(baseaddr, *prop);
                        proxy->assign(token_value);
                        prop = nullptr;
                    } else {
                        return false;
                    }
                    break;
                case jsmn::JSMN_OBJECT:
                    assert(prop);
                    if (prop->type() != etype::object) {
                        return false;
                    } else {
                        auto proxy = make_proxy<object>(baseaddr, *prop);
                        if (!_marshal(*proxy, token_idx, self_idx)) { return false; }
                    }
                    break;
                case jsmn::JSMN_ARRAY: break;
                case jsmn::JSMN_PRIMITIVE:
                    assert(prop);
                    if (prop->type().is_container() ||
                        prop->type().is_one_of(
                          etype::boolean, etype::null, etype::integer, etype::floating_point) ||
                        !visit_property(baseaddr, *prop, _visitor{})) //
                    {
                        return false;
                    }
                    break;
                default: throw std::runtime_error{"Json parsing gone wrong somehow."};
            }

            ++token_idx;
        }
        return true;
    }

    void _marshal_attributes() const {}

private:
    jsmn::jsmn_parser _parse = {};
    std::vector<jsmn::jsmntok_t> _tokens;
    u8str _buffered;
    u8str_view _str;
    object* _pout;
};

} // namespace kangsw::refl::marshal