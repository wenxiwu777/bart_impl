#include <iostream>
#include <string.h>

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    unsigned char *p = new unsigned char[102400000];
    if (!p) {
        std::cout << "memory allocation failed" << std::endl;
    }
    memset(p, 52, 102399998);
    p[102399999] = '\0';
    size_t len = strlen((const char *)p);
    
    delete [] p;
    
    return 0;
}
