#include <stdio.h>
#include "eqparse.h"

int main(int argc, char ** argv){

    char * s = "x = 5+3e-2*9.4-4 456 sin(t-3);lka";
    eqparse_t * eq = eqparse(s, 0);

    eqparse_cleanup(eq);
    return 0;
}
