// SPDX-FileCopyrightText: 2026 Magomed Gadzhiumarov
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Terminal.h"
#include <iostream>

int main(int argc, char *argv[]) {
    Logger logger(true);
    Terminal terminal(argc, argv, logger);

    terminal.enableRawMode();
    while (true) {
        char key;
        std::cin >> key;

        if (key == 'q' || key == 'Q') break;

        terminal.updateTerminalColor(key);
    }
    
    return 0;
}
