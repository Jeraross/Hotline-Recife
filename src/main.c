#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "screen.h"
#include "keyboard.h"
#include "timer.h"

int main() {
    keyboardDestroy();
    screenDestroy();
    timerDestroy();

    return 0;
}
