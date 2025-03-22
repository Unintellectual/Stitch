#define STITCH_IMPLEMENTATION
#include "stitch.h"

void assert_true(const char *test_case, bool result)
{
    if (result) {

        stitch_log(STITCH_INFO, "[SUCCESS] %s", test_case);
    } else {
        stitch_log(STITCH_ERROR, "[FAIL] %s", test_case);
    }
}

void assert_false(const char *test_case, bool result)
{
    if (!result) {
        stitch_log(STITCH_INFO, "[SUCCESS] %s", test_case);
    } else {
        stitch_log(STITCH_ERROR, "[FAIL] %s", test_case);
    }
}

int main(void)
{
    Stich_String_View sv1 = stitch_sv_from_cstr("./example.exe");
    Stich_String_View sv2 = stitch_sv_from_cstr("");

    assert_true("stitch_sv_end_with(sv1, \"./example.exe\")", stitch_sv_end_with(sv1, "./example.exe"));
    assert_true("stitch_sv_end_with(sv1, \".exe\")", stitch_sv_end_with(sv1, ".exe"));
    assert_true("stitch_sv_end_with(sv1, \"e\")", stitch_sv_end_with(sv1, "e"));
    assert_true("stitch_sv_end_with(sv1, \"\")", stitch_sv_end_with(sv1, ""));
    assert_true("stitch_sv_end_with(sv2, \"\")", stitch_sv_end_with(sv2, ""));

    assert_false("stitch_sv_end_with(sv1, \".png\")", stitch_sv_end_with(sv1, ".png"));
    assert_false("stitch_sv_end_with(sv1, \"/path/to/example.exe\")", stitch_sv_end_with(sv1, "/path/to/example.exe"));
    assert_false("stitch_sv_end_with(sv2, \".obj\")", stitch_sv_end_with(sv2, ".obj"));

    return 0;
}
