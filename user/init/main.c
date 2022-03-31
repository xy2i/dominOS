int main(void) {
        int x = 2;
        int y = 3;
        int z __attribute__((unused)) = x + y;
        while(1);
        return 0;
}