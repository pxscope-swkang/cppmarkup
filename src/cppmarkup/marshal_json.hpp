#pragma once
#include "typedefs.hpp"

/**
 * ��� �Ľ��� ���� �� json ���ڸ� ������ ��ȸ ...
 *
 * ������Ƽ ������ ���� ... Json Ʈ������ ���� �ڽ��� ��ġ�� ��Ÿ��(����)
 *
 * �±� �Ľ� ��, ���� ������Ƽ ������ ž ������Ƽ(������Ʈ �Ǵ� ��)�� ������Ƽ ����Ʈ���� �����Ǵ� �±׸� �˻�
 *
 * �±� �˻� �� ������Ƽ ���ÿ� �ش��ϴ� ������Ƽ Ǫ�� ��, value �Ľ� ...
 *
 * ������Ʈ�� �� �ܰ� ��(Ǫ�ôٿ� ���丶Ÿ), ���̸� �Ľ��ϰ� ��������
 *
 * �ش� �±��� ��� ������Ʈ �Ľ� ��ġ�� ���ÿ��� �� ... �θ� �������� �̵�
 *
 * ��Ʈ����Ʈ�� "������Ʈ �̸�~@@ATTR@@" ����, ������ ��
 */

namespace kangsw::markup {

struct json_parse {
    u8string_view source;
};

struct json_dump {
    u8string& buff;

    // �Ʒ��� ������ �ɼ� ...
    int indent         = -1; // 0 �̻��� �� ���� �� ���� ���� ���Ե�
    int initial_indent = 0;
};

marshalerr_t parse(object& to, json_parse from);
marshalerr_t dump(json_dump to, object const& from);

} // namespace kangsw::markup
