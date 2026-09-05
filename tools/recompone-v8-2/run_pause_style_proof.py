"""Single native pause capture; hidden, silent, process-local input only."""
import sys
import run_map_fidelity_gate as gate

original_fixture = gate.fixture_text


def pause_fixture(*args, **kwargs):
    text = original_fixture(*args, **kwargs)
    # Capture poll 601 falls between opening and closing this native menu.
    return text + "\n[after:gameplay@1]\n120+3=START\n660+3=CROSS\n"


if __name__ == "__main__":
    gate.fixture_text = pause_fixture
    sys.exit(gate.main())
