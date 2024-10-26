#include "json.h"
#include <assert.h>

int main() {
    printf("Running json test...\n");
    assert(json_validate("{\"tests\":true,\"aaa\":false}"));
    assert(json_validate("{\"tests\":12.23}"));
    assert(!json_validate("{\"tests\":12.23\"}"));
    assert(!json_validate("{test:12.23}"));
    assert(json_validate("{\"test\":\"12.23\",\"test\":\"12.23\"}"));
    assert(json_validate("{\"test\":\"12.23\",\"test\":13.23}"));
    assert(json_validate("{   \" test \":    \"12   .   23\"   ,    \"test\"   "
                         " :   13.23    }"));
    assert(json_validate(
        "{   \" test \":    \"12   .   23\"   ,    \"test\"    :   1    }"));
    exit(0);
    assert(!json_validate("[1, 2, 3]"));
    assert(!json_validate("{1, 2, 3}]"));
    assert(!json_validate("{1, 2, 3]]"));
    assert(!json_validate("{1, 2, 3\"}"));
    assert(json_validate("{1, 2, 3\\\"}"));

    return 0;
}