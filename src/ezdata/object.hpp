#pragma once
#include <functional>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json_fwd.hpp>
#include <pugixml.hpp>

/**
 * string -> raw_memory(offset, size)
 * raw_memory -> string
 *
 * ������Ʈ�� ���ο� ���Ե� �ν��Ͻ��� �Ľ� �� ����ȭ�� �����ϱ� ���� ���� Ŭ�����Դϴ�.
 * ������Ʈ ��ü�� �����͸� ���� ������, �Ľ� �� ����ȭ ������ ��� CRTP �ν��Ͻ��� static inline
 *vector�� ����˴ϴ�.
 * ������ ������ Ÿ���� �Ľ��ϱ� ���� ������Ʈ�� ���̴� ��� ������ ������ bool(std::span<std::byte>,
 *std::string_view)�� �����ؾ� �մϴ�. ����ȭ�� ���ؼ� void(std::string, std::span<const std::byte>)
 *�Լ��� �����Ǿ�� �մϴ�.
 *
 * �⺻������, ezdata::marshal ���ӽ����̽��� parse �� serialize �Լ��� ã���ϴ�.
 *
 * �Ľ̿� ������ ��� ���ܸ� �����ų�, �ܼ��� ���� ����ü�� ��ȯ�� �� �ֽ��ϴ�. (optional)
 *
 * Json �� XML ������Ʈ Ʈ�� ��ü�� �Ľ� �� ����ȭ�ϴ� �������̽��� �����մϴ�. 
 */
namespace ezdata
{

}