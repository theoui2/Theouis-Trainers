# Shortcut host checks

Run from the launcher repository with a host C++20 compiler:

```sh
g++ -std=c++20 -Wall -Wextra -Werror -fsanitize=address,undefined -g tests/test_shortcuts.cpp -o /tmp/theoui-shortcuts
ASAN_OPTIONS=detect_leaks=0 /tmp/theoui-shortcuts
g++ -std=c++20 -Wall -Wextra -Werror -fsanitize=address,undefined -g tests/test_shortcut_runtime.cpp -o /tmp/theoui-runtime
ASAN_OPTIONS=detect_leaks=0 /tmp/theoui-runtime
g++ -std=c++20 -Wall -Wextra -Werror -fsanitize=address,undefined -g tests/test_catalog.cpp -o /tmp/theoui-catalog
ASAN_OPTIONS=detect_leaks=0 /tmp/theoui-catalog /path/to/SimpleTrainerSwitch.ovl ./Trainers.ovl
g++ -std=c++20 -Wall -Wextra -Werror -fsanitize=address,undefined -g tests/test_menu_names.cpp -o /tmp/theoui-menu-names
ASAN_OPTIONS=detect_leaks=0 /tmp/theoui-menu-names
```

Leak detection is disabled because this environment does not support LSan. The tests check production portable code and the runtime wrapper with fake IPC. They do not emulate the complete Switch input/rendering services or establish successful hardware operation.
