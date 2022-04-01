int main(void) {
        int x = 2;
        int y = 3;
        int z __attribute__((unused)) = x + y;
        unsigned int * kill_me = (unsigned int *)0xB16B00B5;
        *kill_me = 0xcafebabe;
        return 0;
}