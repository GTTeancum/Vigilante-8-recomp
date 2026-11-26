"""Match teardown must survive the real pre-compile patchers running again."""
import unittest

import apply_vehicle_engine_patches as engine


class MatchTeardownTests(unittest.TestCase):
    def site(self, body):
        return "before\n" + engine.TEARDOWN_CALL + body + engine.TEARDOWN_END + "after\n"

    def test_both_patchers_compose(self):
        patch = engine.patch_match_teardown
        v, s = engine.VEHICLE_RELEASE, engine.SPLIT_RELEASE
        self.assertEqual(patch(self.site("")), self.site(v))
        self.assertEqual(patch(self.site(v)), self.site(v))
        # Also repair an otherwise valid split teardown missing vehicle cleanup.
        self.assertEqual(patch(self.site(s)), self.site(v + s))
        self.assertEqual(patch(self.site(v + s)), self.site(v + s))

    def test_invalid_never_validates_as_installed(self):
        v, s = engine.VEHICLE_RELEASE, engine.SPLIT_RELEASE
        for body in (s + v, v + v, v + s + s, "// unrecognized hook\n"):
            with self.subTest(body=body), self.assertRaises(RuntimeError):
                engine.patch_match_teardown(self.site(body))
        site = self.site(v + s)
        for bad in (site + site, site.replace(engine.TEARDOWN_CALL, ""),
                    site.replace(engine.TEARDOWN_END, ""), engine.TEARDOWN_END + engine.TEARDOWN_CALL):
            with self.subTest(site=bad), self.assertRaises(RuntimeError):
                engine.patch_match_teardown(bad)


if __name__ == "__main__":
    unittest.main()
